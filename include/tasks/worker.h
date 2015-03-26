/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#ifndef _TASKS_WORKER_H_
#define _TASKS_WORKER_H_

#include <tasks/dispatcher.h>
#include <tasks/event_task.h>
#include <tasks/logging.h>
#include <tasks/ev_wrapper.h>
#include <thread>
#include <atomic>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <queue>
#include <sstream>
#include <cassert>
#include <vector>

// To enable worker::add_time() set this to 1.
#define ENABLE_ADD_TIME 0
#define ADD_TIME_BUCKETS 10

#if ENABLE_ADD_TIME == 1
#include <chrono>
#endif

namespace tasks {

class event_task;

/// Needed to use std::unique_ptr<>
struct loop_t {
    struct ev_loop* ptr;
    loop_t() : ptr(nullptr) {}
    loop_t(struct ev_loop* p) : ptr(p) {}
};

struct task_func_queue_t {
    std::queue<task_func_t> queue;
    std::mutex mutex;
};

/// Put all queued events into a queue instead of handling them
/// directly from handle_io_event as multiple events can fire and
/// we want to promote the next leader after the ev_loop call
/// returns to avoid calling it from multiple threads.
struct event {
    tasks::event_task* task;
    int revents;
};

class worker {
  public:
    worker(uint8_t id, std::unique_ptr<loop_t>& loop);
    virtual ~worker();

    /// Return the worker id.
    inline uint8_t id() const { return m_id; }

    /// Provide access to the executing worker thread object in the current thread context.
    static worker* get() { return m_worker_ptr; }

    /// Returns true if the worker is a leader.
    inline bool leader() const { return m_leader; }

    inline std::string get_string() const {
        std::ostringstream os;
        os << "worker(" << this << "," << (unsigned int)m_id << ")";
        return os.str();
    }

    /// Return the event loop pointer for this worker.
    inline struct ev_loop* loop_ptr() const {
        struct ev_loop* loop = nullptr;
        switch (dispatcher::run_mode()) {
            case dispatcher::mode::MULTI_LOOP:
                assert(nullptr != m_loop);
                loop = m_loop->ptr;
                break;
            default:
                loop = ev_default_loop(0);
        }
        return loop;
    }

    /// Executes task_func_t directly if called in leader thread context and if the worker is executing itself or
    /// delegates it. Returns true when task_func_t has been executed. If some work needs to be executed in the context
    /// of a worker thread (eg to modify a watcher) this method needs to be used.
    inline bool exec_in_worker_ctx(task_func_t f) {
        if (m_leader && this == worker::get()) {
            // The worker is running an event loop and we are running in the workers thread context, now execute the
            // functor.
            f(m_loop->ptr);
            return true;
        } else {
            async_call(f);
            return false;
        }
    }

    /// Same as exec_in_ctx(task_func_t f)
    inline bool signal_call(task_func_t f) {
        return exec_in_worker_ctx(f);
    }

    /// Put a functor into the async work queue of a worker and notify it.
    inline void async_call(task_func_t f) {
        task_func_queue_t* tfq = (task_func_queue_t*)m_signal_watcher.data;
        {
            std::lock_guard<std::mutex> lock(tfq->mutex);
            tfq->queue.push(f);
        }
        ev_async_send(loop_ptr(), &m_signal_watcher);
    }

    /// Pass the event loop to the worker.
    inline void set_event_loop(std::unique_ptr<loop_t>& loop) {
        m_loop = std::move(loop);
        ev_set_userdata(m_loop->ptr, this);
        m_leader = true;
        m_work_cond.notify_one();
    }

    /// Terminate the worker and wait for it to finish. 
    inline void terminate() {
        tdbg(get_string() << ": waiting to terminate thread" << std::endl);
        m_term = true;
        m_work_cond.notify_one();
        if (m_leader) {
            // interrupt the event loop
            ev_async_send(loop_ptr(), &m_signal_watcher);
        }
        m_thread->join();
        tdbg(get_string() << ": thread done" << std::endl);
    }

    /// Add an event to the workers queue.
    inline void add_event(event e) { m_events_queue.push(e); }

    /// Add an event to the workers queue from a different thread context.
    static void add_async_event(event e) {
        worker* w = dispatcher::instance()->last_worker();
        tdbg("worker: adding async event to worker " << w << std::endl);
        w->async_call([e](struct ev_loop* loop) {
            // get the executing worker
            worker* worker = (tasks::worker*)ev_userdata(loop);
            worker->add_event(e);
        });
    }

