#include <nw/eft/eft_EmitterComplex.h>
#include <nw/eft/eft_EmitterSet.h>
#include <nw/eft/eft_Shader.h>
#include <nw/eft/eft_System.h>
#include <nw/eft/eft_UniformBlock.h>

namespace nw { namespace eft {

void EmitterComplexCalc::CalcEmitter(EmitterInstance* emitter)
{
    EmitterSimpleCalc::CalcEmitter(emitter);
}

void EmitterComplexCalc::CalcStripe(EmitterInstance* emitter, PtclInstance* ptcl, const StripeData* stripeData, const ComplexEmitterData* cres, CpuCore core, bool skipMakeAttribute)
{
    System* eftSystem = emitter->emitterSet->mSystem;

    register s32 cnt = ptcl->cntS - 1;

    PtclStripe* stripe = ptcl->complexParam->stripe;
    if (stripe)
    {
        stripe->uvParam[EFT_TEXTURE_SLOT_0].scroll.x += cres->textureData[EFT_TEXTURE_SLOT_0].uvScroll.x * emitter->frameRate;
        stripe->uvParam[EFT_TEXTURE_SLOT_0].scroll.y += cres->textureData[EFT_TEXTURE_SLOT_0].uvScroll.y * emitter->frameRate;
        stripe->uvParam[EFT_TEXTURE_SLOT_0].scale.x  += cres->textureData[EFT_TEXTURE_SLOT_0].uvScale.x  * emitter->frameRate;
        stripe->uvParam[EFT_TEXTURE_SLOT_0].scale.y  += cres->textureData[EFT_TEXTURE_SLOT_0].uvScale.y  * emitter->frameRate;
        stripe->uvParam[EFT_TEXTURE_SLOT_0].rotateZ  += cres->textureData[EFT_TEXTURE_SLOT_0].uvRot      * emitter->frameRate;

        stripe->uvParam[EFT_TEXTURE_SLOT_1].scroll.x += cres->textureData[EFT_TEXTURE_SLOT_1].uvScroll.x * emitter->frameRate;
        stripe->uvParam[EFT_TEXTURE_SLOT_1].scroll.y += cres->textureData[EFT_TEXTURE_SLOT_1].uvScroll.y * emitter->frameRate;
        stripe->uvParam[EFT_TEXTURE_SLOT_1].scale.x  += cres->textureData[EFT_TEXTURE_SLOT_1].uvScale.x  * emitter->frameRate;
        stripe->uvParam[EFT_TEXTURE_SLOT_1].scale.y  += cres->textureData[EFT_TEXTURE_SLOT_1].uvScale.y  * emitter->frameRate;
        stripe->uvParam[EFT_TEXTURE_SLOT_1].rotateZ  += cres->textureData[EFT_TEXTURE_SLOT_1].uvRot      * emitter->frameRate;

        PtclStripeHistory* histNow = &stripe->hist[stripe->histQEnd];

        if (!(stripe->flag & 1) || stripe->histQEnd < stripeData->stripeNumHistory - 1)
        {
            f32 next = ptcl->cnt + emitter->frameRate;

            s32 count = (s32)(nw::math::FFloor(next) - nw::math::FFloor(ptcl->cnt));
            if (count == 0)
                return;

            if (cres->stripeFlg & EFT_STRIPE_FLAG_EMITTER_COORD)
            {
                histNow->pos = ptcl->pos;
                histNow->emitterSRT.SetIdentity();
            }
            else
            {
                f32 hisRatio = stripeData->stripeHistoryInterpolate;
                if (cnt > 2 && stripeData->stripeNumHistory > 3 && hisRatio < 1.0f && stripe->histQEnd != stripe->histQStart)
                {
                    s32 prevIx  = stripe->histQEnd - 1; if (prevIx  < 0) prevIx  = stripeData->stripeNumHistory - 1;
                    s32 prevIx2 = prevIx           - 1; if (prevIx2 < 0) prevIx2 = stripeData->stripeNumHistory - 1;
                    PtclStripeHistory* histPrev  = &stripe->hist[prevIx];
                    PtclStripeHistory* histPrev2 = &stripe->hist[prevIx2];

                    stripe->p0 += (ptcl->worldPos - stripe->p0) * hisRatio;
                    stripe->p1 += (stripe->p0     - stripe->p1) * hisRatio;

                    histPrev2->pos = stripe->p1;
                    histPrev->pos  = stripe->p1 + (ptcl->worldPos - stripe->p1) * 0.7f;
                    histNow->pos = ptcl->worldPos;
                }
                else
                {
                    histNow->pos = ptcl->worldPos;
                    stripe->p0 = (stripe->p1 = histNow->pos);
                }

                histNow->emitterSRT = emitter->emitterSRT;
            }

            histNow->scale = ptcl->scale.x * emitter->emitterSet->mParticlScaleForCalc.x;

            if (stripe->histQEnd != stripe->histQStart)
            {
                s32 prevIx = stripe->histQEnd - 1; if (prevIx < 0) prevIx = stripeData->stripeNumHistory - 1;
                PtclStripeHistory* histPrev = &stripe->hist[prevIx];

                if (cnt < 2)
                {
                    stripe->interpolateNextDir = histNow->pos - histPrev->pos;
                    if (stripe->interpolateNextDir.Length() > 0.0f)
                        stripe->interpolateNextDir.Normalize();
                }
                else
                {
                    nw::math::VEC3 dir = histNow->pos - histPrev->pos;
                    if (dir.Length() > 0.0f)
                        dir.Normalize();

                    stripe->interpolateNextDir += (dir - stripe->interpolateNextDir) * stripeData->stripeDirInterpolate;
                    if (stripe->interpolateNextDir.Length() > 0.0f)
                        stripe->interpolateNextDir.Normalize();
                }

                histNow->dir = stripe->interpolateNextDir;

                if (stripeData->stripeType == EFT_STRIPE_TYPE_EMITTER_UP_DOWN)
                {
                    histNow->outer.x = histNow->emitterSRT.m[0][1];
                    histNow->outer.y = histNow->emitterSRT.m[1][1];
                    histNow->outer.z = histNow->emitterSRT.m[2][1];
                }
                else
                {
                    nw::math::VEC3 basis(histNow->emitterSRT.m[0][1],
                                         histNow->emitterSRT.m[1][1],
                                         histNow->emitterSRT.m[2][1]);

                    nw::math::VEC3 outer;
                    outer.SetCross(basis, stripe->interpolateNextDir);
                    if (outer.Length() > 0.0f)
                        outer.Normalize();

                    histNow->outer = outer;
                }
            }

            stripe->histQEnd++;
            if (stripe->histQEnd >= stripeData->stripeNumHistory)
                stripe->histQEnd = 0;

            if (stripe->histQEnd == stripe->histQStart)
            {
                stripe->histQStart++;
                if (stripe->histQStart >= stripeData->stripeNumHistory)
                    stripe->histQStart = 0;
            }

            stripe->numHistory++;
            if (stripe->numHistory >= stripeData->stripeNumHistory)
                stripe->numHistory = stripeData->stripeNumHistory;

            stripe->cnt++;
        }

        if (!skipMakeAttribute)
            eftSystem->GetRenderer(core)->MakeStripeAttributeBlock(emitter, ptcl);
    }
}

void EmitterComplexCalc::EmitChildParticle(EmitterInstance* emitter, PtclInstance* ptcl, CpuCore core, const ChildData* childData)
{
    s32 emitTime = (ptcl->life - 1) * childData->childEmitTiming / 100;

    register s32 cnt = ptcl->cntS;

    if (cnt >= emitTime)
    {
        if (ptcl->complexParam->childEmitcnt >= childData->childEmitStep || childData->childEmitStep == 0 && childData->childLife == 1)
        {
            s32 loop = 1;

            if (ptcl->complexParam->childPreEmitcnt > 0.0f)
            {
                if (childData->childEmitStep != 0)
                {
                    loop = (s32)((emitter->cnt - ptcl->complexParam->childPreEmitcnt + ptcl->complexParam->childEmitSaving) / childData->childEmitStep);

                    if (ptcl->complexParam->childEmitSaving >= childData->childEmitStep)
                        ptcl->complexParam->childEmitSaving -= childData->childEmitStep;

                    ptcl->complexParam->childEmitSaving += emitter->cnt - ptcl->complexParam->childPreEmitcnt - loop;
                }
                else
                {
                    loop = (s32)(emitter->cnt - ptcl->complexParam->childPreEmitcnt + ptcl->complexParam->childEmitSaving);

                    if (ptcl->complexParam->childEmitSaving >= childData->childEmitStep)
                        ptcl->complexParam->childEmitSaving -= childData->childEmitStep;

                    ptcl->complexParam->childEmitSaving += emitter->cnt - ptcl->complexParam->childPreEmitcnt - loop;
                }
            }

            mSys->AddPtclAdditionList(ptcl, core);

            ptcl->complexParam->childEmitcnt = 0.0f;
            ptcl->complexParam->childPreEmitcnt = emitter->cnt;
        }
        else
        {
            ptcl->complexParam->childEmitcnt += emitter->frameRate;
        }
    }
}

u32 EmitterComplexCalc::CalcParticle(EmitterInstance* emitter, CpuCore core, bool skipBehavior, bool skipMakeAttribute)
{
    emitter->entryNum = 0;
    if (emitter->ptclNum == 0)
        return 0;

    System*       eftSystem     = emitter->emitterSet->mSystem;
    BillboardType billboardType = emitter->GetBillboardType();
    PtclAttributeBuffer* ptclAttrBuffer = NULL;

    if (!skipMakeAttribute &&
        billboardType != EFT_BILLBOARD_TYPE_STRIPE &&
        billboardType != EFT_BILLBOARD_TYPE_COMPLEX_STRIPE)
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

    const ComplexEmitterData* cres = emitter->GetComplexEmitterData();

    CustomActionParticleCalcCallback particleCB = mSys->GetCurrentCustomActionParticleCalcCallback(emitter);
    CustomActionParticleMakeAttributeCallback particleMakeAttrCB = mSys->GetCurrentCustomActionParticleMakeAttributeCallback(emitter);

    PtclInstance* ptcl = emitter->ptclHead;

    const ChildData*  childData         = emitter->GetChildData();
    const StripeData* stripeData        = NULL;
    u32               shaderAttrFlag    = emitter->shader[EFT_SHADER_TYPE_NORMAL]->GetShaderAttributeFlag();
    PtclStripe*       stripe            = NULL;

    bool isStripe = false;
    if (billboardType == EFT_BILLBOARD_TYPE_STRIPE ||
        billboardType == EFT_BILLBOARD_TYPE_COMPLEX_STRIPE)
    {
        isStripe = true;
        stripeData = emitter->GetStripeData();
    }

    bool isDefaultStripe = false;
    if (billboardType == EFT_BILLBOARD_TYPE_STRIPE)
        isDefaultStripe = true;

    bool isConnectionStripe = false;
    if (billboardType == EFT_BILLBOARD_TYPE_COMPLEX_STRIPE)
        isConnectionStripe = true;

    f32 behaviorLoopF = emitter->cnt - emitter->preCnt;
    u32 behaviorLoopU = (u32)behaviorLoopF;
    f32 behaviorLoopD = behaviorLoopF - behaviorLoopU;
    bool behaviorLoop = false;
    if (emitter->frameRate > 1.0f)
        behaviorLoop  = true;

    PtclInstance* next = NULL;

    while (ptcl)
    {
        next = ptcl->next;

        stripe = ptcl->complexParam->stripe;

        if (!skipBehavior)
        {
            if (isStripe && stripe)
            {
                ptcl->cntS = (s32)ptcl->cnt;
                if (ptcl->cntS >= ptcl->life || (ptcl->life == 1 && ptcl->cnt != 0.0f))
                {
                    if (stripe->histQStart != stripe->histQEnd)
                    {
                        stripe->histQStart++;
                        if (stripe->histQStart >= stripeData->stripeNumHistory)
                            stripe->histQStart = 0;

                        stripe->numHistory--;

                        stripe->uvParam[EFT_TEXTURE_SLOT_0].scroll.x += cres->textureData[EFT_TEXTURE_SLOT_0].uvScroll.x * emitter->frameRate;
                        stripe->uvParam[EFT_TEXTURE_SLOT_0].scroll.y += cres->textureData[EFT_TEXTURE_SLOT_0].uvScroll.y * emitter->frameRate;
                        stripe->uvParam[EFT_TEXTURE_SLOT_0].scale.x  += cres->textureData[EFT_TEXTURE_SLOT_0].uvScale.x  * emitter->frameRate;
                        stripe->uvParam[EFT_TEXTURE_SLOT_0].scale.y  += cres->textureData[EFT_TEXTURE_SLOT_0].uvScale.y  * emitter->frameRate;
                        stripe->uvParam[EFT_TEXTURE_SLOT_0].rotateZ  += cres->textureData[EFT_TEXTURE_SLOT_0].uvRot      * emitter->frameRate;

                        stripe->uvParam[EFT_TEXTURE_SLOT_1].scroll.x += cres->textureData[EFT_TEXTURE_SLOT_1].uvScroll.x * emitter->frameRate;
                        stripe->uvParam[EFT_TEXTURE_SLOT_1].scroll.y += cres->textureData[EFT_TEXTURE_SLOT_1].uvScroll.y * emitter->frameRate;
                        stripe->uvParam[EFT_TEXTURE_SLOT_1].scale.x  += cres->textureData[EFT_TEXTURE_SLOT_1].uvScale.x  * emitter->frameRate;
                        stripe->uvParam[EFT_TEXTURE_SLOT_1].scale.y  += cres->textureData[EFT_TEXTURE_SLOT_1].uvScale.y  * emitter->frameRate;
                        stripe->uvParam[EFT_TEXTURE_SLOT_1].rotateZ  += cres->textureData[EFT_TEXTURE_SLOT_1].uvRot      * emitter->frameRate;

                        emitter->entryNum++;

                        stripe->cnt++;

                        if (!skipMakeAttribute)
                            eftSystem->GetRenderer(core)->MakeStripeAttributeBlock(emitter, ptcl);
                    }
                    else
                    {
                        RemoveParticle(ptcl, core);
                    }

                    stripe->cnt++;

                    if (!skipMakeAttribute)
                        eftSystem->GetRenderer(core)->MakeStripeAttributeBlock(emitter, ptcl);

                    ptcl = next;
                    continue;
                }
            }
            else
            {
                if (CalcParticleKillCondition(ptcl, core))
                {
                    ptcl = next;
                    continue;
                }
            }

            if (behaviorLoop)
            {
                for (u32 i = 0; i < behaviorLoopU; i++)
                    CalcComplexParticleBehavior(emitter, ptcl, 1.0f);

                if (behaviorLoopD != 0.0f)
                    CalcComplexParticleBehavior(emitter, ptcl, behaviorLoopD);
            }
            else
            {
                CalcComplexParticleBehavior(emitter, ptcl, emitter->frameRate);
            }

            if (isStripe && stripe && emitter->frameRate != 0.0f)
                CalcStripe(emitter, ptcl, stripeData, cres, core, skipMakeAttribute);


            if (childData && emitter->frameRate != 0.0f)
                EmitChildParticle(emitter, ptcl, core, childData);

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

        if (!isStripe && !skipMakeAttribute)
        {
            CalcParticleMakeAttr(emitter, ptcl, ptclAttrBuffer, shaderAttrFlag);
            ptclAttrBuffer++;

            if (particleMakeAttrCB != NULL)
                CalcParticleMakeAttrCallback(particleMakeAttrCB, emitter, ptcl, core, skipBehavior);
        }

        if (isDefaultStripe && !skipMakeAttribute)
            eftSystem->GetRenderer(core)->MakeStripeAttributeBlock(emitter, ptcl);

        ptcl = next;
    }

    if (isConnectionStripe)
        eftSystem->GetRenderer(core)->MakeConnectionStripeAttributeBlock(emitter);

    emitter->emitterBehaviorFlag |= EFT_EMITTER_BEHAVIOR_FLAG_IS_CALCULATED;

    return emitter->ptclNum;
}

} } // namespace nw::eft
