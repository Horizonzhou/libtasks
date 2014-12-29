/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#include <tasks/serial/term.h>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

namespace tasks {
namespace serial {

struct termios term::options() {
    if (m_fd < 0) {
        throw term_exception("no open device");
    }
    struct termios opts;
    if (tcgetattr(m_fd, &opts)) {
        close();
        throw term_exception("tcgetattr failed: " + std::string(std::strerror(errno)));
    }
    return opts;
}

void term::set_options(struct termios& opts) {
    if (m_fd < 0) {
        throw term_exception("no open device");
    }
    if (tcsetattr(m_fd, TCSANOW, &opts)) {
        close();
        throw term_exception("tcsetattr failed: " + std::string(std::strerror(errno)));
    }
}

void term::open(std::string port, speed_t baudrate, charsize_t charsize, parity_t parity, stopbits_t stopbits) {
    m_fd = ::open(port.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (m_fd < 0) {
        throw term_exception("opening " + port + std::string(" failed: ") + std::string(std::strerror(errno)));
    }

    // ensure non blocking reads
    if (fcntl(m_fd, F_SETFL, FNDELAY)) {
        close();
        throw term_exception("fcntl failed: " + std::string(std::strerror(errno)));
    }

    // read the current settings
    struct termios opts = options();

    // don't take ownership of the device and enable recieving of data
    opts.c_cflag |= (CLOCAL | CREAD);
    // use raw input/output as a default
    opts.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    opts.c_oflag &= ~OPOST;

    // baudrate
    if (cfsetispeed(&opts, baudrate)) {
        close();
        throw term_exception("cfsetispeed failed: " + std::string(std::strerror(errno)));
    }
    if (cfsetospeed(&opts, baudrate)) {
        close();
        throw term_exception("cfsetospeed failed: " + std::string(std::strerror(errno)));
    }

    // charachter size
    opts.c_cflag &= ~CSIZE;
    opts.c_cflag |= static_cast<tcflag_t>(charsize);

    // parity
    switch (parity) {
        case parity_t::NONE:
            opts.c_cflag &= ~PARENB;
            break;
        case parity_t::EVEN:
            opts.c_cflag |= PARENB;
            opts.c_cflag &= ~PARODD;
            break;
        case parity_t::ODD:
            opts.c_cflag |= PARENB;
            opts.c_cflag |= PARODD;
            break;
        default:
            close();
            throw term_exception("invalid parity parameter");
    }

    // stopbits
    switch (stopbits) {
        case stopbits_t::_1:
            opts.c_cflag &= ~CSTOPB;
            break;
        case stopbits_t::_2:
            opts.c_cflag |= CSTOPB;
        default:
            close();
            throw term_exception("invalid stopbits parameter");
    }

    set_options(opts);
}

void term::open(std::string port, speed_t baudrate, termmode_t mode) {
    switch (mode) {
        case termmode_t::_8N1:
            open(port, baudrate, charsize_t::_8, parity_t::NONE, stopbits_t::_1);
            break;
        case termmode_t::_7S1:
            open(port, baudrate, charsize_t::_7, parity_t::NONE, stopbits_t::_1);
            break;
        case termmode_t::_7E1:
            open(port, baudrate, charsize_t::_7, parity_t::EVEN, stopbits_t::_1);
            break;
        case termmode_t::_7O1:
            open(port, baudrate, charsize_t::_7, parity_t::ODD, stopbits_t::_1);
            break;
        default:
            throw term_exception("invalid mode parameter");
    }
}

void term::close() {
    if (m_fd > -1) {
        ::close(m_fd);
        m_fd = -1;
    }
}

std::streamsize term::write(const char* data, std::size_t len) {
    if (m_fd < 0) {
        throw term_exception("no open device");
    }
    return ::write(m_fd, data, len);
}

std::streamsize term::read(char* data, std::size_t len) {
    if (m_fd < 0) {
        throw term_exception("no open device");
    }
    return ::read(m_fd, data, len);
}

}  // serial
}  // tasks
