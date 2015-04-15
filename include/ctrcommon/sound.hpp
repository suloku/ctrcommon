#ifndef __CTRCOMMON_SOUND_HPP__
#define __CTRCOMMON_SOUND_HPP__

#include "ctrcommon/types.hpp"

typedef enum {
    FORMAT_PCM8 = 0,
    FORMAT_PCM16 = 4096
} SoundFormat;

void* soundAlloc(u32 size);
void soundFree(void* mem);
bool soundPlay(u32 channel, SoundFormat format, u32 sampleRate, void* samples, u32 numSamples);

#endif
