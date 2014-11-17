/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#ifndef _TASKS_ERROR_BASE_H_
#define _TASKS_ERROR_BASE_H_

#include <string>

namespace tasks {

/*!
 * \brief A helper class for basic error reporting.
 */
class error_base {
  public:
    inline bool error() const { return m_error_code > 0; }

    inline uint16_t error_code() const { return m_error_code; }

    inline const std::string& error_message() const { return m_error_message; }

    inline void set_error(uint16_t code, std::string message) {
        m_error_code = code;
        m_error_message = message;
    }

    inline void set_error(std::string message) { set_error(1, message); }

  private:
    uint16_t m_error_code = 0;
    std::string m_error_message;
};

}  // tasks

#endif  // _TASKS_ERROR_BASE_H_
