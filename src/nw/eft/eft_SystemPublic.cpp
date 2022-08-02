#include <nw/eft/eft_Emitter.h>
#include <nw/eft/eft_EmitterSet.h>
#include <nw/eft/eft_Heap.h>
#include <nw/eft/eft_Misc.h>
#include <nw/eft/eft_Particle.h>
#include <nw/eft/eft_Resource.h>
#include <nw/eft/eft_System.h>

#include <new>

namespace nw { namespace eft {

void System::ClearResource(Heap* heap, u32 resId)
{
    if (heap != NULL)
    {
        mResource[resId]->Finalize(heap);
        heap->Free(mResource[resId]);
    }

    else
    {
        Heap* heapTemp = mResource[resId]->GetHeap();
        mResource[resId]->Finalize(heapTemp);
        heapTemp->Free(mResource[resId]);
    }

    mResource[resId] = NULL;
}

void System::EntryResource(Heap* heap, void* bin, u32 resId, bool delayCompile)
{
    if ((u32)bin % 128)
        ERROR("resource memory must be 128 byte alignment.\n");

    if (mResource[resId] != NULL)
        ClearResource(NULL, resId);

    mResource[resId] = new (heap->Alloc(sizeof(Resource))) Resource(heap, bin, resId, this, delayCompile);
}

void System::KillAllEmitter()
{
    memset(mEmitterHead,    0, EFT_GROUP_MAX * sizeof(EmitterInstance*));
    memset(mEmitterSetHead, 0, EFT_GROUP_MAX * sizeof(EmitterSet*));
    memset(mEmitterSetTail, 0, EFT_GROUP_MAX * sizeof(EmitterSet*));
    memset(mStripeHead,     0, EFT_GROUP_MAX * sizeof(PtclStripe*));

    ShaderEmitterFinalizeArg arg;

    for (s32 i = 0; i < mNumEmitterData; i++)
    {
        if (mEmitter[i].calc)
        {
            CustomShaderCallBackID shaderCallback = static_cast<CustomShaderCallBackID>(mEmitter[i].res->userShaderSetting);
            CustomShaderEmitterFinalizeCallback cb = GetCustomShaderEmitterFinalizeCallback(shaderCallback);
            if (cb)
            {
                arg.emitter = &mEmitter[i];
                cb(arg);
            }

            if (mEmitter[i].gpuParticleBufferNum > 0 && mEmitter[i].ptclAttributeBufferGpu)
            {
                FreeFromDynamicHeap(mEmitter[i].ptclAttributeBufferGpu);
                mEmitter[i].ptclAttributeBufferGpu = NULL;
                mEmitter[i].gpuParticleBufferNum = 0;

                mEmitter[i].positionStreamOutBuffer.Finalize();
                mEmitter[i].vectorStreamOutBuffer.Finalize();
            }
        }

        mEmitter[i].calc = NULL;
    }

    for (s32 i = 0; i < mNumPtclData; i++)
        mPtcl[i].res = NULL;

    for (u32 i = 0; i < mNumStripeData; i++)
        mStripe[i].res = NULL;

    for (s32 i = 0; i < mNumEmitterSet; i++)
        mEmitterSet[i].mNumEmitter = 0;

    mNumEmitterSetCalc  = 0;
    mNumFreeEmitter     = mNumEmitterData;
    mNumStripeCalc      = 0;
    mCurPtclIx          = 0;

    for (u32 i = 0; i < EFT_CPU_CORE_MAX; ++i)
    {
        mStripeRemoveIdx[i] = 0;
        mPtclAdditionIdx[i] = 0;

        for (s32 j = 0; j < mNumPtclData; ++j)
            mPtclAdditionArray[i][j] = NULL;

        for (u32 j = 0; j < mNumStripeData; ++j)
            mStripeRemoveArray[i][j] = NULL;
    }
}

void System::KillEmitter(EmitterInstance* emitter)
{
    if (emitter->calc == NULL)
        ERROR("Emitter Double Kill.\n");

    PtclInstance* ptcl = emitter->ptclHead;

    while (ptcl != NULL)
    {
        CustomActionParticleRemoveCallback cb = GetCurrentCustomActionParticleRemoveCallback(ptcl->emitter);
        if (cb)
        {
            ParticleRemoveArg arg;
            arg.particle = ptcl;
            cb(arg);
        }

        ptcl->res = NULL;

        if (ptcl->complexParam && ptcl->complexParam->stripe)
            AddStripeRemoveList(ptcl->complexParam->stripe, EFT_CPU_CORE_1);

        ptcl = ptcl->next;
    }

    ptcl = emitter->childHead;
    while (ptcl != NULL)
    {
        CustomActionParticleRemoveCallback cb = GetCurrentCustomActionParticleRemoveCallback(ptcl->emitter);
        if (cb)
        {
            ParticleRemoveArg arg;
            arg.particle = ptcl;
            cb(arg);
        }

        ptcl->res = NULL;
        ptcl = ptcl->next;
    }

    if (emitter == mEmitterHead[emitter->groupID])
    {
        mEmitterHead[emitter->groupID] = emitter->next;

        if (mEmitterHead[emitter->groupID] != NULL)
            mEmitterHead[emitter->groupID]->prev = NULL;
    }
    else
    {
        if (emitter->next != NULL)
            emitter->next->prev = emitter->prev;

        //if (emitter->prev != NULL) <-- No check, because... Nintendo
            emitter->prev->next = emitter->next;
    }

    CustomShaderCallBackID shaderCallback = static_cast<CustomShaderCallBackID>(emitter->res->userShaderSetting);
    if (GetCustomShaderEmitterFinalizeCallback(shaderCallback))
    {
        ShaderEmitterFinalizeArg arg;
        arg.emitter = emitter;
        GetCustomShaderEmitterFinalizeCallback(shaderCallback)(arg);
    }

    if (emitter->gpuParticleBufferNum != 0 && emitter->ptclAttributeBufferGpu)
    {
        FreeFromDynamicHeap(emitter->ptclAttributeBufferGpu, false);
        emitter->ptclAttributeBufferGpu = NULL;
        emitter->gpuParticleBufferNum = 0;

        emitter->positionStreamOutBuffer.Finalize();
        emitter->vectorStreamOutBuffer.Finalize();
    }

    RemoveStripe_();

    if (emitter->emitterSet)
        emitter->emitterSet->mNumEmitter--;

    emitter->emitterSetCreateID = 0xFFFFFFFF;

    emitter->calc = NULL;
    mNumFreeEmitter++;

    if(emitter->emitterSet->mNumEmitter == 0)
        mNumEmitterSetCalc--;
}

void System::KillEmitterGroup(u8 groupID)
{
    EmitterInstance* emitter = mEmitterHead[groupID];
    while (emitter != NULL)
    {
        KillEmitter(emitter);
        emitter = emitter->next;
    }

    mEmitterSetHead[groupID] = NULL;
    mEmitterSetTail[groupID] = NULL;
}

void System::KillEmitterSet(EmitterSet* set)
{
    if (!set->IsAlive())
        return;

    RemoveEmitterSetFromDrawList(set);

    for (s32 i = 0; i < set->mNumEmitterFirst; i++)
    {
        if (set->mInstance[i]->emitterSetCreateID == set->GetCreateID() &&
            set->mInstance[i]->calc)
        {
            KillEmitter(set->mInstance[i]);
        }
    }
}

s32 System::SearchEmitterSetID(const char* name, s32 resId) const
{
    if (mResource[resId] == NULL)
        return EFT_INVALID_EMITTER_SET_ID;

    return mResource[resId]->SearchEmitterSetID(name);
}

void System::KillEmitterSet(const char* emitterSetName, u32 resId)
{
    s32 setId = SearchEmitterSetID(emitterSetName, resId);

    for (s32 i = 0; i < mNumEmitterSet; i++)
    {
        if (mEmitterSet[i].GetEmitterSetID() == setId &&
            mEmitterSet[i].GetResourceID()   == resId)
        {
            KillEmitterSet(&mEmitterSet[i]);
        }
    }
}

const char* System::SearchEmitterSetName(s32 emsetId, s32 resId) const
{
    if (mResource[resId] == NULL)
        return NULL;

    return mResource[resId]->GetEmitterSetName(emsetId);
}

void System::SetShaderType(ShaderType type, CpuCore core)
{
    mRenderer[core]->SetShaderType(type);
}

} } // namespace nw::eft
