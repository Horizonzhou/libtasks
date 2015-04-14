/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#include <tasks/worker.h>
#include <chrono>

namespace tasks {

#ifndef __clang__
thread_local worker* worker::m_worker_ptr = nullptr;
#else
__thread worker* worker::m_worker_ptr = nullptr;
#endif

worker::worker(uint8_t id, std::unique_ptr<loop_t>& loop) : m_id(id), m_term(false), m_leader(false) {
    // Initialize and add the threads async watcher
    ev_async_init(&m_signal_watcher, tasks_async_callback);
    m_signal_watcher.data = new task_func_queue_t;

    assert(dispatcher::mode::SINGLE_LOOP == dispatcher::run_mode() || nullptr != loop);
    struct ev_loop* loop_raw = nullptr;
    if (nullptr != loop) {
        m_loop = std::move(loop);
        m_leader = true;
        ev_set_userdata(m_loop->ptr, this);
        loop_raw = m_loop->ptr;
    } else {
        loop_raw = ev_default_loop(0);
    }
    ev_async_start(loop_raw, &m_signal_watcher);
    m_thread.reset(new std::thread(&worker::run, this));
}

worker::~worker() {
    tdbg(get_string() << ": dtor" << std::endl);
    task_func_queue_t* tfq = (task_func_queue_t*)m_signal_watcher.data;
    delete tfq;
}

void worker::run() {
    m_worker_ptr = this;
    mark_free();

    while (!m_term) {
        // Wait until promoted to the leader thread
        if (!m_leader) {
            tdbg(get_string() << ": waiting..." << std::endl);
            std::unique_lock<std::mutex> lock(m_work_mutex);
            // Use wait_for to check the term flag
            bool ready = false;
            while (!ready && !m_term) {
                ready = m_work_cond.wait_for(lock, std::chrono::milliseconds(1), [this] { return m_leader == true; });
            }
        }

        // Became leader, so execute the event loop
        while (m_leader && !m_term) {
            tdbg(get_string() << ": running event loop" << std::endl);
            ev_loop(m_loop->ptr, EVLOOP_ONESHOT);
            tdbg(get_string() << ": event loop returned" << std::endl);
            // Check if events got fired
            if (!m_events_queue.empty()) {
                tdbg(get_string() << ": executing events" << std::endl);
                // Now promote the next leader and call the event
                // handlers
                promote_leader();
                // Handle events
                while (!m_events_queue.empty()) {
                    m_events_count++;
                    event event = m_events_queue.front();
                    exec_event_handler(event);
                    m_events_queue.pop();
                }
            }
        }

        if (!m_term) {
            mark_free();
        } else {
            // Shutdown, the leader terminates the loop
            if (m_leader) {
                ev_unloop(m_loop->ptr, EVUNLOOP_ALL);
                ev_loop_destroy(m_loop->ptr);
            }
        }
    }
}

void worker::exec_event_handler(event& event) {
    bool cont = event.task->handle_event(this, event.revents);
    // Trigger the error callbacks if needed.
    if (event.task->error()) {
        event.task->notify_error(this);
    }
    // If the handler wants to continue to run the task we activate the watcher again. Otherwise we
    // delete it if it has auto deletion activated.
    if (cont) {
        event.task->reset_error();
        event.task->start_watcher(this);
    } else {
        if (event.task->auto_delete()) {
            event.task->finish(this);
        }
    }
}

void tasks_async_callback(struct ev_loop* loop, ev_async* w, int /* events */) {
    worker* worker = (tasks::worker*)ev_userdata(loop);
    assert(nullptr != worker);
    task_func_queue_t* tfq = (tasks::task_func_queue_t*)w->data;
    if (nullptr != tfq) {
        std::queue<task_func_t> qcpy;
        {
            std::lock_guard<std::mutex> lock(tfq->mutex);
            tfq->queue.swap(qcpy);
        }
        // Execute all queued functors
        while (!qcpy.empty()) {
            assert(worker->exec_in_worker_ctx(qcpy.front()));
            qcpy.pop();
        }
    }
}

}  // tasks
