/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#ifndef _TASKS_SERIAL_IO_TASK_H_
#define _TASKS_SERIAL_IO_TASK_H_

#include <tasks/io_task_base.h>
#include <tasks/serial/term.h>

namespace tasks {

/*!
 * \brief The serial_io_task implements the base tasks that implement
 *   serial communication.
 */
class serial_io_task : public io_task_base {
  public:
    serial_io_task(int events) : io_task_base(events) {}
    serial_io_task(serial::term& term, int events) : serial_io_task(events) {
        m_term = term;
        if (-1 != m_term.fd()) {
            init_watcher();
        }
    }

    virtual ~serial_io_task() { m_term.close(); }

    /// Provide access to the underlying term object.
    inline serial::term& term() { return m_term; }
    /// Provide const access to the underlying term object.
    inline const serial::term& term() const { return m_term; }

  protected:
    /// Grant socket access to the io_task_base.
    io_base& iob() { return m_term; }
    /// Grant const socket access to the io_task_base.
    const io_base& iob() const { return m_term; }

  private:
    serial::term m_term;
};

}  // tasks

#endif  // _TASKS_SERIAL_IO_TASK_H_
