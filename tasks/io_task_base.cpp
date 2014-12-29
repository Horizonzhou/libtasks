/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#include <tasks/worker.h>
#include <tasks/io_task_base.h>
#include <tasks/logging.h>

namespace tasks {

io_task_base::io_task_base(int events) : m_events(events) {
    tdbg(get_string() << ": ctor" << std::endl);
    m_io.reset(new ev_io);
    ev_init(m_io.get(), tasks_event_callback<ev_io*>);
    m_io->data = this;
}

void io_task_base::init_watcher() {
    tdbg(get_string() << ": init watcher with fd " << iob().fd() << std::endl);
    ev_io_set(m_io.get(), iob().fd(), m_events);
    m_watcher_initialized = true;
}

void io_task_base::set_events(int events) {
    if (m_events != events) {
        tdbg(get_string() << ": setting events to " << events << std::endl);
        m_events = events;
        m_change_pending = true;
    }
}

void io_task_base::start_watcher(worker* worker) {
    assert(m_watcher_initialized);
    worker->signal_call([this](struct ev_loop* loop) {
        if (!ev_is_active(m_io.get())) {
            tdbg(get_string() << ": starting watcher" << std::endl);
            ev_io_start(loop, m_io.get());
        }
    });
}

void io_task_base::stop_watcher(worker* worker) {
    assert(m_watcher_initialized);
    worker->signal_call([this](struct ev_loop* loop) {
        if (ev_is_active(m_io.get())) {
            tdbg(get_string() << ": stopping watcher" << std::endl);
            ev_io_stop(loop, m_io.get());
        }
    });
}

void io_task_base::update_watcher(worker* worker) {
    assert(m_watcher_initialized);
    if (m_change_pending) {
        worker->signal_call([this](struct ev_loop* loop) {
            tdbg(get_string() << ": updating watcher" << std::endl);
            bool active = ev_is_active(m_io.get());
            if (active) {
                ev_io_stop(loop, m_io.get());
            }
            ev_io_set(m_io.get(), iob().fd(), m_events);
            if (active) {
                ev_io_start(loop, m_io.get());
            }
        });
        m_change_pending = false;
    }
}

void io_task_base::dispose(worker* worker) {
    worker->signal_call([this](struct ev_loop* loop) {
        if (ev_is_active(watcher())) {
            tdbg(get_string() << ": disposing io_task_base" << std::endl);
            ev_io_stop(loop, watcher());
        }
        delete this;
    });
}

}  // tasks
