/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#ifndef _TASKS_IO_TASK_BASE_H_
#define _TASKS_IO_TASK_BASE_H_

#include <tasks/event_task.h>
#include <tasks/disposable.h>
#include <tasks/ev_wrapper.h>
#include <tasks/io_base.h>
#include <memory>
#include <sstream>

namespace tasks {

class worker;

/*!
 * \brief This is the base class for io event tasks. It controls an
 *   ev_io watcher that monitors io events on file handles.
 */
class io_task_base : public event_task, public disposable {
  public:
    io_task_base(int events);
    virtual ~io_task_base() {}

    std::string get_string() const {
        std::ostringstream os;
        os << "io_task_base(" << this << ")";
        return os.str();
    }

    /// Return the monitored events.
    inline int events() const { return m_events; }
    /// Return the io watcher object.
    inline ev_io* watcher() const { return m_io.get(); }

    /// Initialize the watcher
    virtual void init_watcher();

    /// Start a watcher in the context of the given worker
    virtual void start_watcher(worker* worker);
    /// Stop a watcher in the context of the given worker
    virtual void stop_watcher(worker* worker);
    /// Udate a watcher in the context of the given worker
    virtual void update_watcher(worker* worker);

    /// Stop the watcher before being deleted
    virtual void dispose(worker* worker);

  protected:
    /// Return the io_base object.
    virtual io_base& iob() = 0;
    /// Return a const io_base object.
    virtual const io_base& iob() const = 0;
    
    /// Update the events the object monitors.
    void set_events(int events);

  private:
    std::unique_ptr<ev_io> m_io;
    bool m_watcher_initialized = false;
    int m_events = EV_UNDEF;
    bool m_change_pending = false;
};

}  // tasks

#endif  // _TASKS_IO_TASK_BASE_H_
