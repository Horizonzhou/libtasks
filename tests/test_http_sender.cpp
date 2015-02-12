/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#include "test_http_sender.h"

#include <iostream>
#include <tasks/net/acceptor.h>

int g_status_code = 0;
std::string g_content_type;
std::atomic<bool> g_done(false);

bool test_handler::handle_response(std::shared_ptr<tasks::net::http_response> response) {
    g_status_code = response->status_code();
    g_content_type = response->header("Content-Type");
    return false;
}

bool test_handler_keepalive::handle_response(std::shared_ptr<tasks::net::http_response> response) {
    g_status_code = response->status_code();
    g_content_type = response->header("Content-Type");
    g_done = true;
    return true;
}

void test_http_sender::requests() {
    auto* sender = new tasks::net::http_sender<test_handler>();

    // Notify us when the tasks is finished
    sender->on_finish([] {
        g_done = true;
    });

    // Connect to remote
    bool send_ok = true;
    std::string error;
    try {
        sender->send(std::make_shared<tasks::net::http_request>("localhost", "/", 18080));
    } catch (tasks::net::socket_exception& e) {
        send_ok = false;
        error = e.what();
    }
    CPPUNIT_ASSERT_MESSAGE(std::string("send error: ") + error, send_ok);

    // wait for the response
    while (!g_done) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // Check returned data
    CPPUNIT_ASSERT_MESSAGE(std::string("g_status_code = ") + std::to_string(g_status_code), g_status_code == 404);
    CPPUNIT_ASSERT_MESSAGE(std::string("g_content_type = ") + g_content_type, g_content_type == "text/html");
}

void test_http_sender::requests_keepalive() {
    // Second run
    auto* sender = new tasks::net::http_sender<test_handler_keepalive>();
    auto request = std::make_shared<tasks::net::http_request>("localhost", "/", 18080);

    g_status_code = 0;
    g_content_type = "";
    g_done = false;
    bool send_ok = true;
    std::string error = "";
    try {
        sender->send(request);
    } catch (tasks::net::socket_exception& e) {
        send_ok = false;
        error = e.message();
    }
    CPPUNIT_ASSERT_MESSAGE(std::string("send error: ") + error, send_ok);

    while (!g_done) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    CPPUNIT_ASSERT_MESSAGE(std::string("g_status_code = ") + std::to_string(g_status_code), g_status_code == 404);
    CPPUNIT_ASSERT_MESSAGE(std::string("g_content_type = ") + g_content_type, g_content_type == "text/html");

    // 3rd run, same sender, same request
    g_status_code = 0;
    g_content_type = "";
    g_done = false;
    send_ok = true;
    error = "";
    try {
        sender->send(request);
    } catch (tasks::net::socket_exception& e) {
        send_ok = false;
        error = e.message();
    }
    CPPUNIT_ASSERT_MESSAGE(std::string("send error: ") + error, send_ok);

    while (!g_done) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    CPPUNIT_ASSERT_MESSAGE(std::string("g_status_code = ") + std::to_string(g_status_code), g_status_code == 404);
    CPPUNIT_ASSERT_MESSAGE(std::string("g_content_type = ") + g_content_type, g_content_type == "text/html");

    // the sender will not get deleted due to keep alive, so we do that now.
    sender->finish();
}

void test_http_sender::requests_close() {
    // create acceptor that is just closing the socket
    auto srv = new tasks::net::acceptor<test_http_close_handler>(18081);
    tasks::dispatcher::instance()->add_event_task(srv);

    auto* sender = new tasks::net::http_sender<test_handler>();

    sender->on_finish([sender] {
        CPPUNIT_ASSERT_MESSAGE(std::string("error: ") + sender->error_message(),
                               sender->error_code() == tasks::tasks_error::SOCKET_NOCON ||
                                   sender->error_code() == tasks::tasks_error::SOCKET_WRITE ||
                                   sender->error_code() == tasks::tasks_error::SOCKET_READ);
        g_done = true;
    });

    tasks::tasks_error error = tasks::tasks_error::UNSET;
    std::string error_str = "";
    g_done = false;
    try {
        sender->send(std::make_shared<tasks::net::http_request>("localhost", "/", 18081));
    } catch (tasks::tasks_exception& e) {
        error = e.error_code();
        error_str = e.message();
    }
    CPPUNIT_ASSERT_MESSAGE(std::string("send error: ") + error_str, error == tasks::tasks_error::UNSET);

    while (!g_done) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    tasks::dispatcher::instance()->remove_event_task(srv);
}






