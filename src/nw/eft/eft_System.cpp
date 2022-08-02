#include <nw/eft/eft_Config.h>
#include <nw/eft/eft_EmitterComplex.h>
#include <nw/eft/eft_EmitterFieldCurlNoise.h>
#include <nw/eft/eft_EmitterSet.h>
#include <nw/eft/eft_EmitterSimpleGpu.h>
#include <nw/eft/eft_Handle.h>
#include <nw/eft/eft_Heap.h>
#include <nw/eft/eft_Misc.h>
#include <nw/eft/eft_Particle.h>
#include <nw/eft/eft_Random.h>
#include <nw/eft/eft_Renderer.h>
#include <nw/eft/eft_System.h>
#include <nw/eft/eft_UniformBlock.h>

#include <cstring>
#include <new>

#include <nn/util/detail/util_Symbol.h>

namespace nw { namespace eft {

bool System::mInitialized = false;

System::System(const Config& config)
{
    NN_UTIL_REFER_SYMBOL("[SDK+NINTENDO:NW4F_1_11_0_eft]");

    if (config.GetEffectHeap())
        Initialize(config.GetEffectHeap(), config.GetEffectHeap(), config);
}

void System::Initialize(Heap* heap, Heap* dynamicHeap, const Config& config)
{
    mNumResource            = config.GetResourceNum() + config.GetViewerResourceNum();
    mNumEmitterData         = config.GetEmitterNum();
    mNumPtclData            = config.GetParticleNum();
    mNumEmitterSet          = config.GetEmitterSetNum();
    mNumStripeData          = config.GetStripeNum();

    SetStaticHeap(heap);
    SetDynamicHeap(dynamicHeap);
    InitializeDelayFreeList(mNumEmitterData * 3);
    SetSuppressOutputLog(config.IsSuppressionLog());

    mGlobalCounter          = 0;
    mCurEmitterIx           = 0;
    mNumFreeEmitter         = mNumEmitterData;
    mCurPtclIx              = 0;
    mCurStripeIx            = 0;
    mNumEmitterCalc         = 0;
    mNumPtclCalc            = 0;
    mNumGpuPtclCalc         = 0;
    mEmitterSetCreateID     = 0;
    mCurEmitterSetIx        = 0;
    mNumEmitterSetCalc      = 0;
    mNumStripeCalc          = 0;
    mRuntimeError           = EFT_RUNTIME_ERROR_NONE;
    mEnableCallbackID       = EFT_CUSTOM_ACTION_CALLBACK_ID_NONE;
    mCalcSwapFlag           = false;

    memset(mEmitterHead,                      0, EFT_GROUP_MAX * sizeof(EmitterInstance*));
    memset(mEmitterSetHead,                   0, EFT_GROUP_MAX * sizeof(EmitterSet*));
    memset(mEmitterSetTail,                   0, EFT_GROUP_MAX * sizeof(EmitterSet*));
    memset(mStripeHead,                       0, EFT_GROUP_MAX * sizeof(PtclStripe*));
    memset(mEnableRenderPath[EFT_CPU_CORE_0], 0, EFT_GROUP_MAX * sizeof(u32));
    memset(mEnableRenderPath[EFT_CPU_CORE_1], 0, EFT_GROUP_MAX * sizeof(u32));
    memset(mEnableRenderPath[EFT_CPU_CORE_2], 0, EFT_GROUP_MAX * sizeof(u32));

    for (u32 i = 0; i < EFT_GROUP_MAX; i++)
        mScreenNum[i] = 1;

    PtclRandom::Initialize();

    InitializeCurlNoise();

    {
        mResourcePtr = AllocFromStaticHeap(sizeof(Resource*) * mNumResource);
        mResource = new (mResourcePtr) Resource*[mNumResource];
        memset(mResource, 0, sizeof(Resource*) * mNumResource);
    }

    for (u32 core = 0; core < EFT_CPU_CORE_MAX; core++)
    {
        mRendererPtr[core] = AllocFromStaticHeap(sizeof(Renderer));
        mRenderer[core] = new (mRendererPtr[core]) Renderer(this, config);

        mRenderer[core]->SetTextureSlot(EFT_TEXTURE_SLOT_CURL_NOISE, GetCurlNoiseTexture());
    }

    {
        mEmitterSetPtr = AllocFromStaticHeap(sizeof(EmitterSet) * mNumEmitterSet + 32);
        mEmitterSet = new (mEmitterSetPtr) EmitterSet[mNumEmitterSet];

        for (s32 i = 0; i < mNumEmitterSet; i++)
            mEmitterSet[i].SetSystem(this);
    }

    {
        mEmitter = static_cast<EmitterInstance*>(AllocFromStaticHeap(sizeof(EmitterInstance) * mNumEmitterData));

        for (s32 i = 0; i < mNumEmitterData; i++)
        {
            mEmitter[i].calc                             = NULL;
            mEmitter[i].emitterStaticUniformBlock        = NULL;
            mEmitter[i].childEmitterStaticUniformBlock   = NULL;
            mEmitter[i].ptclAttributeBuffer              = NULL;
            mEmitter[i].ptclAttributeBufferGpu           = NULL;
            mEmitter[i].gpuParticleBufferNum             = 0;
            mEmitter[i].positionStreamOutBuffer.Invalidate();
            mEmitter[i].vectorStreamOutBuffer.Invalidate();
            mEmitter[i].streamOutFlip = true;
        }
    }

    {
        mPtcl         = static_cast<PtclInstance*>(AllocFromStaticHeap(sizeof(PtclInstance) * mNumPtclData));
        mAlphaAnim0   = static_cast<AlphaAnim   *>(AllocFromStaticHeap(sizeof(AlphaAnim)    * mNumPtclData));
        mAlphaAnim1   = static_cast<AlphaAnim   *>(AllocFromStaticHeap(sizeof(AlphaAnim)    * mNumPtclData));
        mScaleAnim    = static_cast<ScaleAnim   *>(AllocFromStaticHeap(sizeof(ScaleAnim)    * mNumPtclData));
        mComplexParam = static_cast<ComplexParam*>(AllocFromStaticHeap(sizeof(ComplexParam) * mNumPtclData));

        for (s32 i = 0; i < mNumPtclData; i++)
        {
            mPtcl[i].res                            = NULL;
            mPtcl[i].alphaAnim[EFT_ALPHA_KIND_0]    = &mAlphaAnim0[i];
            mPtcl[i].alphaAnim[EFT_ALPHA_KIND_1]    = &mAlphaAnim1[i];
            mPtcl[i].scaleAnim                      = &mScaleAnim[i];
            mPtcl[i].complexParam                   = &mComplexParam[i];
        }
    }

    {
        mEmitterCalcSimplePtr = AllocFromStaticHeap(sizeof(EmitterSimpleCalc));
        mEmitterCalc[EFT_EMITTER_TYPE_SIMPLE] = new (mEmitterCalcSimplePtr) EmitterSimpleCalc(this);

        mEmitterCalcComplexPtr = AllocFromStaticHeap(sizeof(EmitterComplexCalc));
        mEmitterCalc[EFT_EMITTER_TYPE_COMPLEX] = new (mEmitterCalcComplexPtr) EmitterComplexCalc(this);

        mEmitterCalcSimpleGpuPtr = AllocFromStaticHeap(sizeof(EmitterSimpleGpuCalc));
        mEmitterCalc[EFT_EMITTER_TYPE_SIMPLE_GPU] = new (mEmitterCalcSimpleGpuPtr) EmitterSimpleGpuCalc(this);

        EmitterCalc::Initialize();
    }

    for (u32 i = 0; i < EFT_CPU_CORE_MAX; i++)
    {
        mPtclAdditionArray[i] = static_cast<PtclInstance**>(AllocFromStaticHeap(sizeof(PtclInstance*) * mNumPtclData));
        mPtclAdditionIdx[i]   = 0;

        for (s32 j = 0; j < mNumPtclData; j++)
            mPtclAdditionArray[i][j] = NULL;
    }

    {
        mStripe = static_cast<PtclStripe*>(AllocFromStaticHeap(sizeof(PtclStripe) * mNumStripeData));

        for (s32 i = 0; i < mNumStripeData; i++)
        {
            mStripe[i].res             = NULL;
            mStripe[i].histQStart      = 0;
            mStripe[i].histQEnd        = 0;
            mStripe[i].numDrawVertex   = 0;
        }

        u32 stripePtrSize = sizeof(PtclStripe*) * mNumStripeData;
        for (u32 i = 0; i < EFT_CPU_CORE_MAX; i++)
        {
            mStripeRemoveIdx[i]   = 0;
            mStripeRemoveArray[i] = static_cast<PtclStripe**>(AllocFromStaticHeap(stripePtrSize));

            for (u32 j = 0; j < mNumStripeData; j++)
                mStripeRemoveArray[i][j] = NULL;
        }
    }

    for (u32 i = EFT_CUSTOM_ACTION_CALLBACK_ID_NONE; i < EFT_CUSTOM_ACTION_CALLBACK_ID_MAX; i++)
    {
        mCustomActionEmitterMatrixSetCallback[i]    = NULL;
        mCustomActionEmitterPreCalcCallback[i]      = NULL;
        mCustomActionParticleEmitCallback[i]        = NULL;
        mCustomActionParticleRemoveCallback[i]      = NULL;
        mCustomActionParticleCalcCallback[i]        = NULL;
        mCustomActionParticleMakeAttrCallback[i]    = NULL;
        mCustomActionEmitterPostCalcCallback[i]     = NULL;
        mCustomActionEmitterDrawOverrideCallback[i] = NULL;
    }

    for (u32 i = 0; i < EFT_CUSTOM_SHADER_CALLBACK_MAX; i++)
    {
        mCustomShaderEmitterInitializeCallback[i]   = NULL;
        mCustomShaderEmitterFinalizeCallback[i]     = NULL;
        mCustomShaderEmitterCalcPreCallback[i]      = NULL;
        mCustomShaderEmitterCalcPostCallback[i]     = NULL;
        mCustomShaderDrawOverrideCallback[i]        = NULL;
        mCustomShaderRenderStateSetCallback[i]      = NULL;
    }

    for (u32 i = 0; i < EFT_DRAW_PATH_CALLBACK_MAX; i++)
    {
        mDrawPathCallbackFlag[i]                    = 0;
        mDrawPathRenderStateSetCallback[i]          = NULL;
    }

    for (u32 core = 0; core < EFT_CPU_CORE_MAX; core++)
    {
        mSortEmittetSet[core] = static_cast<sortEmitterSets*>(AllocFromStaticHeap(sizeof(sortEmitterSets) * mNumEmitterSet));
        mSortEmitterSetIx[core] = 0;
        mCurrentDrawPathFlag[core] = 0;
    }

    mFieldCommonPlaneEnable = false;
    mFieldCommonPlaneMinX   = -50.f;
    mFieldCommonPlaneMaxX   =  50.f;
    mFieldCommonPlaneY      =   0.f;
    mFieldCommonPlaneMinZ   = -50.f;
    mFieldCommonPlaneMaxZ   =  50.f;

    LOG("System Alloced Size : %d \n", GetAllocedSizeFromStaticHeap());

    mInitialized = true;
}

System::~System()
{
    for (s32 i = 0; i < mNumEmitterData; i++)
    {
        if (mEmitter[i].ptclAttributeBufferGpu)
        {
            FreeFromDynamicHeap(mEmitter[i].ptclAttributeBufferGpu);
            mEmitter[i].positionStreamOutBuffer.Finalize();
            mEmitter[i].vectorStreamOutBuffer.Finalize();
        }
    }

    if (mResourcePtr)                   FreeFromStaticHeap(mResourcePtr);
    if (mEmitterSetPtr)                 FreeFromStaticHeap(mEmitterSetPtr);
    if (mStripe)                        FreeFromStaticHeap(mStripe);
    if (mEmitterCalcSimplePtr)          FreeFromStaticHeap(mEmitterCalcSimplePtr);
    if (mEmitterCalcComplexPtr)         FreeFromStaticHeap(mEmitterCalcComplexPtr);
    if (mEmitterCalcSimpleGpuPtr)       FreeFromStaticHeap(mEmitterCalcSimpleGpuPtr);
    if (mEmitter)                       FreeFromStaticHeap(mEmitter);
    if (mPtcl)                          FreeFromStaticHeap(mPtcl);
    if (mScaleAnim)                     FreeFromStaticHeap(mScaleAnim);
    if (mAlphaAnim0)                    FreeFromStaticHeap(mAlphaAnim0);
    if (mAlphaAnim1)                    FreeFromStaticHeap(mAlphaAnim1);
    if (mComplexParam)                  FreeFromStaticHeap(mComplexParam);

    for (u32 core = 0; core < EFT_CPU_CORE_MAX; core++)
    {
        if (mStripeRemoveArray[core]) FreeFromStaticHeap(mStripeRemoveArray[core]);
        if (mPtclAdditionArray[core]) FreeFromStaticHeap(mPtclAdditionArray[core]);

        if (mRendererPtr[core])
        {
            mRenderer[core]->~Renderer();
            FreeFromStaticHeap(mRendererPtr[core]);
        }

        if (mSortEmittetSet[core])  FreeFromStaticHeap(mSortEmittetSet[core]);
    }

    FinalizeCurlNoise();

    PtclRandom::Finalize();

    EmitterCalc::Finalize();

    FinalizeDelayFreeList();
    SetStaticHeap(NULL);
    SetDynamicHeap(NULL);
    SetSuppressOutputLog(false);
    mInitialized = false;
}

void System::RemoveStripe(PtclStripe* stripe)
{
    s32 groupID = stripe->groupID;

    stripe->res = NULL;

    if (mStripeHead[groupID] == stripe)
    {
        mStripeHead[groupID] = stripe->next;

        if (mStripeHead[groupID] != NULL)
            mStripeHead[groupID]->prev = NULL;
    }
    else
    {
        if (stripe->next != NULL)
            stripe->next->prev = stripe->prev;

        //if (stripe->prev != NULL) <-- No check, because... Nintendo
            stripe->prev->next = stripe->next;
    }
}

void System::RemoveStripe_()
{
    PtclStripe* stripe = NULL;

    for (s32 i = 0; i < EFT_CPU_CORE_MAX; i++)
    {
        for (u32 j = 0; j < mStripeRemoveIdx[i]; j++)
        {
            stripe = mStripeRemoveArray[i][j];
            RemoveStripe(stripe);
            mStripeRemoveArray[i][j] = NULL;
        }

        mStripeRemoveIdx[i] = 0;
    }
}

EmitterSet* System::RemoveEmitterSetFromDrawList(EmitterSet* set)
{
    EmitterSet* next = set->mNext;

    if (set == mEmitterSetHead[set->mGroupID])
    {
        mEmitterSetHead[set->mGroupID] = set->mNext;

        if (mEmitterSetHead[set->mGroupID] != NULL)
            mEmitterSetHead[set->mGroupID]->mPrev = NULL;

        if (set == mEmitterSetTail[set->mGroupID])
            mEmitterSetTail[set->mGroupID] = NULL;
    }
    else
    {
        if (set == mEmitterSetTail[set->mGroupID])
            mEmitterSetTail[set->mGroupID] = set->mPrev;

        if (set->mNext != NULL)
            set->mNext->mPrev = set->mPrev;

        if (set->mPrev != NULL)
            set->mPrev->mNext = set->mNext;

        else
            ERROR("EmitterSet Remove Failed.\n");
    }

    set->mNext = NULL;
    set->mPrev = NULL;

    return next;
}

void System::RemovePtcl()
{
    RemoveStripe_();

    for (u32 i = 0; i < EFT_GROUP_MAX; ++i)
    {
        EmitterSet* set = mEmitterSetHead[i];
        while (set)
        {
            if (!set->IsAlive())
                set = RemoveEmitterSetFromDrawList(set);
            else
                set = set->mNext;
        }
    }
}

void System::AddStripeRemoveList(PtclStripe* stripe, CpuCore core)
{
    u32 idx = mStripeRemoveIdx[core];
    mStripeRemoveArray[core][idx] = stripe;
    mStripeRemoveIdx[core]++;
}

void System::EmitChildParticle()
{
    PtclInstance* p = NULL;

    for (s32 i = 0; i < EFT_CPU_CORE_MAX; i++)
    {
        for (s32 j = 0; j < mPtclAdditionIdx[i]; j++)
        {
            p = mPtclAdditionArray[i][j];
            EmitterComplexCalc::EmitChildParticle(p->emitter, p);
            mPtclAdditionArray[i][j] = NULL;
        }

        mPtclAdditionIdx[i] = 0;
    }
}

void System::AddPtclAdditionList(PtclInstance* ptcl, CpuCore core)
{
    if (mPtclAdditionIdx[core] > mNumPtclData)
        return;

    mPtclAdditionArray[core][mPtclAdditionIdx[core]] = ptcl;
    mPtclAdditionIdx[core]++;
}

PtclStripe* System::AllocAndConnectStripe(EmitterInstance* emitter)
{
    u32 i = 0;
    do
    {
        mCurStripeIx++;
        if (mCurStripeIx >= mNumStripeData)
            mCurStripeIx = 0;

        if (mStripe[mCurStripeIx].res == NULL)
        {
            PtclStripe* stripe = &mStripe[mCurStripeIx];
            const SimpleEmitterData* res = emitter->GetSimpleEmitterData();
            s32 groupID = emitter->groupID;

            if (mStripeHead[groupID] == NULL)
            {
                mStripeHead[groupID] = stripe;
                stripe->prev = (stripe->next = NULL);
            }
            else
            {
                mStripeHead[groupID]->prev = stripe;
                stripe->next = mStripeHead[groupID];
                mStripeHead[groupID] = stripe;
                stripe->prev = NULL;
            }

            stripe->ptcl                    = NULL;
            stripe->histQStart              = 0;
            stripe->histQEnd                = 0;
            stripe->numHistory              = 0;
            stripe->groupID                 = emitter->groupID;
            stripe->res                     = emitter->GetComplexEmitterData();
            stripe->cnt                     = 0;
            stripe->hist[0].outer           = nw::math::VEC3::Zero();
            stripe->uniformBlock            = NULL;
            stripe->uniformBlockForCross    = NULL;
            stripe->attributeBuffer         = NULL;

            stripe->uvParam[0].scroll.x     = res->textureData[0].uvScrollInit.x + res->textureData[0].uvScrollInitRand.x - emitter->rnd.GetF32() * res->textureData[0].uvScrollInitRand.x * 2.0f;
            stripe->uvParam[0].scroll.y     = res->textureData[0].uvScrollInit.y + res->textureData[0].uvScrollInitRand.y - emitter->rnd.GetF32() * res->textureData[0].uvScrollInitRand.y * 2.0f;
            stripe->uvParam[0].scale.x      = res->textureData[0].uvScaleInit.x  + res->textureData[0].uvScaleInitRand.x  - emitter->rnd.GetF32() * res->textureData[0].uvScaleInitRand.x  * 2.0f;
            stripe->uvParam[0].scale.y      = res->textureData[0].uvScaleInit.y  + res->textureData[0].uvScaleInitRand.y  - emitter->rnd.GetF32() * res->textureData[0].uvScaleInitRand.y  * 2.0f;
            stripe->uvParam[0].rotateZ      = res->textureData[0].uvRotInit      + res->textureData[0].uvRotInitRand      - emitter->rnd.GetF32() * res->textureData[0].uvRotInitRand      * 2.0f;

            stripe->uvParam[1].scroll.x     = res->textureData[1].uvScrollInit.x + res->textureData[1].uvScrollInitRand.x - emitter->rnd.GetF32() * res->textureData[1].uvScrollInitRand.x * 2.0f;
            stripe->uvParam[1].scroll.y     = res->textureData[1].uvScrollInit.y + res->textureData[1].uvScrollInitRand.y - emitter->rnd.GetF32() * res->textureData[1].uvScrollInitRand.y * 2.0f;
            stripe->uvParam[1].scale.x      = res->textureData[1].uvScaleInit.x  + res->textureData[1].uvScaleInitRand.x  - emitter->rnd.GetF32() * res->textureData[1].uvScaleInitRand.x  * 2.0f;
            stripe->uvParam[1].scale.y      = res->textureData[1].uvScaleInit.y  + res->textureData[1].uvScaleInitRand.y  - emitter->rnd.GetF32() * res->textureData[1].uvScaleInitRand.y  * 2.0f;
            stripe->uvParam[1].rotateZ      = res->textureData[1].uvRotInit      + res->textureData[1].uvRotInitRand      - emitter->rnd.GetF32() * res->textureData[1].uvRotInitRand      * 2.0f;

            stripe->uvParam[0].offset.x   = 0.0f;
            stripe->uvParam[0].offset.y   = 0.0f;

            if (res->textureData[0].numTexPat > 1) // TexPtnAnim Type Random
            {
                s32 no   = emitter->rnd.GetS32(res->textureData[0].numTexPat);
                s32 no_x = no % res->textureData[0].numTexDivX;
                s32 no_y = no / res->textureData[0].numTexDivX;

                register f32 no_x_f = (f32)no_x;
                register f32 no_y_f = (f32)no_y;

                stripe->uvParam[0].offset.x = res->textureData[0].texUScale * no_x_f;
                stripe->uvParam[0].offset.y = res->textureData[0].texVScale * no_y_f;
            }

            stripe->uvParam[1].offset.x   = 0.0f;
            stripe->uvParam[1].offset.y   = 0.0f;

            if (res->textureData[1].numTexPat > 1) // TexPtnAnim Type Random
            {
                s32 no   = emitter->rnd.GetS32(res->textureData[1].numTexPat);
                s32 no_x = no % res->textureData[1].numTexDivX;
                s32 no_y = no / res->textureData[1].numTexDivX;

                register f32 no_x_f = (f32)no_x;
                register f32 no_y_f = (f32)no_y;

                stripe->uvParam[1].offset.x = res->textureData[1].texUScale * no_x_f;
                stripe->uvParam[1].offset.y = res->textureData[1].texVScale * no_y_f;
            }

            stripe->flag = 0;

            return stripe;
        }
    } while (++i < mNumStripeData);

    WARNING("StripeInstance is Empty.\n");

    return NULL;
}

PtclInstance* System::AllocPtcl()
{
    s32 i = 0;
    do
    {
        mCurPtclIx++;
        if (mCurPtclIx >= mNumPtclData)
            mCurPtclIx = 0;

        if (mPtcl[mCurPtclIx].res == NULL)
        {
            mNumEmittedPtcl++;
            return &mPtcl[mCurPtclIx];
        }
    } while (++i < mNumPtclData);

    WARNING("Particle is Empty.\n");

    return NULL;
}

void System::UpdateEmitterResInfo()
{
    for (u32 i = 0; i < EFT_GROUP_MAX; ++i)
    {
        EmitterInstance* emitter = mEmitterHead[i];
        while (emitter)
        {
            emitter->UpdateResInfo();
            emitter = emitter->next;
        }
    }
}

void System::UpdateEmitterSetResInfo()
{
    for (u8 i = 0; i < EFT_GROUP_MAX; i++)
    {
        EmitterSet* set = GetEmitterSetHead(i);
        u32 flag = 0;

        while (set)
        {
            if (set->IsAlive())
            {
                for (s32 j = 0; j < set->GetNumEmitter(); j++)
                {
                    const EmitterInstance* e = set->GetAliveEmitter(j);

                    flag |= (0x01 << e->res->drawPath);
                }

                set->mDrawPathFlag = flag;
            }

            set = set->mNext;
        }
    }
}

EmitterSet* System::AllocEmitterSet(Handle* handle)
{
    EmitterSet* set = NULL;
    s32 i = 0;
    do
    {
        mCurEmitterSetIx++;
        if (mCurEmitterSetIx >= mNumEmitterSet)
            mCurEmitterSetIx = 0;

        if (mEmitterSet[mCurEmitterSetIx].GetNumEmitter() == 0)
        {
            set = &mEmitterSet[mCurEmitterSetIx];
            break;
        }
    } while (++i < mNumEmitterSet);

    handle->mEmitterSet = set;

    if (set == NULL)
    {
        WARNING("Emitter Set is Empty.\n");
        return NULL;
    }

    return set;
}

EmitterInstance* System::AllocEmitter(u8 groupID)
{
    EmitterInstance* emitter = NULL;
    s32 i = 0;
    do
    {
        mCurEmitterIx++;
        if (mCurEmitterIx >= mNumEmitterData)
            mCurEmitterIx = 0;

        if (mEmitter[mCurEmitterIx].calc == NULL)
        {
            emitter = &mEmitter[mCurEmitterIx];
            break;
        }
    } while (++i < mNumEmitterData);

    if (emitter == NULL)
    {
        WARNING("Emitter is Empty.\n");
        return NULL;
    }

    OSBlockSet(&emitter->ptclNum, 0, sizeof(EmitterInstance) - ((u32)&emitter->ptclNum - (u32)emitter));

    if (mEmitterHead[groupID] == NULL)
    {
        mEmitterHead[groupID] = emitter;
        emitter->prev = (emitter->next = NULL);
    }
    else
    {
        mEmitterHead[groupID]->prev = emitter;
        emitter->next = mEmitterHead[groupID];
        mEmitterHead[groupID] = emitter;
        emitter->prev = NULL;
    }

    mNumFreeEmitter--;
    return emitter;
}

void System::AddEmitterSetToDrawList(EmitterSet* set, u8 groupID)
{
    if(mEmitterSetHead[groupID] == NULL)
    {
        mEmitterSetHead[groupID] = set;
        set->mPrev = NULL;
        set->mNext = NULL;
    }
    else
    {
        mEmitterSetTail[groupID]->mNext = set;
        set->mPrev = mEmitterSetTail[groupID];
        set->mNext = NULL;
    }

    mEmitterSetTail[groupID] = set;
}

} } // namespace nw::eft
