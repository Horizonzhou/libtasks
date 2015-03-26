/*
 * Copyright (c) 2013-2015 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#ifndef _UWSGI_THRIFT_CLIENT_H_
#define _UWSGI_THRIFT_CLIENT_H_

#include <thrift/transport/TVirtualTransport.h>
#include <tasks/net/uwsgi_request.h>
#include <tasks/net/http_response.h>
#include <tasks/net/socket.h>

namespace tasks {
namespace net {

/// A uwsgi blocking client.
class uwsgi_thrift_client
    : public apache::thrift::transport::TVirtualTransport<uwsgi_thrift_client> {
  public:
    uwsgi_thrift_client(const std::string& host, int port) : m_request(host, port) {}
    uwsgi_thrift_client(const std::string& path) : m_request(path) {}
    ~uwsgi_thrift_client() {}

    uint32_t read(uint8_t* data, int32_t size) { return m_response.read((char*)data, size); }

    void write(const uint8_t* data, uint32_t size) { m_request.write((const char*)data, size); }

    uint32_t readEnd() {
        m_request.clear();
        m_response.clear();
        // no keep alive for now
        m_socket.close();
        open();
    }

    void open() {
        if (!isOpen()) {
            m_socket.set_blocking();
            if (m_request.host() != http_base::NO_VAL) {
                m_socket.connect(m_request.host(), m_request.port());
            } else if (m_request.path() != http_base::NO_VAL) {
                m_socket.connect(m_request.path());
            }
        }
    }

    void close() { m_socket.close(); }

    bool isOpen() { return m_socket.fd() != -1; }

    void flush() {
        m_request.write_data(m_socket);
        while (!m_response.done()) {
            m_response.read_data(m_socket);
        }
    }

  private:
    uwsgi_request m_request;
    http_response m_response;
    socket m_socket;
};

}  // net
}  // tasks

#endif  // _UWSGI_THRIFT_CLIENT_H_
