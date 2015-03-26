/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#include <tasks/logging.h>
#include <tasks/net/uwsgi_task.h>

#include <sstream>

namespace tasks {
namespace net {

bool uwsgi_task::handle_event(tasks::worker* /* worker */, int revents) {
    bool success = true;
    try {
        if (EV_READ & revents) {
            m_request.read_data(socket());
            if (m_request.done()) {
                if (UWSGI_VARS == m_request.uwsgi_header().modifier1) {
                    success = handle_request();
                    m_request.clear();
                } else {
                    // No suuport for anything else for now
                    std::ostringstream os;
                    os << "uwsgi_task: unsupported uwsgi packet: "
                       << "modifier1=" << (int)m_request.uwsgi_header().modifier1
                       << " datasize=" << m_request.uwsgi_header().datasize
                       << " modifier2=" << (int)m_request.uwsgi_header().modifier2;
                    tasks_exception e(tasks_error::UWSGI_NOT_IMPL, os.str());
                    set_exception(e);
                    success = false;
                }
            }
        } else if (EV_WRITE & revents) {
            m_response.write_data(socket());
            if (m_response.done()) {
                finish_request();
#if UWSGI_KEEPALIVE == 1
                set_events(EV_READ);
                update_watcher(worker);
#else
                success = false;
#endif
            }
        }
    } catch (tasks::tasks_exception& e) {
        tdbg("uwsgi_task(" << this << "): exception: " << e.what() << std::endl);
        set_exception(e);
        success = false;
    }
    return success;
}

}  // net
}  // tasks
