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

class http_request : public http_base {
  public:
    http_request(std::string host, std::string url = "", int port = 80) : m_host(host), m_url(url), m_port(port) {
        m_headers["Host"] = m_host;
        init_default_headers();
    }

    inline void set_host(std::string host) {
        m_host = host;
    }

    inline void set_url(std::string url) {
        m_url = url;
    }

    inline int port() const { return m_port; }

    void prepare_data_buffer();

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
