/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#include <cppunit/TestCase.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <atomic>

class test_disk_io_task : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(test_disk_io_task);
    CPPUNIT_TEST(write);
    CPPUNIT_TEST(read);
    CPPUNIT_TEST_SUITE_END();

   public:
    void setUp() {}
    void tearDown() {}

   protected:
    void write();
    void read();

   private:
    uint16_t m_total = 500;
};
