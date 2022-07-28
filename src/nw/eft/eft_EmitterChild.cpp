#include <nw/eft/eft_EmitterComplex.h>
#include <nw/eft/eft_EmitterSet.h>
#include <nw/eft/eft_Particle.h>
#include <nw/eft/eft_Renderer.h>
#include <nw/eft/eft_System.h>
#include <nw/eft/eft_UniformBlock.h>

namespace nw { namespace eft {

u32 EmitterComplexCalc::CalcChildParticle(EmitterInstance* emitter, CpuCore core, bool skipBehavior, bool skipMakeAttribute)
{
    System* eftSystem = emitter->emitterSet->mSystem;

    if (!skipMakeAttribute)
    {
        emitter->childPtclAttributeBuffer = static_cast<PtclAttributeBuffer*>(eftSystem->GetRenderer(core)->AllocFromDoubleBuffer(sizeof(PtclAttributeBuffer) * emitter->childPtclNum));
        if (emitter->childPtclAttributeBuffer == NULL)
            return 0;

        emitter->childEmitterDynamicUniformBlock = static_cast<EmitterDynamicUniformBlock*>(eftSystem->GetRenderer(core)->AllocFromDoubleBuffer(sizeof(EmitterDynamicUniformBlock)));
        if (emitter->childEmitterDynamicUniformBlock == NULL)
        {
            emitter->ptclAttributeBuffer = NULL; // NOT childPtclAttributeBuffer... bug?
            return 0;
        }

        nw::ut::FloatColor setColor = emitter->emitterSet->GetColor();
        setColor.r *= emitter->res->colorScale;
        setColor.g *= emitter->res->colorScale;
        setColor.b *= emitter->res->colorScale;

        emitter->childEmitterDynamicUniformBlock->emitterColor0.x = setColor.r * emitter->emitAnimValue[EFT_ANIM_COLOR0_R];
        emitter->childEmitterDynamicUniformBlock->emitterColor0.y = setColor.g * emitter->emitAnimValue[EFT_ANIM_COLOR0_G];
        emitter->childEmitterDynamicUniformBlock->emitterColor0.z = setColor.b * emitter->emitAnimValue[EFT_ANIM_COLOR0_B];
        emitter->childEmitterDynamicUniformBlock->emitterColor0.w = setColor.a * emitter->emitAnimValue[EFT_ANIM_ALPHA] * emitter->fadeAlpha;

        emitter->childEmitterDynamicUniformBlock->emitterColor1.x = setColor.r * emitter->emitAnimValue[EFT_ANIM_COLOR1_R];
        emitter->childEmitterDynamicUniformBlock->emitterColor1.y = setColor.g * emitter->emitAnimValue[EFT_ANIM_COLOR1_G];
        emitter->childEmitterDynamicUniformBlock->emitterColor1.z = setColor.b * emitter->emitAnimValue[EFT_ANIM_COLOR1_B];
        emitter->childEmitterDynamicUniformBlock->emitterColor1.w = setColor.a * emitter->emitAnimValue[EFT_ANIM_ALPHA] * emitter->fadeAlpha;

        GX2EndianSwap(emitter->childEmitterDynamicUniformBlock, sizeof(EmitterDynamicUniformBlock));
    }
    else
    {
        emitter->childPtclAttributeBuffer = NULL;
        emitter->childEmitterDynamicUniformBlock = NULL;
    }

    emitter->childEntryNum = 0;

    const ComplexEmitterData* res = static_cast<const ComplexEmitterData*>(emitter->res);

    UserDataParticleCalcCallback particleCB = mSys->GetCurrentUserDataParticleCalcCallback(emitter);
    UserDataParticleMakeAttributeCallback particleMakeAttrCB = mSys->GetCurrentUserDataParticleMakeAttributeCallback(emitter);
    ParticleCalcArg arg;

    PtclInstance* ptcl = emitter->childHead;

    while (ptcl)
    {
        if (ptcl->res)
        {
            if (!skipBehavior)
            {
                if ((s32)ptcl->cnt >= ptcl->life || (ptcl->life == 1 && ptcl->cnt != 0.0f))
                {
                    RemoveParticle(emitter, ptcl, core);
                    ptcl = ptcl->next;
                    continue;
                }

                if (res->childFlg & EFT_CHILD_FLAG_EMITTER_FOLLOW)
                {
                    ptcl->emitterSRT = emitter->emitterSRT;
                    ptcl->emitterRT = emitter->emitterRT;
                }

                CalcChildParticleBehavior(emitter, ptcl, core);
            }

            if (particleCB)
            {
                arg.emitter = emitter;
                arg.ptcl = ptcl;
                arg.core = core;
                arg.skipBehavior = skipBehavior;
                particleCB(arg);
            }

            if (!skipMakeAttribute)
            {
                MakeParticleAttributeBuffer(&emitter->childPtclAttributeBuffer[emitter->childEntryNum], ptcl, emitter->childShaderArrtFlag, 0.0f);
                ptcl->attributeBuffer     = &emitter->childPtclAttributeBuffer[emitter->childEntryNum];
                emitter->childEntryNum++;

                if (particleMakeAttrCB)
                {
                    arg.emitter = emitter;
                    arg.ptcl = ptcl;
                    arg.core = core;
                    arg.skipBehavior = skipBehavior;
                    particleMakeAttrCB(arg);
                }
            }
        }
        ptcl = ptcl->next;
    }

    emitter->isCalculated = true;
    return emitter->childEntryNum;
}

void EmitterComplexCalc::EmitChildParticle(EmitterInstance* e, PtclInstance* p)
{
    const ComplexEmitterData * __restrict res  = static_cast<const ComplexEmitterData*>(e->res);
    const ChildData          * __restrict cres = reinterpret_cast<const ChildData*>(res + 1);

    for (s32 i = 0; i < cres->childEmitRate; i++)
    {
        PtclInstance* child = mSys->AllocPtcl(EFT_PTCL_TYPE_CHILD);
        if (child)
        {
            f32 baseAlpha;

            child->res      = res;
            child->life     = cres->childLife;
            child->stripe   = NULL;
            child->emitter  = e;

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

            if (res->childFlg & EFT_CHILD_FLAG_ALPHA_INHERIT)
                baseAlpha = p->alpha * p->fluctuationAlpha;
            else
                baseAlpha = cres->childAlpha;

            if (res->childFlg & EFT_CHILD_FLAG_SCALE_INHERIT)
            {
                f32 scaleRnd = 1.0f - cres->childScaleRand * e->rnd.GetF32();

                child->scale.x = p->scale.x * cres->childScaleInheritRate * p->fluctuationScale * scaleRnd * e->emitAnimValue[EFT_ANIM_PTCL_SX];
                child->scale.y = p->scale.y * cres->childScaleInheritRate * p->fluctuationScale * scaleRnd * e->emitAnimValue[EFT_ANIM_PTCL_SY];
            }
            else
            {
                f32 scaleRnd = 1.0f - cres->childScaleRand * e->rnd.GetF32();

                child->scale = cres->childScale * scaleRnd;
            }

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

            if (res->childFlg & EFT_CHILD_FLAG_COLOR0_INHERIT)
            {
                child->color[EFT_COLOR_KIND_0] = p->color[EFT_COLOR_KIND_0];
            }
            else
            {
                child->color[EFT_COLOR_KIND_0] = nw::math::Vector4(cres->childColor0);
            }

            if (res->childFlg & EFT_CHILD_FLAG_COLOR1_INHERIT)
            {
                child->color[EFT_COLOR_KIND_1] = p->color[EFT_COLOR_KIND_1];
            }
            else
            {
                child->color[EFT_COLOR_KIND_1] = nw::math::Vector4(cres->childColor1);
            }

            child->rotVel.x = cres->childRotVel.x + e->rnd.GetF32() * cres->childRotVelRand.x;
            child->rotVel.y = cres->childRotVel.y + e->rnd.GetF32() * cres->childRotVelRand.y;
            child->rotVel.z = cres->childRotVel.z + e->rnd.GetF32() * cres->childRotVelRand.z;

            child->uvScroll.x = 0.0f;
            child->uvScroll.y = 0.0f;
            child->uvRotateZ  = 0;
            child->uvScale.x  = 1.0f;
            child->uvScale.y  = 1.0f;

            if (cres->childAlphaBaseFrame == 0)
            {
                child->alphaAnim->alphaAddSec1 = 0.0f;
                child->alpha                   = baseAlpha;
            }
            else
            {
                child->alphaAnim->alphaAddSec1 = (baseAlpha - cres->childAlphaInit) / (f32)cres->childAlphaBaseFrame;
                child->alpha                   = cres->childAlphaInit;
            }

            f32 invLife = 1.0f / (child->life - cres->childAlphaStartFrame);
            child->alphaAnim->alphaAddSec2 = (cres->childAlphaTarget - baseAlpha) * invLife;

            invLife     = 1.0f / (child->life - cres->childScaleStartFrame);
            child->scaleAnim->scaleAddSec1.x = (cres->childScaleTarget.x - 1.0f) * invLife * child->scale.x;
            child->scaleAnim->scaleAddSec1.y = (cres->childScaleTarget.y - 1.0f) * invLife * child->scale.y;

            child->fluctuationAlpha = 1.0f;
            child->fluctuationScale = 1.0f;

            if (cres->childNumTexPat <= 1)
            {
                child->uvOffset.x  = 0.0f;
                child->uvOffset.y  = 0.0f;
            }
            else
            {
                s32 no   = e->rnd.GetS32(cres->childNumTexPat);
                s32 no_x = no % cres->childNumTexDivX;
                s32 no_y = no / cres->childNumTexDivX;

                register f32 no_x_f = (f32)no_x;
                register f32 no_y_f = (f32)no_y;

                child->uvOffset.x = cres->childTexUScale * no_x_f;
                child->uvOffset.y = cres->childTexVScale * no_y_f;
            }

            math::VEC3 rndVec3_0 = e->rnd.GetNormalizedVec3();
            math::VEC3 rndVec3_1 = e->rnd.GetVec3();

            child->vel.x += rndVec3_0.x * cres->childFigureVel + rndVec3_1.x * cres->childRandVel.x;
            child->vel.y += rndVec3_0.y * cres->childFigureVel + rndVec3_1.y * cres->childRandVel.y;
            child->vel.z += rndVec3_0.z * cres->childFigureVel + rndVec3_1.z * cres->childRandVel.z;

            // Nintendo forgot to set child->posDiff

            child->pos.x = rndVec3_0.x * cres->childInitPosRand + p->pos.x;
            child->pos.y = rndVec3_0.y * cres->childInitPosRand + p->pos.y;
            child->pos.z = rndVec3_0.z * cres->childInitPosRand + p->pos.z;

            child->cnt = 0.0f;
            child->rnd = e->rnd.GetU32Direct();

            if (!(res->childFlg & EFT_CHILD_FLAG_EMITTER_FOLLOW))
            {
                if (e->followType == EFT_FOLLOW_TYPE_ALL)
                {
                    child->emitterRT = e->emitterRT;
                    child->emitterSRT = e->emitterSRT;
                    child->coordinateEmitterRT = &e->emitterRT;
                    child->coordinateEmitterSRT = &e->emitterSRT;
                }
                else
                {
                    child->emitterRT = p->emitterRT;
                    child->emitterSRT = p->emitterSRT;
                    child->coordinateEmitterRT = &p->emitterRT;
                    child->coordinateEmitterSRT = &p->emitterSRT;
                }
            }
            else
            {
                child->coordinateEmitterRT = &e->emitterRT;
                child->coordinateEmitterSRT = &e->emitterSRT;
            }

            child->rnd = e->rnd.GetU32Direct(); // Again... ?

            child->dynamicsRnd = 1.0f + cres->childDynamicsRandom - cres->childDynamicsRandom * e->rnd.GetF32() * 2.0f;

            child->runtimeUserData = 0;

            UserDataParticleEmitCallback particleEmitCB = mSys->GetCurrentUserDataParticleEmitCallback(e);
            if (particleEmitCB)
            {
                ParticleEmitArg arg;
                arg.particle = child;

                if (!particleEmitCB(arg))
                    return RemoveParticle(e, child, EFT_CPU_CORE_1);
            }

            AddChildPtclToList(e, child);
        }
    }
}

} } // namespace nw::eft
