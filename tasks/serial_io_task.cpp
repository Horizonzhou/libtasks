/*
 * Copyright (c) 2013-2015 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#include <tasks/serial_io_task.h>
#include <tasks/logging.h>

namespace tasks {

serial_io_task::serial_io_task(serial::term& term, int events) : serial_io_task(events) {
    m_term = term;
    if (-1 != m_term.fd()) {
        init_watcher();
    }
}

serial_io_task::~serial_io_task() {
    if (m_auto_close) {
        tdbg("serial_io_task: closing terminal" << std::endl);
        m_term.close();
    }
}

}  // tasks
