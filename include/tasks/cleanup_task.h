/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#ifndef _TASKS_CLEANUP_TASK_H_
#define _TASKS_CLEANUP_TASK_H_

#include <tasks/timer_task.h>
#include <tasks/disposable.h>
#include <tasks/dispatcher.h>
#include <tasks/logging.h>

namespace tasks {

class cleanup_task : public timer_task {
  public:
    cleanup_task(disposable* d) : timer_task(1., 1.), m_disposable(d) {}

    bool handle_event(worker* worker, int) {
        if (m_disposable->can_dispose()) {
            tdbg("cleanup_task(" << this << "): disposing " << m_disposable << std::endl);
            m_disposable->dispose(worker);
            return false;  // done
        } else {
            return true;  // retry
        }
    }

  private:
    disposable* m_disposable;
};

}  // tasks

#endif  // _TASKS_CLEANUP_TASK_H_
