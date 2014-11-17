/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#include <cppunit/TestCase.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class test_bitset : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(test_bitset);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();

   public:
    void setUp() {}
    void tearDown() {}

   protected:
    void test();
};
