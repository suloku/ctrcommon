#include "ctrcommon/gpu.hpp"

#include <malloc.h>
#include <string.h>

#include <3ds.h>

#define TEX_ENV_COUNT 6
#define TEX_UNIT_COUNT 3

#define STATE_VIEWPORT (1 << 0)
#define STATE_DEPTH_MAP (1 << 1)
#define STATE_CULL (1 << 2)
#define STATE_STENCIL_TEST (1 << 3)
#define STATE_BLEND (1 << 4)
#define STATE_ALPHA_TEST (1 << 5)
#define STATE_DEPTH_TEST_AND_MASK (1 << 6)
#define STATE_ACTIVE_SHADER (1 << 7)
#define STATE_TEX_ENV (1 << 8)
#define STATE_TEXTURES (1 << 9)

typedef struct {
    DVLB_s* dvlb;
    shaderProgram_s program;
} ShaderData;

typedef struct {
    void* data;
    u32 size;
    u32 numVertices;
    Primitive primitive;

    u64 attributes;
    u8 attributeCount;
    u16 attributeMask;
    u64 attributePermutations;
} VboData;

typedef struct {
    void* data;
    u32 width;
    u32 height;
    u32 size;
    PixelFormat format;
    u32 params;
} TextureData;

typedef struct {
    u16 rgbSources;
    u16 alphaSources;
    u16 rgbOperands;
    u16 alphaOperands;
    CombineFunc rgbCombine;
    CombineFunc alphaCombine;
    u32 constantColor;
} TexEnv;

bool inFrame;

u32 dirtyState;
u32 dirtyTexEnvs;
u32 dirtyTextures;

u32 clearColor;

u32 viewportX;
u32 viewportY;
u32 viewportWidth;
u32 viewportHeight;

float depthNear;
float depthFar;

CullMode cullMode;

bool stencilEnable;
TestFunc stencilFunc;
u8 stencilRef;
u8 stencilMask;
u8 stencilReplace;
StencilOp stencilFail;
StencilOp stencilZFail;
StencilOp stencilZPass;

u8 blendRed;
u8 blendGreen;
u8 blendBlue;
u8 blendAlpha;
BlendEquation blendColorEquation;
BlendEquation blendAlphaEquation;
BlendFactor blendColorSrc;
BlendFactor blendColorDst;
BlendFactor blendAlphaSrc;
BlendFactor blendAlphaDst;

bool alphaEnable;
TestFunc alphaFunc;
u8 alphaRef;

bool depthEnable;
TestFunc depthFunc;

u32 componentMask;

ShaderData* activeShader;

TexEnv texEnv[TEX_ENV_COUNT];

TextureData* activeTextures[3];
u32 enabledTextures;

static u32* gpuFrameBuffer = (u32*) 0x1F119400;
static u32* gpuDepthBuffer = (u32*) 0x1F370800;

