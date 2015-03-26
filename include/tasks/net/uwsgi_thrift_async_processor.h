/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#ifndef _UWSGI_THRIFT_ASYNC_PROCESSOR_H_
#define _UWSGI_THRIFT_ASYNC_PROCESSOR_H_

#include <arpa/inet.h>
#include <boost/shared_ptr.hpp>
#include <thrift/Thrift.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/TApplicationException.h>
#include <unordered_set>
#include <future>
#include <sstream>

#include <tasks/net/uwsgi_task.h>
#include <tasks/net/uwsgi_thrift_transport.h>
#include <tasks/logging.h>

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

namespace tasks {
namespace net {

template <class handler_type>
class uwsgi_thrift_async_processor : public uwsgi_task {
  public:
    using in_transport_type = uwsgi_thrift_transport<uwsgi_request>;
    using out_transport_type = uwsgi_thrift_transport<http_response>;
    using protocol_type = TBinaryProtocol;

    uwsgi_thrift_async_processor(net::socket& s) : uwsgi_task(s) { tdbg(get_string() << ": ctor" << std::endl); }

    virtual ~uwsgi_thrift_async_processor() { tdbg(get_string() << ": dtor" << std::endl); }

    inline std::string get_string() const {
        std::ostringstream os;
        os << "uwsgi_thrift_async_processor(" << this << ")";
        return os.str();
    }

    /// \copydoc uwsgi_task::handle_request
    virtual bool handle_request() {
        boost::shared_ptr<in_transport_type> in_transport(new in_transport_type(request_p()));
        boost::shared_ptr<out_transport_type> out_transport(new out_transport_type(response_p()));
        boost::shared_ptr<protocol_type> in_protocol(new protocol_type(in_transport));
        boost::shared_ptr<protocol_type> out_protocol(new protocol_type(out_transport));

        // Process message
        m_handler.reset(new handler_type());
        tdbg(get_string() << ": created handler " << m_handler.get() << std::endl);
        m_handler->set_uwsgi_task(this);
        m_handler->on_finish([this, out_protocol] {
            if (m_handler->error()) {
                tdbg(get_string() << ": handler " << m_handler.get() << " finished with error(" << m_handler->error()
                                  << "): " << m_handler->error_message() << std::endl);
                write_thrift_error(std::string("Handler Error: ") + m_handler->error_message(),
                                   m_handler->service_name(), out_protocol);
            } else {
                tdbg(get_string() << ": handler " << m_handler.get() << " finished with no error" << std::endl);
                // Fill the response back in.
                out_protocol->writeMessageBegin(m_handler->service_name(), T_REPLY, m_seqid);
                m_handler->result_base().__isset.success = true;
                m_handler->result_base().write(out_protocol.get());
                out_protocol->writeMessageEnd();
                out_protocol->getTransport()->writeEnd();
                out_protocol->getTransport()->flush();
                response().set_status("200 OK");
            }
            response().set_header("Content-Type", "application/x-thrift");
            // Make sure we run in the context of a worker thread
            if (error_code() == tasks_error::UNSET) {
                worker* worker = dispatcher::instance()->get_worker_by_task(this);
                worker->exec_in_worker_ctx([this](struct ev_loop*) {
                    send_response();
                });
            }
            // Allow cleanup now
            enable_dispose();
        });

        try {
            std::string fname;
            TMessageType mtype;
            in_protocol->readMessageBegin(fname, mtype, m_seqid);
            if (mtype != protocol::T_CALL && mtype != protocol::T_ONEWAY) {
                write_thrift_error("invalid message type", m_handler->service_name(), out_protocol);
                send_thrift_response();
            } else if (fname != m_handler->service_name()) {
                write_thrift_error("invalid method name", m_handler->service_name(), out_protocol);
                send_thrift_response();
            } else {
                // read the args from the request
                auto args = std::make_shared<typename handler_type::args_t>();
                args->read(in_protocol.get());
                in_protocol->readMessageEnd();
                in_protocol->getTransport()->readEnd();
                // Make sure the object is kept until the m_handler finishes
                disable_dispose();
                // Call the m_handler
                tdbg(get_string() << ": calling service on handler " << m_handler.get() << std::endl);
                m_handler->service(args);
            }
        } catch (TException& e) {
            write_thrift_error(std::string("TException: ") + e.what(), m_handler->service_name(), out_protocol);
            send_thrift_response();
        }

        return true;
    }

    /// Send the thrift response back to the caller.
    inline void send_thrift_response() {
        response().set_header("Content-Type", "application/x-thrift");
        send_response();
    }

    /// Write a thrift error into the protocol buffer.
    ///
    /// \param msg The error message
    /// \param service_name The name of the thrift service
    /// \param out_protocol The outgoing protocol object
    inline void write_thrift_error(std::string msg, std::string service_name,
                                   boost::shared_ptr<protocol_type> out_protocol) {
        response().set_header("X-UWSGI_THRIFT_ASYNC_PROCESSOR_ERROR", msg);
        response().set_status("400 Bad Request");
        TApplicationException ae(msg);
        out_protocol->writeMessageBegin(service_name, T_EXCEPTION, m_seqid);
        ae.write(out_protocol.get());
        out_protocol->writeMessageEnd();
        out_protocol->getTransport()->writeEnd();
        out_protocol->getTransport()->flush();
    }

  private:
    int32_t m_seqid = 0;
    std::unique_ptr<handler_type> m_handler;
};

}  // net
}  // tasks

#endif  // _UWSGI_THRIFT_ASYNC_PROCESSOR_H_
