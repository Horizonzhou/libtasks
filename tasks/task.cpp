/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#include <tasks/task.h>
#include <tasks/dispatcher.h>

namespace tasks {

void task::finish(worker* worker) {
    for (auto f : m_finish_funcs) {
        f(worker);
    }
    dispatcher::instance()->remove_task(this);
}

}  // tasks
