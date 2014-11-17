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

#include <tasks/net/uwsgi_thrift_async_handler.h>

using namespace tasks;
using namespace tasks::net;

class ip_service_async1 : public uwsgi_thrift_async_handler<IpService_lookup_result, IpService_lookup_args> {
   public:
    void service(std::shared_ptr<args_t> args);
    std::string service_name() const { return "lookup"; }
};

class ip_service_async2 : public uwsgi_thrift_async_handler<IpService_lookup_result, IpService_lookup_args> {
   public:
    void service(std::shared_ptr<args_t> args);
    std::string service_name() const { return "lookup"; }
};

class test_uwsgi_thrift_async : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(test_uwsgi_thrift_async);
    CPPUNIT_TEST(request_finish_in_worker_ctx);
    CPPUNIT_TEST(request_finish_exec);
    CPPUNIT_TEST(request_finish_exec_timeout);
    CPPUNIT_TEST_SUITE_END();

   public:
    void setUp() {}
    void tearDown() {}

   protected:
    void request_finish_in_worker_ctx();
    void request_finish_exec();
    void request_finish_exec_timeout();
    void request(std::string url);

   private:
    std::unique_ptr<net_io_task> m_srv1;
    std::unique_ptr<net_io_task> m_srv2;
    std::unique_ptr<net_io_task> m_srv3;
};
