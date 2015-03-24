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
    serial_io_task(serial::term& term, int events);
    virtual ~serial_io_task();

    /// Provide access to the underlying term object.
    inline serial::term& term() { return m_term; }
    /// Provide const access to the underlying term object.
    inline const serial::term& term() const { return m_term; }
    /// Set the term object.
    inline void set_term(serial::term& term) { m_term = term; }

  protected:
    /// Grant socket access to the io_task_base.
    io_base& iob() { return m_term; }
    /// Grant const socket access to the io_task_base.
    const io_base& iob() const { return m_term; }
    /// Disable automatic closing of the term object in the desctructor.
    void disable_auto_close() {
        m_auto_close = false;
    }

  private:
    serial::term m_term;
    bool m_auto_close = true;
};

}  // tasks

#endif  // _TASKS_SERIAL_IO_TASK_H_
