/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#include <tasks/exec.h>
#include <tasks/exec_task.h>
#include <tasks/dispatcher.h>

namespace tasks {

void exec(exec_task::func_t f) { dispatcher::instance()->add_task(new exec_task(f)); }

void exec(exec_task::func_t f, task::finish_func_void_t ff) {
    exec_task* t = new exec_task(f);
    t->on_finish(ff);
    dispatcher::instance()->add_task(t);
}

}  // tasks