void gpuInit() {
    inFrame = false;

    dirtyState = 0xFFFFFFFF;
    dirtyTexEnvs = 0xFFFFFFFF;
    dirtyTextures = 0xFFFFFFFF;

    clearColor = 0;

    viewportX = 0;
    viewportY = 0;
    viewportWidth = 240;
    viewportHeight = 400;

    depthNear = 0;
    depthFar = 1;

    cullMode = CULL_NONE;

    stencilEnable = false;
    stencilFunc = TEST_ALWAYS;
    stencilRef = 0;
    stencilMask = 0xFF;
    stencilReplace = 0;
    stencilFail = OP_KEEP;
    stencilZFail = OP_KEEP;
    stencilZPass = OP_KEEP;

    blendRed = 0;
    blendGreen = 0;
    blendBlue = 0;
    blendAlpha = 0;
    blendColorEquation = BLEND_ADD;
    blendAlphaEquation = BLEND_ADD;
    blendColorSrc = FACTOR_SRC_ALPHA;
    blendColorDst = FACTOR_ONE_MINUS_SRC_ALPHA;
    blendAlphaSrc = FACTOR_SRC_ALPHA;
    blendAlphaDst = FACTOR_ONE_MINUS_SRC_ALPHA;

    alphaEnable = false;
    alphaFunc = TEST_ALWAYS;
    alphaRef = 0;

    depthEnable = false;
    depthFunc = TEST_GREATER;

    componentMask = GPU_WRITE_ALL;

    activeShader = NULL;

    texEnv[0].rgbSources = TEXENV_SOURCES(SOURCE_TEXTURE0, SOURCE_PRIMARY_COLOR, SOURCE_PRIMARY_COLOR);
    texEnv[0].alphaSources = TEXENV_SOURCES(SOURCE_TEXTURE0, SOURCE_PRIMARY_COLOR, SOURCE_PRIMARY_COLOR);
    texEnv[0].rgbOperands = TEXENV_OPERANDS(0, 0, 0);
    texEnv[0].alphaOperands = TEXENV_OPERANDS(0, 0, 0);
    texEnv[0].rgbCombine = COMBINE_MODULATE;
    texEnv[0].alphaCombine = COMBINE_MODULATE;
    texEnv[0].constantColor = 0xFFFFFFFF;
    for(u8 env = 1; env < TEX_ENV_COUNT; env++) {
        texEnv[env].rgbSources = TEXENV_SOURCES(SOURCE_PREVIOUS, SOURCE_PRIMARY_COLOR, SOURCE_PRIMARY_COLOR);
        texEnv[env].alphaSources = TEXENV_SOURCES(SOURCE_PREVIOUS, SOURCE_PRIMARY_COLOR, SOURCE_PRIMARY_COLOR);
        texEnv[env].rgbOperands = TEXENV_OPERANDS(0, 0, 0);
        texEnv[env].alphaOperands = TEXENV_OPERANDS(0, 0, 0);
        texEnv[env].rgbCombine = COMBINE_REPLACE;
        texEnv[env].alphaCombine = COMBINE_REPLACE;
        texEnv[env].constantColor = 0xFFFFFFFF;
    }

    for(u8 unit = 0; unit < TEX_UNIT_COUNT; unit++) {
        activeTextures[unit] = NULL;
    }

    enabledTextures = 0;

    u32 gpuCmdSize = 0x40000;
    u32* gpuCmd = (u32*) linearAlloc(gpuCmdSize * 4);
    GPU_Init(NULL);
    GPU_Reset(NULL, gpuCmd, gpuCmdSize);
}

