/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#ifndef _TASKS_EXECUTOR_H_
#define _TASKS_EXECUTOR_H_

#include <tasks/exec_task.h>
#include <tasks/logging.h>

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <memory>

namespace tasks {

class executor {
    friend class test_exec;

  public:
    executor();

    virtual ~executor() {
        terminate();
        m_thread->join();
        tdbg("terminated" << std::endl);
    }

    inline bool busy() const { return m_busy; }

    inline void set_busy() { m_busy = true; }

    inline void add_task(exec_task* t) {
        tdbg("add_task " << t << std::endl);
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_task = t;
        }
        m_cond.notify_one();
    }

    inline void terminate() {
        tdbg("terminating" << std::endl);
        m_term = true;
        m_cond.notify_one();
    }

    inline bool terminated() const { return m_term; }

    static void set_timeout(uint32_t timeout) { m_timeout = timeout; }

  private:
    std::atomic<bool> m_busy;
    std::atomic<bool> m_term;
    exec_task* m_task = nullptr;
    std::mutex m_mutex;
    std::condition_variable m_cond;
    static uint32_t m_timeout;
    std::unique_ptr<std::thread> m_thread;

    void run();
};

}  // tasks

#endif  // _TASKS_EXECUTOR_H_
