#include "ctrcommon/gpu.hpp"
#include "ctrcommon/platform.hpp"

#include <malloc.h>
#include <stdio.h>
#include <string.h>

#include <sstream>

#include <3ds.h>

#include "ctrcommon_shader_vsh_shbin.h"
#include "ctrcommon_font_bin.h"

u32 defaultShader = 0;
u32 tempVbo = 0;
u32 dummyTexture = 0;
u32 fontTexture = 0;

float* charVbo;

void gputInit() {
    gpuCreateShader(&defaultShader);
    gpuLoadShader(defaultShader, ctrcommon_shader_vsh_shbin, ctrcommon_shader_vsh_shbin_size);
    gputUseDefaultShader();

    gpuCreateVbo(&tempVbo);
    gpuVboAttributes(tempVbo, ATTRIBUTE(0, 3, ATTR_FLOAT) | ATTRIBUTE(1, 2, ATTR_FLOAT) | ATTRIBUTE(2, 4, ATTR_FLOAT), 3);

    gpuCreateTexture(&dummyTexture);
    gpuTextureInfo(dummyTexture, 64, 64, PIXEL_RGBA8, TEXTURE_MIN_FILTER(FILTER_NEAREST) | TEXTURE_MAG_FILTER(FILTER_NEAREST));
    memset(gpuGetTextureData(dummyTexture), 0xFF, 64 * 64 * 4);

    void* gpuFont = gpuAlloc(ctrcommon_font_bin_size);
    memcpy(gpuFont, ctrcommon_font_bin, ctrcommon_font_bin_size);
    gpuCreateTexture(&fontTexture);
    gpuTextureData(fontTexture, gpuFont, 128, 128, PIXEL_RGBA8, TEXTURE_MIN_FILTER(FILTER_NEAREST) | TEXTURE_MAG_FILTER(FILTER_NEAREST));
    gpuFree(gpuFont);

    // charsX * charsY * verticesPerChar * floatsPerVertex * sizeof(float)
    charVbo = (float*) malloc(50 * 30 * 6 * 9 * sizeof(float));
}

void gputCleanup() {
    if(defaultShader != 0) {
        gpuFreeShader(defaultShader);
        defaultShader = 0;
    }

    if(tempVbo != 0) {
        gpuFreeVbo(tempVbo);
        tempVbo = 0;
    }

    if(fontTexture != 0) {
        gpuFreeTexture(fontTexture);
        fontTexture = 0;
    }

    if(charVbo != NULL) {
        free(charVbo);
        charVbo = NULL;
    }
}

void gputUpdateViewport(u32 width, u32 height) {
    const float orthoMatrix[] = {
            2.0f / (float) width, 0.0f,                  0.0f, -1.0f,
            0.0f,                 2.0f / (float) height, 0.0f, -1.0f,
            0.0f,                 0.0f,                  1.0f,  0.0f,
            0.0f,                 0.0f,                  0.0f,  1.0f
    };

    static const float rotateMatrix[] = {
             0.0f, 1.0f, 0.0f, 0.0f,
            -1.0f, 0.0f, 0.0f, 0.0f,
             0.0f, 0.0f, 1.0f, 0.0f,
             0.0f, 0.0f, 0.0f, 1.0f
    };

    float resultMatrix[4 * 4];
    for(u32 x1 = 0; x1 < 4; x1++) {
        for(u32 y2 = 0; y2 < 4; y2++) {
            resultMatrix[y2 * 4 + x1] = 0;
            for(u32 y1 = 0; y1 < 4; y1++) {
                resultMatrix[y2 * 4 + x1] += orthoMatrix[y1 * 4 + x1] * rotateMatrix[y2 * 4 + y1];
            }
        }
    }

    gpuSetUniform(defaultShader, VERTEX_SHADER, "projection", resultMatrix, 4);
}

void gputUseDefaultShader() {
    gpuUseShader(defaultShader);
}

int gputGetStringWidth(const std::string str, float scale) {
    u32 len = str.length();
    if(len == 0) {
        return 0;
    }

    u32 longestLine = 0;
    u32 currLength = 0;
    for(u32 i = 0; i < len; i++) {
        if(str[i] == '\n') {
            if(currLength > longestLine) {
                longestLine = currLength;
            }

            currLength = 0;
            continue;
        }

        currLength++;
    }

    if(currLength > longestLine) {
        longestLine = currLength;
    }

    return (int) (longestLine * 8 * scale);
}

int gputGetStringHeight(const std::string str, float scale) {
    u32 len = str.length();
    if(len == 0) {
        return 0;
    }

    u32 lines = 1;
    for(u32 i = 0; i < len; i++) {
        if(str[i] == '\n') {
            lines++;
        }
    }

    return (int) (lines * 8 * scale);
}

