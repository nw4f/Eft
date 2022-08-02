#include <nw/eft/eft_Animation.h>
#include <nw/eft/eft_EmitterSet.h>
#include <nw/eft/eft_EmitterSimple.h>
#include <nw/eft/eft_Renderer.h>
#include <nw/eft/eft_Shader.h>
#include <nw/eft/eft_System.h>
#include <nw/eft/eft_UniformBlock.h>

namespace nw { namespace eft {

void EmitterCalc::ApplyAnim(EmitterInstance* emitter)
{
    for (u32 i = 0; i < emitter->emitterAnimNum; i++)
    {
        AnimKeyFrameKey* keys = _getKeyFramAnimKeys( emitter->emitterAnimKey[i] );
        AnimKeyFrameKey* keyE = &keys[emitter->emitterAnimKey[i]->keyNum - 1];
        u32              loop = emitter->emitterAnimKey[i]->isLoop;

        if (emitter->cnt < keyE->x || loop)
            emitter->emitterAnimValue[emitter->emitterAnimKey[i]->target] = CalcAnimKeyFrame(emitter->emitterAnimKey[i], emitter->cnt);

        else
            emitter->emitterAnimValue[emitter->emitterAnimKey[i]->target] = keyE->y;
    }

    if (emitter->emitterBehaviorFlag & EFT_EMITTER_BEHAVIOR_FLAG_MATRIX_ANIM)
    {
        nw::math::VEC3 scale;
        nw::math::VEC3 rotat;
        nw::math::VEC3 trans;
        nw::math::VEC3 scaleOne;
        scaleOne.Set(1.0f, 1.0f, 1.0f);

        scale.Set(emitter->emitterAnimValue[EFT_ANIM_EM_SX], emitter->emitterAnimValue[EFT_ANIM_EM_SY], emitter->emitterAnimValue[EFT_ANIM_EM_SZ]);
        rotat.Set(emitter->emitterAnimValue[EFT_ANIM_EM_RX], emitter->emitterAnimValue[EFT_ANIM_EM_RY], emitter->emitterAnimValue[EFT_ANIM_EM_RZ]);
        trans.Set(emitter->emitterAnimValue[EFT_ANIM_EM_TX], emitter->emitterAnimValue[EFT_ANIM_EM_TY], emitter->emitterAnimValue[EFT_ANIM_EM_TZ]);

        nw::math::MTX34MakeSRT(&emitter->animValEmitterSRT, scale,    rotat, trans);
        nw::math::MTX34MakeSRT(&emitter->animValEmitterRT,  scaleOne, rotat, trans);
    }
}

void EmitterSimpleCalc::EmitSameDistance(const SimpleEmitterData* __restrict res, EmitterInstance* e)
{
    if (!(e->emitterBehaviorFlag & EFT_EMITTER_BEHAVIOR_FLAG_IS_SET_PREV_POS))
    {
        mEmitFunctions[res->volumeType](e);
    }
    else
    {
        nw::math::VEC3 prevPos(e->emitDistPrevPos.x, e->emitDistPrevPos.y, e->emitDistPrevPos.z);
        nw::math::VEC3 currPos(e->emitterRT._03, e->emitterRT._13, e->emitterRT._23);
        nw::math::VEC3 move(prevPos - currPos);

        f32 length = move.Length();

        f32 vessel = e->emitDistVessel;

        f32 virtualLength = length;
        if (virtualLength < res->emitDistMargin)
            virtualLength = 0.0f;

        if (virtualLength == 0.0f)
            virtualLength = res->emitDistMin;
        else if (virtualLength < res->emitDistMin)
            virtualLength = length * res->emitDistMin / length;
        else if (res->emitDistMax < virtualLength)
            virtualLength = length * res->emitDistMax / length;

        vessel += virtualLength;

        s32 count = 0;
        if (res->emitDistUnit != 0.0)
        {
            count = static_cast<s32>(vessel / res->emitDistUnit);
        }

        for (s32 j = 0; j < count; j++)
        {
            vessel -= res->emitDistUnit;

            f32 ratio = 0.0f;
            if (virtualLength != 0.0f)
                ratio = vessel / virtualLength;

            nw::math::VEC3 pos;
            pos = currPos * (1.0f - ratio) + prevPos * ratio;

            e->emitterRT._03  = pos.x;
            e->emitterRT._13  = pos.y;
            e->emitterRT._23  = pos.z;
            e->emitterSRT._03 = pos.x;
            e->emitterSRT._13 = pos.y;
            e->emitterSRT._23 = pos.z;

            mEmitFunctions[res->volumeType](e);

            // No idea why this is done inside the loop and not after it
            e->emitterRT._03  = currPos.x;
            e->emitterRT._13  = currPos.y;
            e->emitterRT._23  = currPos.z;
            e->emitterSRT._03 = currPos.x;
            e->emitterSRT._13 = currPos.y;
            e->emitterSRT._23 = currPos.z;
        }

        e->emitDistVessel = vessel;
    }
}

void EmitterSimpleCalc::CalcEmitter(EmitterInstance* emitter)
{
    const SimpleEmitterData* __restrict res = emitter->res;
    const EmitterSet*        __restrict set = emitter->emitterSet;
    bool                     isParticleEmit = false;

    if (emitter->emitterAnimNum > 0)
        ApplyAnim(emitter);

    emitter->emitterRT .SetMult(emitter->emitterSet->mRT,  emitter->animValEmitterRT );
    emitter->emitterSRT.SetMult(emitter->emitterSet->mSRT, emitter->animValEmitterSRT);

    if (emitter->cnt == 0.0f)
        emitter->emitDistPrevPos.Set(emitter->emitterRT._03, emitter->emitterRT._13, emitter->emitterRT._23);

    EmitterMatrixSetArg arg;
    arg.emitter = emitter;
    CustomActionEmitterMatrixSetCallback emitterMSetCB = mSys->GetCurrentCustomActionEmitterMatrixSetCallback(emitter);
    if (emitterMSetCB)
        emitterMSetCB(arg);

    if (set->IsFadeRequest())
    {
        isParticleEmit = !res->isStopEmitInFade;

        emitter->fadeAlpha -= res->alphaAddInFade * emitter->frameRate;
        if (emitter->fadeAlpha <= 0.0f)
            return mSys->KillEmitter(emitter);
    }
    else
    {
        isParticleEmit = true;

        emitter->fadeAlpha += res->alphaAddInFade * emitter->frameRate;
        if (emitter->fadeAlpha > 1.0f)
            emitter->fadeAlpha = 1.0f;
    }

    register s32 localCnt = (s32)emitter->cnt - set->mStartFrame;
    f32          prevCnt  = emitter->cnt - emitter->frameRate;
    f32          interval = emitter->emissionInterval * emitter->controller->mEmissionInterval;
    const bool   isOneTimeEmitter = (emitter->emitterBehaviorFlag & EFT_EMITTER_BEHAVIOR_FLAG_IS_EMIT) == EFT_EMITTER_BEHAVIOR_FLAG_IS_EMIT;

    if (localCnt < res->emitEndFrame && isParticleEmit ||
        localCnt >= res->emitEndFrame && prevCnt < res->emitEndFrame && isParticleEmit && !isOneTimeEmitter)
    {
        if (localCnt >= res->emitStartFrame)
        {
            if (!isOneTimeEmitter)
            {
                mEmitFunctions[res->volumeType](emitter);
                emitter->preEmitCnt = emitter->cnt;

                if (interval == 0.0f)
                    goto emit_exit;
            }

            if (!res->isEmitDistEnabled)
            {
                f32 loopf    = 0.0f;
                s32 cnt      = (s32)emitter->cnt;
                s32 pcnt     = (s32)emitter->preCnt;
                s32 ecnt     = (s32)emitter->emitCnt;

                if ((s32)interval != 0)
                {
                    if (emitter->frameRate >= 1.0f)
                    {
                        if (ecnt >= interval)
                        {
                            loopf               = 1.0f;
                            emitter->emitCnt    = 0.0f;
                        }
                        else
                        {
                            emitter->emitCnt += emitter->frameRate;
                        }
                    }
                    else
                    {
                        if (emitter->emitCnt >= interval && cnt != pcnt)
                        {
                            loopf               = 1.0f;
                            emitter->emitCnt    = 0.0f;
                            emitter->emitSaving = 0.0f;
                            emitter->preEmitCnt = emitter->cnt;
                        }
                        else
                        {
                            emitter->emitCnt += emitter->frameRate;
                        }
                    }
                }
                else
                {
                    if (emitter->frameRate >= 1.0f)
                        loopf = 1.0f;

                    else
                    {
                        loopf = emitter->cnt - emitter->preEmitCnt + emitter->emitSaving;

                        if (emitter->emitSaving >= interval)
                            emitter->emitSaving -= interval;

                        emitter->emitSaving    += emitter->cnt - emitter->preEmitCnt - (s32)loopf;
                        emitter->emitCnt        = 0.0f;
                        emitter->preEmitCnt     = emitter->cnt;

                        if (res->ptclLife == 1 && emitter->frameRate == 1.0f)
                            loopf = 1.0f;
                    }
                }

                s32 loop = (s32)loopf;

                for (s32 i = 0; i < loop; i++)
                {
                    if (set->mNumParticleEmissionPoint > 0)
                    {
                        for (s32 j = 0; j < set->mNumParticleEmissionPoint; j++)
                        {
                            PtclInstance* saveHead = emitter->ptclHead;

                            mEmitFunctions[res->volumeType](emitter);

                            PtclInstance* ptcl = emitter->ptclHead;
                            while (ptcl != saveHead)
                            {
                                ptcl->pos.x += set->mParticleEmissionPoints[j].x;
                                ptcl->pos.y += set->mParticleEmissionPoints[j].y;
                                ptcl->pos.z += set->mParticleEmissionPoints[j].z;

                                ptcl = ptcl->next;
                            }
                        }
                    }
                    else
                    {
                        mEmitFunctions[res->volumeType](emitter);
                    }
                }

                emitter->emissionInterval = res->emitStep + emitter->rnd.GetS32(res->emitStepRnd);

                if (loop != 0 && emitter->res->isUpdateEmitterMatrixByEmit)
                {
                    emitter->UpdateEmitterInfoByEmit();
                    if (emitterMSetCB)
                        emitterMSetCB(arg);
                }
            }
            else
            {
                EmitSameDistance(res, emitter);
            }
        }
    }
    else
    {
        if (emitter->ptclHead == NULL && emitter->childHead == NULL)
            return mSys->KillEmitter(emitter);
    }

emit_exit:
    emitter->emitterBehaviorFlag |= EFT_EMITTER_BEHAVIOR_FLAG_IS_SET_PREV_POS;
    emitter->emitDistPrevPos.Set(emitter->emitterRT._03, emitter->emitterRT._13, emitter->emitterRT._23);

    emitter->preCnt = emitter->cnt;
    emitter->cnt   += emitter->frameRate;
}

u32 EmitterSimpleCalc::CalcParticle(EmitterInstance* emitter, CpuCore core, bool skipBehavior, bool skipMakeAttribute)
{
    emitter->entryNum = 0;
    if (emitter->ptclNum == 0)
        return 0;

    System* eftSystem = emitter->emitterSet->mSystem;
    PtclAttributeBuffer* ptclAttrBuffer = NULL;

    if (!skipMakeAttribute)
    {
        emitter->ptclAttributeBuffer = static_cast<PtclAttributeBuffer*>(
            eftSystem->GetRenderer(core)->AllocFromDoubleBuffer(sizeof(PtclAttributeBuffer) * emitter->ptclNum));
        if (emitter->ptclAttributeBuffer == NULL)
        {
            emitter->emitterDynamicUniformBlock = NULL;
            return 0;
        }

        emitter->emitterDynamicUniformBlock = MakeEmitterUniformBlock(emitter, core);
        if (emitter->emitterDynamicUniformBlock == NULL)
        {
            emitter->ptclAttributeBuffer = NULL;
            return 0;
        }

        ptclAttrBuffer = emitter->ptclAttributeBuffer;
    }
    else
    {
        emitter->ptclAttributeBuffer = NULL;
        emitter->emitterDynamicUniformBlock = NULL;
    }

    PtclInstance* ptcl = emitter->ptclHead;
    u32 shaderAttrFlag = emitter->shader[EFT_SHADER_TYPE_NORMAL]->GetShaderAttributeFlag();

    CustomActionParticleCalcCallback particleCB = mSys->GetCurrentCustomActionParticleCalcCallback(emitter);
    CustomActionParticleMakeAttributeCallback particleMakeAttrCB = mSys->GetCurrentCustomActionParticleMakeAttributeCallback(emitter);

    f32 behaviorLoopF = emitter->cnt - emitter->preCnt;
    u32 behaviorLoopU = (u32)behaviorLoopF;
    f32 behaviorLoopD = behaviorLoopF - behaviorLoopU;
    bool behaviorLoop = false;
    if (emitter->frameRate > 1.0f)
        behaviorLoop  = true;

    PtclInstance* next = NULL;

    bool bComplex = false;
    if (behaviorLoop)       bComplex = true;
    if (particleCB)         bComplex = true;
    if (particleMakeAttrCB) bComplex = true;
    if (skipBehavior)       bComplex = true;
    if (skipMakeAttribute)  bComplex = true;

    if (bComplex)
    {
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

                if (behaviorLoop)
                {
                    for (u32 i = 0; i < behaviorLoopU; i++)
                        CalcSimpleParticleBehavior(emitter, ptcl);

                    if (behaviorLoopD != 0.0f)
                        CalcSimpleParticleBehavior(emitter, ptcl, behaviorLoopD);
                }
                else
                {
                    CalcSimpleParticleBehavior(emitter, ptcl, emitter->frameRate);
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
            }

            if (!skipMakeAttribute)
            {
                CalcParticleMakeAttr(emitter, ptcl, ptclAttrBuffer, shaderAttrFlag);
                ptclAttrBuffer++;

                if (particleMakeAttrCB)
                    CalcParticleMakeAttrCallback(particleMakeAttrCB, emitter, ptcl, core, skipBehavior);
            }

            ptcl = next;
        }
    }
    else
    {
        while (ptcl)
        {
            next = ptcl->next;

            if (!CalcParticleKillCondition(ptcl, core))
            {
                CalcSimpleParticleBehavior(emitter, ptcl, emitter->frameRate);

                CalcParticleMakeAttr(emitter, ptcl, ptclAttrBuffer, shaderAttrFlag);
                ptclAttrBuffer++;
            }

            ptcl = next;
        }
    }

