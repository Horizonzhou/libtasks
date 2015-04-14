/*
 * Copyright (c) 2013-2015 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#include <tasks/dispatcher.h>
#include <tasks/timer_task.h>
#include <atomic>

#include "test_timer_task.h"

using namespace std;

atomic<int> g_counter{0};

namespace tasks {

class timer_test : public timer_task {
  public:
    timer_test(double after, double repeat, bool immediate) : timer_task(after, repeat, immediate) {}
    bool handle_event(worker*, int) {
        g_counter++;
        return g_counter < 3;
    }
};

void test_timer_task::run() {
    auto* t1 = new timer_test(0., 0.5, true);
    dispatcher::instance()->add_event_task(t1);
    CPPUNIT_ASSERT_MESSAGE("g_counter=" + to_string(g_counter), g_counter == 1);
    this_thread::sleep_for(2s);
    CPPUNIT_ASSERT_MESSAGE("g_counter=" + to_string(g_counter), g_counter == 3);
}

}
