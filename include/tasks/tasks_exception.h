/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#ifndef _TASKS_TASKS_EXCEPTION_H_
#define _TASKS_TASKS_EXCEPTION_H_

#include <string>

namespace tasks {

/*!
 * \brief The base exception class.
 */
class tasks_exception : public std::exception {
  public:
    tasks_exception(std::string what) : m_what(what) {}
    const char* what() const noexcept { return m_what.c_str(); }

  private:
    std::string m_what;
};

}  // tasks

#endif  // _TASKS_TASKS_EXCEPTION_H_
