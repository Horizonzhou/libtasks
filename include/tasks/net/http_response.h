/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#ifndef _HTTP_RESPONSE_H_
#define _HTTP_RESPONSE_H_

#define READ_BUFFER_SIZE_BLOCK 4096

#include <tasks/net/http_base.h>

namespace tasks {
namespace net {

/// The HTTP response implementation.
class http_response : public http_base {
  public:
    http_response() {}

    inline void set_status(std::string status) {
        m_status = status;
        m_status_code = std::atoi(status.c_str());
    }

    /// \return The HTTP status string.
    inline const std::string& status() const { return m_status; }

    /// \return The HTTP status code.
    inline int status_code() const { return m_status_code; }

    /// \copydoc http_base::prepare_data_buffer()
    void prepare_data_buffer();

    /// Read an HTTP response from a socket.
    void read_data(net::socket& sock);

    /// \copydoc http_base::clear()
    void clear() {
        http_base::clear();
        m_status = "";
        m_line_number = 0;
        m_last_line_start = 0;
        m_content_start = 0;
        m_content_length_exists = false;
        m_chunked_enc = false;
    }

  private:
    std::string m_status;
    int m_status_code;
    int m_line_number = 0;
    std::size_t m_last_line_start = 0;
    std::size_t m_content_start = 0;
    bool m_content_length_exists = false;
    bool m_chunked_enc = false;

    void parse_data();
    void parse_line();
    void parse_status();
    void parse_header();
};

}  // net
}  // tasks

#endif  // _HTTP_RESPONSE_H_
