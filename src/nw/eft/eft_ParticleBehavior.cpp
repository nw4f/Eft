#include <nw/eft/eft_Animation.h>
#include <nw/eft/eft_Emitter.h>
#include <nw/eft/eft_EmitterSet.h>
#include <nw/eft/eft_UniformBlock.h>

namespace nw { namespace eft {

#define POSDIFF_MIN_VEL 0.0001

EmitterCalc::ParticleAnimFunction EmitterCalc::mAnimFunctionsSclae[] = {
    EmitterCalc::CalcPtclAnimation,
    EmitterCalc::ptclAnim_Scale_4k3v,
    EmitterCalc::ptclAnim_Scale_8key,
};

EmitterCalc::ParticleAnimFunction EmitterCalc::mAnimFunctionsAlpha0[] = {
    EmitterCalc::CalcPtclAnimation,
    EmitterCalc::ptclAnim_Alpha0_4k3v,
    EmitterCalc::ptclAnim_Alpha0_8key,
};

EmitterCalc::ParticleAnimFunction EmitterCalc::mAnimFunctionsAlpha1[] = {
    EmitterCalc::CalcPtclAnimation,
    EmitterCalc::ptclAnim_Alpha1_4k3v,
    EmitterCalc::ptclAnim_Alpha1_8key,
};

EmitterCalc::ParticleAnimFunction EmitterCalc::mAnimFunctionsColor0[] = {
    EmitterCalc::CalcPtclAnimation,
    EmitterCalc::ptclAnim_Color0_4k3v,
    EmitterCalc::ptclAnim_Color0_8key,
};

EmitterCalc::ParticleAnimFunction EmitterCalc::mAnimFunctionsColor1[] = {
    EmitterCalc::CalcPtclAnimation,
    EmitterCalc::ptclAnim_Color1_4k3v,
    EmitterCalc::ptclAnim_Color1_8key,
};

// No division-by-zero checks whatsoever...

static inline void _interpolate3Colors(PtclInstance* ptcl, ColorKind kind, const SimpleEmitterData* res)
{
    s32 freq = ptcl->life / res->colorNumRepeat[kind];
    if (freq == 0)
        freq = ptcl->life;

    s32 localCnt = ptcl->cntS;
    if (res->colorRepeatStartRand[kind])
        localCnt += ptcl->rnd >> 6;
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
            f32 t = (f32)( localCnt - sec1 ) / (f32)( sec2 - sec1 );

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
                f32 t = (f32)( localCnt - sec2 ) / (f32)( sec3 - sec2 );

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

void EmitterCalc::CalcParticleBehavior(EmitterInstance* __restrict emitter, PtclInstance* __restrict ptcl, f32 frameRate)
{
    const SimpleEmitterData* __restrict res = ptcl->res;

    nw::math::MTX34* mrt  = ptcl->coordinateEmitterRT;

    f32 invRatio = 1.0f - frameRate;

    register u32 behaviorFlag = emitter->particleBehaviorFlag;

    if (emitter->followType == EFT_FOLLOW_TYPE_POS_ONLY)
    {
        ptcl->emitterSRT.m[0][3] = emitter->emitterSRT.m[0][3];
        ptcl->emitterSRT.m[1][3] = emitter->emitterSRT.m[1][3];
        ptcl->emitterSRT.m[2][3] = emitter->emitterSRT.m[2][3];
        ptcl->emitterRT .m[0][3] = emitter->emitterRT .m[0][3];
        ptcl->emitterRT .m[1][3] = emitter->emitterRT .m[1][3];
        ptcl->emitterRT .m[2][3] = emitter->emitterRT .m[2][3];
    }

    register f32 rndRatio = ptcl->dynamicsRnd * frameRate;
    ptcl->pos.x += ptcl->vel.x * rndRatio;
    ptcl->pos.y += ptcl->vel.y * rndRatio;
    ptcl->pos.z += ptcl->vel.z * rndRatio;

    if (behaviorFlag & EFT_PARTICLE_BEHAVIOR_FLAG_AIR_REGIST)
    {
        f32 airRegist = res->emitAirRegist + (1.0f - res->emitAirRegist) * invRatio;
        ptcl->vel.x *= airRegist;
        ptcl->vel.y *= airRegist;
        ptcl->vel.z *= airRegist;
    }

    if (behaviorFlag & EFT_PARTICLE_BEHAVIOR_FLAG_GRAVITY)
    {
        if (res->isWorldGravity)
        {
            f32 gravityScale = frameRate * emitter->emitterAnimValue[EFT_ANIM_GRAVITY];
            register f32 gravRatioX = res->gravity.x * gravityScale;
            register f32 gravRatioY = res->gravity.y * gravityScale;
            register f32 gravRatioZ = res->gravity.z * gravityScale;

            ptcl->vel.x += gravRatioX * mrt->m[0][0] + gravRatioY * mrt->m[1][0] + gravRatioZ * mrt->m[2][0];
            ptcl->vel.y += gravRatioX * mrt->m[0][1] + gravRatioY * mrt->m[1][1] + gravRatioZ * mrt->m[2][1];
            ptcl->vel.z += gravRatioX * mrt->m[0][2] + gravRatioY * mrt->m[1][2] + gravRatioZ * mrt->m[2][2];
        }
        else
        {
            f32 gravityScale = frameRate * emitter->emitterAnimValue[EFT_ANIM_GRAVITY];
            ptcl->vel.x += res->gravity.x * gravityScale;
            ptcl->vel.y += res->gravity.y * gravityScale;
            ptcl->vel.z += res->gravity.z * gravityScale;
        }
    }

    if (behaviorFlag & EFT_PARTICLE_BEHAVIOR_FLAG_ROTATE)
    {
        ptcl->rot.x += ptcl->rotVel.x * frameRate;
        ptcl->rot.y += ptcl->rotVel.y * frameRate;
        ptcl->rot.z += ptcl->rotVel.z * frameRate;
    }

    if (behaviorFlag & EFT_PARTICLE_BEHAVIOR_FLAG_ROT_REGIST)
    {
        f32 rotRegist = res->rotRegist + (1.0f - res->rotRegist) * invRatio;
        ptcl->rotVel *= rotRegist;
    }

    if (res->ptclAnimTypeAlpha0)
        mAnimFunctionsAlpha0[res->ptclAnimTypeAlpha0](emitter, ptcl, frameRate);

    if (res->ptclAnimTypeAlpha1)
        mAnimFunctionsAlpha1[res->ptclAnimTypeAlpha1](emitter, ptcl, frameRate);

    if (res->ptclAnimTypeScale)
        mAnimFunctionsSclae[res->ptclAnimTypeScale](emitter, ptcl, frameRate);

    if (res->ptclAnimTypeColor0)
        mAnimFunctionsColor0[res->ptclAnimTypeColor0](emitter, ptcl, frameRate);

    if (res->ptclAnimTypeColor1)
        mAnimFunctionsColor1[res->ptclAnimTypeColor1](emitter, ptcl, frameRate);

    if (behaviorFlag & EFT_PARTICLE_BEHAVIOR_FLAG_VELOCITY_APPLY_SCALE_Y)
    {
        f32 scalar = ptcl->vel.Length() + ptcl->posDiff.Length() * ptcl->dynamicsRnd;
        f32 power  = ptcl->res->scaleVelAddY * ptcl->dynamicsRnd;
        ptcl->scaleVelY = scalar * power;
    }
}

void EmitterCalc::CalcSimpleParticleBehavior(EmitterInstance* __restrict emitter, PtclInstance* __restrict ptcl, f32 frameRate)
{
    nw::math::MTX34* msrt = ptcl->coordinateEmitterSRT;

    f32 old_pos_x = ptcl->pos.x;
    f32 old_pos_y = ptcl->pos.y;
    f32 old_pos_z = ptcl->pos.z;

    CalcParticleBehavior(emitter, ptcl, frameRate);

    if (emitter->particleBehaviorFlag & EFT_PARTICLE_BEHAVIOR_FLAG_WLD_POSDIF && frameRate != 0.0f)
    {
        nw::math::VEC3 vel;
        vel.x =  ptcl->pos.x - old_pos_x;
        vel.y =  ptcl->pos.y - old_pos_y;
        vel.z =  ptcl->pos.z - old_pos_z;

        if (fabsf(vel.x) > POSDIFF_MIN_VEL || fabsf(vel.y) > POSDIFF_MIN_VEL || fabsf(vel.z) > POSDIFF_MIN_VEL)
        {
            ptcl->posDiff.x += vel.x - ptcl->posDiff.x;
            ptcl->posDiff.y += vel.y - ptcl->posDiff.y;
            ptcl->posDiff.z += vel.z - ptcl->posDiff.z;
        }

        ptcl->worldPosDiff.x = msrt->m[0][0] * ptcl->posDiff.x + msrt->m[0][1] * ptcl->posDiff.y + msrt->m[0][2] * ptcl->posDiff.z;
        ptcl->worldPosDiff.y = msrt->m[1][0] * ptcl->posDiff.x + msrt->m[1][1] * ptcl->posDiff.y + msrt->m[1][2] * ptcl->posDiff.z;
        ptcl->worldPosDiff.z = msrt->m[2][0] * ptcl->posDiff.x + msrt->m[2][1] * ptcl->posDiff.y + msrt->m[2][2] * ptcl->posDiff.z;
    }

    ptcl->worldPos.x = msrt->f._00 * ptcl->pos.x + msrt->f._01 * ptcl->pos.y + msrt->f._02 * ptcl->pos.z + msrt->f._03;
    ptcl->worldPos.y = msrt->f._10 * ptcl->pos.x + msrt->f._11 * ptcl->pos.y + msrt->f._12 * ptcl->pos.z + msrt->f._13;
    ptcl->worldPos.z = msrt->f._20 * ptcl->pos.x + msrt->f._21 * ptcl->pos.y + msrt->f._22 * ptcl->pos.z + msrt->f._23;

    ptcl->cnt += frameRate;
}

void EmitterCalc::CalcComplexParticleBehavior(EmitterInstance* __restrict emitter, PtclInstance* __restrict ptcl, f32 frameRate)
{
    const ComplexEmitterData* __restrict res = ptcl->cres;

    nw::math::MTX34* msrt = ptcl->coordinateEmitterSRT;

    f32 old_pos_x = ptcl->pos.x;
    f32 old_pos_y = ptcl->pos.y;
    f32 old_pos_z = ptcl->pos.z;

    CalcParticleBehavior(emitter, ptcl, frameRate);

    if (res->fluctuationFlg & EFT_FLUCTUATION_FALG_ENABLE)
        CalcFluctuation(emitter, ptcl);

    if (res->fieldFlg != 0)
        _calcField(res, emitter, ptcl, frameRate);

    if (frameRate != 0.0f && emitter->particleBehaviorFlag & EFT_PARTICLE_BEHAVIOR_FLAG_WLD_POSDIF)
    {
        nw::math::VEC3 vel;
        vel.x =  ptcl->pos.x - old_pos_x;
        vel.y =  ptcl->pos.y - old_pos_y;
        vel.z =  ptcl->pos.z - old_pos_z;

        if (fabsf(vel.x) > POSDIFF_MIN_VEL || fabsf(vel.y) > POSDIFF_MIN_VEL || fabsf(vel.z) > POSDIFF_MIN_VEL)
        {
            ptcl->posDiff.x += vel.x - ptcl->posDiff.x;
            ptcl->posDiff.y += vel.y - ptcl->posDiff.y;
            ptcl->posDiff.z += vel.z - ptcl->posDiff.z;
        }

        ptcl->worldPosDiff.x = msrt->m[0][0] * ptcl->posDiff.x + msrt->m[0][1] * ptcl->posDiff.y + msrt->m[0][2] * ptcl->posDiff.z;
        ptcl->worldPosDiff.y = msrt->m[1][0] * ptcl->posDiff.x + msrt->m[1][1] * ptcl->posDiff.y + msrt->m[1][2] * ptcl->posDiff.z;
        ptcl->worldPosDiff.z = msrt->m[2][0] * ptcl->posDiff.x + msrt->m[2][1] * ptcl->posDiff.y + msrt->m[2][2] * ptcl->posDiff.z;
    }

    ptcl->worldPos.x = msrt->f._00 * ptcl->pos.x + msrt->f._01 * ptcl->pos.y + msrt->f._02 * ptcl->pos.z + msrt->f._03;
    ptcl->worldPos.y = msrt->f._10 * ptcl->pos.x + msrt->f._11 * ptcl->pos.y + msrt->f._12 * ptcl->pos.z + msrt->f._13;
    ptcl->worldPos.z = msrt->f._20 * ptcl->pos.x + msrt->f._21 * ptcl->pos.y + msrt->f._22 * ptcl->pos.z + msrt->f._23;

    ptcl->cnt += frameRate;
}

void EmitterCalc::CalcChildParticleBehavior(EmitterInstance* e, PtclInstance* ptcl, f32 frameRate)
{
    const ComplexEmitterData* res  = ptcl->cres;
    const ChildData*          cres = e->GetChildData();

    nw::math::MTX34* mrt  = ptcl->coordinateEmitterRT;
    nw::math::MTX34* msrt = ptcl->coordinateEmitterSRT;

    f32 invRatio = 1.0f - frameRate;

    f32 old_pos_x = ptcl->pos.x;
    f32 old_pos_y = ptcl->pos.y;
    f32 old_pos_z = ptcl->pos.z;

    ptcl->pos.x += ptcl->vel.x * ptcl->dynamicsRnd * frameRate;
    ptcl->pos.y += ptcl->vel.y * ptcl->dynamicsRnd * frameRate;
    ptcl->pos.z += ptcl->vel.z * ptcl->dynamicsRnd * frameRate;

    if (e->childBehaviorFlag & EFT_PARTICLE_BEHAVIOR_FLAG_AIR_REGIST)
    {
        f32 airRegist = cres->childAirRegist + (1.0f - res->emitAirRegist) * invRatio; // No idea why it uses res->emitAirRegist, mistake?
        ptcl->vel.x *= airRegist;
        ptcl->vel.y *= airRegist;
        ptcl->vel.z *= airRegist;
    }

    if (e->childBehaviorFlag & EFT_PARTICLE_BEHAVIOR_FLAG_GRAVITY)
    {
        if (res->childFlg & EFT_CHILD_FLAG_WORLD_FIELD)
        {
            register f32 gravityScale = frameRate * e->emitterAnimValue[EFT_ANIM_GRAVITY_CHILD];
            register f32 gravRatioX = cres->childGravity.x * gravityScale;
            register f32 gravRatioY = cres->childGravity.y * gravityScale;
            register f32 gravRatioZ = cres->childGravity.z * gravityScale;

            ptcl->vel.x += gravRatioX * mrt->m[0][0] + gravRatioY * mrt->m[1][0] + gravRatioZ * mrt->m[2][0];
            ptcl->vel.y += gravRatioX * mrt->m[0][1] + gravRatioY * mrt->m[1][1] + gravRatioZ * mrt->m[2][1];
            ptcl->vel.z += gravRatioX * mrt->m[0][2] + gravRatioY * mrt->m[1][2] + gravRatioZ * mrt->m[2][2];
        }
        else
        {
            register f32 gravityScale = frameRate * e->emitterAnimValue[ EFT_ANIM_GRAVITY_CHILD ];
            ptcl->vel.x += cres->childGravity.x * gravityScale;
            ptcl->vel.y += cres->childGravity.y * gravityScale;
            ptcl->vel.z += cres->childGravity.z * gravityScale;
        }
    }

    AnimType alphaAnim0 = static_cast<AnimType>(cres->childAlphaCalcType[EFT_ALPHA_KIND_0]);
    AnimType alphaAnim1 = static_cast<AnimType>(cres->childAlphaCalcType[EFT_ALPHA_KIND_1]);

    if (alphaAnim0)
        mAnimFunctionsAlpha0[alphaAnim0](e, ptcl, frameRate);

    if (alphaAnim1)
        mAnimFunctionsAlpha1[alphaAnim1](e, ptcl, frameRate);

    if (e->childBehaviorFlag & EFT_PARTICLE_BEHAVIOR_FLAG_SCALE_ANIM)
    {
        if (ptcl->cnt >= cres->childScaleStartFrame)
        {
            ptcl->scale.x += ptcl->scaleAnim->scaleAddSec1.x * frameRate;
            ptcl->scale.y += ptcl->scaleAnim->scaleAddSec1.y * frameRate;
        }
    }

    if (e->childBehaviorFlag & EFT_PARTICLE_BEHAVIOR_FLAG_ROTATE)
    {
        ptcl->rot.x += ptcl->rotVel.x * frameRate;
        ptcl->rot.y += ptcl->rotVel.y * frameRate;
        ptcl->rot.z += ptcl->rotVel.z * frameRate;
    }

    if (e->childBehaviorFlag & EFT_PARTICLE_BEHAVIOR_FLAG_ROT_REGIST)
    {
        f32 rotRegist = cres->childRotRegist + (1.0f - cres->childRotRegist) * invRatio;
        ptcl->rotVel *= rotRegist;
    }

    if (res->childFlg & EFT_CHILD_FLAG_PARENT_FIELD)
        _calcField(res, e, ptcl, frameRate);

    ptcl->worldPos.x = ptcl->emitterSRT.f._00 * ptcl->pos.x + ptcl->emitterSRT.f._01 * ptcl->pos.y + ptcl->emitterSRT.f._02 * ptcl->pos.z + ptcl->emitterSRT.f._03;
    ptcl->worldPos.y = ptcl->emitterSRT.f._10 * ptcl->pos.x + ptcl->emitterSRT.f._11 * ptcl->pos.y + ptcl->emitterSRT.f._12 * ptcl->pos.z + ptcl->emitterSRT.f._13;
    ptcl->worldPos.z = ptcl->emitterSRT.f._20 * ptcl->pos.x + ptcl->emitterSRT.f._21 * ptcl->pos.y + ptcl->emitterSRT.f._22 * ptcl->pos.z + ptcl->emitterSRT.f._23;

    if (e->childBehaviorFlag & EFT_PARTICLE_BEHAVIOR_FLAG_WLD_POSDIF && frameRate != 0.0f)
    {
        nw::math::VEC3 vel;
        vel.x =  ptcl->pos.x - old_pos_x;
        vel.y =  ptcl->pos.y - old_pos_y;
        vel.z =  ptcl->pos.z - old_pos_z;

        if (fabsf(vel.x) > POSDIFF_MIN_VEL || fabsf(vel.y) > POSDIFF_MIN_VEL || fabsf(vel.z) > POSDIFF_MIN_VEL)
        {
            ptcl->posDiff.x += vel.x - ptcl->posDiff.x;
            ptcl->posDiff.y += vel.y - ptcl->posDiff.y;
            ptcl->posDiff.z += vel.z - ptcl->posDiff.z;
        }

        ptcl->worldPosDiff.x = msrt->m[0][0] * ptcl->posDiff.x + msrt->m[0][1] * ptcl->posDiff.y + msrt->m[0][2] * ptcl->posDiff.z;
        ptcl->worldPosDiff.y = msrt->m[1][0] * ptcl->posDiff.x + msrt->m[1][1] * ptcl->posDiff.y + msrt->m[1][2] * ptcl->posDiff.z;
        ptcl->worldPosDiff.z = msrt->m[2][0] * ptcl->posDiff.x + msrt->m[2][1] * ptcl->posDiff.y + msrt->m[2][2] * ptcl->posDiff.z;
    }

    ptcl->cnt += frameRate;
}

void EmitterCalc::MakeParticleAttributeBuffer(PtclAttributeBuffer* __restrict attrBuffer, PtclInstance* __restrict ptcl, u32 shaderAttrFlag)
{
    PtclAttributeBuffer* dst = attrBuffer;
    PtclInstance*        src = ptcl;

    register u32 attrFlag = shaderAttrFlag;

    *(f64*)&dst->pWldPos.x = *(f64*)&src->worldPos.x;
    dst->pWldPos.z         = src->worldPos.z;
    dst->pWldPos.w         = src->cnt;

    if (attrFlag & EFT_SHADER_ATTRIBUTE_HAS_SCALE)
    {
        dst->pScl.x = src->scale.x * src->fluctuationScaleX * ptcl->emitter->emitterSet->mParticlScaleForCalc.x;
        dst->pScl.y = src->scale.y * src->fluctuationScaleY * ptcl->emitter->emitterSet->mParticlScaleForCalc.y + src->scaleVelY;
        dst->pScl.z = 0.0f;
        dst->pScl.w = 0.0f;
    }

    if (attrFlag & EFT_SHADER_ATTRIBUTE_HAS_COLOR0)
    {
        *(f64*)&dst->pColor0.x = *(f64*)&src->color[EFT_COLOR_KIND_0].r;
        dst->pColor0.z = src->color[EFT_COLOR_KIND_0].b;
        dst->pColor0.w = src->alpha0 * src->fluctuationAlpha;
    }

    dst->pRandom.x = src->random[0];
    dst->pRandom.y = src->random[1];
    dst->pRandom.z = src->random[2];
    dst->pRandom.w = src->life;

    if (attrFlag & EFT_SHADER_ATTRIBUTE_HAS_VECTOR)
    {
        dst->pVector.x = src->vel.x;
        dst->pVector.y = src->vel.y;
        dst->pVector.z = src->vel.z;
        dst->pVector.w = src->dynamicsRnd;
    }

    if (attrFlag & EFT_SHADER_ATTRIBUTE_HAS_COLOR1)
    {
        *(f64*)&dst->pColor1.x = *(f64*)&src->color[EFT_COLOR_KIND_1].r;
        dst->pColor1.z = src->color[EFT_COLOR_KIND_1].b;
        dst->pColor1.w = src->alpha1 * src->fluctuationAlpha;
    }

    if (attrFlag & EFT_SHADER_ATTRIBUTE_HAS_ROTATE)
    {
        *(f64*)&dst->pRot.x = *(f64*)&src->rot.x;
        dst->pRot.z         = src->rot.z;
        dst->pRot.w         = 0.0f;
    }

    if (attrFlag & EFT_SHADER_ATTRIBUTE_HAS_WORLD_POS_DIFF)
    {
        *(f64*)&dst->pWldPosDf.x = *(f64*)&src->worldPosDiff.x;
        dst->pWldPosDf.z         = src->worldPosDiff.z;
        dst->pWldPosDf.w         = 1.0f;
    }

    if (attrFlag & EFT_SHADER_ATTRIBUTE_HAS_EMITTER_MATRIX)
    {
        *(f64*)&dst->pEmtMat[0].x = *(f64*)&src->coordinateEmitterRT->m[0][0];
        *(f64*)&dst->pEmtMat[0].z = *(f64*)&src->coordinateEmitterRT->m[0][2];
        *(f64*)&dst->pEmtMat[1].x = *(f64*)&src->coordinateEmitterRT->m[1][0];
        *(f64*)&dst->pEmtMat[1].z = *(f64*)&src->coordinateEmitterRT->m[1][2];
        *(f64*)&dst->pEmtMat[2].x = *(f64*)&src->coordinateEmitterRT->m[2][0];
        *(f64*)&dst->pEmtMat[2].z = *(f64*)&src->coordinateEmitterRT->m[2][2];

        if (ptcl->vel.x == src->coordinateEmitterRT->m[0][1] &&
            ptcl->vel.z == src->coordinateEmitterRT->m[2][1])
        {
            #define EMITTER_UP_MIN_VEL 0.0001f
            dst->pEmtMat[2].y += EMITTER_UP_MIN_VEL;
        }
    }
}

void EmitterCalc::ptclAnim_Scale_8key(EmitterInstance* __restrict e, PtclInstance* __restrict ptcl, f32 frameRate)
{
    KeyFrameAnim* animX = e->particleAnimKey[EFT_ANIM_8KEY_OFFSET(EFT_ANIM_8KEY_PARTICLE_SCALE_X)];
    KeyFrameAnim* animY = e->particleAnimKey[EFT_ANIM_8KEY_OFFSET(EFT_ANIM_8KEY_PARTICLE_SCALE_Y)];

    f32 time = _calcParticleAnimTime(animX, ptcl, EFT_ANIM_8KEY_PARTICLE_SCALE_X);
    u32 index = CalcAnimKeyFrameIndex(animX, time);

    ptcl->scale.x = CalcAnimKeyFrameSimple(animX, time, index) * ptcl->res->baseScale.x * e->emitterAnimValue[EFT_ANIM_PTCL_SX];
    ptcl->scale.y = CalcAnimKeyFrameSimple(animY, time, index) * ptcl->res->baseScale.y * e->emitterAnimValue[EFT_ANIM_PTCL_SY];

    if (e->res->scaleRand.x == e->res->scaleRand.y)
    {
        f32 scaleRndX  = 1.0f - e->res->scaleRand.x * ptcl->random[0];
        ptcl->scale.x *= scaleRndX;
        ptcl->scale.y *= scaleRndX;
    }
    else
    {
        f32 scaleRndX  = 1.0f - e->res->scaleRand.x * ptcl->random[0];
        f32 scaleRndY  = 1.0f - e->res->scaleRand.y * ptcl->random[1];
        ptcl->scale.x *= scaleRndX;
        ptcl->scale.y *= scaleRndY;
    }
}

void EmitterCalc::ptclAnim_Scale_4k3v(EmitterInstance* __restrict e, PtclInstance* __restrict ptcl, f32 frameRate)
{
    if (ptcl->cnt <= ptcl->scaleAnim->scaleSec1)
    {
        ptcl->scale.x += ptcl->scaleAnim->scaleAddSec1.x * frameRate;
        ptcl->scale.y += ptcl->scaleAnim->scaleAddSec1.y * frameRate;
    }
    else if (ptcl->cnt > ptcl->scaleAnim->scaleSec2)
    {
        ptcl->scale.x += ptcl->scaleAnim->scaleAddSec2.x * frameRate;
        ptcl->scale.y += ptcl->scaleAnim->scaleAddSec2.y * frameRate;
    }
}

void EmitterCalc::ptclAnim_Color0_8key(EmitterInstance* __restrict e, PtclInstance* __restrict ptcl, f32 frameRate)
{
    KeyFrameAnim* animR = e->particleAnimKey[EFT_ANIM_8KEY_OFFSET(EFT_ANIM_8KEY_PARTICLE_COLOR0_R)];
    KeyFrameAnim* animG = e->particleAnimKey[EFT_ANIM_8KEY_OFFSET(EFT_ANIM_8KEY_PARTICLE_COLOR0_G)];
    KeyFrameAnim* animB = e->particleAnimKey[EFT_ANIM_8KEY_OFFSET(EFT_ANIM_8KEY_PARTICLE_COLOR0_B)];

    f32 time = _calcParticleAnimTime(animR, ptcl, EFT_ANIM_8KEY_PARTICLE_COLOR0_R);
    u32 index = CalcAnimKeyFrameIndex(animR, time);
    ptcl->color[0].r = CalcAnimKeyFrameSimple(animR, time, index);
    ptcl->color[0].g = CalcAnimKeyFrameSimple(animG, time, index);
    ptcl->color[0].b = CalcAnimKeyFrameSimple(animB, time, index);
}

void EmitterCalc::ptclAnim_Color1_8key(EmitterInstance* __restrict e, PtclInstance* __restrict ptcl, f32 frameRate)
{
    KeyFrameAnim* animR = e->particleAnimKey[EFT_ANIM_8KEY_OFFSET(EFT_ANIM_8KEY_PARTICLE_COLOR1_R)];
    KeyFrameAnim* animG = e->particleAnimKey[EFT_ANIM_8KEY_OFFSET(EFT_ANIM_8KEY_PARTICLE_COLOR1_G)];
    KeyFrameAnim* animB = e->particleAnimKey[EFT_ANIM_8KEY_OFFSET(EFT_ANIM_8KEY_PARTICLE_COLOR1_B)];

    f32 time = _calcParticleAnimTime(animR, ptcl, EFT_ANIM_8KEY_PARTICLE_COLOR1_R);
    u32 index = CalcAnimKeyFrameIndex(animR, time);
    ptcl->color[1].r = CalcAnimKeyFrameSimple(animR, time, index);
    ptcl->color[1].g = CalcAnimKeyFrameSimple(animG, time, index);
    ptcl->color[1].b = CalcAnimKeyFrameSimple(animB, time, index);
}

void EmitterCalc::ptclAnim_Color0_4k3v(EmitterInstance* __restrict e, PtclInstance* __restrict ptcl, f32 frameRate)
{
    _interpolate3Colors(ptcl, EFT_COLOR_KIND_0, ptcl->res);
}

void EmitterCalc::ptclAnim_Color1_4k3v(EmitterInstance* __restrict e, PtclInstance* __restrict ptcl, f32 frameRate)
{
    _interpolate3Colors(ptcl, EFT_COLOR_KIND_1, ptcl->res);
}

void EmitterCalc::ptclAnim_Alpha0_8key(EmitterInstance* __restrict e, PtclInstance* __restrict ptcl, f32 frameRate)
{
    KeyFrameAnim* anim = e->particleAnimKey[EFT_ANIM_8KEY_OFFSET(EFT_ANIM_8KEY_PARTICLE_ALPHA0)];

    f32 time = _calcParticleAnimTime(anim, ptcl, EFT_ANIM_8KEY_PARTICLE_ALPHA0);
    u32 index = CalcAnimKeyFrameIndex(anim, time);

    ptcl->alpha0 = CalcAnimKeyFrameSimple(anim, time, index);
}

void EmitterCalc::ptclAnim_Alpha0_4k3v(EmitterInstance* __restrict e, PtclInstance* __restrict ptcl, f32 frameRate)
{
    if (ptcl->cnt <= ptcl->alphaAnim[EFT_ALPHA_KIND_0]->alphaSec1)
        ptcl->alpha0 += ptcl->alphaAnim[EFT_ALPHA_KIND_0]->alphaAddSec1 * frameRate;

    else if (ptcl->cnt > ptcl->alphaAnim[EFT_ALPHA_KIND_0]->alphaSec2)
        ptcl->alpha0 += ptcl->alphaAnim[EFT_ALPHA_KIND_0]->alphaAddSec2 * frameRate;
}

void EmitterCalc::ptclAnim_Alpha1_8key(EmitterInstance* __restrict e, PtclInstance* __restrict ptcl, f32 frameRate)
{
    KeyFrameAnim* anim = e->particleAnimKey[EFT_ANIM_8KEY_OFFSET(EFT_ANIM_8KEY_PARTICLE_ALPHA1)];

    f32 time = _calcParticleAnimTime(anim, ptcl, EFT_ANIM_8KEY_PARTICLE_ALPHA1);
    u32 index = CalcAnimKeyFrameIndex(anim, time);

    ptcl->alpha1 = CalcAnimKeyFrameSimple(anim, time, index);
}

void EmitterCalc::ptclAnim_Alpha1_4k3v(EmitterInstance* __restrict e, PtclInstance* __restrict ptcl, f32 frameRate)
{
    if (ptcl->cnt <= ptcl->alphaAnim[EFT_ALPHA_KIND_1]->alphaSec1)
        ptcl->alpha1 += ptcl->alphaAnim[EFT_ALPHA_KIND_1]->alphaAddSec1 * frameRate;

    else if (ptcl->cnt > ptcl->alphaAnim[EFT_ALPHA_KIND_1]->alphaSec2)
        ptcl->alpha1 += ptcl->alphaAnim[EFT_ALPHA_KIND_1]->alphaAddSec2 * frameRate;
}

} } // namespace nw::eft
