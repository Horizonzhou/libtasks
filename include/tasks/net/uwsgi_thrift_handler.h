/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#ifndef _UWSGI_THRIFT_HANDLER_H_
#define _UWSGI_THRIFT_HANDLER_H_

#include <tasks/net/uwsgi_task.h>
#include <string>
#include <functional>

namespace tasks {
namespace net {

template <class thrift_interface_type>
class uwsgi_thrift_handler : public thrift_interface_type {
  public:
    inline void set_uwsgi_task(uwsgi_task* t) { m_uwsgi_task = t; }

  protected:
    // Provide some wrappers to access the request and response structures
    // from a service call implementation.
    inline const uwsgi_request& request() const { return m_uwsgi_task->request(); }

    inline const uwsgi_request* request_p() const { return m_uwsgi_task->request_p(); }

    inline http_response& response() { return m_uwsgi_task->response(); }

    inline http_response* response_p() { return m_uwsgi_task->response_p(); }

  private:
    uwsgi_task* m_uwsgi_task = nullptr;
};

}  // net
}  // tasks

#endif  // _UWSGI_THRIFT_HANDLER_H_
