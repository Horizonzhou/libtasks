/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#include <tasks/logging.h>

#ifdef LOGMUTEX
namespace tasks {
std::mutex g_log_mutex;
}
#endif
