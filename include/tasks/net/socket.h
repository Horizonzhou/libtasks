/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#ifndef _TASKS_SOCKET_H_
#define _TASKS_SOCKET_H_

#include <string>
#include <exception>
#include <memory>
#include <cerrno>
#include <sys/types.h>
#include <sys/socket.h>

#include <tasks/io_base.h>
#include <tasks/tasks_exception.h>

#ifdef _OS_LINUX_
#define SEND_RECV_FLAGS MSG_NOSIGNAL
#else
#define SEND_RECV_FLAGS 0
#endif

struct sockaddr_in;

namespace tasks {
namespace net {

enum class socket_type { TCP, UDP };

/// The socket class.
class socket : public io_base {
  public:
    /// Constructor
    ///
    /// \param fd Use fd for the socket object.
    socket(int fd) : io_base(fd) {}

    /// Constructor
    ///
    /// \param socket_type The socket type. The default is socket_type::TCP for stream sockets (used for unix domain
    ///   sockets too). For UDP sockets use socket_type::UDP.
    socket(socket_type = socket_type::TCP);

    /// \return True if the socket type is socket_type::UDP.
    inline bool udp() const { return m_type == socket_type::UDP; }

    /// \return True if the socket type is socket_type::TCP.
    inline bool tcp() const { return m_type == socket_type::TCP; }

    /// \return The the socket type.
    inline socket_type type() const { return m_type; }

    /// \return A pointer to the sockaddr_in struct.
    inline std::shared_ptr<struct sockaddr_in> addr() { return m_addr; }

    /// Set the socket to blocking mode.
    inline void set_blocking() { m_blocking = true; }

    /// Bind for udp sockets. This method can be used to bind udp sockets. For tcp servers
    /// \link socket::listen(std::string path, int queue_size) \endlink or \link socket::listen(int port, std::string
    /// ip, int queue_size) \endlink has to be called.
    ///
    /// \param port The port.
    /// \param ip The ip address in dot notation (optional).
    void bind(int port, std::string ip = "");

    /// Listen for unix domain sockets.
    ///
    /// \param path The path to the socket file.
    /// \param queue_size The liste queue size. The default value is 128.
    void listen(std::string path, int queue_size = 128);

    // Listen for tcp sockets.
    ///
    /// \param port The port.
    /// \param ip The ip address in dot notation (optional).
    /// \param queue_size The liste queue size. The default value is 128.
    void listen(int port, std::string ip = "", int queue_size = 128);

    /// Accept new client connections.
    socket accept();

    /// Connect to a domain socket.
    ///
    /// \param path The path to the socket file.
    void connect(std::string path);

    /// Connect via tcp.
    ///
    /// \param host The hostname or ip address in dot notation.
    /// \param port The port.
    void connect(std::string host, int port);

    /// Call shutdown on the fd.
    void shutdown();

    /// Write data to the socket.
    ///
    /// \param data A pointer to the source data.
    /// \param len The number of bytes to write.
    /// \param port As UDP sockets are not connection based, the remote port has to be passed.
    /// \param ip Optional ip in dot notation for UDP writes.
    std::streamsize write(const char* data, std::size_t len, int port = -1, std::string ip = "");

    /// Read data from a socket.
    ///
    /// \param data A pointer to the destination data.
    /// \param len The number of bytes to write.
    std::streamsize read(char* data, std::size_t len);

  private:
    socket_type m_type = socket_type::TCP;
    bool m_blocking = false;
    std::shared_ptr<struct sockaddr_in> m_addr;

    void bind(int port, std::string ip, bool udp);
    void init_sockaddr(int port, std::string ip = "");
};

}  // net
}  // tasks

#endif  // _TASKS_SOCKET_H_
