/*
 * Copyright (c) 2013-2014 Andreas Pohl <apohl79 at gmail.com>
 *
 * This file is part of libtasks.
 *
 * libtasks is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libtasks is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libtasks.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <tasks/dispatcher.h>
#include <tasks/worker.h>
#include <tasks/executor.h>
#include <tasks/task.h>
#include <tasks/logging.h>
#include <tasks/disposable.h>
#include <tasks/event_task.h>
#include <tasks/exec_task.h>
#include <tasks/cleanup_task.h>
#include <cassert>
#include <cstdarg>
#include <chrono>

namespace tasks {

std::shared_ptr<dispatcher> dispatcher::m_instance = nullptr;
dispatcher::mode dispatcher::m_run_mode = mode::SINGLE_LOOP;

dispatcher::dispatcher(uint8_t num_workers)
    : m_term(false), m_num_workers(num_workers), m_workers_busy(tools::bitset(m_num_workers)), m_rr_worker_id(0) {
    // Create workers
    tdbg("dispatcher: number of cpus is " << (int)m_num_workers << std::endl);
}

void dispatcher::run(int num, ...) {
    // Start the event loop
    start();

    // Start tasks if passed
    if (num > 0) {
        va_list tasks;
        va_start(tasks, num);
        for (int i = 0; i < num; i++) {
            tasks::task* t = va_arg(tasks, tasks::task*);
            add_task(t);
        }
        va_end(tasks);
    }

    // Now we park this thread until someone calls finish()
    join();
}

void dispatcher::run(std::vector<tasks::task*>& tasks) {
    // Start the event loop
    start();

    for (auto t : tasks) {
        add_task(t);
    }

    // Now we park this thread until someone calls finish()
    join();
}

void dispatcher::start() {
    // The first thread becomes the leader or each thread gets its own loop
    struct ev_loop* loop_raw = ev_default_loop(0);
    for (uint8_t i = 0; i < m_num_workers; i++) {
        std::unique_ptr<loop_t> loop = nullptr;
        if (mode::MULTI_LOOP == m_run_mode) {
            if (nullptr == loop_raw) {
                loop_raw = ev_loop_new(0);
            }
            assert(loop_raw != nullptr);
            loop.reset(new loop_t(loop_raw));
            // Force the next iteration to create a new loop struct.
            loop_raw = nullptr;
        }
        m_workers_busy.unset(i);
        auto w = std::make_shared<worker>(i, loop);
        assert(nullptr != w);
        m_workers.push_back(w);
    }
    // Wait for the workers to become available
    for (uint8_t i = 0; i < m_num_workers; i++) {
        while (!m_workers_busy.test(i)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    if (mode::SINGLE_LOOP == m_run_mode) {
        // Promote the first leader
        std::unique_ptr<loop_t> loop(new loop_t);
        loop->ptr = ev_default_loop(0);
        m_workers_busy.unset(0);
        m_workers[0]->set_event_loop(loop);
    }
    m_started = true;
}

void dispatcher::join() {
    std::unique_lock<std::mutex> lock(m_finish_mutex);
    while (!m_term) {
        if (m_finish_cond.wait_for(lock, std::chrono::milliseconds(1000)) != std::cv_status::timeout) {
            tdbg("dispatcher: m_finish_cond.wait_for did not timeout." << std::endl);
        }
    }
    tdbg("dispatcher: m_term is " << (m_term ? "true. " : "false. ") << "Terminating workers..." << std::endl);
    for (auto w : m_workers) {
        w->terminate();
    }
    tdbg("dispatcher: finished" << std::endl);
}

std::shared_ptr<worker> dispatcher::free_worker() {
    if (m_num_workers > 1) {
        tools::bitset::int_type id;
        if (m_workers_busy.next(id, m_last_worker_id)) {
            m_last_worker_id = id;
            tdbg("dispatcher: free_worker(" << id << ")" << std::endl);
            m_workers_busy.unset(id);
            return m_workers[id];
        }
    }
    return nullptr;
}

std::shared_ptr<executor> dispatcher::free_executor() {
    tdbg("searching executor" << std::endl);
    std::lock_guard<std::mutex> lock(m_executor_mutex);
    auto i = m_executors.begin();
    auto end = m_executors.end();
    while (i != end) {
        if ((*i)->terminated()) {
            tdbg("erasing terminated executor " << (*i).get() << std::endl);
            i = m_executors.erase(i);
        } else if (!(*i)->busy()) {
            tdbg("returning executor " << (*i).get() << std::endl);
            (*i)->set_busy();
            return *i;
        } else {
            i++;
        }
    }
    tdbg("creating executor" << std::endl);
    auto e = std::make_shared<executor>();
    m_executors.push_back(e);
    tdbg("returning executor " << e.get() << std::endl);
    return e;
}

void dispatcher::add_free_worker(uint8_t id) {
    tdbg("dispatcher: add_free_worker(" << (unsigned int)id << ")" << std::endl);
    m_workers_busy.set(id);
}

worker* dispatcher::get_worker_by_task(event_task* task) {
    worker* worker = task->assigned_worker();
    if (nullptr == worker) {
        switch (m_run_mode) {
            case mode::MULTI_LOOP:
                // In multi loop mode we pick a worker by round robin
                worker = m_workers[m_rr_worker_id++ % m_num_workers].get();
                break;
            default:
                // In single loop mode use the current executing worker
                worker = worker::get();
                // If we get called from some other thread than a worker we pick the last active worker
                if (nullptr == worker) {
                    worker = m_workers[m_last_worker_id].get();
                }
        }
    }
    return worker;
}

void dispatcher::print_worker_stats() const {
    for (auto& w : m_workers) {
        terr(w->get_string() << ": number of handled events is " << w->events_count() << std::endl);
    }
}

void dispatcher::add_task(task* task) {
    // Check if the system is running
    if (!m_started) {
        terr("dispatcher: You have to call dispatcher::start() before dispatcher::add_task()" << std::endl);
        assert(false);
    }
    // The pass object is no event_task. Now find a worker to add it.
    try {
        event_task* et = dynamic_cast<event_task*>(task);
        if (nullptr != et) {
            add_event_task(et);
            return;
        }
    } catch (std::exception&) {
    }

    // Check if the passed task object is an exec_task.
    try {
        exec_task* et = dynamic_cast<exec_task*>(task);
        if (nullptr != et) {
            add_exec_task(et);
            return;
        }
    } catch (std::exception&) {
    }

    terr("dispatcher: task " << task << " is not an event_task nor an exec_task!");
    assert(false);
}

void dispatcher::add_event_task(event_task* task) {
    worker* worker = get_worker_by_task(task);
    tdbg("add_event_task: adding event_task " << task << " using worker " << worker << std::endl);
    task->init_watcher();
    task->assign_worker(worker);
    task->start_watcher(worker);
}

void dispatcher::add_exec_task(exec_task* task) {
    std::shared_ptr<executor> executor = free_executor();
    tdbg("add_exec_task: adding exec_task " << task << " using executor " << executor << std::endl);
    executor->add_task(task);
}

void dispatcher::remove_task(task* task) {
    // The pass object is no event_task. Now find a worker to add it.
    try {
        event_task* et = dynamic_cast<event_task*>(task);
        if (nullptr != et) {
            remove_event_task(et);
            return;
        }
    } catch (std::exception&) {
    }

    // Check if the passed task object is an exec_task.
    try {
        exec_task* et = dynamic_cast<exec_task*>(task);
        if (nullptr != et) {
            remove_exec_task(et);
            return;
        }
    } catch (std::exception&) {
    }

    terr("dispatcher: task " << task << " is not an event_task nor an exec_task!");
    assert(false);
}

void dispatcher::remove_event_task(event_task* task) {
    // If the task implements the disposable interface, we check if it is ready to be
    // disposed.
    try {
        disposable* disp = dynamic_cast<disposable*>(task);
        if (nullptr != disp) {
            worker* worker = get_worker_by_task(task);
            tdbg("remove_event_task: deposing event_task " << task << " using worker " << worker << std::endl);
            if (disp->can_dispose()) {
                disp->dispose(worker);
            } else {
                // Try later
                cleanup_task* ct = new cleanup_task(disp);
                tdbg("remove_event_task: delayed deposing event_task "
                     << disp << "/" << task << " using worker " << worker << " and cleanup_task " << ct << std::endl);
                ct->init_watcher();
                ct->assign_worker(worker);
                ct->start_watcher(worker);
            }
            return;
        }
    } catch (std::exception&) {
    }
    // No disposable, so delete the task now.
    tdbg("remove_event_task: deleting event_task " << task << std::endl);
    delete task;
}

void dispatcher::remove_exec_task(exec_task* task) {
    tdbg("remove_exec_task: deleting exec_task " << task << std::endl);
    // No disposable object support yet. Just delete it.
    delete task;
}

static void handle_signal(struct ev_loop* /* loop */, ev_signal* sig, int /* revents */) {
    signal_func_t* pfunc = (signal_func_t*)sig->data;
    assert(nullptr != pfunc);
    (*pfunc)(sig->signum);
    delete pfunc;
}

void dispatcher::add_signal_handler(int sig, signal_func_t func) {
    if (m_instance && m_instance->m_started) {
        terr("dispatcher: add_signal_handler can't be used after the event loop has been started!");
        assert(false);
    }
    ev_signal* es = new ev_signal();
    signal_func_t* pfunc = new signal_func_t(func);
    ev_signal_init(es, handle_signal, sig);
    es->data = pfunc;
    ev_signal_start(ev_default_loop(0), es);
}

}  // tasks
