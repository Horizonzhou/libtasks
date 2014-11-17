/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#ifndef _THRIFT_SERVER_WRITER_H_
#define _THRIFT_SERVER_WRITER_H_

#include <string>
#include <boost/shared_ptr.hpp>
#include <thrift/protocol/TProtocol.h>

namespace tasks {
namespace tools {

template <class data_type, class transport_type, class protocol_type>
class thrift_server_writer {
  public:
    thrift_server_writer(std::string name, boost::shared_ptr<transport_type> t, boost::shared_ptr<protocol_type> p)
        : m_name(name), m_transport(t), m_protocol(p) {}

    inline void write(data_type& d) {
        using namespace apache::thrift::protocol;
        m_protocol->writeMessageBegin(m_name, T_REPLY, 0);
        m_protocol->writeStructBegin("result");
        m_protocol->writeFieldBegin("success", T_STRUCT, 0);
        d.write(m_protocol.get());
        m_protocol->writeFieldEnd();
        m_protocol->writeFieldStop();
        m_protocol->writeStructEnd();
        m_protocol->writeMessageEnd();
        m_transport->writeEnd();
        m_transport->flush();
    }

  private:
    std::string m_name;
    boost::shared_ptr<transport_type> m_transport;
    boost::shared_ptr<protocol_type> m_protocol;
};

}  // tools
}  // tasks

#endif  // _THRIFT_SERVER_WRITER_H_
