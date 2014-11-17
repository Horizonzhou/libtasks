/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#include <iostream>
#include <arpa/inet.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/THttpClient.h>
#include <boost/shared_ptr.hpp>
#include <IpService.h>

int main(int argc, char** argv) {
    using namespace apache::thrift::protocol;
    using namespace apache::thrift::transport;
    boost::shared_ptr<THttpClient> transport(new THttpClient("localhost", 8080, "/"));
    boost::shared_ptr<TBinaryProtocol> protocol(new TBinaryProtocol(transport));
    IpServiceClient client(protocol);

    try {
        transport->open();

        std::cout << "Sending request" << std::endl;
        int32_t ip = 123456789;
        ipv6_type ipv6;
        response_type r;
        client.lookup(r, ip, ipv6);

        for (auto& kv : r.key_values) {
            std::cout << "key.id=" << kv.key.id << " key.name=" << kv.key.name << std::endl;
            for (auto& val : kv.values) {
                std::cout << "  val.id=" << val.id << " val.name=" << val.name << std::endl;
            }
        }

        transport->close();
    } catch (TTransportException& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
