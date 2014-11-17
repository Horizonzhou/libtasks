/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#include <arpa/inet.h>
#include <csignal>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/THttpClient.h>
#include <boost/shared_ptr.hpp>

#include <tasks/net/acceptor.h>
#include <tasks/net/uwsgi_thrift_processor.h>
#include <tasks/logging.h>

#include "test_uwsgi_thrift.h"

void ip_service::lookup(response_type& result, const int32_t /* ipv4 */, const ipv6_type& /* ipv6 */) {
    key_value_type kv;
    id_name_type val;
    kv.key.id = 1;
    kv.key.name = "city";
    val.id = 123456;
    val.name = "Berlin";
    kv.values.push_back(val);
    result.key_values.push_back(kv);
    kv.values.clear();

    kv.key.id = 2;
    kv.key.name = "country";
    val.id = 3345677;
    val.name = "Germany";
    kv.values.push_back(val);
    result.key_values.push_back(kv);
}

void test_uwsgi_thrift::request() {
    using namespace tasks;
    using namespace tasks::net;

    m_srv.reset(new acceptor<uwsgi_thrift_processor<IpServiceProcessor, ip_service>>(12345));
    tasks::net_io_task::add_task(m_srv.get());

    using namespace apache::thrift::protocol;
    using namespace apache::thrift::transport;
    boost::shared_ptr<THttpClient> transport(new THttpClient("127.0.0.1", 18080, "/test1"));
    boost::shared_ptr<TBinaryProtocol> protocol(new TBinaryProtocol(transport));
    IpServiceClient client(protocol);

    bool success = true;
    try {
        transport->open();

        int32_t ip = 123456789;
        ipv6_type ipv6;
        response_type r;
        client.lookup(r, ip, ipv6);

        CPPUNIT_ASSERT(r.key_values.size() == 2);
        CPPUNIT_ASSERT(r.key_values[0].key.id == 1);
        CPPUNIT_ASSERT(r.key_values[0].key.name == "city");
        CPPUNIT_ASSERT(r.key_values[0].values.size() == 1);
        CPPUNIT_ASSERT(r.key_values[0].values[0].id == 123456);
        CPPUNIT_ASSERT(r.key_values[0].values[0].name == "Berlin");
        CPPUNIT_ASSERT(r.key_values[1].key.id == 2);
        CPPUNIT_ASSERT(r.key_values[1].key.name == "country");
        CPPUNIT_ASSERT(r.key_values[1].values.size() == 1);
        CPPUNIT_ASSERT(r.key_values[1].values[0].id == 3345677);
        CPPUNIT_ASSERT(r.key_values[1].values[0].name == "Germany");

        transport->close();
    } catch (TTransportException& e) {
        success = false;
    }
    CPPUNIT_ASSERT(success);
}
