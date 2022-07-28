#include <nw/eft/eft_EmitterComplex.h>
#include <nw/eft/eft_EmitterSet.h>
#include <nw/eft/eft_Renderer.h>
#include <nw/eft/eft_System.h>
#include <nw/eft/eft_UniformBlock.h>

namespace nw { namespace eft {

void EmitterComplexCalc::CalcEmitter(EmitterInstance* e)
{
    EmitterSimpleCalc::CalcEmitter(e);
}

void EmitterComplexCalc::CalcComplex(EmitterInstance* e, PtclInstance* ptcl, CpuCore core)
{
    const ComplexEmitterData* __restrict res  = static_cast<const ComplexEmitterData*>(e->res);
    const ChildData*          __restrict cres = reinterpret_cast<const ChildData*>(res + 1);

    register s32 cnt = (s32)ptcl->cnt - 1;

    if (res->billboardType == EFT_BILLBOARD_TYPE_STRIPE)
    {
        const StripeData* sres = reinterpret_cast<const StripeData*>((u32)res + res->stripeDataOffset);

        if (ptcl->stripe)
        {
            PtclStripe* stripe = ptcl->stripe;

            PtclStripeHistory* histNow = &stripe->hist[stripe->histQEnd];

            if (res->stripeFlg & EFT_STRIPE_FLAG_EMITTER_COORD)
            {
                histNow->pos = ptcl->pos;
                histNow->emitterSRT.SetIdentity();
            }
            else
            {
                f32 hisRatio = sres->stripeHistoryInterpolate;
                if (cnt > 2 && sres->stripeNumHistory > 3 && hisRatio < 1.0f && stripe->histQEnd != stripe->histQStart)
                {
                    s32 prevIx  = stripe->histQEnd - 1; if (prevIx  < 0) prevIx  = sres->stripeNumHistory - 1;
                    s32 prevIx2 = prevIx           - 1; if (prevIx2 < 0) prevIx2 = sres->stripeNumHistory - 1;
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

                histNow->emitterSRT = e->emitterSRT;
            }

            histNow->scale = ptcl->scale.x * e->emitterSet->mParticlScaleForCalc.x;

            if (stripe->histQEnd != stripe->histQStart)
            {
                s32 prevIx = stripe->histQEnd - 1; if (prevIx < 0) prevIx = sres->stripeNumHistory - 1;
                PtclStripeHistory* histPrev = &stripe->hist[prevIx];

                if (cnt < 2)
                {
                    stripe->interpolateNextDir = histNow->pos - histPrev->pos;
                    if (stripe->interpolateNextDir.Length() > 0.0f) stripe->interpolateNextDir.Normalize();
                }
                else
                {
                    nw::math::VEC3 dir = histNow->pos - histPrev->pos;
                    if (dir.Length() > 0.0f) dir.Normalize();

                    stripe->interpolateNextDir += (dir - stripe->interpolateNextDir) * sres->stripeDirInterpolate;
                    if (stripe->interpolateNextDir.Length() > 0.0f) stripe->interpolateNextDir.Normalize();
                }

                histNow->dir = stripe->interpolateNextDir;

                if (sres->stripeType == EFT_STRIPE_TYPE_EMITTER_UP_DOWN)
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
                    if (outer.Length() > 0.0f) outer.Normalize();

                    histNow->outer = outer;
                }
            }

            if (++stripe->histQEnd >= sres->stripeNumHistory)
                stripe->histQEnd = 0;

            if (stripe->histQEnd == stripe->histQStart
                && ++stripe->histQStart >= sres->stripeNumHistory)
                stripe->histQStart = 0;

            if (++stripe->numHistory >= sres->stripeNumHistory)
                stripe->numHistory = sres->stripeNumHistory;

            stripe->emitterSRT = e->emitterSRT;
            stripe->cnt++;
        }
    }

    if (res->childFlg & EFT_CHILD_FLAG_ENABLE)
    {
        s32 emitTime = (ptcl->life - 1) * cres->childEmitTiming / 100;
        if (cnt >= emitTime)
        {
            if (ptcl->childEmitCnt >= cres->childEmitStep || (cres->childEmitStep == 0 && cres->childLife == 1))
            {
                s32 loop = 1;

                if (ptcl->childPreEmitCnt > 0.0f)
                {
                    if (cres->childEmitStep != 0)
                    {
                        loop = (s32)((e->cnt - ptcl->childPreEmitCnt + ptcl->childEmitSaving) / cres->childEmitStep);

                        if (ptcl->childEmitSaving >= cres->childEmitStep)
                            ptcl->childEmitSaving -= cres->childEmitStep;

                        ptcl->childEmitSaving += e->cnt - ptcl->childPreEmitCnt - loop;
                    }
                    else
                    {
                        loop = (s32)(e->cnt - ptcl->childPreEmitCnt + ptcl->childEmitSaving);

                        if (ptcl->childEmitSaving >= cres->childEmitStep)
                            ptcl->childEmitSaving -= cres->childEmitStep;

                        ptcl->childEmitSaving += e->cnt - ptcl->childPreEmitCnt - loop;
                    }
                }

                mSys->AddPtclAdditionList(ptcl, core);

                ptcl->childEmitCnt = 0.0f;
                ptcl->childPreEmitCnt = e->cnt;
            }
            else
            {
                ptcl->childEmitCnt += e->frameRate;
            }
        }
    }
}

u32 EmitterComplexCalc::CalcParticle(EmitterInstance* emitter, CpuCore core, bool skipBehavior, bool skipMakeAttribute)
{
    System*       eftSystem     = emitter->emitterSet->mSystem;
    BillboardType billboardType = emitter->GetBillboardType();

    if (!skipMakeAttribute && billboardType != EFT_BILLBOARD_TYPE_STRIPE &&
                              billboardType != EFT_BILLBOARD_TYPE_COMPLEX_STRIPE)
    {
        emitter->ptclAttributeBuffer = static_cast<PtclAttributeBuffer*>(eftSystem->GetRenderer(core)->AllocFromDoubleBuffer(sizeof(PtclAttributeBuffer) * emitter->ptclNum));
        if (emitter->ptclAttributeBuffer == NULL)
            return 0;

        emitter->emitterDynamicUniformBlock = static_cast<EmitterDynamicUniformBlock*>(eftSystem->GetRenderer(core)->AllocFromDoubleBuffer(sizeof(EmitterDynamicUniformBlock)));
        if (emitter->emitterDynamicUniformBlock == NULL)
        {
            emitter->ptclAttributeBuffer = NULL;
            return 0;
        }

        nw::ut::FloatColor setColor = emitter->emitterSet->GetColor();
        setColor.r *= emitter->res->colorScale;
        setColor.g *= emitter->res->colorScale;
        setColor.b *= emitter->res->colorScale;

        emitter->emitterDynamicUniformBlock->emitterColor0.x = setColor.r * emitter->emitAnimValue[EFT_ANIM_COLOR0_R];
        emitter->emitterDynamicUniformBlock->emitterColor0.y = setColor.g * emitter->emitAnimValue[EFT_ANIM_COLOR0_G];
        emitter->emitterDynamicUniformBlock->emitterColor0.z = setColor.b * emitter->emitAnimValue[EFT_ANIM_COLOR0_B];
        emitter->emitterDynamicUniformBlock->emitterColor0.w = setColor.a * emitter->emitAnimValue[EFT_ANIM_ALPHA] * emitter->fadeAlpha;

        emitter->emitterDynamicUniformBlock->emitterColor1.x = setColor.r * emitter->emitAnimValue[EFT_ANIM_COLOR1_R];
        emitter->emitterDynamicUniformBlock->emitterColor1.y = setColor.g * emitter->emitAnimValue[EFT_ANIM_COLOR1_G];
        emitter->emitterDynamicUniformBlock->emitterColor1.z = setColor.b * emitter->emitAnimValue[EFT_ANIM_COLOR1_B];
        emitter->emitterDynamicUniformBlock->emitterColor1.w = setColor.a * emitter->emitAnimValue[EFT_ANIM_ALPHA] * emitter->fadeAlpha;

        GX2EndianSwap(emitter->emitterDynamicUniformBlock, sizeof(EmitterDynamicUniformBlock));
    }
    else
    {
        emitter->ptclAttributeBuffer = NULL;
        emitter->emitterDynamicUniformBlock = NULL;
    }

    emitter->entryNum = 0;

    const ComplexEmitterData* res = static_cast<const ComplexEmitterData*>(emitter->res);

    UserDataParticleCalcCallback particleCB = mSys->GetCurrentUserDataParticleCalcCallback(emitter);
    UserDataParticleMakeAttributeCallback particleMakeAttrCB = mSys->GetCurrentUserDataParticleMakeAttributeCallback(emitter);
    ParticleCalcArg arg;

    PtclInstance* ptcl         = emitter->ptclHead;
    bool          reverseOrder = false;

    if (emitter->res->flg & EFT_EMITTER_FLAG_REVERSE_ORDER_PARTICLE)
    {
        reverseOrder = true;
        ptcl = emitter->ptclTail;
    }

    if (skipBehavior)
    {
        while (ptcl)
        {
            if (ptcl->res)
            {
                if (particleCB)
                {
                    arg.emitter = emitter;
                    arg.ptcl = ptcl;
                    arg.core = core;
                    arg.skipBehavior = skipBehavior;
                    particleCB(arg);
                }

                if (billboardType != EFT_BILLBOARD_TYPE_STRIPE &&
                    billboardType != EFT_BILLBOARD_TYPE_COMPLEX_STRIPE &&
                    !skipMakeAttribute)
                {
                    MakeParticleAttributeBuffer(&emitter->ptclAttributeBuffer[emitter->entryNum], ptcl, emitter->shaderArrtFlag, emitter->res->toCameraOffset);
                    ptcl->attributeBuffer     = &emitter->ptclAttributeBuffer[emitter->entryNum];
                    emitter->entryNum++;

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
            ptcl = reverseOrder ? ptcl->prev : ptcl->next;
        }
    }
    else
    {
        while (ptcl)
        {
            s32 cntS = (s32)ptcl->cnt;

            if (ptcl->res)
            {
                if (cntS >= ptcl->life || (ptcl->life == 1 && ptcl->cnt != 0.0f))
                {
                    PtclStripe* stripe = ptcl->stripe;
                    if (stripe)
                    {
                        const StripeData* sres = reinterpret_cast<const StripeData*>((u32)res + res->stripeDataOffset);

                        if (stripe->histQStart == stripe->histQEnd)
                        {
                            RemoveParticle(emitter, ptcl, core);
                        }
                        else
                        {
                            if (++stripe->histQStart >= sres->stripeNumHistory)
                                stripe->histQStart = 0;

                            stripe->numHistory--;

                            stripe->emitterSRT = emitter->emitterSRT;

                            ptcl->uvScroll.x += res->textureData[0].uvScroll.x;
                            ptcl->uvScroll.y += res->textureData[0].uvScroll.y;
                            ptcl->uvScale.x  += res->textureData[0].uvScale.x;
                            ptcl->uvScale.y  += res->textureData[0].uvScale.y;
                            ptcl->uvRotateZ  += res->textureData[0].uvRot;

                            ptcl->uvSubScroll.x += res->textureData[1].uvScroll.x;
                            ptcl->uvSubScroll.y += res->textureData[1].uvScroll.y;
                            ptcl->uvSubScale.x  += res->textureData[1].uvScale.x;
                            ptcl->uvSubScale.y  += res->textureData[1].uvScale.y;
                            ptcl->uvSubRotateZ  += res->textureData[1].uvRot;

                            emitter->entryNum++;
                        }

                        stripe->cnt++;
                    }
                    else
                    {
                        RemoveParticle(emitter, ptcl, core);
                    }
                }
                else
                {
                    CalcComplexParticleBehavior(emitter, ptcl, core);
                    CalcComplex(emitter, ptcl, core);

                    if (particleCB)
                    {
                        arg.emitter = emitter;
                        arg.ptcl = ptcl;
                        arg.core = core;
                        arg.skipBehavior = false;
                        particleCB(arg);
                    }

                    if (billboardType != EFT_BILLBOARD_TYPE_STRIPE &&
                        billboardType != EFT_BILLBOARD_TYPE_COMPLEX_STRIPE &&
                        !skipMakeAttribute)
                    {
                        MakeParticleAttributeBuffer(&emitter->ptclAttributeBuffer[emitter->entryNum], ptcl, emitter->shaderArrtFlag, emitter->res->toCameraOffset);
                        ptcl->attributeBuffer     = &emitter->ptclAttributeBuffer[emitter->entryNum];
                        emitter->entryNum++;

                        if (particleMakeAttrCB)
                        {
                            arg.emitter = emitter;
                            arg.ptcl = ptcl;
                            arg.core = core;
                            arg.skipBehavior = false;
                            particleMakeAttrCB(arg);
                        }
                    }
                }
            }
            ptcl = reverseOrder ? ptcl->prev : ptcl->next;
        }
    }

    if (res->billboardType == EFT_BILLBOARD_TYPE_STRIPE)
        eftSystem->GetRenderer(core)->MakeStripeAttributeBlock(emitter);

    emitter->isCalculated = true;
    return emitter->entryNum;
}

} } // namespace nw::eft
