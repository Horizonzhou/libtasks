/*
 * Copyright (c) 2013-2014 Andreas Pohl <apohl79 at gmail.com>
 *
 * This file is part of libtasks.
 * 
 * libtasks is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * libtasks is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with libtasks.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _TASKS_SOCKET_H_
#define _TASKS_SOCKET_H_

#include <string>
#include <exception>
#include <memory>
#include <sys/types.h>
#include <sys/socket.h>

#ifdef __linux__
#define SENDTO_FLAGS MSG_NOSIGNAL
#define RECVFROM_FLAGS MSG_NOSIGNAL
#else
#define SENDTO_FLAGS 0
#define RECVFROM_FLAGS 0
#endif

struct sockaddr_in;

namespace tasks {
namespace net {

class socket_exception : public std::exception {
public:
    socket_exception(std::string what) : m_what(what) {}
    const char* what() const noexcept {
        return m_what.c_str();
    }
    
private:
    std::string m_what;
};

class socket {
public:
    socket(int fd = -1) : m_fd(fd) {}
    
    inline int fd() const {
        return m_fd;
    }

    inline bool udp() const {
        return m_udp;
    }

    inline bool tcp() const {
        return !m_udp;
    }

    inline std::shared_ptr<struct sockaddr_in> addr() {
        return m_addr;
    }

    inline void set_blocking() {
        m_blocking = true;
    }

    // bind for udp sockets. This method can be used to bind udp sockets. For tcp servers
    // socket::listen has to be called.
    void bind(int port, std::string ip = "") throw(socket_exception);

    // listen for unix domain sockets
    void listen(std::string path, int queue_size = 128) throw(socket_exception);
    
    // listen for tcp sockets
    void listen(int port, std::string ip = "", int queue_size = 128) throw(socket_exception);

    socket accept() throw(socket_exception);
    
    // connect a domain socket
    void connect(std::string path) throw(socket_exception);

    // connect via tcp
    void connect(std::string host, int port) throw(socket_exception);

    void shutdown();
    void close();
    
    std::streamsize write(const char* data, std::size_t len,
                          int port = -1, std::string ip = "") throw(socket_exception);
    std::streamsize read(char* data, std::size_t len) throw(socket_exception);

private:
    int m_fd = -1;
    bool m_udp = false;
    bool m_blocking = false;
    std::shared_ptr<struct sockaddr_in> m_addr;

    void bind(int port, std::string ip, bool udp) throw(socket_exception);
    void init_sockaddr(int port, std::string ip = "");

};

} // net
} // tasks

#endif // _TASKS_SOCKET_H_
