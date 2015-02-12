/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#include <cppunit/TestCase.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <tasks/dispatcher.h>
#include <tasks/net_io_task.h>
#include <tasks/net/http_sender.h>

#include <mutex>
#include <condition_variable>

class test_handler : public tasks::net::http_response_handler {
  public:
    bool handle_response(std::shared_ptr<tasks::net::http_response> response);
};

class test_handler_keepalive : public tasks::net::http_response_handler {
  public:
    bool handle_response(std::shared_ptr<tasks::net::http_response> response);
};

class test_http_close_handler : public tasks::net_io_task {
  public:
    test_http_close_handler(tasks::net::socket& socket) : net_io_task(socket, EV_READ) {}
    bool handle_event(tasks::worker*, int) { return false; }
};

class test_http_sender : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(test_http_sender);
    CPPUNIT_TEST(requests);
    CPPUNIT_TEST(requests_keepalive);
    CPPUNIT_TEST(requests_close);
    CPPUNIT_TEST_SUITE_END();

   public:
    void setUp() {}
    void tearDown() {}

   protected:
    void requests();
    void requests_keepalive();
    void requests_close();
};
