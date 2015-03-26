/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#include <tasks/logging.h>
#include <sys/socket.h>
#include <cstdlib>

#include <tasks/net/uwsgi_request.h>
#include <tasks/net/socket.h>

namespace tasks {
namespace net {

std::string uwsgi_request::NO_VAL;

void uwsgi_request::read_header(socket& sock) {
    std::streamsize bytes = sock.read((char*)&m_header, sizeof(m_header));
    if (bytes != sizeof(m_header)) {
        throw tasks_exception(tasks_error::UWSGI_HEADER_ERROR, "uwsgi_request: error reading header");
    }
    tdbg("uwsgi_request::read_header: read header successfully, " << bytes << " bytes" << std::endl);
}

void uwsgi_request::read_vars(socket& sock) {
    std::streamsize bytes = sock.read(m_data_buffer.ptr_write(), m_data_buffer.to_write());
    if (bytes > 0) {
        m_data_buffer.move_ptr_write(bytes);
        tdbg("uwsgi_request::read_vars: read data successfully, " << bytes << " bytes" << std::endl);
    }
    if (!m_data_buffer.to_write()) {
        if (UWSGI_VARS == m_header.modifier1) {
            parse_vars();
            // Check if a http body needs to be read
            std::string content_len_s = var("CONTENT_LENGTH");
            if (NO_VAL != content_len_s) {
                std::size_t content_len_i = std::atoi(content_len_s.c_str());
                m_content_buffer.set_size(content_len_i);
                m_state = io_state::READ_CONTENT;
            } else {
                m_state = io_state::DONE;
            }
        }
    }
}

void uwsgi_request::read_content(socket& sock) {
    std::streamsize bytes = sock.read(m_content_buffer.ptr_write(), m_content_buffer.to_write());
    if (bytes > 0) {
        m_content_buffer.move_ptr_write(bytes);
        tdbg("uwsgi_request::read_content: read data successfully, " << bytes << " bytes" << std::endl);
    }
    if (!m_content_buffer.to_write()) {
        m_state = io_state::DONE;
    }
}

void uwsgi_request::read_data(socket& sock) {
    if (io_state::READY == m_state) {
        m_state = io_state::READ_HEADER;
        read_header(sock);
        m_state = io_state::READ_DATA;
        m_data_buffer.set_size(m_header.datasize);
    }
    if (io_state::READ_DATA == m_state) {
        read_vars(sock);
    }
    if (io_state::READ_CONTENT == m_state) {
        read_content(sock);
    }
}

void uwsgi_request::parse_vars() {
    std::size_t pos = 0;
    while (pos < m_data_buffer.size()) {
        uint16_t key_len = *((uint16_t*)m_data_buffer.ptr(pos));
        uint16_t key_start = pos + 2;
        uint16_t val_len = *((uint16_t*)m_data_buffer.ptr(key_start + key_len));
        uint16_t val_start = key_start + key_len + 2;
        if (key_len && val_len) {
            std::string key(m_data_buffer.ptr(key_start), key_len);
            std::string val(m_data_buffer.ptr(val_start), val_len);
            set_header(std::ref(key), std::ref(val));
        }
        pos = val_start + val_len;
    }
}

void uwsgi_request::prepare_data_buffer() {
    // Set the content length
    if (m_content_buffer.size()) {
        set_header("CONTENT_LENGTH", std::to_string(m_content_buffer.size()));
    }
    // We will add the header later, as we don't know the size for the variables yet. We just keep room for it.
    m_data_buffer.set_size(sizeof(m_header));
    m_data_buffer.move_ptr_write(sizeof(m_header));
    // Put all variables in.
    for (auto& kv : headers()) {
        uint16_t len = kv.first.length();
        m_data_buffer.write((const char*)&len, sizeof(uint16_t));
        m_data_buffer.write(kv.first.c_str(), len);
        len = kv.second.length();
        m_data_buffer.write((const char*)&len, sizeof(uint16_t));
        m_data_buffer.write(kv.second.c_str(), len);
    }
    // We know the size of the variables block now.
    auto owrite = m_data_buffer.offset_write();
    m_header.datasize = owrite;// - sizeof(m_header);
    // The header is ready now. First we have to jump back to the start.
    m_data_buffer.move_ptr_write_abs(0);
    // Now we can write the header.
    m_data_buffer.write((const char*)&m_header, sizeof(m_header));
    // Restore the buffers write pointer.
    m_data_buffer.move_ptr_write_abs(owrite);
}

}  // net
}  // tasks
