#include "ctrcommon/nor.hpp"
#include "ctrcommon/platform.hpp"

#include "service.hpp"

#include <3ds.h>

bool hasNor = false;
Error norError = {};

void norInit() {
    Result result = CFGNOR_Initialize(1);
    if(result != 0 && serviceAcquire()) {
        result = CFGNOR_Initialize(1);
    }

    if(result != 0) {
        norError = serviceParseError((u32) result);
    }

    hasNor = result == 0;
}

void norCleanup() {
    if(hasNor) {
        CFGNOR_Shutdown();

        hasNor = false;
        norError = {};
    }
}

bool norRead(u32 offset, void* data, u32 size) {
    if(!hasNor) {
        platformSetError(norError);
        return false;
    }

    Result result = CFGNOR_ReadData(offset, (u32*) data, size);
    if(result != 0) {
        platformSetError(serviceParseError((u32) result));
    }

    return result == 0;
}

bool norWrite(u32 offset, void* data, u32 size) {
    if(!hasNor) {
        platformSetError(norError);
        return false;
    }

    Result result = CFGNOR_WriteData(offset, (u32*) data, size);
    if(result != 0) {
        platformSetError(serviceParseError((u32) result));
    }

    return result == 0;
}