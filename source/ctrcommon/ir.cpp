#include "ctrcommon/ir.hpp"

#include "service.hpp"

#include <malloc.h>

#include <3ds.h>

bool hasIr = false;
Error irError = {};

static u32* irBuffer;

void irInit() {
    irBuffer = (u32*) memalign(0x1000, 0x1000);
    if(irBuffer == NULL) {
        irError = {MODULE_NN_IR, LEVEL_FATAL, SUMMARY_OUT_OF_RESOURCE, DESCRIPTION_OUT_OF_MEMORY};
        return;
    }

    Result result = IRU_Initialize(irBuffer, 0x1000);
    if(result != 0 && serviceAcquire()) {
        result = IRU_Initialize(irBuffer, 0x1000);
    }

    if(result != 0) {
        free(irBuffer);
        irBuffer = NULL;

        irError = serviceParseError((u32) result);
    }

    hasIr = result == 0;
}

void irCleanup() {
    if(hasIr) {
        IRU_Shutdown();
        if(irBuffer != NULL) {
            free(irBuffer);
            irBuffer = NULL;
        }

        hasIr = false;
        irError = {};
    }
}

u32 irGetState() {
    if(!hasIr) {
        platformSetError(irError);
        return 0;
    }

    u32 state;
    IRU_GetIRLEDRecvState(&state);
    return state;
}

void irSetState(u32 state) {
    if(!hasIr) {
        platformSetError(irError);
        return;
    }

    IRU_SetIRLEDState(state);
}