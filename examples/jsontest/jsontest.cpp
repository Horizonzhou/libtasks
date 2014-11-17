/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#include <iostream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <tasks/dispatcher.h>
#include <tasks/net/http_sender.h>

using namespace boost::property_tree;
using namespace boost::property_tree::json_parser;

class json_handler : public tasks::net::http_response_handler {
  public:
    bool handle_response(std::shared_ptr<tasks::net::http_response> response) {
        std::cout << "Got status " << response->status() << std::endl
                  << "Content Length: " << response->content_length() << std::endl << "Response:" << std::endl
                  << std::endl;
        if (response->content_length()) {
            try {
                ptree pt;
                read_json(response->content_istream(), pt);
                print_tree(pt);
            } catch (std::exception& e) {
                std::cerr << "error: " << e.what() << std::endl;
            }
        } else {
            std::cout << "empty response" << std::endl;
        }
        return false;
    }

    void print_tree(ptree& pt, std::string indent = "") {
        for (auto& v : pt) {
            std::cout << indent << v.first;
            if (v.second.empty()) {
                std::cout << " = " << v.second.data() << std::endl;
            } else {
                std::cout << std::endl;
                print_tree(v.second, indent + "  ");
            }
        }
    }
};

int main(int argc, char** argv) {
    // initialize the dispatcher first
    auto disp = tasks::dispatcher::instance();
    disp->start();
    auto* sender = new tasks::net::http_sender<json_handler>();
    // after sending the request we terminate the dispatcher and exit
    sender->on_finish([] { tasks::dispatcher::instance()->terminate(); });
    auto request = std::make_shared<tasks::net::http_request>("graph.facebook.com", "/search?q=test");
    try {
        sender->send(request);
    } catch (tasks::tasks_exception& e) {
        std::cerr << "error: " << e.what() << std::endl;
        delete sender;
        disp->terminate();
    }
    disp->join();
    return 0;
}
