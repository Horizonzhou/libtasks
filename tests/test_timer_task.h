/*
 * Copyright (c) 2013-2015 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#include <cppunit/TestCase.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

namespace tasks {

class test_timer_task : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(test_timer_task);
    CPPUNIT_TEST(run);
    CPPUNIT_TEST_SUITE_END();

   public:
    void setUp() {}
    void tearDown() {}

   protected:
    void run();
};

}