    emitter->emitterBehaviorFlag |= EFT_EMITTER_BEHAVIOR_FLAG_IS_CALCULATED;

    return emitter->entryNum;
}

EmitterDynamicUniformBlock* EmitterSimpleCalc::MakeEmitterUniformBlock(EmitterInstance* emitter, CpuCore core, const ChildData* childData, bool skipBehavior)
{
    System* eftSystem = emitter->emitterSet->mSystem;

    EmitterDynamicUniformBlock* ubo = NULL;

    ubo = static_cast<EmitterDynamicUniformBlock*>(
        eftSystem->GetRenderer(core)->AllocFromDoubleBuffer(sizeof(EmitterDynamicUniformBlock)));
    if (ubo == NULL)
        return NULL;

    nw::ut::Color4f setColor = emitter->emitterSet->GetColor();
    if (childData == NULL)
    {
        setColor.r *= emitter->res->colorScale;
        setColor.g *= emitter->res->colorScale;
        setColor.b *= emitter->res->colorScale;
    }
    else
    {
        if (emitter->GetComplexEmitterData()->childFlg & EFT_CHILD_FLAG_COLOR_SCALE_INHERIT)
		{
            setColor.r *= emitter->res->colorScale;
            setColor.g *= emitter->res->colorScale;
            setColor.b *= emitter->res->colorScale;
		}
	    else
	    {
            setColor.r *= childData->childColorScale;
            setColor.g *= childData->childColorScale;
            setColor.b *= childData->childColorScale;
	    }
    }

    ubo->emitterColor0.x        = setColor.r * emitter->emitterAnimValue[EFT_ANIM_COLOR0_R];
    ubo->emitterColor0.y        = setColor.g * emitter->emitterAnimValue[EFT_ANIM_COLOR0_G];
    ubo->emitterColor0.z        = setColor.b * emitter->emitterAnimValue[EFT_ANIM_COLOR0_B];
    ubo->emitterColor0.w        = setColor.a * emitter->emitterAnimValue[EFT_ANIM_ALPHA] * emitter->fadeAlpha;

    ubo->emitterColor1.x        = setColor.r * emitter->emitterAnimValue[EFT_ANIM_COLOR1_R];
    ubo->emitterColor1.y        = setColor.g * emitter->emitterAnimValue[EFT_ANIM_COLOR1_G];
    ubo->emitterColor1.z        = setColor.b * emitter->emitterAnimValue[EFT_ANIM_COLOR1_B];
    ubo->emitterColor1.w        = setColor.a * emitter->emitterAnimValue[EFT_ANIM_ALPHA] * emitter->fadeAlpha;

    nw::math::VEC2 emsetSacle = emitter->emitterSet->GetParticleScaleForCalc();
    ubo->emitterParam0.x        = emitter->cnt;
    ubo->emitterParam0.y        = emsetSacle.x;
    ubo->emitterParam0.z        = emsetSacle.y;
    ubo->emitterParam0.w        = emitter->frameRate;

    if (skipBehavior)
    {
        ubo->emitterParam1.x    = 0.0f;
        ubo->emitterParam1.y    = 0.0f;
    }
    else
    {
        ubo->emitterParam1.x    = 1.0f;
        ubo->emitterParam1.y    = 1.0f / eftSystem->GetScreenNum((u8)emitter->groupID); // Nintendo intentionally added the "u8" cast
    }

    ubo->emitterParam1.z        = 0.0f;
    ubo->emitterParam1.w        = 0.0f;

    ubo->emitterMatrix          = nw::math::MTX44( emitter->emitterSRT );
    ubo->emitterMatrixRT        = nw::math::MTX44( emitter->emitterRT );

    GX2EndianSwap(ubo, sizeof(EmitterDynamicUniformBlock));

    return ubo;
}

} } // namespace nw::eft
