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

#include <iostream>
#include <memory>
#include <vector>

#include <tasks/dispatcher.h>
#include <tasks/net/http_sender.h>

#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/program_options.hpp>

namespace io = boost::iostreams;
namespace po = boost::program_options;

struct options final {
    bool use_compression;
    std::string host;
    std::string url_path;
    std::string extra_header;
    int32_t port;
};

class test_handler : public tasks::net::http_response_handler {
  public:
    bool handle_response(std::shared_ptr<tasks::net::http_response> response) {
        std::cout << "Got status " << response->status() << std::endl;
        std::cout << "Content length " << response->content_length() << std::endl;
        if (response->content_length()) {
                std::cout << "Content: " << std::endl << response->content_p() << std::endl;
        }
        return false;
    }

  private:
    std::string decompress(const char* compressed, size_t length) {
        //       std::vector<char> decompressed = std::vector<char>();
        std::string decompressed;
        std::vector<char> tmp(compressed, compressed + length);

        io::filtering_ostream os;
        os.push(io::gzip_decompressor());
        os.push(io::back_inserter(decompressed));
        io::write(os, &tmp[0], tmp.size());

        return decompressed;
    }
};

bool get_command_line_options(int argc, char** argv, options& opts) {
    po::options_description description("Allowed options");
    description.add_options()("help", "produce this help message")(
        "compression", po::value<bool>(&opts.use_compression)->default_value(false), "whether to use gzip or deflate")(
        "extra-header", po::value<std::string>(&opts.extra_header)->default_value(""), "an extra header to send")(
        "host", po::value<std::string>(&opts.host), "host to send request to")(
        "port", po::value<int32_t>(&opts.port)->default_value(80), "port to send request to")(
        "path", po::value<std::string>(&opts.url_path)->default_value("/"), "the main url path");
    po::variables_map vmap;
    po::store(po::parse_command_line(argc, argv, description), vmap);
    po::notify(vmap);
    if (vmap.count("host") == 0 || vmap.count("help")) {
        std::cout << description << std::endl;
        return false;
    }
    return true;
}

int main(int argc, char** argv) {
    options opts;
    if (!get_command_line_options(argc, argv, opts)) {
        return -1;
    }
    // initialize the dispatcher first
    auto disp = tasks::dispatcher::instance();
    disp->start();
    auto* sender = new tasks::net::http_sender<test_handler>();
    // after sending the request we terminate the dispatcher and exit
    sender->on_finish([disp] { disp->terminate(); });
    auto request = std::make_shared<tasks::net::http_request>(opts.host, opts.url_path, opts.port);
    auto found = opts.extra_header.find_first_of(":");
    if (found != std::string::npos) {
        auto header_name = opts.extra_header.substr(0, found);
        auto header_value = opts.extra_header.substr(found + 1);
        std::cout << "Setting custom header '" << header_name << "' to '" << header_value << "'" << std::endl;
        request->set_header(header_name, header_value);
    }
    if (opts.use_compression) {
        request->set_header("Accept-Encoding", "gzip");
    }

    std::cout << "sending" << std::endl;
    try {
        sender->send(request);
    } catch (tasks::tasks_exception& e) {
        std::cerr << "error: " << e.what() << std::endl;
        delete sender;
        // shutdown the dispatcher
        disp->terminate();
    }
    disp->join();
    return 0;
}
