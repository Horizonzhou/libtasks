/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#include <tasks/worker.h>
#include <tasks/net_io_task.h>
#include <tasks/logging.h>

namespace tasks {

net_io_task::net_io_task(net::socket& socket, int events) : net_io_task(events) {
    tdbg(get_string() << ": ctor" << std::endl);
    m_socket = socket;
    if (-1 != m_socket.fd()) {
        init_watcher();
    }
}

net_io_task::~net_io_task() {
    tdbg(get_string() << ": dtor" << std::endl);
    // NOTE: The watcher will be stoped by dispose().
    if (m_auto_close) {
        tdbg(get_string() << "closing socket" << std::endl);
        m_socket.close();
    }
}

void net_io_task::add_task(worker* worker, net_io_task* task) {
    worker->signal_call([worker, task](struct ev_loop* loop) {
        tdbg(task->get_string() << ": adding net_io_task" << std::endl);
        task->init_watcher();
        task->assign_worker(worker);
        ev_io_start(loop, task->watcher());
    });
}

void net_io_task::add_task(net_io_task* task) { dispatcher::instance()->add_task(task); }

}  // tasks