void gpuUpdateState() {
    if(!inFrame) {
        return;
    }

    u32 dirtyUpdate = dirtyState;
    dirtyState = 0;

    if(dirtyUpdate & STATE_VIEWPORT) {
        GPU_SetViewport((u32*) osConvertVirtToPhys((u32) gpuDepthBuffer), (u32*) osConvertVirtToPhys((u32) gpuFrameBuffer), viewportX, viewportY, viewportWidth, viewportHeight);
    }

    if(dirtyUpdate & STATE_DEPTH_MAP) {
        GPU_DepthMap(depthNear, depthFar);
    }

    if(dirtyUpdate & STATE_CULL) {
        GPU_SetFaceCulling((GPU_CULLMODE) cullMode);
    }

    if(dirtyUpdate & STATE_STENCIL_TEST) {
        GPU_SetStencilTest(stencilEnable, (GPU_TESTFUNC) stencilFunc, stencilRef, stencilMask, stencilReplace);
        GPU_SetStencilOp((GPU_STENCILOP) stencilFail, (GPU_STENCILOP) stencilZFail, (GPU_STENCILOP) stencilZPass);
    }

    if(dirtyUpdate & STATE_BLEND) {
        GPU_SetBlendingColor(blendRed, blendGreen, blendBlue, blendAlpha);
        GPU_SetAlphaBlending((GPU_BLENDEQUATION) blendColorEquation, (GPU_BLENDEQUATION) blendAlphaEquation, (GPU_BLENDFACTOR) blendColorSrc, (GPU_BLENDFACTOR) blendColorDst, (GPU_BLENDFACTOR) blendAlphaSrc, (GPU_BLENDFACTOR) blendAlphaDst);
    }

    if(dirtyUpdate & STATE_ALPHA_TEST) {
        GPU_SetAlphaTest(alphaEnable, (GPU_TESTFUNC) alphaFunc, alphaRef);
    }

    if(dirtyUpdate & STATE_DEPTH_TEST_AND_MASK) {
        GPU_SetDepthTestAndWriteMask(depthEnable, (GPU_TESTFUNC) depthFunc, (GPU_WRITEMASK) componentMask);
    }

    if(dirtyUpdate & STATE_ACTIVE_SHADER && activeShader != NULL && activeShader->dvlb != NULL) {
        shaderProgramUse(&activeShader->program);
    }

    if(dirtyUpdate & STATE_TEX_ENV && dirtyTexEnvs != 0) {
        u32 texEnvs = dirtyTexEnvs;
        dirtyTexEnvs = 0;
        for(u8 env = 0; env < TEX_ENV_COUNT; env++) {
            if(texEnvs & (1 << env)) {
                GPU_SetTexEnv(env, texEnv[env].rgbSources, texEnv[env].alphaSources, texEnv[env].rgbOperands, texEnv[env].alphaOperands, (GPU_COMBINEFUNC) texEnv[env].rgbCombine, (GPU_COMBINEFUNC) texEnv[env].alphaCombine, texEnv[env].constantColor);
            }
        }
    }

    if(dirtyUpdate & STATE_TEXTURES && dirtyTextures != 0) {
        u32 textures = dirtyTextures;
        dirtyTextures = 0;
        for(u8 unit = 0; unit < TEX_UNIT_COUNT; unit++) {
            if(textures & (1 << unit)) {
                TexUnit texUnit = unit == 0 ? TEXUNIT0 : unit == 1 ? TEXUNIT1 : TEXUNIT2;
                TextureData* textureData = activeTextures[unit];
                if(textureData != NULL && textureData->data != NULL) {
                    GPU_SetTexture((GPU_TEXUNIT) texUnit, (u32*) osConvertVirtToPhys((u32) textureData->data), (u16) textureData->height, (u16) textureData->width, textureData->params, (GPU_TEXCOLOR) textureData->format);
                    enabledTextures |= texUnit;
                } else {
                    enabledTextures &= ~texUnit;
                }
            }
        }

        GPU_SetTextureEnable((GPU_TEXUNIT) enabledTextures);
    }
}

void gpuBeginFrame() {
    if(inFrame) {
        return;
    }

    inFrame = true;
    GPUCMD_SetBufferOffset(0);
}

void gpuEndFrame() {
    if(!inFrame) {
        return;
    }

    inFrame = false;

    GPUCMD_Finalize();
    GPU_FinishDrawing();
    GPUCMD_FlushAndRun(NULL);
    gspWaitForP3D();

    GX_SetDisplayTransfer(NULL, gpuFrameBuffer, (400 << 16) | 240, (u32*) gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL), (400 << 16) | 240, 0x1000);
    gspWaitForPPF();

    GX_SetMemoryFill(NULL, gpuFrameBuffer, clearColor, &gpuFrameBuffer[400 * 240], 0x201, gpuDepthBuffer, 0x00000000, &gpuDepthBuffer[400 * 240], 0x201);
    gspWaitForPSC0();

    gspWaitForVBlank();
    gfxSwapBuffersGpu();
}

void gpuClearColor(u8 red, u8 green, u8 blue, u8 alpha) {
    clearColor = (u32) (((red & 0xFF) << 24) | ((green & 0xFF) << 16) | ((blue & 0xFF) << 8) | (alpha & 0xFF));
}

void gpuViewport(u32 x, u32 y, u32 width, u32 height) {
    viewportX = x;
    viewportY = y;
    viewportWidth = width;
    viewportHeight = height;

    dirtyState |= STATE_VIEWPORT;
}

void gpuDepthMap(float near, float far) {
    depthNear = near;
    depthFar = far;

    dirtyState |= STATE_DEPTH_MAP;
}

