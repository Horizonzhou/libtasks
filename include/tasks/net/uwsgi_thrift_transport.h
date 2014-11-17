/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#ifndef _UWSGI_THRIFT_TRANSPORT_H_
#define _UWSGI_THRIFT_TRANSPORT_H_

#include <thrift/transport/TVirtualTransport.h>

namespace tasks {
namespace net {

template <class T>
class uwsgi_thrift_transport : public apache::thrift::transport::TVirtualTransport<uwsgi_thrift_transport<T> > {
  public:
    uwsgi_thrift_transport(T* r) : m_uwsgi_obj(r) {}
    ~uwsgi_thrift_transport() {}

    uint32_t read(uint8_t* data, int32_t size) { return m_uwsgi_obj->read((char*)data, size); }

    void write(const uint8_t* data, uint32_t size) { m_uwsgi_obj->write((const char*)data, size); }

    void flush() {}

  private:
    T* m_uwsgi_obj;
};

}  // net
}  // tasks

#endif  // _UWSGI_THRIFT_TRANSPORT_H_
