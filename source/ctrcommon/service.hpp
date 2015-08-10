#ifndef __CTRCOMMON_SERVICE_HPP__
#define __CTRCOMMON_SERVICE_HPP__

#include "ctrcommon/platform.hpp"
#include "ctrcommon/types.hpp"

#include <string>

bool serviceAcquire();
Error serviceParseError(u32 error);

#endif