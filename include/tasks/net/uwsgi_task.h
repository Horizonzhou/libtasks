/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#ifndef _UWSGI_TASK_H_
#define _UWSGI_TASK_H_

#include <vector>
#include <unordered_map>
#include <string>
#include <cassert>

#include <tasks/worker.h>
#include <tasks/net_io_task.h>
#include <tasks/net/uwsgi_request.h>
#include <tasks/net/http_response.h>

// The nginx uwsgi module does not support keepalive connections. If
// this gets implemented one day or we find a different webserver that
// supports this, we can enable the following flag.
#define UWSGI_KEEPALIVE 0

namespace tasks {
namespace net {

class uwsgi_task : public tasks::net_io_task {
  public:
    uwsgi_task(net::socket& sock) : tasks::net_io_task(sock, EV_READ) {}
    virtual ~uwsgi_task() {}

    bool handle_event(tasks::worker* worker, int revents);

    // A request handler needs to implement this
    virtual bool handle_request() = 0;

    inline uwsgi_request& request() { return m_request; }

    inline uwsgi_request* request_p() { return &m_request; }

    inline http_response& response() { return m_response; }

    inline http_response* response_p() { return &m_response; }

    inline void send_response() {
        worker* w = worker::get();
        assert(nullptr != w);
        set_events(EV_WRITE);
        update_watcher(w);
    }

  protected:
    uwsgi_request m_request;
    http_response m_response;

    inline void finish_request() { m_response.clear(); }
};

}  // net
}  // tasks

#endif  // _UWSGI_TASK_H_