void gpuCullMode(CullMode mode) {
    cullMode = mode;

    dirtyState |= STATE_CULL;
}

void gpuStencilFunc(bool enable, TestFunc func, u8 ref, u8 mask, u8 replace) {
    stencilEnable = enable;
    stencilFunc = func;
    stencilRef = ref;
    stencilMask = mask;
    stencilReplace = replace;

    dirtyState |= STATE_STENCIL_TEST;
}

void gpuStencilOp(StencilOp fail, StencilOp zfail, StencilOp zpass) {
    stencilFail = fail;
    stencilZFail = zfail;
    stencilZPass = zpass;

    dirtyState |= STATE_STENCIL_TEST;
}

void gpuBlendColor(u8 red, u8 green, u8 blue, u8 alpha) {
    blendRed = red;
    blendGreen = green;
    blendBlue = blue;
    blendAlpha = alpha;

    dirtyState |= STATE_BLEND;
}

void gpuBlendFunc(BlendEquation colorEquation, BlendEquation alphaEquation, BlendFactor colorSrc, BlendFactor colorDst, BlendFactor alphaSrc, BlendFactor alphaDst) {
    blendColorEquation = colorEquation;
    blendAlphaEquation = alphaEquation;
    blendColorSrc = colorSrc;
    blendColorDst = colorDst;
    blendAlphaSrc = alphaSrc;
    blendAlphaDst = alphaDst;

    dirtyState |= STATE_BLEND;
}

void gpuAlphaFunc(bool enable, TestFunc func, u8 ref) {
    alphaEnable = enable;
    alphaFunc = func;
    alphaRef = ref;

    dirtyState |= STATE_ALPHA_TEST;
}

void gpuDepthFunc(bool enable, TestFunc func) {
    depthEnable = enable;
    depthFunc = func;

    dirtyState |= STATE_DEPTH_TEST_AND_MASK;
}

void gpuColorMask(bool red, bool green, bool blue, bool alpha) {
    componentMask = red ? componentMask | GPU_WRITE_RED : componentMask & ~GPU_WRITE_RED;
    componentMask = green ? componentMask | GPU_WRITE_GREEN : componentMask & ~GPU_WRITE_GREEN;
    componentMask = blue ? componentMask | GPU_WRITE_BLUE : componentMask & ~GPU_WRITE_BLUE;
    componentMask = alpha ? componentMask | GPU_WRITE_ALPHA : componentMask & ~GPU_WRITE_ALPHA;

    dirtyState |= STATE_DEPTH_TEST_AND_MASK;
}

void gpuDepthMask(bool depth) {
    componentMask = depth ? componentMask | GPU_WRITE_DEPTH : componentMask & ~GPU_WRITE_DEPTH;

    dirtyState |= STATE_DEPTH_TEST_AND_MASK;
}

void gpuCreateShader(u32* shader) {
    if(shader == NULL) {
        return;
    }

    *shader = (u32) malloc(sizeof(ShaderData));
    memset((void*) *shader, 0, sizeof(ShaderData));
}

void gpuFreeShader(u32 shader) {
    ShaderData* shdr = (ShaderData*) shader;
    if(shdr == NULL) {
        return;
    }

    if(shdr->dvlb != NULL) {
        shaderProgramFree(&shdr->program);
        DVLB_Free(shdr->dvlb);
    }

    free(shdr);
}

void gpuLoadShader(u32 shader, const void* data, u32 size) {
    if(data == NULL) {
        return;
    }

    ShaderData* shdr = (ShaderData*) shader;
    if(shdr == NULL) {
        return;
    }

    if(shdr->dvlb != NULL) {
        shaderProgramFree(&shdr->program);
        DVLB_Free(shdr->dvlb);
    }

    shdr->dvlb = DVLB_ParseFile((u32*) data, size);
    shaderProgramInit(&shdr->program);
    shaderProgramSetVsh(&shdr->program, &shdr->dvlb->DVLE[0]);
}