void gputDrawString(const std::string str, int x, int y, float scale, u8 red, u8 green, u8 blue, u8 alpha) {
    static const float charSize = 8.0f / 128.0f;

    const float size = 8 * scale;
    const float r = (float) red / 255.0f;
    const float g = (float) green / 255.0f;
    const float b = (float) blue / 255.0f;
    const float a = (float) alpha / 255.0f;

    u32 len = str.length();
    float cx = x;
    float cy = y + gputGetStringHeight(str, scale) - 8;
    u32 vboIndex = 0;
    for(u32 i = 0; i < len; i++) {
        char c = str[i];
        if(c == '\n') {
            cx = x;
            cy -= size;
            continue;
        }

        const float texX1 = (c % 16) * charSize;
        const float texY1 = 1.0f - ((c / 16 + 1) * charSize);
        const float texX2 = texX1 + charSize;
        const float texY2 = texY1 + charSize;

        const float vboData[] = {
                cx, cy, -0.1f, texX1, texY1, r, g, b, a,
                cx + size, cy, -0.1f, texX2, texY1, r, g, b, a,
                cx + size, cy + size, -0.1f, texX2, texY2, r, g, b, a,
                cx + size, cy + size, -0.1f, texX2, texY2, r, g, b, a,
                cx, cy + size, -0.1f, texX1, texY2, r, g, b, a,
                cx, cy, -0.1f, texX1, texY1, r, g, b, a
        };

        memcpy(charVbo + (vboIndex * 6 * 9), vboData, sizeof(vboData));
        cx += size;
        vboIndex++;
    }

    gpuVboData(tempVbo, charVbo, vboIndex * 6 * 9 * sizeof(float), vboIndex * 6, PRIM_TRIANGLES);

    gpuBindTexture(TEXUNIT0, fontTexture);
    gpuDrawVbo(tempVbo);

    // Flush the GPU command buffer so we can safely reuse the VBO.
    gpuFlush();
}

void gputDrawRectangle(int x, int y, u32 width, u32 height, u8 red, u8 green, u8 blue, u8 alpha) {
    const float r = (float) red / 255.0f;
    const float g = (float) green / 255.0f;
    const float b = (float) blue / 255.0f;
    const float a = (float) alpha / 255.0f;

    const float vboData[] = {
            (float) x, (float) y, -0.1f, 0.0f, 0.0f, r, g, b, a,
            (float) x + width, (float) y, -0.1f, 1.0f, 0.0f, r, g, b, a,
            (float) x + width, (float) y + height, -0.1f, 1.0f, 1.0f, r, g, b, a,
            (float) x + width, (float) y + height, -0.1f, 1.0f, 1.0f, r, g, b, a,
            (float) x, (float) y + height, -0.1f, 0.0f, 1.0f, r, g, b, a,
            (float) x, (float) y, -0.1f, 0.0f, 0.0f, r, g, b, a
    };

    gpuVboData(tempVbo, vboData, sizeof(vboData), sizeof(vboData) / (9 * 4), PRIM_TRIANGLES);

    gpuBindTexture(TEXUNIT0, dummyTexture);
    gpuDrawVbo(tempVbo);

    // Flush the GPU command buffer so we can safely reuse the VBO.
    gpuFlush();
}

void gputTakeScreenshot() {
    u32 headerSize = 0x36;
    u32 imageSize = 400 * 480 * 3;

    u8* header = (u8*) malloc(headerSize);
    memset(header, 0, headerSize);

    *(u16*) &header[0x0] = 0x4D42;
    *(u32*) &header[0x2] = headerSize + imageSize;
    *(u32*) &header[0xA] = headerSize;
    *(u32*) &header[0xE] = 0x28;
    *(u32*) &header[0x12] = 400;
    *(u32*) &header[0x16] = 480;
    *(u32*) &header[0x1A] = 0x00180001;
    *(u32*) &header[0x22] = imageSize;

    u8* image = (u8*) malloc(imageSize);
    memset(image, 0, imageSize);

    if(gfxGetScreenFormat(GFX_TOP) == GSP_BGR8_OES) {
        u8* top = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
        for(u32 x = 0; x < 400; x++) {
            for(u32 y = 0; y < 240; y++) {
                u8* src = &top[((240 - y - 1) + x * 240) * 3];
                u8* dst = &image[((479 - y) * 400 + x) * 3];

                *(u16*) dst = *(u16*) src;
                dst[2] = src[2];
            }
        }
    }

    if(gfxGetScreenFormat(GFX_BOTTOM) == GSP_BGR8_OES) {
        u8* bottom = gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, NULL, NULL);
        for(u32 x = 0; x < 320; x++) {
            for(u32 y = 0; y < 240; y++) {
                u8* src = &bottom[((240 - y - 1) + x * 240) * 3];
                u8* dst = &image[((479 - (y + 240)) * 400 + (x + 40)) * 3];

                *(u16*) dst = *(u16*) src;
                dst[2] = src[2];
            }
        }
    }

    std::stringstream fileStream;
    fileStream << "/screenshot_" << platformGetTime() << ".bmp";
    std::string file = fileStream.str();

    FILE* fd = fopen(file.c_str(), "wb");
    if(fd) {
        fwrite(header, 1, headerSize, fd);
        fwrite(image, 1, imageSize, fd);
        fclose(fd);
    }

    free(header);
    free(image);
}