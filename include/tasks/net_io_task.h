/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#ifndef _TASKS_NET_IO_TASK_H_
#define _TASKS_NET_IO_TASK_H_

#include <tasks/event_task.h>
#include <tasks/disposable.h>
#include <tasks/ev_wrapper.h>
#include <tasks/net/socket.h>
#include <memory>
#include <sstream>

namespace tasks {

class worker;

class net_io_task : public event_task, public disposable {
  public:
    net_io_task(int events);
    net_io_task(net::socket& socket, int events);
    virtual ~net_io_task();

    inline std::string get_string() const {
        std::ostringstream os;
        os << "net_io_task(" << this << "," << m_socket.fd() << ":" << m_events << ")";
        return os.str();
    }

    inline net::socket& socket() { return m_socket; }

    inline int events() const { return m_events; }

    inline ev_io* watcher() const { return m_io.get(); }

    void init_watcher();

    // Start a watcher in the context of the given worker
    void start_watcher(worker* worker);
    // Stop a watcher in the context of the given worker
    void stop_watcher(worker* worker);
    // Udate a watcher in the context of the given worker
    void update_watcher(worker* worker);

    // Stop the watcher before being deleted
    virtual void dispose(worker* worker);

    // This public method can be used to add io tasks outside of a worker thread
    // context. If io tasks should be created within the context of a worker thread,
    // you should use the protected non static method and pass a worker thread
    // pointer.
    static void add_task(net_io_task* task);

  protected:
    void set_socket(net::socket& socket);
    void set_events(int events);
    void add_task(worker* worker, net_io_task* task);

  private:
    std::unique_ptr<ev_io> m_io;
    bool m_watcher_initialized = false;
    net::socket m_socket;
    int m_events = EV_UNDEF;
    bool m_change_pending = false;
};

}  // tasks

#endif  // _TASKS_NET_IO_TASK_H_
