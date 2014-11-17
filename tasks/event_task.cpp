/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#include <tasks/event_task.h>
#include <tasks/dispatcher.h>

namespace tasks {

void event_task::assign_worker(worker* worker) {
    if (dispatcher::mode::MULTI_LOOP == dispatcher::run_mode()) {
        if (nullptr == m_worker) {
            m_worker = worker;
        }
        assert(worker == m_worker);
    }
}

}  // tasks
