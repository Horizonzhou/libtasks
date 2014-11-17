/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#ifndef _TASKS_DISPOSABLE_H_
#define _TASKS_DISPOSABLE_H_

#include <atomic>

namespace tasks {

/*!
 * \brief Base class for objects/tasks that can be deleted.
 *
 * We implement a simplistic garbage collector using timer tasks. If
 * a task with auto_delete enabled can't be deleted directly by a
 * worker thread, a dispose_task gets created that periodically tries
 * to delete the task.
 */
class disposable {
  public:
    disposable() : m_can_dispose(true) {}
    virtual ~disposable() {}

    /*!
     * \brief Check if a task can be disposed or not.
     *
     * \return True if the task can be disposed. False otherwise.
     */
    inline bool can_dispose() const { return m_can_dispose; }

    /*!
     * \brief Mark a task for being not disposable.
     */
    inline void disable_dispose() { m_can_dispose = false; }

    /*!
     * \brief Mark a task for being disposable.
     */
    inline void enable_dispose() { m_can_dispose = true; }

    /*! \brief Dispose an object.
     *
     * Dispose is called to delete a task. Instead of calling delete
     * directly we enable for hooking in here. This is required by
     * the net_io_task for example to stop the watcher.
     */
    virtual void dispose(worker*) = 0;

  private:
    std::atomic<bool> m_can_dispose;
};

}  // tasks

#endif  // _TASKS_DISPOSABLE_H_
