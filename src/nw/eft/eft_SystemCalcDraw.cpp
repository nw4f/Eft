#include <nw/eft/eft_Data.h>
#include <nw/eft/eft_Emitter.h>
#include <nw/eft/eft_EmitterSet.h>
#include <nw/eft/eft_Misc.h>
#include <nw/eft/eft_Renderer.h>
#include <nw/eft/eft_Shader.h>
#include <nw/eft/eft_System.h>

#include <cstdlib>
#include <cstring>

namespace nw { namespace eft {

void System::BeginFrame()
{
    mNumEmitterCalc = 0;
    mNumPtclCalc    = 0;
    mNumGpuPtclCalc = 0;
    mNumEmittedPtcl = 0;
    mNumStripeCalc  = 0;
    mEnableGroupID  = 0;
    mMarkingSwapFlag = false;
    mGlobalCounter++;

    mStreamOutEmitter = NULL;

    memset(mEnableRenderPath, 0, EFT_GROUP_MAX * sizeof(u32) * EFT_CPU_CORE_MAX);

    RemoveStripe_();

    FlushDelayFreeList();
}

void System::SwapDoubleBuffer()
{
    for (u32 core = 0; core < EFT_CPU_CORE_MAX; core++)
        mRenderer[core]->SwapDoubleBuffer();

    mCalcSwapFlag = true;
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
                const ComplexEmitterData* res  = static_cast<const ComplexEmitterData*>(emitter->res);
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

void System::CalcEmitter(u8 groupID, f32 frameRate)
{
    mEnableGroupID |= (u64)((u64)0x1 << (u64)(groupID));

    EmitterInstance* emitter = mEmitterHead[groupID];
    while (emitter != NULL)
    {
        CalcEmitter(emitter, frameRate);
        emitter = emitter->next;
    }
}

void System::CalcEmitter(EmitterInstance* emitter, f32 frameRate)
{
    if (!emitter->emitterSet->IsStopCalc() &&
        ((mEnableGroupID & (u64)((u64)0x1 << (u64)emitter->groupID)) != false))
    {
        if (emitter->frameRate != frameRate)
        {
          //emitter->emitCnt    = 0.0f;
            emitter->preEmitCnt = emitter->cnt;
          //emitter->emitSaving = 0.0f;
            emitter->frameRate  = frameRate;
        }

        if (GetCurrentCustomActionEmitterPreCalcCallback(emitter))
        {
            EmitterPreCalcArg arg;
            arg.emitter = emitter;
            GetCurrentCustomActionEmitterPreCalcCallback(emitter)(arg);
        }
        else if (GetCustomShaderEmitterPreCalcCallback(static_cast<CustomShaderCallBackID>(emitter->res->userShaderSetting)))
        {
            ShaderEmitterPreCalcArg arg;
            arg.emitter = emitter;
            GetCustomShaderEmitterPreCalcCallback(static_cast<CustomShaderCallBackID>(emitter->res->userShaderSetting))(arg);
        }
        else
        {
            emitter->calc->CalcEmitter(emitter);
        }

        if (GetCurrentCustomActionEmitterPostCalcCallback(emitter))
        {
            EmitterPostCalcArg arg;
            arg.emitter = emitter;
            GetCurrentCustomActionEmitterPostCalcCallback(emitter)(arg);
        }

        if (emitter->shader[EFT_SHADER_TYPE_NORMAL]->IsUseStreamOut())
        {
            if (mStreamOutEmitter == NULL)
            {
                mStreamOutEmitter = emitter;
                mStreamOutEmitterTail = mStreamOutEmitter;
                mStreamOutEmitterTail->nextStreamOutList = NULL;
            }
            else
            {
                mStreamOutEmitterTail->nextStreamOutList = emitter;
                mStreamOutEmitterTail = emitter;
                mStreamOutEmitterTail->nextStreamOutList = NULL;
            }
        }

        mNumEmitterCalc++;
    }
}

void System::CalcParticle(EmitterInstance* emitter, CpuCore core)
{
    if (emitter == NULL || emitter->calc == NULL)
        return;

    CustomShaderEmitterPostCalcCallback particleEmitterPostCB = GetCustomShaderEmitterPostCalcCallback(static_cast<CustomShaderCallBackID>(emitter->res->userShaderSetting));

    bool skipBehavior = false;
    if ((mEnableGroupID & (u64)((u64)0x1 << (u64)emitter->groupID)) == 0)
        skipBehavior = true;
    if (emitter->emitterSet->IsStopCalc())
        skipBehavior = true;

    if (particleEmitterPostCB)
    {
        ShaderEmitterPostCalcArg arg;
        arg.emitter = emitter;
        arg.isChild = false;
        arg.skipBehavior = skipBehavior;
        particleEmitterPostCB(arg);
    }

    s32 numCalc = emitter->calc->CalcParticle(emitter, core, skipBehavior, false);
    mNumPtclCalc += numCalc;

    if (emitter->IsGpuAcceleration())
        mNumGpuPtclCalc += numCalc;

    if (emitter->res->billboardType == EFT_BILLBOARD_TYPE_STRIPE)
        mNumStripeCalc += numCalc;

    if (emitter->res->billboardType == EFT_BILLBOARD_TYPE_COMPLEX_STRIPE)
        mNumStripeCalc++;

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

        CustomShaderEmitterPostCalcCallback childParticleEmitterPostCB = GetCustomShaderEmitterPostCalcCallback(static_cast<CustomShaderCallBackID>(childData->childUserShaderSetting));
        if (childParticleEmitterPostCB)
        {
            ShaderEmitterPostCalcArg arg;
            arg.emitter = emitter;
            arg.isChild = true;
            arg.skipBehavior = skipBehavior;
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
    GX2Invalidate(static_cast<GX2InvalidateType>(GX2_INVALIDATE_ATTRIB_BUFFER |
                                                 GX2_INVALIDATE_TEXTURE       |
                                                 GX2_INVALIDATE_UNIFORM_BLOCK |
                                                 GX2_INVALIDATE_SHADER),
                  NULL, 0xFFFFFFFF);
}

void System::SwapStreamOutBuffer()
{
    if (mStreamOutEmitter == NULL)
        return;

    EmitterInstance* temp = mStreamOutEmitter;
    while (temp)
    {
        if (temp->streamOutFlip)
            temp->streamOutFlip = false;
        else
            temp->streamOutFlip = true;

        temp = temp->nextStreamOutList;
    }
}

void System::CalcStreamOutEmittter()
{
    if (mStreamOutEmitter == NULL)
        return;

    mRenderer[GetCurrentCore()]->BeginStremOut();
    {
        EmitterInstance* temp = mStreamOutEmitter;
        while (temp)
        {
            mRenderer[GetCurrentCore()]->CalcStremOutParticle(temp, true);
            temp = temp->nextStreamOutList;
        }
    }
    mRenderer[GetCurrentCore()]->EndStremOut();
}

void System::BeginRender(const nw::math::MTX44& proj, const nw::math::MTX34& view, const nw::math::VEC3& camPos, f32 nearClip, f32 farClip)
{
    mViewMatrix[GetCurrentCore()] = nw::math::MTX44(view);
    mRenderer[GetCurrentCore()]->BeginRender(proj, view, camPos, nearClip, farClip);
}

s32 System::ComparePtclViewZ(const void* a, const void* b)
{
    const sortEmitterSets* src = static_cast<const sortEmitterSets*>(a);
    const sortEmitterSets* dst = static_cast<const sortEmitterSets*>(b);

    if (src->z < 0.0f && dst->z < 0.0f)
    {
        if (src->z < dst->z)
            return -1;
    }
    else
    {
        if (src->z > dst->z)
            return -1;
    }

    return 1;
}

static u32 Float32ToBits24(f32 value)
{
    enum
    {
        SIGN32 = 0x80000000,
        SIGN24 = 0x00800000,

        EXP_BIAS32 = 127,
        EXP_BIAS24 = 63,
        EXP_MASK32 = 0x7F800000,
        EXP_MASK24 = 0x007F0000,

        FRACTION_WIDTH32 = 23,
        FRACTION_MASK32  = 0x007FFFFF,
        FRACTION_WIDTH24 = 16,
        FRACTION_MASK24  = 0x0000FFFF
    };

    u32 bits32 = *reinterpret_cast<u32*>(&value);

    u32 sign = bits32 & SIGN32;
    s32 exp = (s32)((bits32 & EXP_MASK32) >> FRACTION_WIDTH32);
    u32 fraction = bits32 & FRACTION_MASK32;

    u32 bits24 = 0;
    bits24 |= (sign != 0) ? SIGN24 : 0;

    if ((bits32 & ~SIGN32) == 0)
        exp = 0;

    else
        exp = exp - EXP_BIAS32 + EXP_BIAS24;

    fraction = fraction >> (FRACTION_WIDTH32 - FRACTION_WIDTH24);

    if (exp < 0)
    {
    }
    else if (exp > 127)
    {
        bits24 = (u32)0x7F << FRACTION_WIDTH24;
    }
    else
    {
        bits24 |= fraction & FRACTION_MASK24;
        bits24 |= ((u32)exp & 0x7F) << FRACTION_WIDTH24;
    }

    return bits24;
}

void System::AddSortBuffer(u8 groupID, u32 drawPathFlag)
{
    s32 core = GetCurrentCore();
    EmitterSet* set = mEmitterSetHead[groupID];
    if (set)
    {
        while(set)
        {
            if (set->IsAlive() && (set->mDrawPathFlag & drawPathFlag))
            {
                if (!set->mIsStopDraw && set->GetNumEmitter() != 0 && set->mDrawPathFlag)
                {
                    nw::math::VEC3 rPos;
                    set->GetPos(rPos);
                    mSortEmittetSet[core][mSortEmitterSetIx[core]].emitterSet = set;
                    f32 viewZ = mViewMatrix[core].f._20 * rPos.x + mViewMatrix[core].f._21 * rPos.y + mViewMatrix[core].f._22 * rPos.z + mViewMatrix[core].f._23 * 1.0f;
                    mSortEmittetSet[core][mSortEmitterSetIx[core]].z = (set->GetDrawPriority() << 24) | Float32ToBits24(viewZ);
                    mSortEmitterSetIx[core]++;
                }
                set = set->mNext;
            }
            else
            {
                set = set->mNext;
            }
        }
    }

    mCurrentDrawPathFlag[core] |= drawPathFlag;
}

void System::RenderSortBuffer(void* userParam, RenderEmitterProfilerCallback profiler)
{
    s32 core = GetCurrentCore();

    if (mSortEmitterSetIx[core] == 0)
    {
        mCurrentDrawPathFlag[core] = 0;
        return;
    }

    qsort(mSortEmittetSet[core], mSortEmitterSetIx[core], sizeof(sortEmitterSets), ComparePtclViewZ);

    for (u32 i = 0; i < mSortEmitterSetIx[core]; i++)
    {
        if (profiler)
        {
            RenderEmitterProfilerArg arg;
            arg.system = this;
            arg.emitterSet = mSortEmittetSet[core][i].emitterSet;
            arg.emitter = NULL;
            arg.beforeRenderEmitter = true;
            arg.cacheFlush = true;
            arg.userParam = userParam;
            profiler(arg);
        }

        for (s32 j = mSortEmittetSet[core][i].emitterSet->GetNumCreatedEmitter() - 1; j >= 0; --j)
        {
            if (mSortEmittetSet[core][i].emitterSet->GetCreateID() == mSortEmittetSet[core][i].emitterSet->mInstance[j]->emitterSetCreateID &&
                mSortEmittetSet[core][i].emitterSet->mInstance[j]->calc &&
                (mCurrentDrawPathFlag[core] & (1 << mSortEmittetSet[core][i].emitterSet->mInstance[j]->res->drawPath)))
            {
                u32 flag = mSortEmittetSet[core][i].emitterSet->GetUserData(); // <--- Unused, but function literally does not match without it
                flag = 0;

                EmitterInstance* emitter = mSortEmittetSet[core][i].emitterSet->mInstance[j];
                if (profiler)
                {
                    RenderEmitterProfilerArg arg;
                    arg.system = this;
                    arg.emitterSet = NULL;
                    arg.beforeRenderEmitter = false;
                    arg.emitter = emitter;
                    arg.cacheFlush = true;
                    arg.userParam = userParam;
                    profiler(arg);
                }
                else
                {
                    RenderEmitter(emitter, userParam);
                }
            }
        }

        if (profiler)
        {
            RenderEmitterProfilerArg arg;
            arg.system = this;
            arg.emitterSet = mSortEmittetSet[core][i].emitterSet;
            arg.beforeRenderEmitter = false;
            arg.emitter = NULL;
            arg.cacheFlush = true;
            arg.userParam = userParam;
            profiler(arg);
        }
    }

    mSortEmitterSetIx[core] = 0;
    mCurrentDrawPathFlag[core] = 0;
}

void System::RenderEmitter(const EmitterInstance* emitter, void* userParam)
{
    if (emitter == NULL)
        return;

    CpuCore core = GetCurrentCore();

    if (GetCurrentCustomActionEmitterDrawOverrideCallback(emitter))
    {
        EmitterDrawOverrideArg arg;
        arg.emitter = emitter;
        arg.renderer = mRenderer[ core ];
        arg.cacheFlush = true;
        arg.userParam = userParam;
        GetCurrentCustomActionEmitterDrawOverrideCallback(emitter)(arg);
    }
    else
    {
        mRenderer[core]->EntryParticle(emitter, userParam);
    }
}

void System::EndRender()
{
    s32 core = GetCurrentCore();
    mRenderer[core]->EndRender();
}

} } // namespace nw::eft
