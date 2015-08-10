#include "service.hpp"

#include "../libkhax/khax.h"

#include <malloc.h>
#include <stdio.h>

#include <functional>
#include <map>

#include <3ds.h>
#include <ctrcommon/platform.hpp>

#define GET_BITS(v, s, e) (((v) >> (s)) & ((1 << ((e) - (s) + 1)) - 1))

static bool servicesAcquired = false;

bool serviceAcquire() {
    if(!servicesAcquired) {
        Result result = khaxInit();
        servicesAcquired = result == 0;
        if(!servicesAcquired) {
            platformSetError(serviceParseError((u32) result));
        }
    }

    return servicesAcquired;
}

Error serviceParseError(u32 error) {
    Error err;

    err.module = (ErrorModule) GET_BITS(error, 10, 17);
    err.level = (ErrorLevel) GET_BITS(error, 27, 31);
    err.summary = (ErrorSummary) GET_BITS(error, 21, 26);
    err.description = (ErrorDescription) GET_BITS(error, 0, 9);

    return err;
}