/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#ifndef _HTTP_REQUEST_H_
#define _HTTP_REQUEST_H_

#include <tasks/net/http_base.h>

namespace tasks {
namespace net {

/// The HTTP request implementation.
class http_request : public http_base {
  public:
    /// Constructor
    ///
    /// \param host The host
    /// \param url The URL
    /// \param port The port
    http_request(const std::string& host, std::string url = "", int port = 80)
        : m_host(host), m_url(url), m_port(port) {
        m_headers["Host"] = m_host;
        init_default_headers();
    }

    /// Set the host.
    inline void set_host(const std::string& host) { m_host = host; }

    /// Set the URL.
    inline void set_url(const std::string& url) { m_url = url; }

    /// Set the port.
    inline void set_port(int port) { m_port = port; }

    /// \return the host.
    inline const std::string& host() const { return m_host; }

    /// \return the URL.
    inline const std::string& url() const { return m_url; }

    /// \return the port.
    inline int port() const { return m_port; }

    /// \copydoc http_base::prepare_data_buffer()
    void prepare_data_buffer();

    /// \copydoc http_base::clear()
    void clear() {
        http_base::clear();
        init_default_headers();
        m_headers["Host"] = m_host;
    }

  private:
    std::string m_host;
    std::string m_url;
    int m_port;

    void init_default_headers();
};

}  // net
}  // tasks

#endif  // _HTTP_REQUEST_H_
