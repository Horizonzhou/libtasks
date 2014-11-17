/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#ifndef _TASKS_EXEC_H_
#define _TASKS_EXEC_H_

#include <tasks/exec_task.h>

namespace tasks {

/*!
 * \brief Execute code in a separate executor thread.
 *
 * Execute code in a separate executor thread that does not block a
 * worker thread. This can be useful to implement async interfaces
 * using libraries that do not provide async interfaces.
 *
 * \param f The functor to execute.
 */
void exec(exec_task::func_t f);

/*!
 * \brief Execute code in a separate executor thread.
 *
 * Execute code in a separate executor thread that does not block a
 * worker thread. This can be useful to implement async interfaces
 * using libraries that do not provide async interfaces.
 *
 * \param f The functor to execute.
 * \param ff The finish functor to be executed when f has been
 *           executed.
 */
void exec(exec_task::func_t f, task::finish_func_void_t ff);
}

#endif  // _TASKS_EXEC_H_
