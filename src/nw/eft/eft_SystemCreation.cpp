#include <nw/eft/eft_Data.h>
#include <nw/eft/eft_Emitter.h>
#include <nw/eft/eft_EmitterSet.h>
#include <nw/eft/eft_Handle.h>
#include <nw/eft/eft_Random.h>
#include <nw/eft/eft_Resource.h>
#include <nw/eft/eft_Shader.h>
#include <nw/eft/eft_System.h>

#if EFT_IS_CAFE_WUT || !EFT_IS_CAFE
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#endif

namespace nw { namespace eft {

void System::InitializeEmitter(EmitterInstance* emitter, const SimpleEmitterData* res, s32 resourceID, s32 emitterSetID, u32 setRndSeed, bool nonUpdateCreateID)
{
    Random* globalRnd = PtclRandom::GetGlobalRandom();

    if (res->randomSeed == 0xFFFFFFFF)
        emitter->rnd.SetSeed(setRndSeed);
    else if (res->randomSeed == 0)
        emitter->rnd.SetSeed(globalRnd->GetU32());
    else
        emitter->rnd.SetSeed(res->randomSeed);

    if (!nonUpdateCreateID)
        emitter->emitterSetCreateID = mEmitterSetCreateID;

    emitter->Init(res);
    emitter->calc = mEmitterCalc[emitter->res->type];
    emitter->controller->SetFollowType(res->ptclFollowType);

    {
        VertexShaderKey vertexKey;
        FragmentShaderKey fragmentKey;

        if (emitter->res->type == EFT_EMITTER_TYPE_COMPLEX)
        {
            vertexKey.MakeKeyFromComplexEmitterData(
                static_cast<const ComplexEmitterData*>(emitter->res),
                NULL
            );
        }
        else
        {
            vertexKey.MakeKeyFromSimpleEmitterData(
                emitter->res,
                NULL
            );
        }

        fragmentKey.MakeKeyFromSimpleEmitterData(emitter->res, NULL);

        emitter->shader[EFT_SHADER_TYPE_NORMAL] = mResource[resourceID]->GetShader(emitterSetID, &vertexKey, &fragmentKey);
    }

    if (strlen((char*)emitter->res->userShaderDefine1) > 0)
    {
        VertexShaderKey vertexKey;
        FragmentShaderKey fragmentKey;

        if (emitter->res->type == EFT_EMITTER_TYPE_COMPLEX)
        {
            vertexKey.MakeKeyFromComplexEmitterData(
                static_cast<const ComplexEmitterData*>(emitter->res),
                (char*)emitter->res->userShaderDefine1
            );
        }
        else
        {
            vertexKey.MakeKeyFromSimpleEmitterData(
                emitter->res,
                (char*)emitter->res->userShaderDefine1
            );
        }

        fragmentKey.MakeKeyFromSimpleEmitterData(emitter->res, (char*)emitter->res->userShaderDefine1);

        emitter->shader[EFT_SHADER_TYPE_USER_DEF1] = mResource[resourceID]->GetShader(emitterSetID, &vertexKey, &fragmentKey);
    }

    if (strlen((char*)emitter->res->userShaderDefine2) > 0)
    {
        VertexShaderKey vertexKey;
        FragmentShaderKey fragmentKey;

        if (emitter->res->type == EFT_EMITTER_TYPE_COMPLEX)
        {
            vertexKey.MakeKeyFromComplexEmitterData(
                static_cast<const ComplexEmitterData*>(emitter->res),
                (char*)emitter->res->userShaderDefine2
            );
        }
        else
        {
            vertexKey.MakeKeyFromSimpleEmitterData(
                emitter->res,
                (char*)emitter->res->userShaderDefine2
            );
        }

        fragmentKey.MakeKeyFromSimpleEmitterData(emitter->res, (char*)emitter->res->userShaderDefine2);

        emitter->shader[EFT_SHADER_TYPE_USER_DEF2] = mResource[resourceID]->GetShader(emitterSetID, &vertexKey, &fragmentKey);
    }

    if (emitter->res->type == EFT_EMITTER_TYPE_COMPLEX)
    {
        const ComplexEmitterData* complex = static_cast<const ComplexEmitterData*>(emitter->res);
        if (complex->childFlg & EFT_CHILD_FLAG_ENABLE)
        {
            const ChildData* cres = reinterpret_cast<const ChildData*>(complex + 1);
            {
                VertexShaderKey vertexKey;
                FragmentShaderKey fragmentKey;

                vertexKey  .MakeKeyFromChildData(cres, NULL);
                fragmentKey.MakeKeyFromChildData(cres, NULL, complex->childFlg);

                emitter->childShader[EFT_SHADER_TYPE_NORMAL] = mResource[resourceID]->GetShader(emitterSetID, &vertexKey, &fragmentKey);
            }

            if (strlen((char*)cres->childUserShaderDefine1) > 0)
            {
                VertexShaderKey vertexKey;
                FragmentShaderKey fragmentKey;

                vertexKey  .MakeKeyFromChildData(cres, (char*)cres->childUserShaderDefine1);
                fragmentKey.MakeKeyFromChildData(cres, (char*)cres->childUserShaderDefine1, complex->childFlg);

                emitter->childShader[EFT_SHADER_TYPE_USER_DEF1] = mResource[resourceID]->GetShader(emitterSetID, &vertexKey, &fragmentKey);
            }

            if (strlen((char*)cres->childUserShaderDefine2) > 0)
            {
                VertexShaderKey vertexKey;
                FragmentShaderKey fragmentKey;

                vertexKey  .MakeKeyFromChildData(cres, (char*)cres->childUserShaderDefine2);
                fragmentKey.MakeKeyFromChildData(cres, (char*)cres->childUserShaderDefine2, complex->childFlg);

                emitter->childShader[EFT_SHADER_TYPE_USER_DEF2] = mResource[resourceID]->GetShader(emitterSetID, &vertexKey, &fragmentKey);
            }
        }
    }

    emitter->UpdateResInfo();

    emitter->primitive = NULL;
    if (emitter->res->meshType == EFT_MESH_TYPE_PRIMITIVE)
        emitter->primitive = mResource[resourceID]->GetPrimitive(emitterSetID, emitter->res->primitiveFigure.index);

    emitter->childPrimitive = NULL;
    if (emitter->res->type == EFT_EMITTER_TYPE_COMPLEX)
    {
        const ComplexEmitterData* complex = static_cast<const ComplexEmitterData*>(emitter->res);
        if (complex->childFlg & EFT_CHILD_FLAG_ENABLE)
        {
            const ChildData* cres = reinterpret_cast<const ChildData*>(complex + 1);
            // if (cres->childMeshType == EFT_MESH_TYPE_PRIMITIVE) <-- Nintendo forgot to do this
                emitter->childPrimitive = mResource[resourceID]->GetPrimitive(emitterSetID, cres->childPrimitiveFigure.index);
        }
    }
}

bool System::CreateEmitterSetID(Handle* handle, const nw::math::MTX34& mtx, s32 emitterSetID, s32 resourceID, u8 groupID, u32 emitterMask)
{
    Random* globalRnd = PtclRandom::GetGlobalRandom();

    s32 numEmitter = mResource[resourceID]->GetNumEmitter(emitterSetID);
    if (numEmitter > mNumFreeEmitter)
        return false;

    EmitterSet* set = AllocEmitterSet(handle);
    if (set == NULL)
        return false;

    {
        set->Reset(mtx);
        set->mEmitterCreateID   = mEmitterSetCreateID;
        set->mResourceID        = resourceID;
        set->mEmitterSetID      = emitterSetID;
        set->mNext              = NULL;
        set->mPrev              = NULL;
        set->mGroupID           = groupID;
        set->mRuntimeUserData   = 0;

        AddEmitterSetToDrawList(set, groupID);
    }

    u32 rndSeed = globalRnd->GetU32();

    handle->mCreateID = mEmitterSetCreateID;

    for (s32 emitterID = numEmitter - 1; emitterID >= 0; emitterID--)
    {
        if (!(emitterMask & 1 << emitterID))
            continue;

        EmitterInstance* emitter = AllocEmitter(groupID);
        if (emitter == NULL)
            break;

        const SimpleEmitterData* res = static_cast<const SimpleEmitterData*>(mResource[resourceID]->GetEmitterData(emitterSetID, emitterID));

        set->mInstance[set->mNumEmitter++] = emitter;

        emitter->emitterSet = set;
        emitter->controller = &set->mController[emitterID];
        emitter->controller->mEmitter = emitter;
        emitter->controller->Reset();
        emitter->groupID    = groupID;

        set->mDrawPathFlag |= 1 << res->drawPath;

        InitializeEmitter(emitter, res, resourceID, emitterSetID, rndSeed);

        if (emitter->res->ptclLife == EFT_INFINIT_LIFE)
            set->mIsHaveInfinityEmitter = true;
    }

    set->mNumEmitterFirst = set->mNumEmitter;

    mNumEmitterSetCalc++;
    mEmitterSetCreateID++;

    return true;
}

void System::ReCreateEmitter(void** resList, s32 numResList, s32 resId, s32 setId, bool killOnly)
{
    for (s32 i = 0; i < mNumEmitterSet; i++)
    {
        EmitterSet* set = &mEmitterSet[i];

        bool isReCreate = false;
        u8   groupId    = 0;

        for (s32 j = 0; j < set->mNumEmitterFirst; j++)
        {
            EmitterInstance* emitter = set->mInstance[j];
            if (emitter->calc != NULL && emitter->emitterSetCreateID == set->GetCreateID())
            {
                for (s32 k = 0; k < numResList; k++)
                {
                    if (emitter->res == resList[k])
                    {
                        isReCreate = true;
                        groupId    = emitter->groupID;
                        break;
                    }
                }
            }
        }

        if (isReCreate)
        {
            if (killOnly)
                set->Kill();
            else
                ReCreateEmitter(set, resId, setId, groupId);
        }
    }
}

void System::ReCreateEmitter(EmitterSet* set, s32 resourceID, s32 emitterSetID, u8 groupID)
{
    Random* globalRnd = PtclRandom::GetGlobalRandom();
    u32 rndSeed = globalRnd->GetU32();

    for (s32 i = set->mNumEmitterFirst; i < EFT_EMITTER_INSET_NUM; i++)
        set->mController[i] = set->mController[0];

    EmitterSet saveSet = *set;

    for (s32 i = 0; i < set->mNumEmitterFirst; i++)
        if (set->mInstance[i]->emitterSetCreateID == set->GetCreateID() && set->mInstance[i]->calc)
            KillEmitter(set->mInstance[i]);

    *set = saveSet;
    set->mNumEmitter = 0;
    set->mIsHaveInfinityEmitter = false;

    s32 numEmitter = mResource[resourceID]->GetNumEmitter(emitterSetID);

    if (numEmitter > mNumFreeEmitter)
        return;

    for (s32 emitterID = numEmitter - 1; emitterID >= 0; emitterID--)
    {
        EmitterInstance* emitter = AllocEmitter(groupID);
        if (emitter == NULL)
            break;

        const SimpleEmitterData* res = static_cast<const SimpleEmitterData*>(mResource[resourceID]->GetEmitterData(emitterSetID, emitterID));

        set->mInstance[set->mNumEmitter++] = emitter;

        emitter->emitterSet = set;
        emitter->controller = &set->mController[emitterID];
        emitter->controller->mEmitter = emitter;
        emitter->controller->Reset();
        emitter->groupID    = groupID;

        set->mDrawPathFlag |= 1 << res->drawPath;

        emitter->emitterSetCreateID = set->mEmitterCreateID;

        InitializeEmitter(emitter, res, resourceID, emitterSetID, rndSeed, true);

        if (emitter->res->ptclLife == EFT_INFINIT_LIFE)
            set->mIsHaveInfinityEmitter = true;
    }

    set->mNumEmitterFirst = set->mNumEmitter;

    mNumEmitterSetCalc++;
}

} } // namespace nw::eft

#if EFT_IS_CAFE_WUT || !EFT_IS_CAFE
#pragma GCC diagnostic pop
#endif
