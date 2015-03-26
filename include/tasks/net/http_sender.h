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

    /// Contructor
    ///
    /// \param handler The response handler to be called.
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

    /// \return True if connected.
    inline bool connected() const {
        return socket().fd() != -1;
    }

    /// Send out an http_request or uwsgi_request. The http_sender will automatically be added to the task system when
    /// calling this method.
    inline void send(std::shared_ptr<http_base> request) {
        m_request = request;
        std::string remote;
        bool tcp = true;
        if (request->host() != http_base::NO_VAL) {
            // Remote host via TCP
            remote = request->host();
        } else if (request->path() != http_base::NO_VAL) {
            // Remote host via unix domain
            remote = request->path();
            tcp = false;
        } else {
            throw tasks_exception(tasks_error::HTTP_SENDER_INVALID_REMOTE, "No host or path given");
        }
        tdbg("http_sender: Sending request to " << remote << std::endl);
        // Find the worker if keepalive is used, and this is not the first request, or the object is reused to connect
        // to a different host.
        tasks::worker* worker = tasks::dispatcher::instance()->get_worker_by_task(this);
        if (connected() && m_remote != remote) {
            // Stop the watcher and close an existing connection.
            tdbg("http_sender: Closing connection to " << m_remote << std::endl);
            stop_watcher(worker);
            socket().close();
        }
        m_remote = remote;
        set_events(EV_WRITE);
        if (!connected()) {
            // Connect
            if (tcp) {
                tdbg("http_sender: Connecting " << m_remote << ":" << request->port() << std::endl);
                socket().connect(m_remote, request->port());
            } else {
                tdbg("http_sender: Connecting " << m_remote << std::endl);
                socket().connect(m_remote);
            }
            tasks::dispatcher::instance()->add_event_task(this);
        } else {
            update_watcher(worker);
        }
    }

  private:
    std::shared_ptr<http_base> m_request;
    std::shared_ptr<http_response> m_response;
    std::shared_ptr<handler_type> m_handler;
    std::string m_remote;
};

}  // net
}  // tasks

#endif  // _HTTP_SENDER_H_
