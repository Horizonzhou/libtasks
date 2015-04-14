/*
 * Copyright (c) 2013-2015 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#ifndef _TASKS_TIMER_TASK_H_
#define _TASKS_TIMER_TASK_H_

#include <tasks/event_task.h>
#include <tasks/ev_wrapper.h>
#include <memory>
#include <sstream>

namespace tasks {

class worker;

/// A timer task implementation.
class timer_task : public event_task {
  public:
    /// Constructor
    ///
    /// \param after The time in seconds to the first execution. Note: The tasks \link event_task::handle_event(worker*
    ///   worker, int events) \endlink is guaranteed to be invoked only after its timeout has passed (not at, so on
    ///   systems with very low-resolution clocks this might introduce a small delay). If you need immediate esxecution,
    ///   see the immediate parameter below.
    /// \param repeat The time in seconds to repeat the timer. If you set this parameter to 0, the timer will be
    ///   executed only once.
    /// \param immediate Enable immediate execution if the "after" parameter is set to 0. The handle_event method will
    ///   be executed when adding the task via dispatcher::add_task(task* task) by the start_watcher(worker* worker)
    ///   method within the thread context of the calling thread. Otherwise the first execution is triggered through the
    ///   event loop and is executed by one of the worker threads. The parameter has no effect if "after" is not set to
    ///   0.
    timer_task(double after, double repeat, bool immediate = false);

    virtual ~timer_task();

    inline std::string get_string() const {
        std::ostringstream os;
        os << "timer_task(" << this << ")";
        return os.str();
    }

    /// \return The underlying watcher object.
    inline ev_timer* watcher() const { return m_timer.get(); }
    /// \return After value.
    inline double after() const { return m_after; }
    /// \return Repeat value.
    inline double repeat() const { return m_repeat; }

    void init_watcher() {}
    void start_watcher(worker* worker);
    void stop_watcher(worker* worker);

  private:
    std::unique_ptr<ev_timer> m_timer;
    double m_after = 0.;
    double m_repeat = 0.;
    bool m_immediate = false;
};

}  // tasks

#endif  // _TASKS_TIMER_TASK_H_
