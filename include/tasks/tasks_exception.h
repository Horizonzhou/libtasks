/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#ifndef _TASKS_TASKS_EXCEPTION_H_
#define _TASKS_TASKS_EXCEPTION_H_

#include <string>
#include <sstream>

namespace tasks {

enum class tasks_error {
    /// Error on socket sys call
    SOCKET_SOCKET,
    /// Error on setsockopt sys call
    SOCKET_SOCKOPT_NOSIGPIPE,
    /// Error on setsockopt sys call
    SOCKET_SOCKOPT_REUSEADDR,
    /// Error on fnctl sys call
    SOCKET_FNCTL,
    /// Error on bind sys call
    SOCKET_BIND,
    /// Error on chmod sys call (domain sockets)
    SOCKET_CHMOD,
    /// Error on listen sys call
    SOCKET_LISTEN,
    /// Error when calling listen on a UDP socket
    SOCKET_LISTEN_UDP,
    /// Error on accept sys call
    SOCKET_ACCEPT,
    /// Error on connect sys call
    SOCKET_CONNECT,
    /// Error when trying to connect to a host that can't be resolved
    SOCKET_NOHOST,
    /// Error on sendto sys call
    SOCKET_WRITE,
    /// Error on recvfrom sys call
    SOCKET_READ,
    /// Error when reading from a disconnected socket
    SOCKET_NOCON,
    /// Error on chunked encoding, that is not available yet
    HTTP_NOT_IMPL,
    /// HTTP Sender remote error, happens if no valid remote endpoint is given
    HTTP_SENDER_INVALID_REMOTE,
    /// HTTP errors
    HTTP_NO_CONTENT_LENGTH,
    HTTP_INVALID_STATUS_CODE,
    HTTP_INVALID_HEADER,
    /// UWSGI errors
    UWSGI_HEADER_ERROR,
    UWSGI_NOT_IMPL,
    UWSGI_THRIFT_TRANSPORT,
    UWSGI_THRIFT_HANDLER,
    /// Term errors
    TERM_NO_DEVICE,
    TERM_TCGETATTR,
    TERM_TCSETATTR,
    TERM_OPEN,
    TERM_FCNTL,
    TERM_CFSETOSPEED,
    TERM_CFSETISPEED,
    TERM_INVALID_PARITY,
    TERM_INVALID_STOPBITS,
    TERM_INVALID_MODE,
    /// Disk IO
    DISKIO_INVALID_EVENT,
    UNSET
};

/// Tasks execption class.
class tasks_exception : public std::exception {
  public:
    /// Default contructor.
    tasks_exception() {}

    /// Contruct an object by error_code, message and errno.
    tasks_exception(tasks_error error, const std::string& what, int sys_errno)
        : m_error(error), m_what(what), m_sys_errno(sys_errno) {}

    /// Construct an object by error_code and message.
    tasks_exception(tasks_error error, const std::string& what) : m_error(error), m_what(what) {}

    /// Return the error code.
    tasks_error error_code() const noexcept {
        return m_error;
    }

    /// Return the errno if available.
    int sys_errno() const noexcept {
        return m_sys_errno;
    }

    /// Return the error message.
    const std::string& message() const noexcept {
        return m_what;
    }

    /// Return the error message.
    const char* what() const noexcept {
        return m_what.c_str();
    }

    /// Reset the exception.
    void reset() {
        m_error = tasks_error::UNSET;
        m_what = "";
        m_sys_errno = 0;
    }

  private:
    tasks_error m_error = tasks_error::UNSET;
    std::string m_what;
    int m_sys_errno = 0;
};

// Keep old exception classes for backward compatibility
namespace net {
typedef tasks_exception socket_exception;
typedef tasks_exception http_exception;
}
namespace serial {
typedef tasks_exception term_exception;
}

}  // tasks

#endif  // _TASKS_TASKS_EXCEPTION_H_
