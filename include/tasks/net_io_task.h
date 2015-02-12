/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#ifndef _TASKS_NET_IO_TASK_H_
#define _TASKS_NET_IO_TASK_H_

#include <tasks/io_task_base.h>
#include <tasks/net/socket.h>

namespace tasks {

class worker;

/*!
 * \brief The net_io_task implements the base for socket based network tasks.
 */
class net_io_task : public io_task_base {
  public:
    net_io_task(int events) : io_task_base(events) {}
    net_io_task(net::socket& socket, int events);
    virtual ~net_io_task();

    /// A debug helper.
    inline std::string get_string() const {
        std::ostringstream os;
        os << "net_io_task(" << this << "," << iob().fd() << ":" << events() << ")";
        return os.str();
    }

    /// Provide access to the underlying socket object.
    inline net::socket& socket() {
        return m_socket;
    }
    /// Provide const access to the underlying socket object.
    inline const net::socket& socket() const {
        return m_socket;
    }

    /// This public method can be used to add io tasks outside of a worker thread
    /// context. If io tasks should be created within the context of a worker thread,
    /// you should use the protected non static method and pass a worker thread
    /// pointer.
    static void add_task(net_io_task* task);

  protected:
    void add_task(worker* worker, net_io_task* task);

    /// Grant socket access to the io_task_base.
    io_base& iob() {
        return m_socket;
    }
    /// Grant const socket access to the io_task_base.
    const io_base& iob() const {
        return m_socket;
    }
    /// Disable automatic closing of the socket in the desctructor.
    void disable_auto_close() {
        m_auto_close = false;
    }

  private:
    net::socket m_socket;
    bool m_auto_close = true;
};

}  // tasks

#endif  // _TASKS_NET_IO_TASK_H_
