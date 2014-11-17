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
#include <tasks/tools/buffer.h>

namespace tasks {
namespace net {

class uwsgi_exception : public tasks::tasks_exception {
  public:
    uwsgi_exception(std::string what) : tasks::tasks_exception(what) {}
};

class uwsgi_request {
  public:
    typedef std::unordered_map<std::string, std::string> uwsgi_vars_t;

    static std::string NO_VAL;

    uwsgi_request() { m_header = {0, 0, 0}; }

    inline const std::string& var(std::string key) const {
        auto it = m_vars.find(key);
        if (m_vars.end() != it) {
            return it->second;
        }
        return NO_VAL;
    }

    inline const uwsgi_vars_t& vars() const { return m_vars; }

    inline void print_header() const {
        std::cout << "header:"
                  << " modifier1=" << (int)m_header.modifier1 << " datasize=" << m_header.datasize
                  << " modifier2=" << (int)m_header.modifier2 << std::endl;
    }

    inline void print_vars() const {
        for (auto kv : m_vars) {
            std::cout << kv.first << " = " << kv.second << std::endl;
        }
    }

    inline void write(const char* data, std::size_t size) { m_content_buffer.write(data, size); }

    inline std::streamsize read(char* data, std::size_t size) { return m_content_buffer.read(data, size); }

    void read_data(socket& sock);

    inline bool done() const { return m_state == io_state::DONE; }

    inline uwsgi_packet_header& header() { return m_header; }

    inline void clear() {
        m_state = io_state::READY;
        m_header = {0, 0, 0};
        m_data_buffer.clear();
        m_content_buffer.clear();
        if (m_vars.size()) {
            m_vars.clear();
        }
    }

  private:
    uwsgi_packet_header m_header;
    tasks::tools::buffer m_data_buffer;
    tasks::tools::buffer m_content_buffer;
    io_state m_state = io_state::READY;
    uwsgi_vars_t m_vars;

    void read_header(socket& sock);
    void read_vars(socket& sock);
    void read_content(socket& sock);
    void parse_vars();
};

}  // net
}  // tasks

#endif  // _UWSGI_REQUEST_H_
