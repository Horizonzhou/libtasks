/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#ifndef _UWSGI_REQUEST_H_
#define _UWSGI_REQUEST_H_

#include <string>
#include <cstring>
#include <unordered_map>
#include <vector>
#include <iostream>

#include <tasks/tasks_exception.h>
#include <tasks/net/socket.h>
#include <tasks/net/uwsgi_structs.h>
#include <tasks/net/io_state.h>
#include <tasks/net/http_base.h>
#include <tasks/tools/buffer.h>

namespace tasks {
namespace net {

/// The uwsgi protocol implementation for a request. The response is a HTTP/1.1 response.
class uwsgi_request : public http_base {
  public:
    /// For backward compatibility.
    using uwsgi_vars_t = http_base::headers_t;

    static std::string NO_VAL;

    uwsgi_request() : http_base() { m_header = {0, 0, 0}; }

    uwsgi_request(const std::string& host, int port) : http_base(), m_host(host), m_port(port) {}

    uwsgi_request(const std::string& path) : http_base(), m_path(path) {}

    /// The host interface for remote connections.
    const std::string& host() const { return m_host; }

    /// The port interface for remote connections.
    int port() const { return m_port; }

    /// The path interface for unix domain socket connections.
    const std::string& path() const { return m_path; }

    /// Provide access to uwsgi parameters.
    ///
    /// \param key The parameter name.
    /// \return The parameter value.
    inline const std::string& var(const std::string& key) const {
        const std::string& val = header(key);
        if (val == http_base::NO_VAL) {
            return NO_VAL;
        }
        return val;
    }

    /// \return The parameter map.
    inline const uwsgi_vars_t& vars() const { return headers(); }

    inline void print_header() const {
        std::cout << "header:"
                  << " modifier1=" << (int)m_header.modifier1 << " datasize=" << m_header.datasize
                  << " modifier2=" << (int)m_header.modifier2 << std::endl;
    }

    inline void print_vars() const {
        for (auto& kv : headers()) {
            std::cout << kv.first << " = " << kv.second << std::endl;
        }
    }

    /// Read request data from a socket.
    void read_data(socket& sock);

    /// \return The uwsgi header struct.
    inline uwsgi_packet_header& uwsgi_header() { return m_header; }

    /// Reset an object.
    inline void clear() {
        http_base::clear();
        m_header = {0, 0, 0};
    }

  private:
    uwsgi_packet_header m_header;
    std::string m_host = http_base::NO_VAL;
    std::string m_path = http_base::NO_VAL;
    int m_port = -1;

    /// Read the header from a socket.
    void read_header(socket& sock);

    /// Read the uwsgi parameters from a socket.
    void read_vars(socket& sock);

    /// Read POST data into the content buffer.
    void read_content(socket& sock);

    /// Parse the uswgi parameters into a hash map.
    void parse_vars();

    /// \copydoc http_base::prepare_data_buffer()
    void prepare_data_buffer();
};

}  // net
}  // tasks

#endif  // _UWSGI_REQUEST_H_