void gpuUseShader(u32 shader) {
    ShaderData* shdr = (ShaderData*) shader;
    if(shdr == NULL) {
        return;
    }

    activeShader = shdr;

    dirtyState |= STATE_ACTIVE_SHADER;
}

void gpuGetUniformBool(u32 shader, int id, bool* value) {
    if(value == NULL) {
        return;
    }

    ShaderData* shdr = (ShaderData*) shader;
    if(shdr == NULL) {
        return;
    }

    shaderInstanceGetBool(shdr->program.vertexShader, id, value);
}

void gpuSetUniformBool(u32 shader, int id, bool value) {
    ShaderData* shdr = (ShaderData*) shader;
    if(shdr == NULL) {
        return;
    }

    shaderInstanceSetBool(shdr->program.vertexShader, id, value);
}

void gpuSetUniform(u32 shader, const char* name, const void* data, u32 elements) {
    if(name == NULL || data == NULL) {
        return;
    }

    ShaderData* shdr = (ShaderData*) shader;
    if(shdr == NULL) {
        return;
    }

    Result res = shaderInstanceGetUniformLocation(shdr->program.vertexShader, name);
    if(res >= 0) {
        // TODO: state variables to make sure this happens within frame?
        GPU_SetFloatUniform(GPU_VERTEX_SHADER, (u32) res, (u32*) data, elements);
    }
}

void gpuCreateVbo(u32* vbo) {
    if(vbo == NULL) {
        return;
    }

    *vbo = (u32) malloc(sizeof(VboData));
    memset((void*) *vbo, 0, sizeof(VboData));
}

void gpuFreeVbo(u32 vbo) {
    VboData* vboData = (VboData*) vbo;
    if(vboData == NULL) {
        return;
    }

    if(vboData->data != NULL) {
        linearFree(vboData->data);
    }

    free(vboData);
}

void gpuVboData(u32 vbo, const void* data, u32 size, u32 numVertices, Primitive primitive) {
    if(data == NULL) {
        return;
    }

    VboData* vboData = (VboData*) vbo;
    if(vboData == NULL) {
        return;
    }

    if(vboData->data == NULL || vboData->size != size) {
        if(vboData->data != NULL) {
            linearFree(vboData->data);
        }

        vboData->data = linearMemAlign(size, 0x80);
    }

    memcpy(vboData->data, data, size);

    vboData->size = size;
    vboData->numVertices = numVertices;
    vboData->primitive = primitive;
}

void gpuVboAttributes(u32 vbo, u64 attributes, u8 attributeCount) {
    VboData* vboData = (VboData*) vbo;
    if(vboData == NULL) {
        return;
    }

    vboData->attributes = attributes;
    vboData->attributeCount = attributeCount;
    vboData->attributeMask = 0xFFC;
    vboData->attributePermutations = 0;
    for(u32 i = 0; i < vboData->attributeCount; i++) {
        vboData->attributePermutations |= i << (i * 4);
    }
}

// Using GPUCMD_AddSingleParam doesn't compile due to its use of temporary arrays as arguments.
void gpuAddSingle(u32 header, u32 param) {
    GPUCMD_Add(header, &param, 1);
}

void gpuDrawVbo(u32 vbo) {
    if(!inFrame) {
        return;
    }

    VboData* vboData = (VboData*) vbo;
    if(vboData == NULL) {
        return;
    }

    gpuUpdateState();

    static u32 attributeBufferOffset = 0;
    GPU_SetAttributeBuffers(vboData->attributeCount, (u32*) osConvertVirtToPhys((u32) vboData->data), vboData->attributes, vboData->attributeMask, vboData->attributePermutations, 1, &attributeBufferOffset, &vboData->attributePermutations, &vboData->attributeCount);

    gpuAddSingle(0x000F0200, (osConvertVirtToPhys((u32) vboData->data)) >> 3);
    gpuAddSingle(0x0002025E, (GPU_Primitive_t) vboData->primitive);
    gpuAddSingle(0x0002025F, 0x00000001);
    gpuAddSingle(0x000F0227, 0x80000000);
    gpuAddSingle(0x000F0228, vboData->numVertices);
    gpuAddSingle(0x00010253, 0x00000001);
    gpuAddSingle(0x00010245, 0x00000000);
    gpuAddSingle(0x000F022E, 0x00000001);
    gpuAddSingle(0x0008025E, 0x00000000);
    gpuAddSingle(0x0008025E, 0x00000000);
    gpuAddSingle(0x00010245, 0x00000001);
    gpuAddSingle(0x000F0231, 0x00000001);
    gpuAddSingle(0x000F0111, 0x00000001);
}

