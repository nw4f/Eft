#include <nw/eft/eft_EmitterSet.h>
#include <nw/eft/eft_EmitterSimpleGpu.h>
#include <nw/eft/eft_System.h>
#include <nw/eft/eft_UniformBlock.h>

namespace nw { namespace eft {

void EmitterSimpleGpuCalc::CalcEmitter(EmitterInstance* emitter)
{
    if (emitter->ptclAttributeBufferGpu == NULL)
        return;

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

        emitter->fadeAlpha -= res->alphaAddInFade;
        if (emitter->fadeAlpha <= 0.0f)
            return mSys->KillEmitter(emitter);
    }
    else
    {
        isParticleEmit = true;

        emitter->fadeAlpha += res->alphaAddInFade;
        if (emitter->fadeAlpha > 1.0f)
            emitter->fadeAlpha = 1.0f;
    }

    register s32 localCnt = (s32)emitter->cnt - set->mStartFrame;
    f32          interval = emitter->emissionInterval * emitter->controller->mEmissionInterval;

    if (localCnt < res->emitEndFrame && isParticleEmit)
    {
        if (localCnt >= res->emitStartFrame)
        {
            if (!(emitter->emitterBehaviorFlag & EFT_EMITTER_BEHAVIOR_FLAG_IS_EMIT))
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
        if (res->isOnetime)
        {
            u32 maxLife = (u32)(res->emitEndFrame + res->ptclLife + 1);
            if (maxLife < (u32)localCnt)
                return mSys->KillEmitter(emitter);
        }
        else
        {
            emitter->fadeCnt += emitter->frameRate;
            if (res->ptclLife < emitter->fadeCnt)
                return mSys->KillEmitter(emitter);
        }
    }

emit_exit:
    emitter->emitterBehaviorFlag |= EFT_EMITTER_BEHAVIOR_FLAG_IS_SET_PREV_POS;
    emitter->emitDistPrevPos.Set(emitter->emitterRT._03, emitter->emitterRT._13, emitter->emitterRT._23);

    emitter->preCnt = emitter->cnt;
    emitter->cnt += emitter->frameRate;

    PtclInstance* ptcl = emitter->ptclHead;
    u32 start = emitter->gpuParticleBufferFillNum;
    while (ptcl)
    {
        PtclAttributeBufferGpu* buff = &emitter->ptclAttributeBufferGpu[emitter->gpuParticleBufferFillNum];

        buff->pWldPos.x         = ptcl->pos.x;
        buff->pWldPos.y         = ptcl->pos.y;
        buff->pWldPos.z         = ptcl->pos.z;

        buff->pWldPos.w         = ptcl->emitTime;

        buff->pScl.x            = ptcl->scale.x;
        buff->pScl.y            = ptcl->scale.y;

        buff->pVector.x         = ptcl->vel.x;
        buff->pVector.y         = ptcl->vel.y;
        buff->pVector.z         = ptcl->vel.z;
        buff->pVector.w         = ptcl->dynamicsRnd;

        buff->pRandom.x         = ptcl->random[0];
        buff->pRandom.y         = ptcl->random[1];
        buff->pRandom.z         = ptcl->random[2];
        buff->pRandom.w         = ptcl->life;

        buff->pRot.x            = ptcl->rot.x;
        buff->pRot.y            = ptcl->rot.y;
        buff->pRot.z            = ptcl->rot.z;
        buff->pRot.w            = 0.0f;

        *(f64*)&buff->pEmtMat[0].x = *(f64*)&ptcl->coordinateEmitterSRT->m[0][0];
        *(f64*)&buff->pEmtMat[0].z = *(f64*)&ptcl->coordinateEmitterSRT->m[0][2];
        *(f64*)&buff->pEmtMat[1].x = *(f64*)&ptcl->coordinateEmitterSRT->m[1][0];
        *(f64*)&buff->pEmtMat[1].z = *(f64*)&ptcl->coordinateEmitterSRT->m[1][2];
        *(f64*)&buff->pEmtMat[2].x = *(f64*)&ptcl->coordinateEmitterSRT->m[2][0];
        *(f64*)&buff->pEmtMat[2].z = *(f64*)&ptcl->coordinateEmitterSRT->m[2][2];

        emitter->gpuParticleBufferFillNum++;
        if (emitter->gpuParticleBufferFillNum == emitter->gpuParticleBufferNum)
        {
            DCFlushRange(&emitter->ptclAttributeBufferGpu[start], sizeof(PtclAttributeBufferGpu) * (emitter->gpuParticleBufferNum - start));
            emitter->gpuParticleBufferFillNum = 0;
            start = 0;
        }

        if (emitter->entryNum < emitter->gpuParticleBufferNum)
            emitter->entryNum++;

        PtclInstance* next = ptcl->next;
        RemoveParticle(ptcl, EFT_CPU_CORE_1);
        ptcl = next;
    }

    DCFlushRange(&emitter->ptclAttributeBufferGpu[start], sizeof(PtclAttributeBufferGpu) * (emitter->gpuParticleBufferFillNum - start));
    emitter->ptclNum = 1;
}

u32 EmitterSimpleGpuCalc::CalcParticle(EmitterInstance* emitter, CpuCore core, bool skipBehavior, bool skipMakeAttribute)
{
    if (skipMakeAttribute)
        return emitter->entryNum;

    emitter->emitterDynamicUniformBlock = MakeEmitterUniformBlock(emitter, core, NULL, skipBehavior);
    if (emitter->emitterDynamicUniformBlock == NULL)
        return 0;

    emitter->emitterBehaviorFlag |= EFT_EMITTER_BEHAVIOR_FLAG_IS_CALCULATED;

    return emitter->entryNum;
}

} } // namespace nw::eft
