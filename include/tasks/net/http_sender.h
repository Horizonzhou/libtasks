/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#ifndef _HTTP_SENDER_H_
#define _HTTP_SENDER_H_

#include <memory>
#include <cassert>
#include <cstring>

#include <tasks/dispatcher.h>
#include <tasks/worker.h>
#include <tasks/logging.h>
#include <tasks/net_io_task.h>
#include <tasks/net/http_request.h>
#include <tasks/net/http_response.h>
#include <tasks/net/socket.h>

namespace tasks {
namespace net {

class http_response_handler {
  public:
    virtual bool handle_response(std::shared_ptr<http_response> response) = 0;
};

template <class handler_type>
class http_sender : public net_io_task {
  public:
    http_sender() : net_io_task(EV_UNDEF), m_response(new http_response()) {}

    http_sender(std::shared_ptr<handler_type> handler)
        : net_io_task(EV_UNDEF), m_response(new http_response()), m_handler(handler) {}

    /// \copydoc event_task::handle_event
    bool handle_event(tasks::worker* worker, int events) {
        bool success = true;
        try {
            if (EV_READ & events) {
                m_response->read_data(socket());
                if (m_response->done()) {
                    if (nullptr == m_handler) {
                        m_handler = std::make_shared<handler_type>();
                    }
                    success = m_handler->handle_response(m_response);
                    m_response->clear();
                }
            } else if (EV_WRITE & events) {
                m_request->write_data(socket());
                if (m_request->done()) {
                    // Reset the request buffer to be able to reuse the same object again
                    m_request->clear();
                    // Read the response
                    set_events(EV_READ);
                    update_watcher(worker);
                }
            }
        } catch (tasks::tasks_exception& e) {
            set_exception(e);
            success = false;
        }
        return success;
    }

    inline bool connected() const {
        return socket().fd() != -1;
    }

    /// Send out an http_request. The http_sender will automatically be added to the task system when calling this
    /// method.
    inline void send(std::shared_ptr<http_request> request) {
        m_request = request;
        const std::string& host = m_request->header("Host");
        tdbg("http_sender: Sending request to " << host << std::endl);
        // Find the worker if keepalive is used, and this is not the first request, or the object is reused to connect
        // to a different host.
        tasks::worker* worker = tasks::dispatcher::instance()->get_worker_by_task(this);
        if (connected() && m_host != host) {
            // Stop the watcher and close an existing connection.
            tdbg("http_sender: Closing connection to " << m_host << ":" << m_port << std::endl);
            stop_watcher(worker);
            socket().close();
        }
        m_host = host;
        m_port = m_request->port();
        m_request->set_header("Host", m_host);
        set_events(EV_WRITE);
        if (!connected()) {
            // Connect
            tdbg("http_sender: Connecting " << m_host << ":" << m_port << std::endl);
            socket().connect(m_host, m_port);
            tasks::dispatcher::instance()->add_event_task(this);
        } else {
            update_watcher(worker);
        }
    }

  private:
    std::shared_ptr<http_request> m_request;
    std::shared_ptr<http_response> m_response;
    std::shared_ptr<handler_type> m_handler;
    std::string m_host;
    int m_port = 80;
};

}  // net
}  // tasks

#endif  // _HTTP_SENDER_H_
