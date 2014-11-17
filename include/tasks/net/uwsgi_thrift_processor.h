/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#ifndef _THRIFT_PROCESSOR_H_
#define _THRIFT_PROCESSOR_H_

#include <arpa/inet.h>
#include <boost/shared_ptr.hpp>
#include <thrift/protocol/TBinaryProtocol.h>

#include <tasks/net/uwsgi_task.h>
#include <tasks/net/uwsgi_thrift_transport.h>

namespace tasks {
namespace net {

template <class processor_type, class handler_type>
class uwsgi_thrift_processor : public tasks::net::uwsgi_task {
  public:
    uwsgi_thrift_processor(net::socket& s) : uwsgi_task(s) {}

    virtual bool handle_request() {
        using namespace apache::thrift::protocol;
        using namespace apache::thrift::transport;
        typedef uwsgi_thrift_transport<uwsgi_request> in_transport_type;
        typedef uwsgi_thrift_transport<http_response> out_transport_type;
        typedef TBinaryProtocol protocol_type;

        boost::shared_ptr<in_transport_type> in_transport(new in_transport_type(request_p()));
        boost::shared_ptr<out_transport_type> out_transport(new out_transport_type(response_p()));
        boost::shared_ptr<protocol_type> in_protocol(new protocol_type(in_transport));
        boost::shared_ptr<protocol_type> out_protocol(new protocol_type(out_transport));

        // Process message
        try {
            boost::shared_ptr<handler_type> handler(new handler_type());
            handler->set_uwsgi_task(this);
            processor_type proc(handler);

            // Read a thrift object from the request, call the handler and write the respnding
            // thrift object to the uwsgi response.
            if (proc.process(in_protocol, out_protocol, NULL)) {
                response().set_status("200 OK");
            } else {
                response().set_status("400 Bad Request");
            }
        } catch (TTransportException& e) {
            set_error(std::string("TTransportException: ") + std::string(e.what()));
            response().set_status("400 Bad Request");
        }

        response().set_header("Content-Type", "application/x-thrift");
        send_response();

        return true;
    }
};

}  // net
}  // tasks

#endif  // _THRIFT_PROCESSOR_H_
