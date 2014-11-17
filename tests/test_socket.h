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

class echo_handler : public tasks::net_io_task {
   public:
    echo_handler(tasks::net::socket& socket) : net_io_task(socket, EV_READ) {}
    bool handle_event(tasks::worker* worker, int revents);

   private:
    std::queue<std::vector<char> > m_write_queue;
    ssize_t m_write_offset = 0;
};

class test_socket : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(test_socket);
    CPPUNIT_TEST(tcp);
    CPPUNIT_TEST(udp);
    CPPUNIT_TEST(unix);
    CPPUNIT_TEST_SUITE_END();

   public:
    void setUp() {}
    void tearDown() {}

   protected:
    void tcp();
    void udp();
    void unix();
};
