/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#ifndef _QUEUE_H_
#define _QUEUE_H_

#include <tasks/tools/spinlock.h>
#include <mutex>

namespace tasks {
namespace tools {

#ifndef CACHE_LINE_SIZE
#define CACHE_LINE_SIZE 64
#endif

/*
 * A thread safe queue
 *
 * Thx Herb Sutter for this implementation!
 */
template <typename T>
class queue {
  public:
    queue() : m_first(new node(T())), m_last(m_first) {}
    ~queue() {
        while (nullptr != m_first) {
            node* n = m_first;
            m_first = m_first->next;
            delete n;
        }
    }

    bool pop(T& res) {
        // No scope lock here to release the lock before deleting the
        // old node
        m_pop_lock.lock();
        if (nullptr != m_first->next) {
            node* old = m_first;
            m_first = m_first->next;
            // for big data types we should optimize here and move the real copy
            // out of the critical section
            res = m_first->val;
            m_pop_lock.unlock();
            delete old;
            return true;
        }
        m_pop_lock.unlock();
        return false;
    }

    bool push(const T& v) {
        node* n = new node(v);
        std::lock_guard<spinlock> lock(m_push_lock);
        m_last->next = n;
        m_last = n;
        return true;
    }

  private:
    struct node {
        node(T v) : val(v), next(nullptr) {}
        T val;
        std::atomic<node*> next;
        char pad[CACHE_LINE_SIZE - sizeof(T) - sizeof(std::atomic<node*>)];
    };

    char pad0[CACHE_LINE_SIZE];
    node* m_first;
    char pad1[CACHE_LINE_SIZE];
    spinlock m_pop_lock;
    char pad2[CACHE_LINE_SIZE];
    node* m_last;
    char pad3[CACHE_LINE_SIZE];
    spinlock m_push_lock;
    char pad4[CACHE_LINE_SIZE];
};

}  // tools
}  // tasks

#endif  // _QUEUE_H_
