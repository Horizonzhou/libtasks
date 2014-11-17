/*
 * Copyright (c) 2013-2014 ADTECH GmbH
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

class timer_task : public event_task {
  public:
    timer_task(double after, double repeat);
    virtual ~timer_task();

    inline std::string get_string() const {
        std::ostringstream os;
        os << "timer_task(" << this << ")";
        return os.str();
    }

    inline ev_timer* watcher() const { return m_timer.get(); }

    inline double after() const { return m_after; }

    inline double repeat() const { return m_repeat; }

    void init_watcher() {}
    void start_watcher(worker* worker);
    void stop_watcher(worker* worker);

  private:
    std::unique_ptr<ev_timer> m_timer;
    double m_after = 0;
    double m_repeat = 0.;
};

}  // tasks

#endif  // _TASKS_TIMER_TASK_H_
