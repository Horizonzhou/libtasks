/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#ifndef _UWSGI_STRUCTS_H_
#define _UWSGI_STRUCTS_H_

namespace tasks {
namespace net {

enum uwsgi_packet_type : uint8_t { UWSGI_VARS = 0 };

struct __attribute__((__packed__)) uwsgi_packet_header {
    uint8_t modifier1;
    uint16_t datasize;
    uint8_t modifier2;
};

}  // net
}  // tasks

#endif  // _UWSGI_STRUCTS_H_
