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

/// The base class for the uwsgi protocol implementation.
class uwsgi_task : public tasks::net_io_task {
  public:
    uwsgi_task(net::socket& sock) : tasks::net_io_task(sock, EV_READ) {}
    virtual ~uwsgi_task() {}

    /// \copydoc event_task::handle_event
    bool handle_event(tasks::worker* worker, int revents);

    /// A uwsgi request handler needs to implement this. This method gets called after a uwsgi request has been
    /// deserialized successfully.
    virtual bool handle_request() = 0;

    /// \return A reference to the underlying request object.
    inline uwsgi_request& request() { return m_request; }

    /// \return A const reference to the underlying request object.
    inline const uwsgi_request& request() const { return m_request; }

    /// \return A pointer to the underlying request onject.
    inline uwsgi_request* request_p() { return &m_request; }

    /// \return A const pointer to the underlying request onject.
    inline const uwsgi_request* request_p() const { return &m_request; }

    /// \return A reference to the underlying response object.
    inline http_response& response() { return m_response; }

    /// \return A const reference to the underlying response object.
    inline const http_response& response() const { return m_response; }

    /// \return A pointer to the underlying response onject.
    inline http_response* response_p() { return &m_response; }

    /// \return A const pointer to the underlying response onject.
    inline const http_response* response_p() const { return &m_response; }

    /// Send the resonse back.
    inline void send_response() {
        worker* w = worker::get();
        assert(nullptr != w);
        set_events(EV_WRITE);
        update_watcher(w);
    }

  protected:
    uwsgi_request m_request;
    http_response m_response;

    /// Called after a request has been responded.
    inline void finish_request() { m_response.clear(); }
};

}  // net
}  // tasks

#endif  // _UWSGI_TASK_H_
