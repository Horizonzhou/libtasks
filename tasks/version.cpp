/*
 * Copyright (c) 2013-2014 ADTECH GmbH
 * Licensed under MIT (https://github.com/adtechlabs/libtasks/blob/master/COPYING)
 *
 * Author: Andreas Pohl
 */

#include <tasks/version.h>

namespace tasks {

const char* version_helper::version = TASKS_VERSION;

const char* version() { return version_helper::version; }
}
