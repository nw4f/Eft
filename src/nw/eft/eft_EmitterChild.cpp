#include <nw/eft/eft_EmitterComplex.h>
#include <nw/eft/eft_EmitterSet.h>
#include <nw/eft/eft_Particle.h>
#include <nw/eft/eft_Renderer.h>
#include <nw/eft/eft_System.h>

namespace nw { namespace eft {

static inline f32 _initialize3v4kAnimWithMiddleAlpha(AlphaAnim* anim, const anim3v4Key* key, s32 maxTime, f32 middleAlpha)
{
    anim->alphaSec1 = (s32)(key->section1 * maxTime);
    anim->alphaSec2 = (s32)(key->section2 * maxTime);

    if (anim->alphaSec1 == 0)
        anim->alphaAddSec1 = 0.0f;

    else
        anim->alphaAddSec1 = (middleAlpha - key->init) / (f32)anim->alphaSec1;

    if (key->section2 == 1.0f)
        anim->alphaAddSec2 = 0.0f;

    else
    {
        f32 goal = key->init + key->diff21 + key->diff32;
        anim->alphaAddSec2 = (goal - middleAlpha) / (f32)(maxTime - anim->alphaSec2);
    }

    if (anim->alphaSec1 == 0)
        return middleAlpha;

    return key->init - anim->alphaAddSec1;
}

u32 EmitterComplexCalc::CalcChildParticle(EmitterInstance* emitter, CpuCore core, bool skipBehavior, bool skipMakeAttribute)
{
    emitter->childEntryNum = 0;
    if (emitter->childPtclNum == 0)
        return 0;

    System* eftSystem = emitter->emitterSet->mSystem;

    const ComplexEmitterData* res  = emitter->GetComplexEmitterData();
    const ChildData*          cres = emitter->GetChildData();

    if (!skipMakeAttribute)
    {
        emitter->childPtclAttributeBuffer = static_cast<PtclAttributeBuffer*>(
            eftSystem->GetRenderer(core)->AllocFromDoubleBuffer(sizeof(PtclAttributeBuffer) * emitter->childPtclNum));
        if (emitter->childPtclAttributeBuffer == NULL)
        {
            emitter->childEmitterDynamicUniformBlock = NULL;
            return 0;
        }

        emitter->childEmitterDynamicUniformBlock = MakeEmitterUniformBlock(emitter, core, cres);
        if (emitter->childEmitterDynamicUniformBlock == NULL)
        {
            emitter->childPtclAttributeBuffer = NULL;
            return 0;
        }
    }
    else
    {
        emitter->childPtclAttributeBuffer = NULL;
        emitter->childEmitterDynamicUniformBlock = NULL;
    }

    CustomActionParticleCalcCallback particleCB = mSys->GetCurrentCustomActionParticleCalcCallback(emitter);
    CustomActionParticleMakeAttributeCallback particleMakeAttrCB = mSys->GetCurrentCustomActionParticleMakeAttributeCallback(emitter);

    u32 shaderAttrFlag = emitter->childShader[EFT_SHADER_TYPE_NORMAL]->GetShaderAttributeFlag();

    f32 behaviorLoopF = emitter->cnt - emitter->preCnt;
    u32 behaviorLoopU = (u32)behaviorLoopF;
    f32 behaviorLoopD = behaviorLoopF - behaviorLoopU;
    bool behaviorLoop = false;
    if (emitter->frameRate > 1.0f)
        behaviorLoop  = true;

    PtclInstance* next = NULL;

    PtclInstance* ptcl = emitter->childHead;
    while (ptcl)
    {
        next = ptcl->next;

        if (!skipBehavior)
        {
            if (CalcParticleKillCondition(ptcl, core))
            {
                ptcl = next;
                continue;
            }

            if (res->childFlg & EFT_CHILD_FLAG_EMITTER_FOLLOW)
            {
                ptcl->emitterSRT = emitter->emitterSRT;
                ptcl->emitterRT  = emitter->emitterRT;
            }

            if (behaviorLoop)
            {
                for (u32 i = 0; i < behaviorLoopU; i++)
                    CalcChildParticleBehavior(emitter, ptcl);

                if (behaviorLoopD != 0.0f)
                    CalcChildParticleBehavior(emitter, ptcl, behaviorLoopD);
            }
            else
            {
                CalcChildParticleBehavior(emitter, ptcl, emitter->frameRate);
            }
        }

        if (particleCB)
        {
            CalcParticleCalcCallback(particleCB, emitter, ptcl, core, skipBehavior);

            if (ptcl->res == NULL)
            {
                ptcl = next;
                continue;
            }
        }

        if (!skipMakeAttribute)
        {
            ptcl->attributeBuffer = &emitter->childPtclAttributeBuffer[emitter->childEntryNum];
            MakeParticleAttributeBuffer(ptcl->attributeBuffer, ptcl, shaderAttrFlag);
            emitter->childEntryNum++;

            if (particleMakeAttrCB)
                CalcParticleMakeAttrCallback(particleMakeAttrCB, emitter, ptcl, core, skipBehavior);
        }

        ptcl = ptcl->next;
    }

    emitter->emitterBehaviorFlag |= EFT_EMITTER_BEHAVIOR_FLAG_IS_CALCULATED;

    return emitter->childEntryNum;
}

void EmitterComplexCalc::EmitChildParticle(EmitterInstance* e, PtclInstance* p)
{
    const ComplexEmitterData* __restrict res  = reinterpret_cast<const ComplexEmitterData*>(e->res);
    const ChildData         * __restrict cres = reinterpret_cast<const ChildData*>(res + 1);

    for (s32 i = 0; i < cres->childEmitRate; i++)
    {
        PtclInstance* child = mSys->AllocPtcl();
        if (child == NULL)
            break;

        child->res                  = res;
        child->cres                 = res;
        child->life                 = cres->childLife;
        child->emitter              = e;
        child->type                 = EFT_PTCL_TYPE_CHILD;
        child->complexParam->stripe = NULL;
        child->cnt                  = 0.0f;
        child->cntS                 = 0;
        child->rnd                  = e->rnd.GetU32Direct();
        child->runtimeUserData      = 0;

        child->fluctuationAlpha     = 1.0f;
        child->fluctuationScaleX    = 1.0f;
        child->fluctuationScaleY    = 1.0f;

        child->random[0] = e->rnd.GetF32();
        child->random[1] = e->rnd.GetF32();
        child->random[2] = e->rnd.GetF32();
        child->random[3] = e->rnd.GetF32();

        if (res->childFlg & EFT_CHILD_FLAG_VEL_INHERIT)
        {
            child->vel.x = p->posDiff.x * cres->childVelInheritRate;
            child->vel.y = p->posDiff.y * cres->childVelInheritRate;
            child->vel.z = p->posDiff.z * cres->childVelInheritRate;
        }
        else
        {
            child->vel = nw::math::VEC3::Zero();
        }

        nw::math::VEC3 rndVec3_0 = e->rnd.GetNormalizedVec3();
        nw::math::VEC3 rndVec3_1 = e->rnd.GetVec3();

        child->vel.x += rndVec3_0.x * cres->childFigureVel + rndVec3_1.x * cres->childRandVel.x;
        child->vel.y += rndVec3_0.y * cres->childFigureVel + rndVec3_1.y * cres->childRandVel.y;
        child->vel.z += rndVec3_0.z * cres->childFigureVel + rndVec3_1.z * cres->childRandVel.z;

        child->posDiff = child->vel;

        child->pos.x = rndVec3_0.x * cres->childInitPosRand + p->pos.x - child->vel.x;
        child->pos.y = rndVec3_0.y * cres->childInitPosRand + p->pos.y - child->vel.y;
        child->pos.z = rndVec3_0.z * cres->childInitPosRand + p->pos.z - child->vel.z;

        if (res->childFlg & EFT_CHILD_FLAG_COLOR0_INHERIT)
            child->color[EFT_COLOR_KIND_0] = p->color[EFT_COLOR_KIND_0];
        else
            child->color[EFT_COLOR_KIND_0] = nw::math::Vector4(cres->childColor0);

        if (res->childFlg & EFT_CHILD_FLAG_COLOR1_INHERIT)
            child->color[EFT_COLOR_KIND_1] = p->color[EFT_COLOR_KIND_1];
        else
            child->color[EFT_COLOR_KIND_1] = nw::math::Vector4(cres->childColor1);

        child->color[EFT_COLOR_KIND_1].a = 1.0f;

        if (res->childFlg & EFT_CHILD_FLAG_ROTATE_INHERIT)
        {
            if (cres->childRotType == EFT_ROT_TYPE_ROTX)
            {
                child->rot.x = p->rot.x;
                child->rot.y = 0.0f;
                child->rot.z = 0.0f;
            }
            else if (cres->childRotType == EFT_ROT_TYPE_ROTY)
            {
                child->rot.x = 0.0f;
                child->rot.y = p->rot.y;
                child->rot.z = 0.0f;
            }
            else if (cres->childRotType == EFT_ROT_TYPE_ROTZ)
            {
                child->rot.x = 0.0f;
                child->rot.y = 0.0f;
                child->rot.z = p->rot.z;
            }
            else
            {
                child->rot = p->rot;
            }
        }
        else
        {
            child->rot.x = cres->childInitRot.x + e->rnd.GetF32() * cres->childInitRotRand.x;
            child->rot.y = cres->childInitRot.y + e->rnd.GetF32() * cres->childInitRotRand.y;
            child->rot.z = cres->childInitRot.z + e->rnd.GetF32() * cres->childInitRotRand.z;
        }

        child->rotVel.x = cres->childRotVel.x + e->rnd.GetF32() * cres->childRotVelRand.x;
        child->rotVel.y = cres->childRotVel.y + e->rnd.GetF32() * cres->childRotVelRand.y;
        child->rotVel.z = cres->childRotVel.z + e->rnd.GetF32() * cres->childRotVelRand.z;

        if (res->childFlg & EFT_CHILD_FLAG_ROTATE_INHERIT)
        {
            if (cres->isRotDirRand[0] && e->rnd.GetF32() > 0.5f)
                child->rotVel.x *= -1.0f;

            if (cres->isRotDirRand[1] && e->rnd.GetF32() > 0.5f)
                child->rotVel.y *= -1.0f;

            if (cres->isRotDirRand[2] && e->rnd.GetF32() > 0.5f)
                child->rotVel.z *= -1.0f;
        }
        else
        {
            if (cres->isRotDirRand[0] && e->rnd.GetF32() > 0.5f)
            {
                child->rot.x *= -1.0f;
                child->rotVel.x *= -1.0f;
            }
            if (cres->isRotDirRand[1] && e->rnd.GetF32() > 0.5f)
            {
                child->rot.y *= -1.0f;
                child->rotVel.y *= -1.0f;
            }
            if (cres->isRotDirRand[2] && e->rnd.GetF32() > 0.5f)
            {
                child->rot.z *= -1.0f;
                child->rotVel.z *= -1.0f;
            }
        }

        s32 maxTime = child->life - 1;

        if (res->childFlg & EFT_CHILD_FLAG_ALPHA0_INHERIT)
        {
            if (cres->childAlphaCalcType[EFT_ALPHA_KIND_0] == EFT_ALPHA_CALC_TYPE_FIXED)
                child->alpha0 = p->alpha0 * p->fluctuationAlpha;

            else if (cres->childAlphaCalcType[EFT_ALPHA_KIND_0] == EFT_ALPHA_CALC_TYPE_3V4KEY)
                child->alpha0 = _initialize3v4kAnimWithMiddleAlpha(child->alphaAnim[EFT_ALPHA_KIND_0],
                                                                   &cres->childAlpha3v4kAnim[EFT_ALPHA_KIND_0],
                                                                   maxTime,
                                                                   p->alpha0 * p->fluctuationAlpha);
        }
        else
        {
            if (cres->childAlphaCalcType[EFT_ALPHA_KIND_0] == EFT_ALPHA_CALC_TYPE_FIXED)
                child->alpha0 = cres->childAlpha3v4kAnim[EFT_ALPHA_KIND_0].init + cres->childAlpha3v4kAnim[EFT_ALPHA_KIND_0].diff21;

            else if (cres->childAlphaCalcType[EFT_ALPHA_KIND_0] == EFT_ALPHA_CALC_TYPE_3V4KEY)
                child->alpha0 = _initialize3v4kAnim(child->alphaAnim[EFT_ALPHA_KIND_0], &cres->childAlpha3v4kAnim[EFT_ALPHA_KIND_0], maxTime);
        }

        if (res->childFlg & EFT_CHILD_FLAG_ALPHA1_INHERIT)
        {
            if (cres->childAlphaCalcType[EFT_ALPHA_KIND_1] == EFT_ALPHA_CALC_TYPE_FIXED)
                child->alpha1 = p->alpha1 * p->fluctuationAlpha;

            else if (cres->childAlphaCalcType[EFT_ALPHA_KIND_1] == EFT_ALPHA_CALC_TYPE_3V4KEY)
                child->alpha1 = _initialize3v4kAnimWithMiddleAlpha(child->alphaAnim[EFT_ALPHA_KIND_1],
                                                                   &cres->childAlpha3v4kAnim[EFT_ALPHA_KIND_1],
                                                                   maxTime,
                                                                   p->alpha1 * p->fluctuationAlpha);
        }
        else
        {
            if (cres->childAlphaCalcType[EFT_ALPHA_KIND_1] == EFT_ALPHA_CALC_TYPE_FIXED)
                child->alpha1 = cres->childAlpha3v4kAnim[EFT_ALPHA_KIND_1].init + cres->childAlpha3v4kAnim[EFT_ALPHA_KIND_1].diff21;

            else if (cres->childAlphaCalcType[EFT_ALPHA_KIND_1] == EFT_ALPHA_CALC_TYPE_3V4KEY)
                child->alpha1 = _initialize3v4kAnim(child->alphaAnim[EFT_ALPHA_KIND_1], &cres->childAlpha3v4kAnim[EFT_ALPHA_KIND_1], maxTime);
        }

        if (res->childFlg & EFT_CHILD_FLAG_SCALE_INHERIT)
        {
            f32 scaleRnd    = 1.0f - cres->childScaleRand *  e->rnd.GetF32();

            child->scale.x  = p->scale.x * cres->childScaleInheritRate * p->fluctuationScaleX * scaleRnd * e->emitterAnimValue[EFT_ANIM_PTCL_SX];
            child->scale.y  = p->scale.y * cres->childScaleInheritRate * p->fluctuationScaleY * scaleRnd * e->emitterAnimValue[EFT_ANIM_PTCL_SY];
        }
        else
        {
            f32 scaleRnd    = 1.0f - cres->childScaleRand *  e->rnd.GetF32();
            child->scale    = cres->childScale * scaleRnd;
        }

        f32 invLife = 1.0f / (child->life - cres->childScaleStartFrame);
        child->scaleAnim->scaleAddSec1.x = (cres->childScaleTarget.x - 1.0f) * invLife * child->scale.x;
        child->scaleAnim->scaleAddSec1.y = (cres->childScaleTarget.y - 1.0f) * invLife * child->scale.y;

        child->scaleVelY = 0.0f;

        if (!(res->childFlg & EFT_CHILD_FLAG_EMITTER_FOLLOW))
        {
            if (e->followType == EFT_FOLLOW_TYPE_ALL)
            {
                child->emitterRT            = e->emitterRT;
                child->emitterSRT           = e->emitterSRT;
                child->coordinateEmitterRT  = &e->emitterRT;
                child->coordinateEmitterSRT = &e->emitterSRT;
            }
            else
            {
                child->emitterRT            = p->emitterRT;
                child->emitterSRT           = p->emitterSRT;
                child->coordinateEmitterRT  = &p->emitterRT;
                child->coordinateEmitterSRT = &p->emitterSRT;
            }
        }
        else
        {
            child->coordinateEmitterRT  = &e->emitterRT;
            child->coordinateEmitterSRT = &e->emitterSRT;
        }

        child->rnd = e->rnd.GetU32Direct(); // Again... ?

        child->dynamicsRnd = 1.0f + cres->childDynamicsRandom - cres->childDynamicsRandom * e->rnd.GetF32() * 2.0f;

        CustomActionParticleEmitCallback particleEmitCB = mSys->GetCurrentCustomActionParticleEmitCallback(e);
        if (particleEmitCB)
        {
            ParticleEmitArg arg;
            arg.particle = child;

            if (!particleEmitCB(arg))
                return RemoveParticle(child, EFT_CPU_CORE_1);
        }

        AddChildPtclToList(e, child);
    }
}

} } // namespace nw::eft
