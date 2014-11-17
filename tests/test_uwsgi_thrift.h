/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#include <cppunit/TestCase.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <queue>
#include <vector>

#include <IpService.h>  // Thrift generated

#include <tasks/net/uwsgi_thrift_handler.h>

class ip_service : public tasks::net::uwsgi_thrift_handler<IpServiceIf> {
   public:
    void lookup(response_type& result, const int32_t ipv4, const ipv6_type& ipv6);
};

class test_uwsgi_thrift : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(test_uwsgi_thrift);
    CPPUNIT_TEST(request);
    CPPUNIT_TEST_SUITE_END();

   public:
    void setUp() {}
    void tearDown() {}

   protected:
    void request();

   private:
    std::unique_ptr<tasks::net_io_task> m_srv;
};
