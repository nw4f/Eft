#include <nw/eft/eft_Config.h>
#include <nw/eft/eft_EmitterComplex.h>
#include <nw/eft/eft_EmitterSet.h>
#include <nw/eft/eft_Handle.h>
#include <nw/eft/eft_Heap.h>
#include <nw/eft/eft_Particle.h>
#include <nw/eft/eft_Random.h>
#include <nw/eft/eft_Renderer.h>
#include <nw/eft/eft_System.h>
#include <nw/eft/eft_UniformBlock.h>

#include <cstring>
#include <new>

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#endif // __GNUC__

namespace nw { namespace eft {

System::System(const Config& config)
{
    mInitialized = false;

    if (config.GetEffectHeap())
        Initialize(config.GetEffectHeap(), config);
}

void System::Initialize(Heap* heap, const Config& config)
{
    mHeap = heap;

    mNumResource        = config.GetResourceNum();
    mNumEmitterData     = config.GetEmitterNum();
    mNumPtclData        = config.GetParticleNum();
    mNumEmitterSet      = config.GetEmitterSetNum();
    mNumStripe          = config.GetStripeNum();
    mNumEmitterDataMask = mNumEmitterData - 1;
    mNumPtclDataMask    = mNumPtclData    - 1;
    mNumEmitterSetMask  = mNumEmitterSet  - 1;
    mNumStripeMask      = mNumStripe      - 1;

    _530                = 0;
    mCurEmitterIx       = 0;
    mNumFreeEmitter     = mNumEmitterData;
    mCurPtclIx          = 0;
    mCurStripeIx        = 0;
    mNumEmitterCalc     = 0;
    mNumPtclCalc        = 0;
    mNumStripeCalc      = 0;
    mEmitterSetCreateID = 0;
    mCurEmitterSetIx    = 0;
    mNumEmitterSetCalc  = 0;
    mEnableCallbackID   = EFT_USER_DATA_CALLBACK_ID_NONE;
    mCalcSwapFlag       = false;

    /* MemUtil::FillZero */ memset(mEmitterHead,                      0, EFT_GROUP_MAX * sizeof(EmitterInstance*));
    /* MemUtil::FillZero */ memset(mEmitterSetHead,                   0, EFT_GROUP_MAX * sizeof(EmitterSet*));
    /* MemUtil::FillZero */ memset(mEmitterSetTail,                   0, EFT_GROUP_MAX * sizeof(EmitterSet*));
    /* MemUtil::FillZero */ memset(mStripeHead,                       0, EFT_GROUP_MAX * sizeof(PtclStripe*));
    /* MemUtil::FillZero */ memset(mEnableRenderPath[EFT_CPU_CORE_0], 0, EFT_GROUP_MAX * sizeof(u32));
    /* MemUtil::FillZero */ memset(mEnableRenderPath[EFT_CPU_CORE_1], 0, EFT_GROUP_MAX * sizeof(u32));
    /* MemUtil::FillZero */ memset(mEnableRenderPath[EFT_CPU_CORE_2], 0, EFT_GROUP_MAX * sizeof(u32));

    PtclRandom::CreateRandomTbl(mHeap);

    ResourcePtr = mHeap->Alloc(sizeof(Resource*) * (mNumResource + nw::eft::Config::EFT_DEFAULT_RESOURCE_VIEWER_RESERVE_NUM));
    mResource = new (ResourcePtr) Resource*[(mNumResource + nw::eft::Config::EFT_DEFAULT_RESOURCE_VIEWER_RESERVE_NUM)];
    /* MemUtil::FillZero */ memset(mResource, 0, sizeof(Resource*) * (mNumResource + nw::eft::Config::EFT_DEFAULT_RESOURCE_VIEWER_RESERVE_NUM));

    for (u32 core = 0; core < EFT_CPU_CORE_MAX; core++)
    {
        mRendererPtr[core] = mHeap->Alloc(sizeof(Renderer));
        mRenderer[core] = new (mRendererPtr[core]) Renderer(mHeap, this, config);
    }

    {
        mEmitterSetPtr = mHeap->Alloc(sizeof(EmitterSet) * mNumEmitterSet + 32);
        mEmitterSet = new (mEmitterSetPtr) EmitterSet[mNumEmitterSet];

        for (s32 i = 0; i < mNumEmitterSet; i++)
            mEmitterSet[i].SetSystem(this);
    }

    {
        mEmitters = static_cast<EmitterInstance*>(mHeap->Alloc(sizeof(EmitterInstance) * mNumEmitterData));
        mEmitterStaticUniformBlock = static_cast<EmitterStaticUniformBlock*>(mHeap->Alloc(sizeof(EmitterStaticUniformBlock) * mNumEmitterData * 2));

        for (s32 i = 0; i < mNumEmitterData; i++)
        {
            mEmitters[i].calc = NULL;
            mEmitters[i].emitterStaticUniformBlock = &mEmitterStaticUniformBlock[i];
            mEmitters[i].childEmitterStaticUniformBlock = &mEmitterStaticUniformBlock[mNumEmitterData + i];
        }
    }

    {
        mPtcls = static_cast<PtclInstance*>(mHeap->Alloc(sizeof(PtclInstance) * mNumPtclData));
        mAlphaAnim = static_cast<AlphaAnim*>(mHeap->Alloc(sizeof(AlphaAnim) * mNumPtclData));
        mScaleAnim = static_cast<ScaleAnim*>(mHeap->Alloc(sizeof(ScaleAnim) * mNumPtclData));

        for (s32 i = 0; i < mNumPtclData; i++)
        {
            mPtcls[i].res       = NULL;
            mPtcls[i].alphaAnim = &mAlphaAnim[i];
            mPtcls[i].scaleAnim = &mScaleAnim[i];
        }
    }

    {
        mEmitterCalcSimplePtr = mHeap->Alloc(sizeof(EmitterSimpleCalc));
        mEmitterCalc[EFT_EMITTER_TYPE_SIMPLE] = new (mEmitterCalcSimplePtr) EmitterSimpleCalc(this);

        mEmitterCalcComplexPtr = mHeap->Alloc(sizeof(EmitterComplexCalc));
        mEmitterCalc[EFT_EMITTER_TYPE_COMPLEX] = new (mEmitterCalcComplexPtr) EmitterComplexCalc(this);

        EmitterCalc::Initialize(mHeap);
    }

    for (u32 i = 0; i < EFT_CPU_CORE_MAX; i++)
    {
        u32 ptclPtrSize         = sizeof(PtclInstance*) * mNumPtclData;
        mPtclRemoveArray[i]     = static_cast<PtclInstance**>(mHeap->Alloc(ptclPtrSize));
        mPtclAdditionArray[i]   = static_cast<PtclInstance**>(mHeap->Alloc(ptclPtrSize));
        mPtclRemoveIdx[i]       = 0;
        mPtclAdditionIdx[i]     = 0;

        for (s32 j = 0; j < mNumPtclData; j++)
        {
            mPtclRemoveArray[i][j]      = NULL;
            mPtclAdditionArray[i][j]    = NULL;
        }
    }

    {
        mStripes = static_cast<PtclStripe*>(mHeap->Alloc(sizeof(PtclStripe) * mNumStripe));

        for (s32 i = 0; i < mNumStripe; i++)
        {
            mStripes[i].res         = NULL;
            mStripes[i].histQStart  = 0;
            mStripes[i].histQEnd    = 0;
        }
    }

    for (u32 i = 0; i < EFT_CALLBACK_MAX; i++)
    {
        mUserDataEmitterPreCalcCallback[i]      = NULL;
        mUserDataParticleEmitCallback[i]        = NULL;
        mUserDataParticleRemoveCallback[i]      = NULL;
        mUserDataParticleCalcCallback[i]        = NULL;
        mUserDataParticleMakeAttrCallback[i]    = NULL;
        mUserDataEmitterPostCalcCallback[i]     = NULL;
        mUserDataEmitterDrawOverrideCallback[i] = NULL;
    }

    for (u32 i = 0; i < EFT_USER_SHADER_CALLBACK_MAX; i++)
    {
        mUserShaderEmitterCalcPostCallback[i]   = NULL;
        mUserShaderDrawOverrideCallback[i]      = NULL;
        mUserShaderRenderStateSetCallback[i]    = NULL;
    }

    for (u32 core = 0; core < EFT_CPU_CORE_MAX; core++)
    {
        mSortEmittetSet[core]       = static_cast<sortEmitterSets*>(mHeap->Alloc(sizeof(sortEmitterSets) * mNumEmitterSet));
        mSortEmitterSetIx[core]     = 0;
        mCurrentDrawPathFlag[core]  = 0;
    }

    mInitialized = true;
}

System::~System()
{
    if (ResourcePtr)                mHeap->Free(ResourcePtr);
    if (mEmitterSetPtr)             mHeap->Free(mEmitterSetPtr);
    if (mStripes)                   mHeap->Free(mStripes);
    if (mEmitterCalcSimplePtr)      mHeap->Free(mEmitterCalcSimplePtr);
    if (mEmitterCalcComplexPtr)     mHeap->Free(mEmitterCalcComplexPtr);
    if (mEmitters)                  mHeap->Free(mEmitters);
    if (mEmitterStaticUniformBlock) mHeap->Free(mEmitterStaticUniformBlock);
    if (mPtcls)                     mHeap->Free(mPtcls);
    if (mScaleAnim)                 mHeap->Free(mScaleAnim);
    if (mAlphaAnim)                 mHeap->Free(mAlphaAnim);

    for (u32 core = 0; core < EFT_CPU_CORE_MAX; core++)
    {
        if (mPtclRemoveArray[core])     mHeap->Free(mPtclRemoveArray[core]);
        if (mPtclAdditionArray[core])   mHeap->Free(mPtclAdditionArray[core]);

        if (mRendererPtr[core])
        {
            mRenderer[core]->~Renderer();
            mHeap->Free(mRendererPtr[core]);
        }

        if (mSortEmittetSet[core])      mHeap->Free(mSortEmittetSet[core]);
    }

    PtclRandom::DestroyRandomTbl(mHeap);

    EmitterCalc::Finalize(mHeap);
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

void System::RemovePtcl_()
{
    for (s32 i = 0; i < EFT_CPU_CORE_MAX; i++)
    {
        for (u32 j = 0; j < mPtclRemoveIdx[i]; j++)
        {
            PtclInstance* ptcl = mPtclRemoveArray[i][j];

            // EmitterCalc::RemoveParticle(PtclInstance*, CpuCore)
            {
                if (ptcl->type == EFT_PTCL_TYPE_CHILD) ptcl->emitter->childPtclNum--;
                else                                   ptcl->emitter->ptclNum--;

                if (ptcl->emitter->ptclHead == ptcl)
                {
                    ptcl->emitter->ptclHead = ptcl->next;

                    if (ptcl->emitter->ptclHead != NULL)
                        ptcl->emitter->ptclHead->prev = NULL;

                    if (ptcl->emitter->ptclTail == ptcl)
                        ptcl->emitter->ptclTail = NULL;
                }
                else if (ptcl->emitter->childHead == ptcl)
                {
                    ptcl->emitter->childHead = ptcl->next;

                    if (ptcl->emitter->childHead != NULL)
                        ptcl->emitter->childHead->prev = NULL;

                    if (ptcl->emitter->childTail == ptcl)
                        ptcl->emitter->childTail = NULL;
                }
                else if (ptcl->emitter->ptclTail == ptcl)
                {
                    ptcl->emitter->ptclTail = ptcl->prev;

                    if (ptcl->emitter->ptclTail != NULL)
                        ptcl->emitter->ptclTail->next = NULL;
                }
                else if (ptcl->emitter->childTail == ptcl)
                {
                    ptcl->emitter->childTail = ptcl->prev;

                    if (ptcl->emitter->childTail != NULL)
                        ptcl->emitter->childTail->next = NULL;
                }
                else
                {
                    if (ptcl->next != NULL)
                        ptcl->next->prev = ptcl->prev;

                    //if (ptcl->prev != NULL) <-- No check, because... Nintendo
                        ptcl->prev->next = ptcl->next;
                }

                UserDataParticleRemoveCallback cb = GetCurrentUserDataParticleRemoveCallback(ptcl->emitter);
                if (cb)
                {
                    ParticleRemoveArg arg = { .particle = ptcl };
                    cb(arg);
                }

                ptcl->res = NULL;

                if (ptcl->stripe != NULL)
                {
                    RemoveStripe(ptcl->stripe);
                    ptcl->stripe = NULL;
                }
            }

            mPtclRemoveArray[i][j] = NULL;
        }

        mPtclRemoveIdx[i] = 0;
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
    }

    set->mNext = NULL;
    set->mPrev = NULL;

    return next;
}

void System::RemovePtcl()
{
    RemovePtcl_();

    for(u32 i = 0; i < EFT_GROUP_MAX; i++)
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

void System::AddPtclRemoveList(PtclInstance* ptcl, CpuCore core)
{
    mPtclRemoveArray[core][mPtclRemoveIdx[core]] = ptcl;
    mPtclRemoveIdx[core]++;

    ptcl->life = 0;
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

PtclStripe* System::AllocAndConnectStripe(EmitterInstance* emitter, PtclInstance* ptcl)
{
    s32 i = 0;
    do
    {
        mCurStripeIx = (mCurStripeIx + 1) & mNumStripeMask;

        if (mStripes[mCurStripeIx].res == NULL)
        {
            PtclStripe* stripe = &mStripes[mCurStripeIx];
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

            stripe->histQStart = (stripe->histQEnd = 0);
            stripe->ptcl       = ptcl;
            stripe->numHistory = 0;
            stripe->res        = static_cast<const ComplexEmitterData*>(emitter->res);
            stripe->groupID    = emitter->groupID;
            stripe->cnt        = 0;
            stripe->hist[0].outer = nw::math::VEC3::Zero();

            return stripe;
        }
    } while (++i < mNumStripe);

    return NULL;
}

PtclInstance* System::AllocPtcl(PtclType type)
{
    s32 i = 0;
    do
    {
        mCurPtclIx = (mCurPtclIx + 1) & mNumPtclDataMask;

        if (mPtcls[mCurPtclIx].res == NULL)
        {
            mNumEmittedPtcl++;
            mPtcls[mCurPtclIx].type = type;
            return &mPtcls[mCurPtclIx];
        }
    } while (++i < mNumPtclData);

    return NULL;
}

void System::UpdateEmitterResInfo()
{
    for (u32 i = 0; i < EFT_GROUP_MAX; i++)
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
        mCurEmitterSetIx = (mCurEmitterSetIx + 1) & mNumEmitterSetMask;

        if (mEmitterSet[mCurEmitterSetIx].GetNumEmitter() == 0)
        {
            set = &mEmitterSet[mCurEmitterSetIx];
            break;
        }
    } while (++i < mNumEmitterSet);

    handle->mEmitterSet = set;

    if (set == NULL)
        return NULL;

    return set;
}

EmitterInstance* System::AllocEmitter(u8 groupID)
{
    EmitterInstance* emitter = NULL;
    s32 i = 0;
    do
    {
        mCurEmitterIx = (mCurEmitterIx + 1) & mNumEmitterDataMask;

        if (mEmitters[mCurEmitterIx].calc == NULL)
        {
            emitter = &mEmitters[mCurEmitterIx];
            break;
        }
    } while (++i < mNumEmitterData);

    if (emitter == NULL)
        return NULL;

    if (mEmitterHead[groupID] == NULL)
    {
        mEmitterHead[groupID] = emitter;
        emitter->prev = (emitter->next = NULL);
    }
    else
    {
        mEmitterHead[groupID]->prev = emitter;
        emitter->next               = mEmitterHead[groupID];
        mEmitterHead[groupID]       = emitter;
        emitter->prev               = NULL;
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

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif // __GNUC__
