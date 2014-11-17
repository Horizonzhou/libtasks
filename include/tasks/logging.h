/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#ifndef _TASKS_LOGGING_H_
#define _TASKS_LOGGING_H_

#include <iostream>
#include <iomanip>
#include <thread>
#include <vector>

#ifdef LOGMUTEX
#include <mutex>
namespace tasks {
extern std::mutex g_log_mutex;
}
#define _LOGMUTEX std::lock_guard<std::mutex> _log_lock(tasks::g_log_mutex)
#else
#define _LOGMUTEX
#endif

#ifdef _WITH_PUT_TIME
#define ttime_init                      \
    std::time_t t = std::time(nullptr); \
    std::tm tm = *std::localtime(&t);
#define tput_time(t, f) std::put_time(t, f)
#else
#define ttime_init
#define tput_time(t, f) ""
#endif

#define tlog(s, m)                                                                                          \
    {                                                                                                       \
        ttime_init;                                                                                         \
        _LOGMUTEX;                                                                                          \
        s << "[" << tput_time(&tm, "%h %e %T") << " " << std::setw(14) << std::this_thread::get_id() << " " \
          << std::setw(16) << __FILE__ << ":" << std::setw(3) << std::setfill('0') << __LINE__ << "] "      \
          << std::setfill(' ') << m << std::flush;                                                          \
    }

#ifdef _DEBUG_OUTPUT
#define tdbg(m) tlog(std::clog, m)
#else
#define tdbg(m)
#endif

#define terr(m) tlog(std::clog, m)

#endif  // _TASKS_LOGGING_H_