    /// Handle an I/O event.
    void handle_io_event(ev_io* watcher, int revents);

    /// Handle a timer event.
    void handle_timer_event(ev_timer* watcher);

    /// Return the number of events the worker has handled until now.
    inline uint64_t events_count() const { return m_events_count; }

#if ENABLE_ADD_TIME == 1
    /// If you need some internal time measurements local to the worker threads, you can
    /// enable this method and drop times in microseconds into this. An average value will
    /// be printed to STDOUT for every 5000 measures.
    ///
    /// Example:
    ///
    /// Measure the time it takes to handle some request:
    ///
    ///   auto s = std::chrono::high_resolution_clock::now();
    ///   success = handle_request();
    ///   auto e = std::chrono::high_resolution_clock::now();
    ///   uint64_t delta = std::chrono::duration_cast<std::chrono::microseconds>(e - s).count();
    ///   worker->add_time(0, delta);
    ///
    inline void add_time(uint64_t idx, uint64_t t) {
        m_time_total[idx] += t;
        m_time_count[idx]++;
        if (m_time_count[idx] == 5000) {
            std::cout << get_string() << " time(" << idx << "): avg " << (double)m_time_total[idx] / 5000 << " micros"
                      << std::endl;
            m_time_total[idx] = 0;
            m_time_count[idx] = 0;
        }
    }

    /// Start measuring for a given index.
    inline void measure_begin(uint64_t idx) {
        m_time_beg[idx] = std::chrono::high_resolution_clock::now();
    }

    /// Finish measuring for a given index. The time is added via add_time(uint64_t idx, uint64_t t).
    inline void measure_end(uint64_t idx) {
        auto end = std::chrono::high_resolution_clock::now();
        uint64_t delta = std::chrono::duration_cast<std::chrono::microseconds>(end - m_time_beg[idx]).count();
        add_time(idx, delta);
    }
#endif

  private:
#ifndef __clang__
    thread_local static worker* m_worker_ptr;  /// A thread local pointer to the worker thread
#else
    __thread static worker* m_worker_ptr;  /// A thread local pointer to the worker thread
#endif
    uint8_t m_id;
    uint64_t m_events_count = 0;
    std::unique_ptr<loop_t> m_loop;
    std::atomic<bool> m_term;
    std::atomic<bool> m_leader;
    std::mutex m_work_mutex;
    std::condition_variable m_work_cond;
    std::queue<event> m_events_queue;

#if ENABLE_ADD_TIME == 1
    uint64_t m_time_total[ADD_TIME_BUCKETS];
    uint64_t m_time_count[ADD_TIME_BUCKETS];
    std::chrono::high_resolution_clock::time_point m_time_beg[ADD_TIME_BUCKETS];
#endif

    /// Every worker has an async watcher to be able to call
    /// into the leader thread context.
    ev_async m_signal_watcher;

    std::unique_ptr<std::thread> m_thread;

    /// Find a free worker and promote it to become the next leader. 
    inline void promote_leader() {
        if (dispatcher::mode::SINGLE_LOOP == dispatcher::run_mode()) {
            std::shared_ptr<worker> w = dispatcher::instance()->free_worker();
            if (nullptr != w) {
                // If we find a free worker, we promote it to the next
                // leader. This thread stays leader otherwise.
                m_leader = false;
                w->set_event_loop(m_loop);
            }
        }
    }

    /// Let the dispatcher know that this thread is free to become a leader and take on some work.
    inline void mark_free() {
        if (dispatcher::mode::SINGLE_LOOP == dispatcher::run_mode()) {
            dispatcher::instance()->add_free_worker(id());
        }
    }

    /// Main method of the thread.
    void run();
};

/* CALLBACKS */

/// Callback for I/O events.
template <typename EV_t>
void tasks_event_callback(struct ev_loop* loop, EV_t w, int e) {
    worker* worker = (tasks::worker*)ev_userdata(loop);
    assert(nullptr != worker);
    event_task* task = (tasks::event_task*)w->data;
    task->stop_watcher(worker);
    event event = {task, e};
    worker->add_event(event);
}

/// Callback for async events.
void tasks_async_callback(struct ev_loop* loop, ev_async* w, int events);

}  // tasks

#endif  // _TASKS_WORKER_H_
