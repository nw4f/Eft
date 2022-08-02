#ifndef EFT_UNIFORM_BLOCK_H_
#define EFT_UNIFORM_BLOCK_H_

#include <nw/typeDef.h>
#include <nw/eft/eft_Enum.h>

namespace nw { namespace eft {

struct ViewUniformBlock
{
    nw::math::MTX44     viewMat;
    nw::math::MTX44     projMat;
    nw::math::MTX44     vpMat;
    nw::math::MTX44     bldMat;
    nw::math::VEC4      eyeVec;
    nw::math::VEC4      eyePos;
    nw::math::VEC4      depthBufferTexMat;
    nw::math::VEC4      frameBufferTexMat;
    nw::math::VEC4      viewParam;
};
static_assert(sizeof(ViewUniformBlock) == 0x150, "nw::eft::ViewUniformBlock size mismatch");

struct EmitterDynamicUniformBlock
{
    nw::math::VEC4      emitterColor0;
    nw::math::VEC4      emitterColor1;
    nw::math::VEC4      emitterParam0;
    nw::math::VEC4      emitterParam1;
    nw::math::MTX44     emitterMatrix;
    nw::math::MTX44     emitterMatrixRT;
};
static_assert(sizeof(EmitterDynamicUniformBlock) == 0xC0, "nw::eft::EmitterDynamicUniformBlock size mismatch");

struct TexUvShiftAnimUbo
{
    nw::math::VEC2      scrollAdd;
    nw::math::VEC2      scrollInit;
    nw::math::VEC2      scrollInitRand;
    nw::math::VEC2      scaleAdd;
    nw::math::VEC2      scaleInit;
    nw::math::VEC2      scaleInitRand;
    f32                 rotAdd;
    f32                 rotInit;
    f32                 rotInitRand;
    f32                 randomType;
    nw::math::VEC2      uvScale;
    nw::math::VEC2      uvDiv;
};
static_assert(sizeof(TexUvShiftAnimUbo) == 0x50, "nw::eft::TexUvShiftAnimUbo size mismatch");

struct TexPtnAnimUbo
{
    f32                 ptnTableNum;
    f32                 ptnFreq;
    f32                 ptnNum;
    f32                 dummy;
    s32                 ptnTable[EFT_TEXTURE_PATTERN_NUM];
};
static_assert(sizeof(TexPtnAnimUbo) == 0x90, "nw::eft::TexPtnAnimUbo size mismatch");

struct FluctuationUbo
{
    f32                 scale;
    f32                 freq;
    f32                 phase;
    f32                 phaseRandom;
};
static_assert(sizeof(FluctuationUbo) == 0x10, "nw::eft::FluctuationUbo size mismatch");

struct EmitterStaticUniformBlock
{
    s32                 flag[4];
    TexUvShiftAnimUbo   texShiftAnim0;
    TexUvShiftAnimUbo   texShiftAnim1;
    TexUvShiftAnimUbo texShiftAnim2;
    TexPtnAnimUbo       texPtnAnim0;
    TexPtnAnimUbo       texPtnAnim1;
    FluctuationUbo      fluctuationX;
    FluctuationUbo      fluctuationY;

    nw::math::VEC4      rotBasis;

    nw::math::VEC4      fresnelMinMax;
    nw::math::VEC4      nearAlpha;
    nw::math::VEC4      rotateVel;
    nw::math::VEC4      rotateVelRand;

    nw::math::VEC4      alphaAnim0;
    nw::math::VEC4      alphaAnim1;
    nw::math::VEC4      scaleAnim0;
    nw::math::VEC4      scaleAnim1;

    nw::math::VEC4      scaleAnim2;
    nw::math::VEC4      color0[8];
    nw::math::VEC4      color1[8];
    nw::math::VEC4      color0Anim;

    nw::math::VEC4      color1Anim;
    nw::math::VEC4      vectorParam;
    nw::math::VEC4      gravityParam;
    nw::math::VEC4      alphaAnim2;

    nw::math::VEC4      fieldRandom;
    nw::math::VEC4      fieldCollision;
    nw::math::VEC4      fieldPosAdd;

    nw::math::VEC4      fieldConvergence;
    nw::math::VEC4      fieldMagnet;
    nw::math::VEC4      fieldSpin;
    nw::math::VEC4      fieldCoordEmitter;

    nw::math::VEC4      fieldCurlNoise0;
    nw::math::VEC4      fieldCurlNoise1;

    nw::math::VEC4      shaderParam;
    nw::math::VEC4      shaderParamAnim0;
    nw::math::VEC4      shaderParamAnim1;
    nw::math::VEC4      _align256;
};
static_assert(sizeof(EmitterStaticUniformBlock) == 0x500, "nw::eft::EmitterStaticUniformBlock size mismatch");

struct StripeVertexBuffer
{
    nw::math::VEC4      pos;
    nw::math::VEC4      outer;
    nw::math::VEC4      tex;
    nw::math::VEC4      dir;
};
static_assert(sizeof(StripeVertexBuffer) == 0x40, "nw::eft::StripeVertexBuffer size mismatch");

struct StripeUniformBlock
{
    nw::math::VEC4      stParam;
    nw::math::VEC4      uvScrollAnim;
    nw::math::VEC4      uvScaleRotAnim0;
    nw::math::VEC4      uvScaleRotAnim1;
    nw::math::VEC4      vtxColor0;
    nw::math::VEC4      vtxColor1;
    nw::math::MTX44     emitterMat;
};
static_assert(sizeof(StripeUniformBlock) == 0xA0, "nw::eft::StripeUniformBlock size mismatch");

} } // namespace nw::eft

#endif // EFT_UNIFORM_BLOCK_H_
