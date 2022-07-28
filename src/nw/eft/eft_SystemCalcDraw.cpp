#include <nw/eft/eft_Data.h>
#include <nw/eft/eft_Emitter.h>
#include <nw/eft/eft_EmitterSet.h>
#include <nw/eft/eft_Renderer.h>
#include <nw/eft/eft_System.h>

#include <cstring>

namespace nw { namespace eft {

void System::BeginFrame()
{
    mNumEmitterCalc = 0;
    mNumPtclCalc = 0;
    mNumEmittedPtcl = 0;
    mNumStripeCalc = 0;
    mEnableGroupID = 0;

    /* MemUtil::FillZero */ memset(mEnableRenderPath, 0, EFT_GROUP_MAX * sizeof(u32) * EFT_CPU_CORE_MAX);

    RemovePtcl_();
}

void System::SwapDoubleBuffer()
{
    for (u8 i = 0; i < EFT_GROUP_MAX; i++)
    {
        EmitterInstance* emitter = mEmitterHead[i];
        while (emitter)
        {
            emitter->stripeBuffer                    = NULL;
            emitter->ptclAttributeBuffer             = NULL;
            emitter->childPtclAttributeBuffer        = NULL;
            emitter->emitterDynamicUniformBlock      = NULL;
            emitter->childEmitterDynamicUniformBlock = NULL;
            emitter->isCalculated                    = false;

            emitter = emitter->next;
        }
    }

    for (u32 core = 0; core < EFT_CPU_CORE_MAX; core++)
        mRenderer[core]->SwapDoubleBuffer();

    mCalcSwapFlag = true;
}

void System::CalcEmitter(u8 groupID, f32 frameRate)
{
    mEnableGroupID |= (u64)((u64)0x1 << (u64)(groupID));

    EmitterInstance* emitter = mEmitterHead[groupID];
    while (emitter)
    {
        if (!emitter->emitterSet->IsStopCalc())
        {
            if (emitter->frameRate != frameRate)
            {
                emitter->emitCnt    = 0.0f;
                emitter->preEmitCnt = emitter->cnt;
                emitter->emitSaving = 0.0f;
                emitter->frameRate  = frameRate;
            }

            if (GetCurrentUserDataEmitterPreCalcCallback(emitter))
            {
                EmitterPreCalcArg arg = { .emitter = emitter };
                GetCurrentUserDataEmitterPreCalcCallback(emitter)(arg);
            }
            else
            {
                emitter->calc->CalcEmitter(emitter);
            }

            if (GetCurrentUserDataEmitterPostCalcCallback(emitter))
            {
                EmitterPostCalcArg arg = { .emitter = emitter };
                GetCurrentUserDataEmitterPostCalcCallback(emitter)(arg);
            }

            mNumEmitterCalc++;
        }

        emitter = emitter->next;
    }
}

void System::CalcParticle(EmitterInstance* emitter, CpuCore core)
{
    if (emitter == NULL || emitter->calc == NULL)
        return;

    bool skipBehavior = false;
    if ((mEnableGroupID & (u64)((u64)0x1 << (u64)emitter->groupID)) == 0)
        skipBehavior = true;
    if (emitter->emitterSet->IsStopCalc())
        skipBehavior = true;

    UserShaderEmitterPostCalcCallback particleEmitterPostCB = GetUserShaderEmitterPostCalcCallback(static_cast<UserShaderCallBackID>(emitter->res->userShaderSetting));
    if (particleEmitterPostCB)
    {
        ShaderEmitterPostCalcArg arg = {
            .emitter = emitter,
            .skipBehavior = skipBehavior,
            .isChild = false,
        };
        particleEmitterPostCB(arg);
    }

    mNumPtclCalc += emitter->calc->CalcParticle(emitter, core, skipBehavior, false);
    mEnableRenderPath[core][emitter->groupID] |= 1 << emitter->res->drawPath;
}

void System::CalcChildParticle(EmitterInstance* emitter, CpuCore core)
{
    if (emitter == NULL || emitter->calc == NULL || emitter->GetEmitterType() != EFT_EMITTER_TYPE_COMPLEX)
        return;

    bool skipBehavior = false;
    if ((mEnableGroupID & (u64)((u64)0x1 << (u64)emitter->groupID)) == 0)
        skipBehavior = true;
    if (emitter->emitterSet->IsStopCalc())
        skipBehavior = true;

    if (emitter->IsHasChildParticle())
    {
        const ChildData* childData = emitter->GetChildData();

        UserShaderEmitterPostCalcCallback childParticleEmitterPostCB = GetUserShaderEmitterPostCalcCallback(static_cast<UserShaderCallBackID>(childData->childUserShaderSetting));
        if (childParticleEmitterPostCB)
        {
            ShaderEmitterPostCalcArg arg = {
                .emitter = emitter,
                .skipBehavior = skipBehavior,
                .isChild = true,
            };
            childParticleEmitterPostCB(arg);
        }
    }

    if (emitter->IsHasChildParticle())
        mNumPtclCalc += emitter->calc->CalcChildParticle(emitter, core, skipBehavior, false);
}

void System::FlushCache()
{
    for (u32 core = 0; core < EFT_CPU_CORE_MAX; core++)
        mRenderer[core]->FlushCache();
}

void System::FlushGpuCache()
{
    GX2Invalidate(static_cast<GX2InvalidateType>(GX2_INVALIDATE_ATTRIB_BUFFER
                                                 | GX2_INVALIDATE_TEXTURE
                                                 | GX2_INVALIDATE_UNIFORM_BLOCK
                                                 | GX2_INVALIDATE_SHADER), NULL, 0xFFFFFFFF);
}

void System::CalcParticle(bool cacheFlush)
{
    for (u32 i = 0; i < EFT_GROUP_MAX; i++)
    {
        EmitterInstance* emitter = mEmitterHead[i];
        while (emitter != NULL)
        {
            CalcParticle(emitter, EFT_CPU_CORE_1);
            mEnableRenderPath[EFT_CPU_CORE_1][emitter->groupID] |= 1 << emitter->res->drawPath;

            if (emitter->res->type == EFT_EMITTER_TYPE_COMPLEX)
            {
                const ComplexEmitterData* res = static_cast<const ComplexEmitterData*>(emitter->res);
                if (res->childFlg & EFT_CHILD_FLAG_ENABLE)
                {
                    EmitChildParticle();
                    CalcChildParticle(emitter, EFT_CPU_CORE_1);
                }
            }

            emitter = emitter->next;
        }
    }

    RemovePtcl();

    if (cacheFlush)
    {
        FlushCache();
        FlushGpuCache();
    }
}

void System::Calc(bool cacheFlush)
{
    if (!mCalcSwapFlag)
    {
        SwapDoubleBuffer();
        if (mEnableGroupID != 0)
        {
            mEnableGroupID = 0;
            CalcParticle(cacheFlush);
        }
    }

    mCalcSwapFlag = false;
}

void System::BeginRender(const nw::math::MTX44& proj, const nw::math::MTX34& view, const nw::math::VEC3& camPos, f32 nearClip, f32 farClip)
{
    mViewMatrix[GetCurrentCore()] = nw::math::MTX44(view);
    mRenderer[GetCurrentCore()]->BeginRender(proj, view, camPos, nearClip, farClip);
}

void System::RenderEmitter(EmitterInstance* emitter, bool cacheFlush, void* userParam)
{
    if (emitter == NULL)
        return;

    CpuCore core = GetCurrentCore();

    if (!emitter->isCalculated && (emitter->ptclNum || emitter->childPtclNum))
    {
        if (emitter->ptclNum > 0)
            emitter->calc->CalcParticle(emitter, core, true, false);

        if (emitter->IsHasChildParticle() && emitter->childPtclNum > 0)
            emitter->calc->CalcChildParticle(emitter, core, true, false);

        if (cacheFlush)
        {
            FlushCache();
            FlushGpuCache();
        }
    }

    if (GetCurrentUserDataEmitterDrawOverrideCallback(emitter))
    {
        EmitterDrawOverrideArg arg = {
            .emitter = emitter,
            .renderer = mRenderer[core],
            .cacheFlush = cacheFlush,
            .userParam = userParam,
        };
        GetCurrentUserDataEmitterDrawOverrideCallback(emitter)(arg);
    }
    else
    {
        mRenderer[core]->EntryParticle(emitter, cacheFlush, userParam);
    }
}

void System::EndRender()
{
    mRenderer[GetCurrentCore()]->EndRender();
}

} } // namespace nw::eft
