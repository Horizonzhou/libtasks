/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#ifndef _TASKS_EXEC_TASK_H_
#define _TASKS_EXEC_TASK_H_

#include <tasks/task.h>
#include <tasks/logging.h>
#include <functional>
#include <sstream>

namespace tasks {

class exec_task : public task {
  public:
    typedef std::function<void()> func_t;

    exec_task(func_t f) : m_func(f) {}
    virtual ~exec_task() {}

    inline std::string get_string() const {
        std::ostringstream os;
        os << "exec_task(" << this << ")";
        return os.str();
    }

    virtual void execute() {
        tdbg(get_string() << ": executing m_func" << std::endl);
        m_func();
    }

  private:
    func_t m_func;
};

}  // tasks

#endif  // _TASKS_EXEC_TASK_H_
