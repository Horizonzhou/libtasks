/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#include <vector>

#include <tasks/dispatcher.h>
#include <tasks/logging.h>
#include <tasks/ev_wrapper.h>
#include <tasks/net/acceptor.h>
#include <tasks/net/socket.h>

#include <echoserver.h>

#ifdef PROFILER
#include <google/profiler.h>
#endif

using namespace tasks;
using namespace tasks::net;

std::atomic<int> stats::m_req_count;
std::atomic<int> stats::m_clients;

bool echo_handler::handle_event(tasks::worker* worker, int events) {
    if (events & EV_READ) {
        try {
            std::vector<char> buf(1024);
            std::size_t bytes = socket().read(&buf[0], buf.size());
            tdbg("echo_handler: read " << bytes << " bytes" << std::endl);
            buf.resize(bytes);
            m_write_queue.push(std::move(buf));
        } catch (socket_exception& e) {
            terr("echo_handler::handle_event: " << e.what() << std::endl);
            return false;
        }
    }
    if (events & EV_WRITE) {
        if (!m_write_queue.empty()) {
            std::vector<char>& buf = m_write_queue.front();
            try {
                std::size_t len = buf.size() - m_write_offset;
                std::size_t bytes = socket().write(&buf[m_write_offset], len);
                tdbg("echo_handler: wrote " << bytes << " bytes" << std::endl);
                if (bytes == len) {
                    // buffer send completely
                    m_write_queue.pop();
                    m_write_offset = 0;
                    stats::inc_req();
                } else {
                    m_write_offset += bytes;
                }
            } catch (socket_exception& e) {
                terr("echo_handler::handle_event: " << e.what() << std::endl);
                return false;
            }
        }
    }
    if (m_write_queue.empty()) {
        set_events(EV_READ);
        update_watcher(worker);
    } else {
        set_events(EV_READ | EV_WRITE);
        update_watcher(worker);
    }
    return true;
}

bool stats::handle_event(tasks::worker* worker, int events) {
    std::time_t now = std::time(nullptr);
    std::time_t diff = now - m_last;
    int count;
    count = m_req_count.exchange(0, std::memory_order_relaxed);
    m_last = now;
    int qps = count / diff;
    std::cout << qps << " req/s, num of clients " << m_clients << std::endl;
    dispatcher::instance()->print_worker_stats();
    return true;
}

void handle_signal(int sig) {
    terr("Got signal " << sig << std::endl);
    dispatcher::instance()->terminate();
}

int main(int argc, char** argv) {
#ifdef PROFILER
    ProfilerStart("echoserver.prof");
#endif
    stats s;
    acceptor<echo_handler> srv(12345);
    // dispatcher::init_workers(1);
    dispatcher::add_signal_handler(SIGINT, handle_signal);
    auto tasks = std::vector<task*>{&srv, &s};
    dispatcher::instance()->run(tasks);
#ifdef PROFILER
    ProfilerStop();
#endif
    return 0;
}
