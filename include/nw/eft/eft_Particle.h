#ifndef EFT_PARTICLE_H_
#define EFT_PARTICLE_H_

#include <nw/eft/eft_Data.h>
#include <nw/eft/eft_Shader.h>

namespace nw { namespace eft {

struct EmitterInstance;
struct PtclInstance;

enum
{
    cNumStripeHistory     = 256
};

struct PtclStripeHistory
{
    nw::math::VEC3          pos;
    f32                     scale;
    nw::math::MTX34         emitterSRT;
    nw::math::VEC3          outer;
    nw::math::VEC3          dir;
};
static_assert(sizeof(PtclStripeHistory) == 0x58, "nw::eft::PtclStripeHistory size mismatch");

struct UvParam
{
    f32                 rotateZ;
    nw::math::VEC2      offset;
    nw::math::VEC2      scroll;
    nw::math::VEC2      scale;
};
static_assert(sizeof(UvParam) == 0x1C, "nw::eft::UvParam size mismatch");

struct PtclStripe
{
    PtclInstance*                       ptcl;
    s32                                 histQStart;
    s32                                 histQEnd;
    PtclStripeHistory                   hist[cNumStripeHistory];
    s32                                 numHistory;
    u32                                 groupID;
    const ComplexEmitterData*           res;
    s32                                 cnt;
    nw::math::VEC3                      interpolateNextDir;
    nw::math::VEC3                      p0;
    nw::math::VEC3                      p1;
    PtclStripe*                         prev;
    PtclStripe*                         next;
    u32                                 starDrawVertex;
    u32                                 numDrawVertex;
    StripeUniformBlock*                 uniformBlock;
    StripeUniformBlock*                 uniformBlockForCross;
    StripeVertexBuffer*                 attributeBuffer;
    UvParam                             uvParam[2];
    u32                                 flag;
};
static_assert(sizeof(PtclStripe) == 0x5898, "nw::eft::PtclStripe size mismatch");

struct AlphaAnim
{
    f32                 alphaAddSec1;
    f32                 alphaAddSec2;
    s32                 alphaSec1;
    s32                 alphaSec2;
};
static_assert(sizeof(AlphaAnim) == 0x10, "nw::eft::AlphaAnim size mismatch");

struct ScaleAnim
{
    nw::math::VEC2      scaleAddSec1;
    nw::math::VEC2      scaleAddSec2;
    s32                 scaleSec1;
    s32                 scaleSec2;
};
static_assert(sizeof(ScaleAnim) == 0x18, "nw::eft::ScaleAnim size mismatch");

struct ComplexParam
{
    PtclStripe*         stripe;
    f32                 childEmitcnt;
    f32                 childPreEmitcnt;
    f32                 childEmitSaving;
};
static_assert(sizeof(ComplexParam) == 0x10, "nw::eft::ComplexParam size mismatch");

struct PtclInstance
{
    f32                         cnt;
    f32                         emitTime;
    s32                         cntS;
    s32                         life;
    f32                         dynamicsRnd;
    nw::math::VEC3              pos;
    nw::math::VEC3              posDiff;
    nw::math::VEC3              vel;
    nw::math::VEC3              worldPos;
    nw::math::VEC3              worldPosDiff;
    nw::math::VEC3              rot;
    nw::math::VEC3              rotVel;
    f32                         alpha0;
    f32                         alpha1;
    nw::math::VEC2              scale;
    nw::ut::Color4f             color[EFT_COLOR_KIND_MAX];
    f32                         fluctuationAlpha;
    f32                         fluctuationScaleX;
    f32                         fluctuationScaleY;
    nw::math::MTX34             emitterRT;
    nw::math::MTX34             emitterSRT;
    nw::math::MTX34*            coordinateEmitterRT;
    nw::math::MTX34*            coordinateEmitterSRT;
    PtclAttributeBuffer*        attributeBuffer;
    const SimpleEmitterData*    res;
    const ComplexEmitterData*   cres;
    ScaleAnim*                  scaleAnim;
    AlphaAnim*                  alphaAnim[EFT_ALPHA_KIND_MAX];
    ComplexParam*               complexParam;
    PtclInstance*               prev;
    PtclInstance*               next;
    EmitterInstance*            emitter;
    PtclType                    type;
    u32                         rnd;
    f32                         scaleVelY;
    u32                         runtimeUserData;
    f32                         random[4];
    s32                         fieldCollisionCnt;

    void Remove(CpuCore core);

    f32 GetParticleCounter() const
    {
        return cnt;
    }

    s32 GetParticleLife() const
    {
        return life;
    }

    nw::math::VEC3& GetPos()
    {
        return pos;
    }

    nw::math::VEC3& GetWorldPos()
    {
        return worldPos;
    }

    nw::math::VEC3& GetVelocity()
    {
        return vel;
    }

    nw::math::VEC3& GetRotation()
    {
        return rot;
    }

    nw::math::VEC3& GetRotateVelocity()
    {
        return rotVel;
    }

    f32 GetAlpha() const
    {
        return alpha0;
    }

    void SetAlpha(f32 alphaParam)
    {
        alpha0 = alphaParam;
    }

    nw::math::VEC2& GetScale()
    {
        return scale;
    }

    u32 GetRuntimeUserData() const
    {
        return runtimeUserData;
    }

    void SetRuntimeUserData(u32 data)
    {
        runtimeUserData = data;
    }

    PtclType GetParticleType()
    {
        return type;
    }

    void GetDrawColor0(nw::ut::Color4f* dstColor) const;
    void GetDrawColor1(nw::ut::Color4f* dstColor) const;
    void GetDrawScale(nw::math::VEC2* dstScale) const;
};
static_assert(sizeof(PtclInstance) == 0x158, "PtclInstance size mismatch");

} } // namespace nw::eft

#endif // EFT_PARTICLE_H_
