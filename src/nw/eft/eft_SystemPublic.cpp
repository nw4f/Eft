#include <nw/eft/eft_Emitter.h>
#include <nw/eft/eft_EmitterSet.h>
#include <nw/eft/eft_Heap.h>
#include <nw/eft/eft_Particle.h>
#include <nw/eft/eft_Resource.h>
#include <nw/eft/eft_System.h>

#include <new>

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#endif // __GNUC__

namespace nw { namespace eft {

void System::ClearResource(Heap* heap, s32 resId)
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

void System::EntryResource(Heap* heap, void* bin, s32 resId)
{
    if (mResource[resId] != NULL)
        ClearResource(NULL, resId);

    mResource[resId] = new (heap->Alloc(sizeof(Resource))) Resource(heap, bin, resId, this);
}

void System::KillEmitter(EmitterInstance* e)
{
    PtclInstance* ptcl = e->ptclHead;
    while (ptcl != NULL)
    {
        AddPtclRemoveList(ptcl, EFT_CPU_CORE_1);
        ptcl = ptcl->next;
    }

    ptcl = e->childHead;
    while (ptcl != NULL)
    {
        AddPtclRemoveList(ptcl, EFT_CPU_CORE_1);
        ptcl = ptcl->next;
    }

    e->calc = NULL;
    e->emitterSet->mNumEmitter--;
    e->emitterSetCreateID = 0xFFFFFFFF;

    if (e->emitterSet->mNumEmitter == 0)
        mNumEmitterSetCalc--;
    mNumFreeEmitter++;

    if (e == mEmitterHead[e->groupID])
    {
        mEmitterHead[e->groupID] = e->next;

        if (mEmitterHead[e->groupID] != NULL)
            mEmitterHead[e->groupID]->prev = NULL;
    }
    else
    {
        if (e->next != NULL)
            e->next->prev = e->prev;

        //if (e->prev != NULL) <-- No check, because... Nintendo
            e->prev->next = e->next;
    }

    RemovePtcl_();
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

    for (s32 i = 0; i < set->mNumEmitterFirst; i++)
        if (set->mInstance[i]->emitterSetCreateID == set->GetCreateID() && set->mInstance[i]->calc)
            KillEmitter(set->mInstance[i]);

    RemoveEmitterSetFromDrawList(set);
}

void System::KillEmitterSet(const char* emitterSetName, s32 resId)
{
    u32 setId = SearchEmitterSetID(emitterSetName, resId);

    for (s32 i = 0; i < mNumEmitterSet; i++)
        if (mEmitterSet[i].GetEmitterSetID() == setId && mEmitterSet[i].GetResourceID() == resId)
            KillEmitterSet(&mEmitterSet[i]);
}

s32 System::SearchEmitterSetID(const char* name, s32 resId) const
{
    return mResource[resId]->SearchEmitterSetID(name);
}

EmitterInstance* System::GetEmitter(u8 groupID, u32 idx)
{
    u32 cnt = 0;
    EmitterInstance* top = mEmitterHead[groupID];

    while (top)
    {
        if (cnt == idx)
            return top;

        cnt++;
        top = top->next;
    }

    return NULL;
}

} } // namespace nw::eft

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif // __GNUC__
