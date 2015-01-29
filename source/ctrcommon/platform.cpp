#include "ctrcommon/common.hpp"

#include <stdio.h>
#include <string.h>

#include <3ds.h>

extern bool amInitialized;
extern bool nsInitialized;

bool platform_init() {
    if(srvInit() != 0 || aptInit() != 0 || hidInit(NULL) != 0 || fsInit() != 0 || sdmcInit() != 0) {
        return false;
    }

    gfxInitDefault();
    return true;
}

void platform_cleanup() {
    if(amInitialized) {
        amExit();
        amInitialized = false;
    }

    if(nsInitialized) {
        nsExit();
        nsInitialized = false;
    }

    sdmcExit();
    fsExit();
    gfxExit();
    hidExit();
    aptExit();
    srvExit();
}

bool platform_is_running() {
    return aptMainLoop();
}

u64 platform_get_time() {
    return osGetTime();
}

void platform_delay(int ms) {
    svcSleepThread(ms * 1000000);
}

void platform_printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    int len = vsnprintf(NULL, 0, format, args);
    va_end(args);

    char str[len + 1];

    va_list args2;
    va_start(args2, format);
    vsnprintf(str, (size_t) len + 1, format, args2);
    va_end(args2);

    svcOutputDebugString(str, strlen(str));
}