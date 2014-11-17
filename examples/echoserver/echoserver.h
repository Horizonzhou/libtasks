/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#ifndef _ECHO_SERVER_H_
#define _ECHO_SERVER_H_

#include <tasks/net_io_task.h>
#include <tasks/timer_task.h>
#include <tasks/worker.h>
#include <vector>
#include <queue>
#include <atomic>
#include <ctime>

class stats : public tasks::timer_task {
  public:
    stats() : timer_task(10., 10.) { m_last = std::time(nullptr); }

    bool handle_event(tasks::worker*, int revents);

    static void inc_req() { m_req_count++; }

    static void inc_clients() { m_clients++; }

    static void dec_clients() { m_clients--; }

  private:
    static std::atomic<int> m_req_count;
    static std::atomic<int> m_clients;
    std::time_t m_last;
};

class echo_handler : public tasks::net_io_task {
  public:
    echo_handler(tasks::net::socket& socket) : net_io_task(socket, EV_READ) { stats::inc_clients(); }

    ~echo_handler() { stats::dec_clients(); }

    bool handle_event(tasks::worker* worker, int revents);

  private:
    std::queue<std::vector<char> > m_write_queue;
    ssize_t m_write_offset = 0;
};

#endif  // _ECHO_SERVER_H_
