/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#ifndef _IO_BASE_H_
#define _IO_BASE_H_

#include <unistd.h>

/*!
 * \brief Base class for socket and term.
 */
class io_base {
  public:
    io_base() {}
    io_base(int fd) : m_fd(fd) {}
    virtual ~io_base() {}

    inline int fd() const { return m_fd; }

    virtual void close() {
        if (m_fd > -1) {
            ::close(m_fd);
            m_fd = -1;
        }
    }

  protected:
    int m_fd = -1;
};

#endif  // _IO_BASE_H_
