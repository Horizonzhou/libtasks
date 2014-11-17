/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#include <tasks/net/http_request.h>

namespace tasks {
namespace net {

void http_request::init_default_headers() { m_headers["User-Agent"] = "libtasks"; }

void http_request::prepare_data_buffer() {
    assert(m_url.length() > 0);
    std::string ctlen;
    // GET/POST
    if (m_content_buffer.size()) {
        m_data_buffer.write("POST ", 5);
        ctlen = "Content-Length: " + std::to_string(m_content_buffer.size());
    } else {
        m_data_buffer.write("GET ", 4);
    }
    m_data_buffer.write(m_url.c_str(), m_url.length());
    m_data_buffer.write(" HTTP/1.1", 9);
    m_data_buffer.write(CRLF, CRLF_SIZE);
    // Headers
    for (auto kv : m_headers) {
        m_data_buffer.write(kv.first.c_str(), kv.first.length());
        m_data_buffer.write(": ", 2);
        m_data_buffer.write(kv.second.c_str(), kv.second.length());
        m_data_buffer.write(CRLF, CRLF_SIZE);
    }
    // Content length
    m_data_buffer.write(ctlen.c_str(), ctlen.length());
    // End
    m_data_buffer.write(CRLF, CRLF_SIZE);
    m_data_buffer.write(CRLF, CRLF_SIZE);
}

}  // net
}  // tasks
