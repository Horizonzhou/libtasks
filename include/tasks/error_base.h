/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#ifndef _TASKS_ERROR_BASE_H_
#define _TASKS_ERROR_BASE_H_

#include <string>
#include <cstring>
#include <tasks/tasks_exception.h>

namespace tasks {

/// A helper class for basic error reporting.
class error_base {
  public:
    error_base() {}
    virtual ~error_base() {}

    /// Return true if an error occured.
    inline bool error() const {
        return m_exception.error_code() != tasks_error::UNSET;
    }

    /// Return the error code.
    inline tasks_error error_code() const {
        return m_exception.error_code();
    }

    /// Return the error message.
    inline const std::string& error_message() const {
        return m_exception.message();
    }

    /// Return the errno if available.
    inline int sys_errno() const {
        return m_exception.sys_errno();
    }

    /// Return the errno description if available.
    inline std::string sys_errno_str() const {
        return std::strerror(sys_errno());
    }

    /// Return the underlying exception object.
    inline const tasks_exception& exception() const {
        return m_exception;
    }

    /// Set an exception to report an error.
    inline void set_exception(tasks_exception& e) {
        m_exception = e;
    }

    /// Reset the error state.
    inline void reset_error() {
        m_exception.reset();
    }

  private:
    tasks_exception m_exception;
};

}  // tasks

#endif  // _TASKS_ERROR_BASE_H_
