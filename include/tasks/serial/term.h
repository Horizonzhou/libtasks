/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#ifndef _TERM_H_
#define _TERM_H_

#include <string>
#include <exception>
#include <ios>
#include <termios.h>

#include <tasks/io_base.h>
#include <tasks/tasks_exception.h>

namespace tasks {
namespace serial {

enum class termmode_t { _8N1, _7S1, _7E1, _7O1 };
enum class charsize_t : tcflag_t { _5 = CS5, _6 = CS6, _7 = CS7, _8 = CS8 };
enum class parity_t { NONE, EVEN, ODD };
enum class stopbits_t { _1, _2 };

/*!
 * \brief A class that allows to read and write from/to terminal devices.
 *
 * This is a wrapper class around the POSIX terminal interface. Not all
 * capabilities have been implemented. You can use the options()
 * method to set unsupported options like flow control etc for example.
 *
 */
class term : public io_base {
  public:
    /// Contruct an object.
    term() {}

    /*!
     * \brief Contruct an object by providing a terminal file descriptor.
     *
     * \param fd The file descriptor.
     */
    term(int fd) : io_base(fd) {}

    /*!
     * \brief Open a terminal device.
     *
     * \param port The device port (eg: /dev/ttyUSB0)
     * \param baudrate The baudrate (default: 9600)
     * \param charsize The size of a character (default: chrarsize_t::CS8 for 8 bits)
     * \param parity The parity (default: parity_t::NONE)
     * \param stopbits The number of stopbits (default: stopbits_t::SB1)
     */
    void open(std::string port, speed_t baudrate = B9600, charsize_t charsize = charsize_t::_8,
              parity_t parity = parity_t::NONE, stopbits_t stopbits = stopbits_t::_1);

    /*!
     * \brief Open a terminal device.
     *
     * \param port The device port (eg: /dev/ttyUSB0)
     * \param baudrate The baudrate (default: 9600)
     * \param mode The mode the device should be opened with (default:
     *   termmode_t::_8N1 for 8N1 mode)
     */
    void open(std::string port, speed_t baudrate = B9600, termmode_t mode = termmode_t::_8N1);
    
    /// Access terminal options.
    struct termios options();
    /// Set terminal options.
    void set_options(struct termios& opts);
    ///Access the baudrate.
    speed_t baudrate() const;
    /// Set the baudrate.
    void baudrate(speed_t br);
    /// Close the terminal.
    void close();
    
    /*!
     * \brief Write data to the terminal.
     *
     * \param data A poiter to the data buffer.
     * \param len The number of bytes to write.
     * \return The number of bytes written to the terminal. 
     */
    std::streamsize write(const char* data, std::size_t len);

    /*!
     * \brief Read data from the terminal.
     *
     * \param data A poiter to the data buffer.
     * \param len The number of bytes to read.
     * \return The number of bytes read from the terminal.
     */
    std::streamsize read(char* data, std::size_t len);
};

}  // serial
}  // tasks

#endif  // _TERM_H_
