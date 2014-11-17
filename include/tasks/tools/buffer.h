/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#ifndef _BUFFER_H_
#define _BUFFER_H_

#include <vector>
#include <cstring>
#include <cassert>
#include <streambuf>

#include <tasks/logging.h>

namespace tasks {
namespace tools {

class buffer : public std::streambuf {
  public:
    buffer() {
        setg(ptr_begin(), ptr_begin(), ptr_end());
        setp(ptr_begin(), ptr_end());
    }

    buffer(std::size_t size) : buffer() { set_size(size); }

    inline char* ptr_write() { return pptr(); }

    inline char* ptr_read() { return gptr(); }

    inline const char* ptr_read() const { return gptr(); }

    inline char* ptr(std::size_t pos) {
        assert(pos <= m_size);
        return &m_buffer[pos];
    }

    inline const char* ptr(std::size_t pos) const {
        assert(pos <= m_size);
        return &m_buffer[pos];
    }

    inline char* ptr_begin() { return &m_buffer[0]; }

    inline const char* ptr_begin() const { return &m_buffer[0]; }

    inline char* ptr_end() { return &m_buffer[m_size]; }

    inline const char* ptr_end() const { return &m_buffer[m_size]; }

    inline std::size_t offset_write() const { return pptr() - ptr_begin(); }

    inline std::size_t offset_read() const { return gptr() - ptr_begin(); }

    inline void move_ptr_write(std::size_t s) { setp(ptr_write() + s, ptr_end()); }

    inline void move_ptr_read(std::size_t s) { setg(ptr_begin(), ptr_read() + s, ptr_end()); }

    inline void move_ptr_write_abs(std::size_t pos) { setp(ptr(pos), ptr_end()); }

    inline void move_ptr_read_abs(std::size_t pos) { setg(ptr_begin(), ptr(pos), ptr_end()); }

    inline std::streamsize to_write() const { return ptr_end() - pptr(); }

    inline std::streamsize to_read() const { return ptr_end() - gptr(); }

    inline std::size_t size() const { return m_size; }

    inline void set_size(std::size_t s) {
        auto p_rd = ptr_read();
        auto p_wr = ptr_write();
        if (m_buffer.size() < s) {
            auto oread = offset_read();
            auto owrite = offset_write();
            m_buffer.resize(s + 1024);
            p_rd = ptr(oread);
            p_wr = ptr(owrite);
        }
        m_size = s;
        setg(ptr_begin(), p_rd, ptr_end());
        setp(p_wr, ptr_end());
    }

    inline void shrink() { m_buffer.resize(m_size); }

    inline std::size_t buffer_size() { return m_buffer.size(); }

    inline std::streamsize write(const char_type* data, std::streamsize size) { return xsputn(data, size); }

    inline std::streamsize read(char_type* data, std::streamsize size) { return xsgetn(data, size); }

    int_type sputc(char_type c) {
        set_size(m_size + 1);
        return std::streambuf::sputc(c);
    }

    int_type overflow(int_type ch) { return sputc(traits_type::to_char_type(ch)); }

    inline void clear() {
        m_size = 0;
        setg(ptr_begin(), ptr_begin(), ptr_end());
        setp(ptr_begin(), ptr_end());
    }

  protected:
    // std::streambuf override
    std::streamsize xsputn(const char_type* s, std::streamsize count) {
        set_size(m_size + count);
        return std::streambuf::xsputn(s, count);
    }

    // std::streambuf override
    std::streamsize xsgetn(char_type* s, std::streamsize count) {
        auto size = count;
        auto bytes_left = to_read();
        if (size > bytes_left) {
            size = bytes_left;
        }
        return std::streambuf::xsgetn(s, size);
    }

  private:
    std::vector<char> m_buffer;
    std::size_t m_size = 0;
};

}  // tools
}  // tasks

#endif  // _BUFFER_H_
