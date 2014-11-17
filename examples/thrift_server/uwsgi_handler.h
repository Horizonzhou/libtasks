/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#ifndef _UWSGI_HANDLER_H_
#define _UWSGI_HANDLER_H_

#include <boost/shared_ptr.hpp>
#include <memory>

#include <tasks/net/uwsgi_task.h>

#include "stats.h"
#include "test_service.h"

class uwsgi_handler : public tasks::net::uwsgi_task {
  public:
    uwsgi_handler(tasks::net::socket& s) : uwsgi_task(s) { stats::inc_clients(); }

    ~uwsgi_handler() { stats::dec_clients(); }

    bool handle_request();
};

#endif  // _UWSGI_HANDLER_H_
