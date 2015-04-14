/*
 * Copyright (c) 2013-2015 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#include <tasks/worker.h>
#include <tasks/timer_task.h>
#include <tasks/logging.h>
#include <unistd.h>

namespace tasks {

timer_task::timer_task(double after, double repeat, bool immediate)
    : m_after(after), m_repeat(repeat), m_immediate(immediate) {
    tdbg(get_string() << ": ctor" << std::endl);
    std::unique_ptr<ev_timer> timer(new ev_timer);
    m_timer = std::move(timer);
    // If "after" is 0 and "immediate" is true we will not schedule the first execution through the event loop but
    // execute it directly in start_watcher.
    if (0. == after && immediate) {
        after += repeat;
    }
    ev_timer_init(m_timer.get(), tasks_event_callback<ev_timer*>, after, repeat);
    m_timer->data = this;
}

timer_task::~timer_task() { tdbg(get_string() << ": dtor" << std::endl); }

void timer_task::start_watcher(worker* worker) {
    if (m_immediate) {
        m_immediate = false;
        event event = {this, 0};
        worker->exec_event_handler(event);
    } else {
        worker->exec_in_worker_ctx([this](struct ev_loop* loop) {
            if (!ev_is_active(m_timer.get())) {
                tdbg(get_string() << ": starting watcher" << std::endl);
                ev_timer_start(loop, m_timer.get());
            }
        });
    }
}

void timer_task::stop_watcher(worker* worker) {
    worker->exec_in_worker_ctx([this](struct ev_loop* loop) {
        if (ev_is_active(m_timer.get())) {
            tdbg(get_string() << ": stopping watcher" << std::endl);
            ev_timer_stop(loop, m_timer.get());
        }
    });
}

}  // tasks
