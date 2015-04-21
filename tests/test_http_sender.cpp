/*
 * Copyright (c) 2013-2014 Andreas Pohl <apohl79 at gmail.com>
 *
 * This file is part of libtasks.
 *
 * libtasks is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libtasks is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libtasks.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "test_http_sender.h"

#include <iostream>

const std::string nikola_tesla(
    "Nikola Tesla (10 July 1856 – 7 January 1943) was a Serbian inventor, electrical engineer, mechanical engineer, "
    "physicist, and futurist best known for his contributions to the design of the modern alternating current (AC) "
    "electricity supply system.");

bool test_http_sender::m_compressed = false;
int test_http_sender::m_status_code = 0;
std::string test_http_sender::m_content_type;
std::string test_http_sender::m_text;

bool test_handler::handle_response(std::shared_ptr<tasks::net::http_response> response) {
    test_http_sender::m_status_code = response->status_code();
    test_http_sender::m_content_type = response->header("Content-Type");
    test_http_sender::m_compressed = response->compressed();
    return false;
}

bool test_compressed_handler::handle_response(std::shared_ptr<tasks::net::http_response> response) {
    test_http_sender::m_status_code = response->status_code();
    test_http_sender::m_content_type = response->header("Content-Type");
    test_http_sender::m_compressed = response->compressed();
    test_http_sender::m_text = response->content_p();
    return false;
}

void test_http_sender::setUp() {
    m_status_code = 0;
    m_compressed = false;
    m_content_type = "";
    m_text = "";
}

void test_http_sender::requests() {
    auto* sender = new tasks::net::http_sender<test_handler>();
    std::atomic<bool> done(false);

    // Notify us when the task is finished.
    sender->on_finish([&] {
        done = true;
        m_cond.notify_one();
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

    // Wait for the response
    std::unique_lock<std::mutex> lock(m_mutex);
    m_cond.wait_for(lock, std::chrono::seconds(2), [&done] { return done.load(); });
    lock.unlock();

    // Check returned data
    CPPUNIT_ASSERT_MESSAGE(std::string("m_status_code = ") + std::to_string(m_status_code), m_status_code == 404);
    CPPUNIT_ASSERT_MESSAGE(std::string("m_content_type = ") + m_content_type, m_content_type == "text/html");
    CPPUNIT_ASSERT_MESSAGE(std::string("m_compressed = ") + std::to_string(m_compressed), !m_compressed);

    // Second run
    done = false;
    sender = new tasks::net::http_sender<test_handler>();
    sender->on_finish([this] { m_cond.notify_one(); });

    send_ok = true;
    error = "";
    try {
        sender->send(std::make_shared<tasks::net::http_request>("127.0.0.1", "/", 18080));
    } catch (tasks::net::socket_exception& e) {
        send_ok = false;
        error = e.what();
    }
    CPPUNIT_ASSERT_MESSAGE(std::string("send error: ") + error, send_ok);

    std::unique_lock<std::mutex> lock2(m_mutex);
    m_cond.wait_for(lock2, std::chrono::seconds(2), [&done] { return done.load(); });

    CPPUNIT_ASSERT_MESSAGE(std::string("m_status_code = ") + std::to_string(m_status_code), m_status_code == 404);
    CPPUNIT_ASSERT_MESSAGE(std::string("m_content_type = ") + m_content_type, m_content_type == "text/html");
    CPPUNIT_ASSERT_MESSAGE(std::string("m_compressed = ") + std::to_string(m_compressed), !m_compressed);
    lock2.unlock();
}

void test_http_sender::compressed_requests() {
    auto* sender = new tasks::net::http_sender<test_compressed_handler>();
    std::atomic<bool> done(false);

    // Notify us when the task is finished
    sender->on_finish([&] {
        done = true;
        m_cond.notify_one();
    });

    // Connect to remote
    bool send_ok = true;
    std::string error;
    auto request = std::make_shared<tasks::net::http_request>("localhost", "/nikola_tesla.txt", 28080);
    request->set_header("Accept-Encoding", "gzip");
    try {
        sender->send(request);
    } catch (tasks::net::socket_exception& e) {
        send_ok = false;
        error = e.what();
    }
    CPPUNIT_ASSERT_MESSAGE(std::string("send error: ") + error, send_ok);

    // Wait for the response
    std::unique_lock<std::mutex> lock(m_mutex);
    m_cond.wait_for(lock, std::chrono::seconds(2), [&done] { return done.load(); });
    lock.unlock();
    // Check returned data
    CPPUNIT_ASSERT_MESSAGE(std::string("m_status_code = ") + std::to_string(m_status_code), m_status_code == 200);
    CPPUNIT_ASSERT_MESSAGE(std::string("m_content_type = ") + m_content_type, m_content_type == "text/plain");
    CPPUNIT_ASSERT_MESSAGE(std::string("m_compressed = ") + std::to_string(m_compressed), m_compressed);
    CPPUNIT_ASSERT_MESSAGE(std::string("m_text = ") + m_text, m_text == nikola_tesla);

    // Second run
    done = false;
    sender = new tasks::net::http_sender<test_compressed_handler>();
    sender->on_finish([this] { m_cond.notify_one(); });

    send_ok = true;
    error = "";
    request = std::make_shared<tasks::net::http_request>("127.0.0.1", "/nikola_tesla.txt", 28080);
    request->set_header("Accept-Encoding", "gzip");
    try {
        sender->send(request);
    } catch (tasks::net::socket_exception& e) {
        send_ok = false;
        error = e.what();
    }
    CPPUNIT_ASSERT_MESSAGE(std::string("send error: ") + error, send_ok);

    std::unique_lock<std::mutex> lock2(m_mutex);
    m_cond.wait_for(lock2, std::chrono::seconds(2), [&done] { return done.load(); });

    CPPUNIT_ASSERT_MESSAGE(std::string("m_status_code = ") + std::to_string(m_status_code), m_status_code == 200);
    CPPUNIT_ASSERT_MESSAGE(std::string("m_content_type = ") + m_content_type, m_content_type == "text/plain");
    CPPUNIT_ASSERT_MESSAGE(std::string("m_compressed = ") + std::to_string(m_compressed), m_compressed);
    CPPUNIT_ASSERT_MESSAGE(std::string("m_text = ") + m_text, m_text == nikola_tesla);
}
