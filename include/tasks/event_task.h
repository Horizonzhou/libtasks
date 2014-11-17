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

// Signals to enter a worker thread context. Passed to worker::signal_call().
typedef std::function<void(struct ev_loop*)> task_func_t;

class event_task : public task, public error_base {
  public:
    virtual ~event_task() {}

    /*!
     * \brief Will be called for each event.
     *
     * Each task needs to implement the handle_event method. Returns
     * true if the task stays active and false otherwise. The task
     * will be deleted if false is returned and auto_delete() returns
     * true.
     *
     * \param worker The worker thread executing the task.
     * \param events The events bitmask.
     * \return True to continue execution. False to remove the task.
     */
    virtual bool handle_event(worker* worker, int events) = 0;

    virtual void init_watcher() = 0;
    virtual void stop_watcher(worker* worker) = 0;
    virtual void start_watcher(worker* worker) = 0;

    /*!
     * \brief Return a pointer to the assigned worker.
     */
    inline worker* assigned_worker() const { return m_worker; }

    /*!
     * \brief Assigns a worker to the task in multi loop mode.
     *
     * For multi loop mode a task does not leave the context of a
     * worker thread, as each thread runs its own event loop. That
     * also means this worker has to execute a dispose action. As
     * dispose allows to be called from outside of the task system (a
     * non worker thread context), a handle to the worker the task
     * belongs to is needed.
     *
     * The method does nothing in single loop mode.
     *
     * \param worker The worker to assign.
     */
    void assign_worker(worker* worker);

  private:
    worker* m_worker = nullptr;
};

}  // tasks

#endif  // _TASKS_EVENT_TASK_H_
