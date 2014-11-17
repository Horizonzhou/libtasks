/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#include <tasks/executor.h>
#include <chrono>

namespace tasks {

// An executor dies after 1min idle time per default.
uint32_t executor::m_timeout = 60;

executor::executor() : m_busy(true), m_term(false) { m_thread.reset(new std::thread(&executor::run, this)); }

void executor::run() {
    tdbg("run: entered" << std::endl);
    while (!m_term) {
        std::unique_lock<std::mutex> lock(m_mutex);
        int idle_runs = 10 * m_timeout;  // 10 checks per second
        int run = 0;
        while (!m_cond.wait_for(lock, std::chrono::milliseconds(100), [this] { return nullptr != m_task || m_term; })) {
            if (++run >= idle_runs) {
                // idle timeout
                m_term = true;
            }
        }
        if (!m_term) {
            tdbg("executing task " << m_task << std::endl);
            m_task->execute();
            tdbg("done executing task " << m_task << std::endl);
            if (m_task->auto_delete()) {
                m_task->finish(nullptr);
            }
            m_task = nullptr;
            m_busy = false;
        }
    }
    tdbg("run: leaving" << std::endl);
}

}  // tasks
