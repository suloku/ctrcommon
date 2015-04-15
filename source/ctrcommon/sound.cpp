#include "ctrcommon/sound.hpp"

#include "service.hpp"

#include <3ds.h>

void* soundAlloc(u32 size) {
    return linearAlloc(size);
}

void soundFree(void* mem) {
    linearFree(mem);
}

bool soundPlay(u32 channel, SoundFormat format, u32 sampleRate, void* samples, u32 numSamples) {
    if(!serviceRequire("csnd")) {
        return false;
    }

    if(channel > 7) {
        channel = 7;
    }

    GSPGPU_FlushDataCache(NULL, (u8*) samples, numSamples * (format == FORMAT_PCM16 ? 2 : 1));
    Result res = csndPlaySound((int) (8 + channel), format | SOUND_ONE_SHOT, sampleRate, samples, samples, numSamples * (format == FORMAT_PCM16 ? 2 : 1));
    if(res != 0) {
        platformSetError(serviceParseError((u32) res));
    }

    return res == 0;
}