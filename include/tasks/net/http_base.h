/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#ifndef _HTTP_BASE_H_
#define _HTTP_BASE_H_

#include <unordered_map>
#include <iostream>
#include <istream>
#include <ostream>
#include <cstring>
#include <cstdlib>
#include <boost/algorithm/string/predicate.hpp>

#include <tasks/tasks_exception.h>
#include <tasks/net/socket.h>
#include <tasks/net/io_state.h>
#include <tasks/tools/buffer.h>

#define CRLF "\r\n"
#define CRLF_SIZE 2

namespace tasks {
namespace net {

class http_base {
  public:
    static const std::string NO_VAL;

    http_base() : m_content_istream(&m_content_buffer), m_content_ostream(&m_content_buffer) {}

    virtual ~http_base() {}

    inline void set_state(io_state state) { m_state = state; }

    inline io_state state() const { return m_state; }

    /// Set an HTTP header.
    ///
    /// \param header The header name.
    /// \param value The header value.
    inline void set_header(std::string header, std::string value) {
        m_headers[header] = value;
        if (boost::iequals(header, "Content-Length")) {
            m_content_length = std::atoi(value.c_str());
        }
    }

    /// \param name The header name.
    /// \return The header value.
    inline const std::string& header(std::string name) const {
        auto h = m_headers.find(name);
        if (m_headers.end() != h) {
            return h->second;
        }
        return NO_VAL;
    }

    /// \return The content length.
    inline std::size_t content_length() const { return m_content_length; }

    /// \return A pointer to the content buffer if it contains data or nullptr if it is empty.
    inline const char* content_p() const {
        if (m_content_length) {
            return m_content_buffer.ptr_read();
        }
        return nullptr;
    }

    /// Write a string to the content buffer.
    ///
    /// \param s The string to copy.
    inline std::size_t write(std::string s) { return m_content_buffer.write(s.c_str(), s.length()); }

    /// Write a string to the content buffer.
    ///
    /// \param s The string to copy.
    inline std::size_t write(const std::string& s) { return m_content_buffer.write(s.c_str(), s.length()); }

    /// Write data to the content buffer.
    ///
    /// \param data A pointer to the source data.
    /// \param size The number of bytes to copy.
    inline std::size_t write(const char* data, std::size_t size) { return m_content_buffer.write(data, size); }

    /// Copy data from the content buffer into a destination buffer.
    ///
    /// \param data A pointer to the destination.
    /// \param size The number of bytes to copy.
    inline std::size_t read(char* data, std::size_t size) { return m_content_buffer.read(data, size); }

    /// \return An std::istream to access the content buffer.
    inline std::istream& content_istream() { return m_content_istream; }

    /// \return An std::ostream to access the content buffer.
    inline std::ostream& content_ostream() { return m_content_ostream; }

    /// Prepare a HTTP request/response to be sent.
    virtual void prepare_data_buffer() = 0;

    /// Write a HTTP object to a socket.
    void write_data(socket& sock);

    inline void print() const {
        for (auto& kv : m_headers) {
            std::cout << kv.first << ": " << kv.second << std::endl;
        }
        std::cout << content_p();
    }

    /// \return True after an HTTP object has been read from a socket and parsed or written to a socket.
    inline bool done() const { return m_state == io_state::DONE; }

    /// Reset the http object.
    virtual void clear() {
        m_data_buffer.clear();
        m_content_buffer.clear();
        m_content_length = 0;
        if (m_headers.size() > 0) {
            m_headers.clear();
        }
        m_state = io_state::READY;
    }

  protected:
    tasks::tools::buffer m_data_buffer;
    tasks::tools::buffer m_content_buffer;
    io_state m_state = io_state::READY;
    std::unordered_map<std::string, std::string> m_headers;
    std::size_t m_content_length = 0;
    std::istream m_content_istream;
    std::ostream m_content_ostream;

    void write_headers(socket& sock);
    void write_content(socket& sock);
};

}  // net
}  // tasks

#endif  // _HTTP_BASE_H_
