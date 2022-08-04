#include <nw/eft/eft_Emitter.h>
#include <nw/eft/eft_EmitterSet.h>
#include <nw/eft/eft_UniformBlock.h>

#if EFT_IS_CAFE_WUT || !EFT_IS_CAFE
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#endif

namespace nw { namespace eft {

// No division-by-zero checks whatsoever...

static inline void _calcTexPatAnim(PtclInstance* ptcl, const TextureEmitterData* textureData)
{
    register s32 cnt = (s32)ptcl->cnt;
    register u32 patIx;

    // Four animation types:
    // 1. Fit to lifespan
    // 2. Clamp
    // 3. Loop
    // 4. Random (Not handled here; handled in Emit function)

    //const bool fitToLifespan = textureData->texPatFreq == 0;
    //const bool clamp         = textureData->isTexPatAnimClump;
    //const bool loop          = !fitToLifespan && !clamp;

    if (textureData->texPatFreq == 0)
        patIx = cnt * textureData->texPatTblUse / ptcl->life; // (time / lifespan) * size
    else
        patIx = cnt / textureData->texPatFreq;                //  time / period

    if (textureData->isTexPatAnimClump &&
        textureData->texPatTblUse <= patIx)
    {
        return;
    }

    if (textureData->texPatFreq != 0 &&
        !textureData->isTexPatAnimClump &&
        textureData->isTexPatAnimRand)
    {
        patIx += ptcl->rnd;
    }

    patIx = textureData->texPatTbl[patIx % textureData->texPatTblUse];

    s32 no_x = patIx % textureData->numTexDivX;
    s32 no_y = patIx / textureData->numTexDivX;

    ptcl->uvOffset.x = textureData->texUScale * no_x;
    ptcl->uvOffset.y = textureData->texVScale * no_y;
}

static inline void _calcSubTexPatAnim(PtclInstance* ptcl, const TextureEmitterData* textureData)
{
    register s32 cnt = (s32)ptcl->cnt;
    register u32 patIx;

    // Four animation types:
    // 1. Fit to lifespan
    // 2. Clamp
    // 3. Loop
    // 4. Random (Not handled here; handled in Emit function)

    //const bool fitToLifespan = textureData->texPatFreq == 0;
    //const bool clamp         = textureData->isTexPatAnimClump;
    //const bool loop          = !fitToLifespan && !clamp;

    if (textureData->texPatFreq == 0)
        patIx = cnt * textureData->texPatTblUse / ptcl->life; // (time / lifespan) * size
    else
        patIx = cnt / textureData->texPatFreq;                //  time / period

    if (textureData->isTexPatAnimClump &&
        textureData->texPatTblUse <= patIx)
    {
        return;
    }

    if (textureData->texPatFreq != 0 &&
        !textureData->isTexPatAnimClump &&
        textureData->isTexPatAnimRand)
    {
        patIx += ptcl->rnd;
    }

    patIx = textureData->texPatTbl[patIx % textureData->texPatTblUse];

    s32 no_x = patIx % textureData->numTexDivX;
    s32 no_y = patIx / textureData->numTexDivX;

    ptcl->uvSubOffset.x = textureData->texUScale * no_x;
    ptcl->uvSubOffset.y = textureData->texVScale * no_y;
}

static inline void _calcChildTexPatAnim(const ComplexEmitterData* __restrict res, const ChildData* __restrict cres, PtclInstance* __restrict ptcl)
{
    register s32 cnt = (s32)ptcl->cnt;
    register u32 patIx;

    // Four animation types:
    // 1. Fit to lifespan
    // 2. Clamp
    // 3. Loop
    // 4. Random (Not handled here; handled in Emit function)

    //const bool fitToLifespan = cres->childTexPatFreq == 0;
    //const bool clamp         = cres->isChildTexPatAnimClump;
    //const bool loop          = !fitToLifespan && !clamp;

    if (cres->childTexPatFreq == 0)
        patIx = cnt * cres->childTexPatTblUse / ptcl->life; // (time / lifespan) * size
    else
        patIx = cnt / cres->childTexPatFreq;                //  time / period

    if (cres->isChildTexPatAnimClump &&
        u32(cres->childTexPatTblUse) <= patIx)
    {
        return;
    }

    if (cres->childTexPatFreq != 0 &&
        !cres->isChildTexPatAnimClump &&
        res->childFlg & EFT_CHILD_FLAG_IS_TEXTURE_PAT_ANIM_RAND)
    {
        patIx += ptcl->rnd;
    }

    patIx = cres->childTexPatTbl[patIx % cres->childTexPatTblUse];

    s32 no_x = patIx % cres->childNumTexDivX;
    s32 no_y = patIx / cres->childNumTexDivX;

    ptcl->uvOffset.x = cres->childTexUScale * no_x;
    ptcl->uvOffset.y = cres->childTexVScale * no_y;
}

static inline void _interpolate3Colors(const SimpleEmitterData* __restrict res, PtclInstance* __restrict ptcl, ColorKind kind)
{
    s32 freq = ptcl->life / res->colorNumRepeat[kind];
    if (freq == 0)
        freq = ptcl->life;

    s32 localCnt = (s32)ptcl->cnt - 1;
    if (res->colorRepeatStartRand[kind]) localCnt += ptcl->rnd >> 6;
    localCnt %= freq;

    s32 sec1 = (res->colorSection1[kind] * freq) / 100;
    if (localCnt < sec1)
    {
        ptcl->color[kind].r = res->color[kind][0].r;
        ptcl->color[kind].g = res->color[kind][0].g;
        ptcl->color[kind].b = res->color[kind][0].b;
    }
    else
    {
        s32 sec2 = (res->colorSection2[kind] * freq) / 100;
        if (localCnt < sec2)
        {
            f32 t = (f32)(localCnt - sec1) / (f32)(sec2 - sec1);

            register f32 val1 = res->color[kind][1].r - res->color[kind][0].r;
            register f32 val2 = res->color[kind][1].g - res->color[kind][0].g;
            register f32 val3 = res->color[kind][1].b - res->color[kind][0].b;

            ptcl->color[kind].r = res->color[kind][0].r + val1 * t;
            ptcl->color[kind].g = res->color[kind][0].g + val2 * t;
            ptcl->color[kind].b = res->color[kind][0].b + val3 * t;
        }
        else
        {
            s32 sec3 = (res->colorSection3[kind] * freq) / 100;
            if (localCnt < sec3)
            {
                f32 t = (f32)(localCnt - sec2) / (f32)(sec3 - sec2);

                register f32 val1 = res->color[kind][2].r - res->color[kind][1].r;
                register f32 val2 = res->color[kind][2].g - res->color[kind][1].g;
                register f32 val3 = res->color[kind][2].b - res->color[kind][1].b;

                ptcl->color[kind].r = res->color[kind][1].r + val1 * t;
                ptcl->color[kind].g = res->color[kind][1].g + val2 * t;
                ptcl->color[kind].b = res->color[kind][1].b + val3 * t;
            }
            else
            {
                ptcl->color[kind].r = res->color[kind][2].r;
                ptcl->color[kind].g = res->color[kind][2].g;
                ptcl->color[kind].b = res->color[kind][2].b;
            }
        }
    }
}

u32 EmitterCalc::CalcSimpleParticleBehavior(EmitterInstance* __restrict e, PtclInstance* __restrict ptcl, CpuCore core)
{
    const SimpleEmitterData* __restrict res = e->res;

    register s32 cnt = (s32)ptcl->cnt;

    nw::math::MTX34* mrt  = ptcl->coordinateEmitterRT;
    nw::math::MTX34* msrt = ptcl->coordinateEmitterSRT;

    f32 old_pos_x = ptcl->pos.x;
    f32 old_pos_y = ptcl->pos.y;
    f32 old_pos_z = ptcl->pos.z;

    f32 invRatio = 1.0f - e->frameRate;

    if (e->followType == EFT_FOLLOW_TYPE_POS_ONLY)
    {
        ptcl->emitterSRT.m[0][3] = e->emitterSRT.m[0][3];
        ptcl->emitterSRT.m[1][3] = e->emitterSRT.m[1][3];
        ptcl->emitterSRT.m[2][3] = e->emitterSRT.m[2][3];
        ptcl->emitterRT .m[0][3] = e->emitterRT .m[0][3];
        ptcl->emitterRT .m[1][3] = e->emitterRT .m[1][3];
        ptcl->emitterRT .m[2][3] = e->emitterRT .m[2][3];
    }

    ptcl->pos.x += ptcl->vel.x * ptcl->dynamicsRnd * e->frameRate;
    ptcl->pos.y += ptcl->vel.y * ptcl->dynamicsRnd * e->frameRate;
    ptcl->pos.z += ptcl->vel.z * ptcl->dynamicsRnd * e->frameRate;

    if (e->behaviorFlag & EFT_EMITTER_BEHAVIOR_FLAG_AIR_REGIST)
    {
        f32 airRegist = res->airRegist + (1.0f - res->airRegist) * invRatio;
        ptcl->vel.x *= airRegist;
        ptcl->vel.y *= airRegist;
        ptcl->vel.z *= airRegist;
    }

    if (e->behaviorFlag & EFT_EMITTER_BEHAVIOR_FLAG_GRAVITY)
    {
        if (res->isWorldGravity)
        {
            ptcl->vel.x +=  res->gravity.x * e->frameRate * mrt->m[0][0] +
                            res->gravity.y * e->frameRate * mrt->m[1][0] +
                            res->gravity.z * e->frameRate * mrt->m[2][0];
            ptcl->vel.y +=  res->gravity.x * e->frameRate * mrt->m[0][1] +
                            res->gravity.y * e->frameRate * mrt->m[1][1] +
                            res->gravity.z * e->frameRate * mrt->m[2][1];
            ptcl->vel.z +=  res->gravity.x * e->frameRate * mrt->m[0][2] +
                            res->gravity.y * e->frameRate * mrt->m[1][2] +
                            res->gravity.z * e->frameRate * mrt->m[2][2];
        }
        else
        {
            ptcl->vel.x += res->gravity.x * e->frameRate;
            ptcl->vel.y += res->gravity.y * e->frameRate;
            ptcl->vel.z += res->gravity.z * e->frameRate;
        }
    }

    if (e->behaviorFlag & EFT_EMITTER_BEHAVIOR_FLAG_ALPHA_ANIM)
    {
        if (cnt <= ptcl->alphaAnim->alphaSec1)
            ptcl->alpha += ptcl->alphaAnim->alphaAddSec1 * e->frameRate;

        else if (cnt > ptcl->alphaAnim->alphaSec2)
            ptcl->alpha += ptcl->alphaAnim->alphaAddSec2 * e->frameRate;
    }

    if (e->behaviorFlag & EFT_EMITTER_BEHAVIOR_FLAG_SCALE_ANIM)
    {
        if (cnt <= ptcl->scaleAnim->scaleSec1)
        {
            ptcl->scale.x += ptcl->scaleAnim->scaleAddSec1.x * e->frameRate;
            ptcl->scale.y += ptcl->scaleAnim->scaleAddSec1.y * e->frameRate;
        }
        else if (cnt > ptcl->scaleAnim->scaleSec2)
        {
            ptcl->scale.x += ptcl->scaleAnim->scaleAddSec2.x * e->frameRate;
            ptcl->scale.y += ptcl->scaleAnim->scaleAddSec2.y * e->frameRate;
        }
    }

    if (e->behaviorFlag & EFT_EMITTER_BEHAVIOR_FLAG_ROTATE)
    {
        ptcl->rot.x += ptcl->rotVel.x * e->frameRate;
        ptcl->rot.y += ptcl->rotVel.y * e->frameRate;
        ptcl->rot.z += ptcl->rotVel.z * e->frameRate;
    }

    if (e->behaviorFlag & EFT_EMITTER_BEHAVIOR_FLAG_ROT_REGIST)
    {
        f32 rotRegist = res->rotRegist + (1.0f - res->rotRegist) * invRatio;
        ptcl->rotVel.x *= rotRegist;
        ptcl->rotVel.y *= rotRegist;
        ptcl->rotVel.z *= rotRegist;
    }

    if (e->behaviorFlag & EFT_EMITTER_BEHAVIOR_FLAG_COLOR0_ANIM)
        _interpolate3Colors(res, ptcl, EFT_COLOR_KIND_0);

    if (e->behaviorFlag & EFT_EMITTER_BEHAVIOR_FLAG_COLOR1_ANIM)
        _interpolate3Colors(res, ptcl, EFT_COLOR_KIND_1);

    if (e->behaviorFlag & EFT_EMITTER_BEHAVIOR_FLAG_TEXTURE0_PTN_ANIM)
        _calcTexPatAnim(ptcl, &res->textureData[EFT_TEXTURE_SLOT_0]);

    if (e->behaviorFlag & EFT_EMITTER_BEHAVIOR_FLAG_TEXTURE0_UV_ANIM)
    {
        ptcl->uvScroll.x += res->textureData[EFT_TEXTURE_SLOT_0].uvScroll.x * e->frameRate;
        ptcl->uvScroll.y += res->textureData[EFT_TEXTURE_SLOT_0].uvScroll.y * e->frameRate;
        ptcl->uvScale.x  += res->textureData[EFT_TEXTURE_SLOT_0].uvScale.x  * e->frameRate;
        ptcl->uvScale.y  += res->textureData[EFT_TEXTURE_SLOT_0].uvScale.y  * e->frameRate;
        ptcl->uvRotateZ  += res->textureData[EFT_TEXTURE_SLOT_0].uvRot      * e->frameRate;
    }

    if (e->behaviorFlag & EFT_EMITTER_BEHAVIOR_FLAG_TEXTURE1_CALC)
    {
        if (e->behaviorFlag & EFT_EMITTER_BEHAVIOR_FLAG_TEXTURE1_PTN_ANIM)
            _calcSubTexPatAnim(ptcl, &res->textureData[EFT_TEXTURE_SLOT_1]);

        if (e->behaviorFlag & EFT_EMITTER_BEHAVIOR_FLAG_TEXTURE1_UV_ANIM)
        {
            ptcl->uvSubScroll.x += res->textureData[EFT_TEXTURE_SLOT_1].uvScroll.x * e->frameRate;
            ptcl->uvSubScroll.y += res->textureData[EFT_TEXTURE_SLOT_1].uvScroll.y * e->frameRate;
            ptcl->uvSubScale.x  += res->textureData[EFT_TEXTURE_SLOT_1].uvScale.x  * e->frameRate;
            ptcl->uvSubScale.y  += res->textureData[EFT_TEXTURE_SLOT_1].uvScale.y  * e->frameRate;
            ptcl->uvSubRotateZ  += res->textureData[EFT_TEXTURE_SLOT_1].uvRot      * e->frameRate;
        }
    }

    ptcl->worldPos.x = msrt->f._00 * ptcl->pos.x + msrt->f._01 * ptcl->pos.y + msrt->f._02 * ptcl->pos.z + msrt->f._03;
    ptcl->worldPos.y = msrt->f._10 * ptcl->pos.x + msrt->f._11 * ptcl->pos.y + msrt->f._12 * ptcl->pos.z + msrt->f._13;
    ptcl->worldPos.z = msrt->f._20 * ptcl->pos.x + msrt->f._21 * ptcl->pos.y + msrt->f._22 * ptcl->pos.z + msrt->f._23;

    if (e->behaviorFlag & EFT_EMITTER_BEHAVIOR_FLAG_WLD_POSDIF)
    {
        #define POSDIFF_MIN_VEL 0.0001f
        nw::math::VEC3 vel;
        vel.x =  ptcl->pos.x - old_pos_x;
        vel.y =  ptcl->pos.y - old_pos_y;
        vel.z =  ptcl->pos.z - old_pos_z;

        if (std::fabs(vel.x) > POSDIFF_MIN_VEL ||
            std::fabs(vel.y) > POSDIFF_MIN_VEL ||
            std::fabs(vel.z) > POSDIFF_MIN_VEL)
        {
            ptcl->posDiff.x += vel.x - ptcl->posDiff.x;
            ptcl->posDiff.y += vel.y - ptcl->posDiff.y;
            ptcl->posDiff.z += vel.z - ptcl->posDiff.z;
        }

        ptcl->worldPosDiff.x =  msrt->m[0][0] * ptcl->posDiff.x * e->frameRate +
                                msrt->m[0][1] * ptcl->posDiff.y * e->frameRate +
                                msrt->m[0][2] * ptcl->posDiff.z * e->frameRate;
        ptcl->worldPosDiff.y =  msrt->m[1][0] * ptcl->posDiff.x * e->frameRate +
                                msrt->m[1][1] * ptcl->posDiff.y * e->frameRate +
                                msrt->m[1][2] * ptcl->posDiff.z * e->frameRate;
        ptcl->worldPosDiff.z =  msrt->m[2][0] * ptcl->posDiff.x * e->frameRate +
                                msrt->m[2][1] * ptcl->posDiff.y * e->frameRate +
                                msrt->m[2][2] * ptcl->posDiff.z * e->frameRate;
    }

    ptcl->cnt += e->frameRate;
    return 0;
}

u32 EmitterCalc::CalcComplexParticleBehavior(EmitterInstance* __restrict e, PtclInstance* __restrict ptcl, CpuCore core)
{
    const ComplexEmitterData* __restrict res = static_cast<const ComplexEmitterData*>(e->res);

    register s32 cnt = (s32)ptcl->cnt;

    nw::math::MTX34* mrt  = ptcl->coordinateEmitterRT;
    nw::math::MTX34* msrt = ptcl->coordinateEmitterSRT;

    f32 old_pos_x = ptcl->pos.x;
    f32 old_pos_y = ptcl->pos.y;
    f32 old_pos_z = ptcl->pos.z;

    f32 invRatio = 1.0f - e->frameRate;

    if (e->followType == EFT_FOLLOW_TYPE_POS_ONLY)
    {
        ptcl->emitterSRT.m[0][3] = e->emitterSRT.m[0][3];
        ptcl->emitterSRT.m[1][3] = e->emitterSRT.m[1][3];
        ptcl->emitterSRT.m[2][3] = e->emitterSRT.m[2][3];
        ptcl->emitterRT .m[0][3] = e->emitterRT .m[0][3];
        ptcl->emitterRT .m[1][3] = e->emitterRT .m[1][3];
        ptcl->emitterRT .m[2][3] = e->emitterRT .m[2][3];
    }

    ptcl->pos.x += ptcl->vel.x * ptcl->dynamicsRnd * e->frameRate;
    ptcl->pos.y += ptcl->vel.y * ptcl->dynamicsRnd * e->frameRate;
    ptcl->pos.z += ptcl->vel.z * ptcl->dynamicsRnd * e->frameRate;

    if (e->behaviorFlag & EFT_EMITTER_BEHAVIOR_FLAG_AIR_REGIST)
    {
        f32 airRegist = res->airRegist + (1.0f - res->airRegist) * invRatio;
        ptcl->vel.x *= airRegist;
        ptcl->vel.y *= airRegist;
        ptcl->vel.z *= airRegist;
    }

    if (e->behaviorFlag & EFT_EMITTER_BEHAVIOR_FLAG_GRAVITY)
    {
        if (res->isWorldGravity)
        {
            ptcl->vel.x +=  res->gravity.x * e->frameRate * mrt->m[0][0] +
                            res->gravity.y * e->frameRate * mrt->m[1][0] +
                            res->gravity.z * e->frameRate * mrt->m[2][0];
            ptcl->vel.y +=  res->gravity.x * e->frameRate * mrt->m[0][1] +
                            res->gravity.y * e->frameRate * mrt->m[1][1] +
                            res->gravity.z * e->frameRate * mrt->m[2][1];
            ptcl->vel.z +=  res->gravity.x * e->frameRate * mrt->m[0][2] +
                            res->gravity.y * e->frameRate * mrt->m[1][2] +
                            res->gravity.z * e->frameRate * mrt->m[2][2];
        }
        else
        {
            ptcl->vel.x += res->gravity.x * e->frameRate;
            ptcl->vel.y += res->gravity.y * e->frameRate;
            ptcl->vel.z += res->gravity.z * e->frameRate;
        }
    }

    if (e->behaviorFlag & EFT_EMITTER_BEHAVIOR_FLAG_ALPHA_ANIM)
    {
        if (cnt <= ptcl->alphaAnim->alphaSec1)
            ptcl->alpha += ptcl->alphaAnim->alphaAddSec1 * e->frameRate;

        else if (cnt > ptcl->alphaAnim->alphaSec2)
            ptcl->alpha += ptcl->alphaAnim->alphaAddSec2 * e->frameRate;
    }

    if (e->behaviorFlag & EFT_EMITTER_BEHAVIOR_FLAG_SCALE_ANIM)
    {
        if (cnt <= ptcl->scaleAnim->scaleSec1)
        {
            ptcl->scale.x += ptcl->scaleAnim->scaleAddSec1.x * e->frameRate;
            ptcl->scale.y += ptcl->scaleAnim->scaleAddSec1.y * e->frameRate;
        }
        else if (cnt > ptcl->scaleAnim->scaleSec2)
        {
            ptcl->scale.x += ptcl->scaleAnim->scaleAddSec2.x * e->frameRate;
            ptcl->scale.y += ptcl->scaleAnim->scaleAddSec2.y * e->frameRate;
        }
    }

    if (e->behaviorFlag & EFT_EMITTER_BEHAVIOR_FLAG_ROTATE)
    {
        ptcl->rot.x += ptcl->rotVel.x * e->frameRate;
        ptcl->rot.y += ptcl->rotVel.y * e->frameRate;
        ptcl->rot.z += ptcl->rotVel.z * e->frameRate;
    }

    if (e->behaviorFlag & EFT_EMITTER_BEHAVIOR_FLAG_ROT_REGIST)
    {
        f32 rotRegist = res->rotRegist + (1.0f - res->rotRegist) * invRatio;
        ptcl->rotVel *= rotRegist;
    }

    if (e->behaviorFlag & EFT_EMITTER_BEHAVIOR_FLAG_COLOR0_ANIM)
        _interpolate3Colors(res, ptcl, EFT_COLOR_KIND_0);

    if (e->behaviorFlag & EFT_EMITTER_BEHAVIOR_FLAG_COLOR1_ANIM)
        _interpolate3Colors(res, ptcl, EFT_COLOR_KIND_1);

    if (e->behaviorFlag & EFT_EMITTER_BEHAVIOR_FLAG_TEXTURE0_PTN_ANIM)
        _calcTexPatAnim(ptcl, &res->textureData[EFT_TEXTURE_SLOT_0]);

    if (e->behaviorFlag & EFT_EMITTER_BEHAVIOR_FLAG_TEXTURE0_UV_ANIM)
    {
        ptcl->uvScroll.x += res->textureData[EFT_TEXTURE_SLOT_0].uvScroll.x * e->frameRate;
        ptcl->uvScroll.y += res->textureData[EFT_TEXTURE_SLOT_0].uvScroll.y * e->frameRate;
        ptcl->uvScale.x  += res->textureData[EFT_TEXTURE_SLOT_0].uvScale.x  * e->frameRate;
        ptcl->uvScale.y  += res->textureData[EFT_TEXTURE_SLOT_0].uvScale.y  * e->frameRate;
        ptcl->uvRotateZ  += res->textureData[EFT_TEXTURE_SLOT_0].uvRot      * e->frameRate;
    }

    if (e->behaviorFlag & EFT_EMITTER_BEHAVIOR_FLAG_TEXTURE1_CALC)
    {
        if (e->behaviorFlag & EFT_EMITTER_BEHAVIOR_FLAG_TEXTURE1_PTN_ANIM)
            _calcSubTexPatAnim(ptcl, &res->textureData[EFT_TEXTURE_SLOT_1]);

        if (e->behaviorFlag & EFT_EMITTER_BEHAVIOR_FLAG_TEXTURE1_UV_ANIM)
        {
            ptcl->uvSubScroll.x += res->textureData[EFT_TEXTURE_SLOT_1].uvScroll.x * e->frameRate;
            ptcl->uvSubScroll.y += res->textureData[EFT_TEXTURE_SLOT_1].uvScroll.y * e->frameRate;
            ptcl->uvSubScale.x  += res->textureData[EFT_TEXTURE_SLOT_1].uvScale.x  * e->frameRate;
            ptcl->uvSubScale.y  += res->textureData[EFT_TEXTURE_SLOT_1].uvScale.y  * e->frameRate;
            ptcl->uvSubRotateZ  += res->textureData[EFT_TEXTURE_SLOT_1].uvRot      * e->frameRate;
        }
    }

    if (res->fluctuationFlg & EFT_FLUCTUATION_FALG_ENABLE)
        CalcFluctuation(e, ptcl);

    if (res->fieldFlg != 0)
        _calcField(res, e, ptcl);

    #define POSDIFF_MIN_VEL 0.0001f
    nw::math::VEC3 vel;
    vel.x =  ptcl->pos.x - old_pos_x;
    vel.y =  ptcl->pos.y - old_pos_y;
    vel.z =  ptcl->pos.z - old_pos_z;
    if (std::fabs(vel.x) > POSDIFF_MIN_VEL ||
        std::fabs(vel.y) > POSDIFF_MIN_VEL ||
        std::fabs(vel.z) > POSDIFF_MIN_VEL)
    {
        ptcl->posDiff.x += vel.x - ptcl->posDiff.x;
        ptcl->posDiff.y += vel.y - ptcl->posDiff.y;
        ptcl->posDiff.z += vel.z - ptcl->posDiff.z;
    }

    ptcl->worldPos.x = msrt->f._00 * ptcl->pos.x + msrt->f._01 * ptcl->pos.y + msrt->f._02 * ptcl->pos.z + msrt->f._03;
    ptcl->worldPos.y = msrt->f._10 * ptcl->pos.x + msrt->f._11 * ptcl->pos.y + msrt->f._12 * ptcl->pos.z + msrt->f._13;
    ptcl->worldPos.z = msrt->f._20 * ptcl->pos.x + msrt->f._21 * ptcl->pos.y + msrt->f._22 * ptcl->pos.z + msrt->f._23;

    if (e->behaviorFlag & EFT_EMITTER_BEHAVIOR_FLAG_WLD_POSDIF)
    {
        ptcl->worldPosDiff.x =  msrt->m[0][0] * ptcl->posDiff.x * e->frameRate +
                                msrt->m[0][1] * ptcl->posDiff.y * e->frameRate +
                                msrt->m[0][2] * ptcl->posDiff.z * e->frameRate;
        ptcl->worldPosDiff.y =  msrt->m[1][0] * ptcl->posDiff.x * e->frameRate +
                                msrt->m[1][1] * ptcl->posDiff.y * e->frameRate +
                                msrt->m[1][2] * ptcl->posDiff.z * e->frameRate;
        ptcl->worldPosDiff.z =  msrt->m[2][0] * ptcl->posDiff.x * e->frameRate +
                                msrt->m[2][1] * ptcl->posDiff.y * e->frameRate +
                                msrt->m[2][2] * ptcl->posDiff.z * e->frameRate;
    }

    ptcl->cnt += e->frameRate;
    return 0;
}

u32 EmitterCalc::CalcChildParticleBehavior(EmitterInstance* __restrict e, PtclInstance* __restrict ptcl, CpuCore core)
{
    const ComplexEmitterData* __restrict res  = static_cast<const ComplexEmitterData*>(ptcl->res);
    const ChildData*          __restrict cres = reinterpret_cast<const ChildData*>(res + 1);

    register s32 cnt = (s32)ptcl->cnt;

    nw::math::MTX34* mrt  = ptcl->coordinateEmitterRT;
    nw::math::MTX34* msrt = ptcl->coordinateEmitterSRT;

    f32 old_pos_x = ptcl->pos.x;
    f32 old_pos_y = ptcl->pos.y;
    f32 old_pos_z = ptcl->pos.z;

    f32 invRatio = 1.0f - e->frameRate;

    ptcl->pos.x += ptcl->vel.x * ptcl->dynamicsRnd * e->frameRate;
    ptcl->pos.y += ptcl->vel.y * ptcl->dynamicsRnd * e->frameRate;
    ptcl->pos.z += ptcl->vel.z * ptcl->dynamicsRnd * e->frameRate;

    //if (e->behaviorFlag & EFT_EMITTER_BEHAVIOR_FLAG_AIR_REGIST)
    //{
        f32 airRegist = cres->childAirRegist + (1.0f - res->airRegist) * invRatio; // No idea why it uses res->airRegist, mistake?
        ptcl->vel.x *= airRegist;
        ptcl->vel.y *= airRegist;
        ptcl->vel.z *= airRegist;
    //}

    //if (e->behaviorFlag & EFT_EMITTER_BEHAVIOR_FLAG_GRAVITY)
    //{
        if (res->isWorldGravity)
        {
            ptcl->vel.x +=  cres->childGravity.x * e->frameRate * mrt->m[0][0] +
                            cres->childGravity.y * e->frameRate * mrt->m[1][0] +
                            cres->childGravity.z * e->frameRate * mrt->m[2][0];
            ptcl->vel.y +=  cres->childGravity.x * e->frameRate * mrt->m[0][1] +
                            cres->childGravity.y * e->frameRate * mrt->m[1][1] +
                            cres->childGravity.z * e->frameRate * mrt->m[2][1];
            ptcl->vel.z +=  cres->childGravity.x * e->frameRate * mrt->m[0][2] +
                            cres->childGravity.y * e->frameRate * mrt->m[1][2] +
                            cres->childGravity.z * e->frameRate * mrt->m[2][2];
        }
        else
        {
            ptcl->vel.x += cres->childGravity.x * e->frameRate;
            ptcl->vel.y += cres->childGravity.y * e->frameRate;
            ptcl->vel.z += cres->childGravity.z * e->frameRate;
        }
    //}

    //if (e->behaviorFlag & EFT_EMITTER_BEHAVIOR_FLAG_ALPHA_ANIM)
    //{
        if (cnt < cres->childAlphaBaseFrame)
            ptcl->alpha += ptcl->alphaAnim->alphaAddSec1 * e->frameRate;

        else if (cnt >= cres->childAlphaStartFrame)
            ptcl->alpha += ptcl->alphaAnim->alphaAddSec2 * e->frameRate;
    //}

    //if (e->behaviorFlag & EFT_EMITTER_BEHAVIOR_FLAG_SCALE_ANIM)
    //{
        if (cnt >= cres->childScaleStartFrame)
        {
            ptcl->scale.x += ptcl->scaleAnim->scaleAddSec1.x * e->frameRate;
            ptcl->scale.y += ptcl->scaleAnim->scaleAddSec1.y * e->frameRate;
        }
    //}

    //if (e->behaviorFlag & EFT_EMITTER_BEHAVIOR_FLAG_ROTATE)
    //{
        ptcl->rot.x += ptcl->rotVel.x * e->frameRate;
        ptcl->rot.y += ptcl->rotVel.y * e->frameRate;
        ptcl->rot.z += ptcl->rotVel.z * e->frameRate;
    //}

    //if (e->behaviorFlag & EFT_EMITTER_BEHAVIOR_FLAG_ROT_REGIST)
    //{
        f32 rotRegist = cres->childRotRegist + (1.0f - cres->childRotRegist) * invRatio;
        ptcl->rotVel *= rotRegist;
    //}

    if (res->childFlg & EFT_CHILD_FLAG_IS_TEXTURE_PAT_ANIM)
        _calcChildTexPatAnim(res, cres, ptcl);

    if (res->childFlg & EFT_CHILD_FLAG_PARENT_FIELD)
        _calcField(res, e, ptcl);

    #define POSDIFF_MIN_VEL 0.0001f
    nw::math::VEC3 vel;
    vel.x =  ptcl->pos.x - old_pos_x;
    vel.y =  ptcl->pos.y - old_pos_y;
    vel.z =  ptcl->pos.z - old_pos_z;

    if (std::fabs(vel.x) > POSDIFF_MIN_VEL ||
        std::fabs(vel.y) > POSDIFF_MIN_VEL ||
        std::fabs(vel.z) > POSDIFF_MIN_VEL)
    {
        ptcl->posDiff.x += vel.x - ptcl->posDiff.x;
        ptcl->posDiff.y += vel.y - ptcl->posDiff.y;
        ptcl->posDiff.z += vel.z - ptcl->posDiff.z;
    }

    ptcl->worldPos.x = ptcl->emitterSRT.f._00 * ptcl->pos.x + ptcl->emitterSRT.f._01 * ptcl->pos.y + ptcl->emitterSRT.f._02 * ptcl->pos.z + ptcl->emitterSRT.f._03;
    ptcl->worldPos.y = ptcl->emitterSRT.f._10 * ptcl->pos.x + ptcl->emitterSRT.f._11 * ptcl->pos.y + ptcl->emitterSRT.f._12 * ptcl->pos.z + ptcl->emitterSRT.f._13;
    ptcl->worldPos.z = ptcl->emitterSRT.f._20 * ptcl->pos.x + ptcl->emitterSRT.f._21 * ptcl->pos.y + ptcl->emitterSRT.f._22 * ptcl->pos.z + ptcl->emitterSRT.f._23;

    //if (e->behaviorFlag & EFT_EMITTER_BEHAVIOR_FLAG_WLD_POSDIF)
    //{
        ptcl->worldPosDiff.x =  msrt->m[0][0] * ptcl->posDiff.x * e->frameRate +
                                msrt->m[0][1] * ptcl->posDiff.y * e->frameRate +
                                msrt->m[0][2] * ptcl->posDiff.z * e->frameRate;
        ptcl->worldPosDiff.y =  msrt->m[1][0] * ptcl->posDiff.x * e->frameRate +
                                msrt->m[1][1] * ptcl->posDiff.y * e->frameRate +
                                msrt->m[1][2] * ptcl->posDiff.z * e->frameRate;
        ptcl->worldPosDiff.z =  msrt->m[2][0] * ptcl->posDiff.x * e->frameRate +
                                msrt->m[2][1] * ptcl->posDiff.y * e->frameRate +
                                msrt->m[2][2] * ptcl->posDiff.z * e->frameRate;
    //}

    ptcl->cnt += e->frameRate;
    return 0;
}

void EmitterCalc::MakeParticleAttributeBuffer(PtclAttributeBuffer* __restrict attrBuffer, PtclInstance* __restrict ptcl, u32 shaderAttrFlag, f32 cameraOffset)
{
    PtclAttributeBuffer*    dst     = attrBuffer;
    PtclInstance*           src     = ptcl;

    register u32 attrFlag = shaderAttrFlag;

    const EmitterSet* set = src->emitter->emitterSet;

    dst->pWldPos.x = src->worldPos.x;
    dst->pWldPos.y = src->worldPos.y;
    dst->pWldPos.z = src->worldPos.z;
    dst->pWldPos.w = cameraOffset;

    dst->pScl.x = src->scale.x * set->mParticlScaleForCalc.x * src->fluctuationScale;
    dst->pScl.y = src->scale.y * set->mParticlScaleForCalc.y * src->fluctuationScale;
    dst->pScl.z = src->uvRotateZ;
    dst->pScl.w = src->uvSubRotateZ;

    dst->pColor0.x = src->color[EFT_COLOR_KIND_0].r;
    dst->pColor0.y = src->color[EFT_COLOR_KIND_0].g;
    dst->pColor0.z = src->color[EFT_COLOR_KIND_0].b;
    dst->pColor0.w = src->alpha * src->fluctuationAlpha;

    if (attrFlag & EFT_SHADER_ATTRIBUTE_HAS_COLOR1)
    {
        dst->pColor1.x = src->color[EFT_COLOR_KIND_1].r;
        dst->pColor1.y = src->color[EFT_COLOR_KIND_1].g;
        dst->pColor1.z = src->color[EFT_COLOR_KIND_1].b;
        dst->pColor1.w = src->alpha * src->fluctuationAlpha;
    }

    dst->pTexAnim.x = src->uvOffset.x + src->uvScroll.x;
    dst->pTexAnim.y = src->uvOffset.y - src->uvScroll.y;
    dst->pTexAnim.z = src->uvScale.x;
    dst->pTexAnim.w = src->uvScale.y;

    if (attrFlag & EFT_SHADER_ATTRIBUTE_HAS_WORLD_POS_DIFF)
    {
        dst->pWldPosDf.x = src->worldPosDiff.x;
        dst->pWldPosDf.y = src->worldPosDiff.y;
        dst->pWldPosDf.z = src->worldPosDiff.z;
    }

    if (attrFlag & EFT_SHADER_ATTRIBUTE_HAS_ROTATE)
    {
        dst->pRot.x = src->rot.x;
        dst->pRot.y = src->rot.y;
        dst->pRot.z = src->rot.z;
        dst->pRot.w = 0.0f;
    }

    if (attrFlag & EFT_SHADER_ATTRIBUTE_HAS_SUB_TEX_ANIM)
    {
        dst->pSubTexAnim.x = src->uvSubOffset.x + src->uvSubScroll.x;
        dst->pSubTexAnim.y = src->uvSubOffset.y - src->uvSubScroll.y;
        dst->pSubTexAnim.z = src->uvSubScale.x;
        dst->pSubTexAnim.w = src->uvSubScale.y;
    }

    if (attrFlag & EFT_SHADER_ATTRIBUTE_HAS_EMITTER_MATRIX)
    {
        dst->pEmtMat[0].x = src->coordinateEmitterRT->m[0][0];
        dst->pEmtMat[0].y = src->coordinateEmitterRT->m[0][1];
        dst->pEmtMat[0].z = src->coordinateEmitterRT->m[0][2];
        dst->pEmtMat[0].w = src->coordinateEmitterRT->m[0][3];
        dst->pEmtMat[1].x = src->coordinateEmitterRT->m[1][0];
        dst->pEmtMat[1].y = src->coordinateEmitterRT->m[1][1];
        dst->pEmtMat[1].z = src->coordinateEmitterRT->m[1][2];
        dst->pEmtMat[1].w = src->coordinateEmitterRT->m[1][3];
        dst->pEmtMat[2].x = src->coordinateEmitterRT->m[2][0];
        dst->pEmtMat[2].y = src->coordinateEmitterRT->m[2][1];
        dst->pEmtMat[2].z = src->coordinateEmitterRT->m[2][2];
        dst->pEmtMat[2].w = src->coordinateEmitterRT->m[2][3];
    }
}

} } // namespace nw::eft

#if EFT_IS_CAFE_WUT || !EFT_IS_CAFE
#pragma GCC diagnostic pop
#endif
