/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#include <arpa/inet.h>
#include <cstring>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sstream>
#include <sys/stat.h>
#include <sys/un.h>
#include <tasks/logging.h>
#include <tasks/net/socket.h>
#include <unistd.h>

namespace tasks {
namespace net {

socket::socket(socket_type type) : io_base(), m_type(type) {
    if (socket_type::UDP == m_type) {
        m_fd = ::socket(AF_INET, SOCK_DGRAM, 0);
        if (m_fd < 0) {
            throw tasks_exception(tasks_error::SOCKET_SOCKET, "socket failed: " + std::string(std::strerror(errno)),
                                  errno);
        }
    } else if (socket_type::TCP != m_type) {
        terr("socket: Invalid socket_type! Using TCP.");
        m_type = socket_type::TCP;
    }
}

void socket::listen(std::string path, int queue_size) {
    m_fd = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (m_fd < 0) {
        throw tasks_exception(tasks_error::SOCKET_SOCKET, "socket failed: " + std::string(std::strerror(errno)), errno);
    }
#ifndef _OS_LINUX_
    int on = 1;
    if (setsockopt(m_fd, SOL_SOCKET, SO_NOSIGPIPE, (char *)&on, sizeof(on))) {
        throw tasks_exception(tasks_error::SOCKET_SOCKOPT_NOSIGPIPE,
                              "setsockopt SO_NOSIGPIPE failed: " + std::string(std::strerror(errno)), errno);
    }
#endif
    if (!m_blocking) {
        if (fcntl(m_fd, F_SETFL, fcntl(m_fd, F_GETFL, 0) | O_NONBLOCK)) {
            throw tasks_exception(tasks_error::SOCKET_FNCTL, "fcntl failed: " + std::string(std::strerror(errno)),
                                  errno);
        }
    }
    struct sockaddr_un addr;
    bzero(&addr, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    std::strcpy(addr.sun_path, path.c_str());
    unlink(addr.sun_path);
#ifdef _OS_LINUX_
    if (::bind(m_fd, (struct sockaddr *)&addr, sizeof(addr.sun_family) + path.length())) {
        throw tasks_exception(tasks_error::SOCKET_BIND, "bind failed: " + std::string(std::strerror(errno)), errno);
    }
#else
    if (::bind(m_fd, (struct sockaddr *)&addr, SUN_LEN(&addr))) {
        throw tasks_exception(tasks_error::SOCKET_BIND, "bind failed: " + std::string(std::strerror(errno)), errno);
    }
#endif
    if (chmod(path.c_str(), S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)) {
        throw tasks_exception(tasks_error::SOCKET_CHMOD, "chmod failed: " + std::string(std::strerror(errno)), errno);
    }
    if (::listen(m_fd, queue_size) != 0) {
        throw tasks_exception(tasks_error::SOCKET_LISTEN, "listen failed: " + std::string(std::strerror(errno)), errno);
    }
}

void socket::listen(int port, std::string ip, int queue_size) {
    if (udp()) {
        throw tasks_exception(tasks_error::SOCKET_LISTEN_UDP, "listen failed: can't be called for UDP sockets");
    }
    bind(port, ip, false /* mark this object as tcp socket */);
    if (::listen(m_fd, queue_size)) {
        throw tasks_exception(tasks_error::SOCKET_LISTEN, "listen failed: " + std::string(std::strerror(errno)), errno);
    }
}

void socket::bind(int port, std::string ip) {
    bind(port, ip, true /* mark this object as udp socket */);
}

void socket::bind(int port, std::string ip, bool udp) {
    int on = 1;
    m_type = socket_type::UDP;
    m_fd = ::socket(AF_INET, udp ? SOCK_DGRAM : SOCK_STREAM, 0);
    if (m_fd < 0) {
        throw tasks_exception(tasks_error::SOCKET_SOCKET, "socket failed: " + std::string(std::strerror(errno)), errno);
    }
    if (setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on))) {
        throw tasks_exception(tasks_error::SOCKET_SOCKOPT_REUSEADDR,
                              "setsockopt SO_REUSEADDR failed: " + std::string(std::strerror(errno)), errno);
    }
#ifndef _OS_LINUX_
    if (setsockopt(m_fd, SOL_SOCKET, SO_NOSIGPIPE, (char *)&on, sizeof(on))) {
        throw tasks_exception(tasks_error::SOCKET_SOCKOPT_NOSIGPIPE,
                              "setsockopt SO_NOSIGPIPE failed: " + std::string(std::strerror(errno)), errno);
    }
#endif
    if (!m_blocking) {
        if (fcntl(m_fd, F_SETFL, fcntl(m_fd, F_GETFL, 0) | O_NONBLOCK)) {
            throw tasks_exception(tasks_error::SOCKET_FNCTL, "fcntl failed: " + std::string(std::strerror(errno)),
                                  errno);
        }
    }
    init_sockaddr(port, ip);
    if (::bind(m_fd, (struct sockaddr *)m_addr.get(), sizeof(*(m_addr.get())))) {
        throw tasks_exception(tasks_error::SOCKET_BIND, "bind failed: " + std::string(std::strerror(errno)), errno);
    }
}

void socket::init_sockaddr(int port, std::string ip) {
    if (nullptr == m_addr) {
        m_addr = std::make_shared<struct sockaddr_in>();
    }
    bzero(m_addr.get(), sizeof(struct sockaddr_in));
    m_addr->sin_family = AF_INET;
    if (ip.length()) {
        if (inet_pton(AF_INET, ip.c_str(), &(m_addr->sin_addr)) < 1) {
            terr("socket: Invalid ip " << ip << "! Binding to 0.0.0.0!" << std::endl);
            m_addr->sin_addr.s_addr = INADDR_ANY;
        }
    } else {
        m_addr->sin_addr.s_addr = INADDR_ANY;
    }
    m_addr->sin_port = htons(port);
}

socket socket::accept() {
    struct sockaddr_in addr;
    bzero(&addr, sizeof(struct sockaddr_in));
    socklen_t len = sizeof(addr);
    int client = ::accept(m_fd, (struct sockaddr *)&addr, &len);
    if (client < 0) {
        throw tasks_exception(tasks_error::SOCKET_ACCEPT, "accept failed: " + std::string(std::strerror(errno)), errno);
    }
    return socket(client);
}

void socket::connect(std::string path) {
    m_fd = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (m_fd < 0) {
        throw tasks_exception(tasks_error::SOCKET_SOCKET, "socket failed: " + std::string(std::strerror(errno)), errno);
    }
#ifndef _OS_LINUX_
    int on = 1;
    if (setsockopt(m_fd, SOL_SOCKET, SO_NOSIGPIPE, (char *)&on, sizeof(on))) {
        throw tasks_exception(tasks_error::SOCKET_SOCKOPT_NOSIGPIPE,
                              "setsockopt SO_NOSIGPIPE failed: " + std::string(std::strerror(errno)), errno);
    }
#endif
    struct sockaddr_un addr;
    bzero(&addr, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    std::strcpy(addr.sun_path, path.c_str());

#ifdef _OS_LINUX_
    if (::connect(m_fd, (struct sockaddr *)&addr, sizeof(addr.sun_family) + path.length())) {
        throw tasks_exception(tasks_error::SOCKET_CONNECT, "connect failed: " + std::string(std::strerror(errno)),
                              errno);
    }
#else
    addr.sun_len = SUN_LEN(&addr);
    if (::connect(m_fd, (struct sockaddr *)&addr, SUN_LEN(&addr))) {
        throw tasks_exception(tasks_error::SOCKET_CONNECT, "connect failed: " + std::string(std::strerror(errno)),
                              errno);
    }
#endif

    if (!m_blocking) {
        if (fcntl(m_fd, F_SETFL, fcntl(m_fd, F_GETFL, 0) | O_NONBLOCK)) {
            throw tasks_exception(tasks_error::SOCKET_FNCTL, "fcntl failed: " + std::string(std::strerror(errno)),
                                  errno);
        }
    }
}

void socket::connect(std::string host, int port) {
    struct hostent *remote = gethostbyname(host.c_str());
    if (nullptr == remote) {
        throw tasks_exception(tasks_error::SOCKET_NOHOST, "Host " + host + " not found");
    }
    m_fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (m_fd < 0) {
        throw tasks_exception(tasks_error::SOCKET_SOCKET, "socket failed: " + std::string(std::strerror(errno)), errno);
    }
#ifndef _OS_LINUX_
    int on = 1;
    if (setsockopt(m_fd, SOL_SOCKET, SO_NOSIGPIPE, (char *)&on, sizeof(on))) {
        throw tasks_exception(tasks_error::SOCKET_SOCKOPT_NOSIGPIPE,
                              "setsockopt SO_NOSIGPIPE failed: " + std::string(std::strerror(errno)), errno);
    }
#endif
    struct sockaddr_in addr;
    bzero(&addr, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    std::memcpy(&addr.sin_addr, remote->h_addr_list[0], remote->h_length);
    addr.sin_port = htons(port);
    if (::connect(m_fd, (struct sockaddr *)&addr, sizeof(addr))) {
        throw tasks_exception(tasks_error::SOCKET_CONNECT, "connect failed: " + std::string(std::strerror(errno)),
                              errno);
    }
    if (!m_blocking) {
        if (fcntl(m_fd, F_SETFL, fcntl(m_fd, F_GETFL, 0) | O_NONBLOCK)) {
            throw tasks_exception(tasks_error::SOCKET_FNCTL, "fcntl failed: " + std::string(std::strerror(errno)),
                                  errno);
        }
    }
}

void socket::shutdown() {
    if (m_fd > -1) {
        ::shutdown(m_fd, SHUT_RDWR);
        m_fd = -1;
    }
}

std::streamsize socket::write(const char *data, std::size_t len, int port, std::string ip) {
    if (m_fd == -1 && udp()) {
        m_fd = ::socket(AF_INET, SOCK_DGRAM, 0);
        if (m_fd < 0) {
            throw tasks_exception(tasks_error::SOCKET_SOCKET, "socket failed: " + std::string(std::strerror(errno)),
                                  errno);
        }
    }
    if (port > -1) {
        init_sockaddr(port, ip);
    }
    const sockaddr *addr = nullptr;
    socklen_t addr_len = 0;
    if (nullptr != m_addr) {
        addr = (const sockaddr *)m_addr.get();
        addr_len = sizeof(*addr);
    }
    ssize_t bytes = sendto(m_fd, data, len, SEND_RECV_FLAGS, addr, addr_len);
    if (bytes < 0 && errno != EAGAIN) {
        std::stringstream s;
        s << "error writing to client file descriptor " << m_fd << ": " << std::strerror(errno);
        throw tasks_exception(tasks_error::SOCKET_WRITE, s.str(), errno);
    }
    return bytes;
}

std::streamsize socket::read(char *data, std::size_t len) {
    sockaddr *addr = nullptr;
    socklen_t addr_len = 0;
    if (udp() && nullptr != m_addr) {
        addr = (sockaddr *)m_addr.get();
        addr_len = sizeof(*addr);
    }
    ssize_t bytes = recvfrom(m_fd, data, len, SEND_RECV_FLAGS, addr, &addr_len);
    if (bytes < 0 && errno != EAGAIN) {
        std::stringstream s;
        s << "error reading from client file descriptor " << m_fd << ": " << std::strerror(errno);
        throw tasks_exception(tasks_error::SOCKET_READ, s.str(), errno);
    } else if (bytes == 0) {
        std::stringstream s;
        s << "client " << m_fd << " disconnected";
        throw tasks_exception(tasks_error::SOCKET_NOCON, s.str());
    }
    return bytes;
}

}  // net
}  // tasks
