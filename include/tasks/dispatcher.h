/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#ifndef _TASKS_DISPATCHER_H_
#define _TASKS_DISPATCHER_H_

#include <vector>
#include <list>
#include <condition_variable>
#include <mutex>
#include <memory>
#include <atomic>
#include <unistd.h>

#include <tasks/ev_wrapper.h>
#include <tasks/tools/bitset.h>
#include <tasks/logging.h>

namespace tasks {

class worker;
class executor;
class task;
class event_task;
class exec_task;
class disposable;

struct signal_data;

typedef std::function<void(int)> signal_func_t;

class dispatcher {
    friend class test_exec;

  public:
    enum class mode { SINGLE_LOOP, MULTI_LOOP };

    dispatcher(uint8_t num_workers);

    /// Use this method to override the number of worker threads. The default is the
    /// number of CPU's. This method needs to be called before the first call to
    /// instance().
    static void init_workers(uint8_t num_workers) {
        if (nullptr == m_instance) {
            m_instance = std::make_shared<dispatcher>(num_workers);
        }
    }

    /// Set the run mode.
    /// The default is to run a leader/followers system (MODE_SINGLE_LOOP) in
    /// which only one event loop exists that is passed from worker to worker. An
    /// alternative is to run an event loop in each worker (MODE_MULTI_LOOP). This can
    /// improve the responsiveness and throughput in some situations.
    ///
    /// Note: This method has to be called before creating the dispatcher singleton.
    ///       It will fail when called later.
    ///
    /// Available Modes:
    ///   SINGLE_LOOP (Default)
    ///   MULTI_LOOP
    static void init_run_mode(mode m) {
        if (nullptr != m_instance) {
            terr("ERROR: dispatcher::init_run_mode must be called before anything else!" << std::endl);
            assert(false);
        }
        m_run_mode = m;
    }

    static mode run_mode() { return m_run_mode; }

    static std::shared_ptr<dispatcher> instance() {
        if (nullptr == m_instance) {
            // Create as many workers as we have CPU's per default
            m_instance = std::make_shared<dispatcher>(sysconf(_SC_NPROCESSORS_ONLN));
        }
        return m_instance;
    }

    static void destroy() {
        if (nullptr != m_instance) {
            if (!m_instance->m_term) {
                m_instance->terminate();
                m_instance->join();
            }
            m_instance.reset();
            m_instance = nullptr;
        }
    }

    /// Add a signal handler.
    /// You have to call this before calling start() or run().
    static void add_signal_handler(int sig, signal_func_t func);

    /// Get a free worker to promote it to the leader.
    std::shared_ptr<worker> free_worker();

    /// Find a free executor. If non is found a new executor gets created.
    std::shared_ptr<executor> free_executor();

    /// When a worker finishes his work he returns to the free worker queue.
    void add_free_worker(uint8_t id);

    /// Returns the last promoted worker from the workers vector. This can be useful
    /// to add tasks in situations where a worker handle is not available.
    inline worker* last_worker() {
        return m_workers[m_last_worker_id].get();
    }

    /// Return the worker assigned to a task. If no worker is assigned to the given task, a worker is picked depending
    /// on the run mode.
    worker* get_worker_by_task(event_task* task);

    /// Add a task to the system. This method will find out if the task is an event or exec task and handles it
    /// accordingly.
    void add_task(task* task);
    /// Add an event task to the system.
    void add_event_task(event_task* task);
    /// Add an exec task to the system.
    void add_exec_task(exec_task* task);

    /// Remove a task from the system.
    void remove_task(task* task);
    void remove_event_task(event_task* task);
    void remove_exec_task(exec_task* task);

    /// This method starts the system and blocks until terminate() gets called.
    [[deprecated]] void run(int num, ...);

    /// This method starts the system and blocks until terminate() gets called.
    void run(std::vector<tasks::task*>& tasks);

    /// Start the event loop. Do not block.
    void start();

    /// Wait for the dispatcher to finish.
    void join();

    /// Terminate the workers and die.
    inline void terminate() {
        m_term = true;
        m_finish_cond.notify_one();
    }

    void print_worker_stats() const;

  private:
    static std::shared_ptr<dispatcher> m_instance;
    std::atomic<bool> m_term;

    /// All worker threads
    std::vector<std::shared_ptr<worker> > m_workers;
    uint8_t m_num_workers = 0;

    /// All executor threads
    std::list<std::shared_ptr<executor> > m_executors;
    std::mutex m_executor_mutex;

    static mode m_run_mode;

    /// State of the workers used for maintaining the leader/followers
    tools::bitset m_workers_busy;
    tools::bitset::int_type m_last_worker_id = 0;

    /// A round robin counter to add tasks to the system. In case each
    /// worker runs it's own event loop this is useful to distribute tasks
    /// across the workers.
    std::atomic<uint8_t> m_rr_worker_id;

    /// Condition variable/mutex used to wait for finishing up
    std::condition_variable m_finish_cond;
    std::mutex m_finish_mutex;

    bool m_started = false;
};

}  // tasks

#endif  // _TASKS_DISPATCHER_H_
