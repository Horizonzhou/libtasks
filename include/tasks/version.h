/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#ifndef _TASKS_VERSION_H_
#define _TASKS_VERSION_H_

namespace tasks {

struct version_helper {
    static const char* version;
};

const char* version();
}

#endif  // _TASKS_VERSION_H_
