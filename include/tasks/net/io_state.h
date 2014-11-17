/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#ifndef _IO_STATE_H_
#define _IO_STATE_H_

namespace tasks {
namespace net {

enum class io_state : uint8_t { READY, DONE, READ_HEADER, READ_DATA, READ_CONTENT, WRITE_DATA, WRITE_CONTENT };

}  // net
}  // tasks

#endif  // _IO_STATE_H_