void gpuTexEnv(u32 env, u16 rgbSources, u16 alphaSources, u16 rgbOperands, u16 alphaOperands, CombineFunc rgbCombine, CombineFunc alphaCombine, u32 constantColor) {
    if(env >= TEX_ENV_COUNT) {
        return;
    }

    texEnv[env].rgbSources = rgbSources;
    texEnv[env].alphaSources = alphaSources;
    texEnv[env].rgbOperands = rgbOperands;
    texEnv[env].alphaOperands = alphaOperands;
    texEnv[env].rgbCombine = rgbCombine;
    texEnv[env].alphaCombine = alphaCombine;
    texEnv[env].constantColor = constantColor;

    dirtyState |= STATE_TEX_ENV;
    dirtyTexEnvs |= (1 << env);
}

void gpuCreateTexture(u32* texture) {
    if(texture == NULL) {
        return;
    }

    *texture = (u32) malloc(sizeof(TextureData));
    memset((void*) *texture, 0, sizeof(TextureData));
}

void gpuFreeTexture(u32 texture) {
    TextureData* textureData = (TextureData*) texture;
    if(textureData == NULL) {
        return;
    }

    if(textureData->data != NULL) {
        linearFree(textureData->data);
    }

    free(textureData);
}

void gpuTextureData(u32 texture, const void* data, u32 inWidth, u32 inHeight, PixelFormat inFormat, u32 outWidth, u32 outHeight, PixelFormat outFormat, u32 params) {
    TextureData* textureData = (TextureData*) texture;
    if(textureData == NULL) {
        return;
    }

    u32 size = outWidth * outHeight;
    switch(outFormat) {
        case PIXEL_RGBA8:
        case PIXEL_ETC1:
        case PIXEL_ETC1A4:
            size *= 4;
            break;
        case PIXEL_RGB8:
            size *= 3;
            break;
        case PIXEL_RGBA5551:
        case PIXEL_RGB565:
        case PIXEL_RGBA4:
        case PIXEL_LA8:
        case PIXEL_HILO8:
            size *= 2;
            break;
        case PIXEL_L8:
        case PIXEL_A8:
        case PIXEL_LA4:
        case PIXEL_L4:
            break;
    }

    if(textureData->data == NULL || textureData->size != size) {
        if(textureData->data != NULL) {
            linearFree(textureData->data);
        }

        textureData->data = linearMemAlign(size, 0x80);
        for(u8 unit = 0; unit < 3; unit++) {
            if(activeTextures[unit] == textureData) {
                dirtyState |= STATE_TEXTURES;
                dirtyTextures |= (1 << unit);
                break;
            }
        }
    }

    GX_SetDisplayTransfer(NULL, (u32*) data, (inHeight << 16) | inWidth, (u32*) textureData->data, (outHeight << 16) | outWidth, (u32) ((1 << 1) | (inFormat << 8) | (outFormat << 12)));
    gspWaitForPPF();

    textureData->width = outWidth;
    textureData->height = outHeight;
    textureData->size = size;
    textureData->format = outFormat;
    textureData->params = params;
}

void gpuBindTexture(TexUnit unit, u32 texture) {
    u32 unitIndex = unit == TEXUNIT0 ? 0 : unit == TEXUNIT1 ? 1 : 2;
    activeTextures[unitIndex] = (TextureData*) texture;

    dirtyState |= STATE_TEXTURES;
    dirtyTextures |= (1 << unitIndex);
}

