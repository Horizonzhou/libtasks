/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#ifndef _TASKS_EVENT_TASK_H_
#define _TASKS_EVENT_TASK_H_

#include <tasks/task.h>
#include <tasks/error_base.h>
#include <tasks/dispatcher.h>

namespace tasks {

/// Function type for functors that get executed in the context of a worker thread. See
/// worker::exec_in_worker_ctx(task_func_t f).
typedef std::function<void(struct ev_loop*)> task_func_t;

class event_task : public task, public error_base {
  public:
    typedef std::function<void(worker* worker, const tasks_exception& e)> error_func_worker_t;
    typedef std::function<void(const tasks_exception& e)> error_func_void_t;
    struct error_func_t {
        error_func_t(error_func_worker_t f) : m_type(0), m_f_worker(f) {}
        error_func_t(error_func_void_t f) : m_type(1), m_f_void(f) {}

        void operator()(worker* worker, const tasks_exception& e) {
            switch (m_type) {
                case 0:
                    m_f_worker(worker, e);
                    break;
                case 1:
                    m_f_void(e);
                    break;
            }
        }

        int m_type = 0;
        error_func_worker_t m_f_worker;
        error_func_void_t m_f_void;
    };

    virtual ~event_task() {}

    /// Will be called for each I/O or timer event.
    ///
    /// Each task needs to implement the handle_event method. Returns true if the task stays active and false otherwise.
    /// The task will be deleted if false is returned and auto_delete() returns true.
    ///
    /// \param worker The worker thread executing the task.
    /// \param events The events bitmask.
    /// \return True to continue execution. False to remove the task.
    ///
    virtual bool handle_event(worker* worker, int events) = 0;

    /// Initialize the underlying watcher object.
    virtual void init_watcher() = 0;

    /// Deactivate the underlying watcher.
    virtual void stop_watcher(worker* worker) = 0;

    /// Activate the underlying watcher to listen for I/O or timer events.
    virtual void start_watcher(worker* worker) = 0;

    /// Returns a pointer to the assigned worker.
    inline worker* assigned_worker() const { return m_worker; }

    /// Assigns a worker to the task in multi loop mode.
    ///
    /// For multi loop mode a task does not leave the context of a worker thread, as each thread runs its own event
    /// loop. That also means this worker has to execute a dispose action. As dispose allows to be called from outside
    /// of the task system (a non worker thread context), a handle to the worker the task belongs to is needed. The
    /// method does nothing in single loop mode.
    ///
    /// \param worker The worker to assign.
    void assign_worker(worker* worker);

    /// Called by a worker when a task reports an error. All error callbacks will be executed. A worker calls this
    /// method after handle_event.
    void notify_error(worker* worker = nullptr);

    /// If a task failed it can execute callback functions. Note that no locks will be used at this
    /// level.
    inline void on_error(error_func_worker_t f) {
        m_error_funcs.push_back(error_func_t(f));
    }

    /// Install an error callback.
    inline void on_error(error_func_void_t f) {
        m_error_funcs.push_back(error_func_t(f));
    }

  private:
    worker* m_worker = nullptr;
    std::vector<error_func_t> m_error_funcs;
};

}  // tasks

#endif  // _TASKS_EVENT_TASK_H_
