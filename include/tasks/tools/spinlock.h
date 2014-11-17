/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#ifndef _SPINLOCK_H_
#define _SPINLOCK_H_

#include <atomic>

namespace tasks {
namespace tools {

class spinlock {
  public:
    spinlock() : m_lock(false) {}

    inline void lock() {
        while (m_lock.exchange(true)) {
        }
    }

    inline void unlock() { m_lock = false; }

  private:
    std::atomic<bool> m_lock;
};

}  // tools
}  // tasks

#endif  // _SPINLOCK_H_
