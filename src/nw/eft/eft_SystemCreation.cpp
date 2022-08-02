#include <nw/eft/eft_Data.h>
#include <nw/eft/eft_Emitter.h>
#include <nw/eft/eft_EmitterSet.h>
#include <nw/eft/eft_Handle.h>
#include <nw/eft/eft_Misc.h>
#include <nw/eft/eft_Random.h>
#include <nw/eft/eft_Resource.h>
#include <nw/eft/eft_Shader.h>
#include <nw/eft/eft_System.h>

namespace nw { namespace eft {

void System::InitializeEmitter(EmitterInstance* emitter,
                               const SimpleEmitterData* res,
                               EmitterStaticUniformBlock* emitterStaticUbo,
                               EmitterStaticUniformBlock* childStaticUbo,
                               u32 resourceID,
                               s32 emitterSetID,
                               u32 setRndSeed,
                               bool nonUpdateCreateID)
{
    Random* globalRnd = PtclRandom::GetGlobalRandom();

    switch (res->randomSeed)
    {
    case 0:
        emitter->rnd.SetSeed(globalRnd->GetU32());
        break;
    case 0xFFFFFFFF:
        emitter->rnd.SetSeed(setRndSeed);
        break;
    default:
        emitter->rnd.SetSeed(res->randomSeed);
        break;
    }

    if (!nonUpdateCreateID)
        emitter->emitterSetCreateID = mEmitterSetCreateID;

    emitter->emitterStaticUniformBlock      = emitterStaticUbo;
    emitter->childEmitterStaticUniformBlock = childStaticUbo;

    emitter->Init(res);

    if (res->volumeType == EFT_VOLUME_TYPE_PRIMITIVE)
        emitter->emitVolumePrimitive = mResource[resourceID]->GetPrimitive(emitterSetID, res->emitVolumeFigure.index);

    else
        emitter->emitVolumePrimitive = NULL;

    const ChildData* childData = emitter->GetChildData();

    emitter->shader[EFT_SHADER_TYPE_NORMAL] = mResource[resourceID]->GetShader(emitterSetID, emitter->res->shaderIndex);

    emitter->shader[EFT_SHADER_TYPE_USER_DEF1] = NULL;
    if (emitter->res->userShaderIndex1 != 0)
        emitter->shader[EFT_SHADER_TYPE_USER_DEF1] = mResource[resourceID]->GetShader(emitterSetID, emitter->res->userShaderIndex1);

    emitter->shader[EFT_SHADER_TYPE_USER_DEF2] = NULL;
    if (emitter->res->userShaderIndex2 != 0)
        emitter->shader[EFT_SHADER_TYPE_USER_DEF2] = mResource[resourceID]->GetShader(emitterSetID, emitter->res->userShaderIndex2);

    if (childData)
    {
        emitter->childShader[EFT_SHADER_TYPE_NORMAL] = mResource[resourceID]->GetShader(emitterSetID, childData->childShaderIndex);

        emitter->childShader[EFT_SHADER_TYPE_USER_DEF1] = NULL;
        if (childData->childUserShaderIndex1 != 0)
            emitter->childShader[EFT_SHADER_TYPE_USER_DEF1] = mResource[resourceID]->GetShader(emitterSetID, childData->childUserShaderIndex1);

        emitter->childShader[EFT_SHADER_TYPE_USER_DEF2] = NULL;
        if (childData->childUserShaderIndex2 != 0)
            emitter->childShader[EFT_SHADER_TYPE_USER_DEF2] = mResource[resourceID]->GetShader(emitterSetID, childData->childUserShaderIndex2);
    }

    if (emitter->shader[EFT_SHADER_TYPE_NORMAL]->GetVertexShaderKey().IsUseGpuAcceleration())
        emitter->calc = mEmitterCalc[EFT_EMITTER_TYPE_SIMPLE_GPU];

    else
        emitter->calc = mEmitterCalc[emitter->res->type];

    emitter->primitive = NULL;
    if (emitter->res->meshType == EFT_MESH_TYPE_PRIMITIVE)
        emitter->primitive = mResource[resourceID]->GetPrimitive(emitterSetID, emitter->res->primitiveFigure.index);

    emitter->childPrimitive = NULL;
    if (childData)
    {
        if(childData->childMeshType == EFT_MESH_TYPE_PRIMITIVE)
            emitter->childPrimitive = mResource[resourceID]->GetPrimitive(emitterSetID, childData->childPrimitiveFigure.index);
    }

    emitter->UpdateResInfo();

    emitter->userCustomPtr = NULL;

    CustomShaderCallBackID shaderCallback = static_cast<CustomShaderCallBackID>(res->userShaderSetting);
    if (GetCustomShaderEmitterInitializeCallback(shaderCallback))
    {
        ShaderEmitterInitializeArg arg;
        arg.emitter = emitter;
        GetCustomShaderEmitterInitializeCallback(shaderCallback)(arg);
    }
}

bool System::CreateEmitterSetID(Handle* handle, const math::MTX34& mtx, s32 emitterSetID, u32 resourceID, u8 groupID, u32 emitterMask)
{
    if (resourceID >= mNumResource || mResource[resourceID] == NULL || mResource[resourceID]->GetNumEmitterSet() <= emitterSetID)
        return false;

    Random* globalRnd = PtclRandom::GetGlobalRandom();

    s32 numEmitter = mResource[resourceID]->GetNumEmitter(emitterSetID);
    if (numEmitter > mNumFreeEmitter)
    {
        WARNING("Emitter is Empty.\n");
        return false;
    }

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

        const SimpleEmitterData*   res  = static_cast<const SimpleEmitterData*>(mResource[resourceID]->GetEmitterData(emitterSetID, emitterID));
        EmitterStaticUniformBlock* eubo = mResource[resourceID]->GetEmitterStaticUBO(emitterSetID, emitterID);
        EmitterStaticUniformBlock* cubo = mResource[resourceID]->GetChildEmitterStaticUBO(emitterSetID, emitterID);

        set->mInstance[set->mNumEmitter++] = emitter;
        emitter->emitterSet = set;

        emitter->groupID = groupID;

        emitter->controller = &set->mController[emitterID];
        emitter->controller->mEmitter = emitter;
        emitter->controller->Reset();

        set->mDrawPathFlag |= 1 << res->drawPath;

        InitializeEmitter(emitter, res, eubo, cubo, resourceID, emitterSetID, rndSeed);

        if (emitter->res->ptclLife == EFT_INFINIT_LIFE)
            set->mIsHaveInfinityEmitter = true;
    }

    set->mNumEmitterFirst = set->mNumEmitter;

    mNumEmitterSetCalc++;
    mEmitterSetCreateID++;

    return true;
}

void System::ReCreateEmitter(void** resList, s32 numResList, u32 resId, s32 setId, bool killOnly)
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

void System::ReCreateEmitter(EmitterSet* set, u32 resourceID, s32 emitterSetID, u8 groupID)
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
    {
        WARNING("EmitterInstance is Empty.\n");
        return;
    }

    for (s32 emitterID = numEmitter - 1; emitterID >= 0; emitterID--)
    {
        EmitterInstance* emitter = AllocEmitter(groupID);
        if (emitter == NULL)
            break;

        const SimpleEmitterData*   res  = static_cast<const SimpleEmitterData*>(mResource[resourceID]->GetEmitterData(emitterSetID, emitterID));
        EmitterStaticUniformBlock* eubo = mResource[resourceID]->GetEmitterStaticUBO(emitterSetID, emitterID);
        EmitterStaticUniformBlock* cubo = mResource[resourceID]->GetChildEmitterStaticUBO(emitterSetID, emitterID);

        set->mInstance[set->mNumEmitter++] = emitter;

        emitter->emitterSet = set;
        emitter->controller = &set->mController[emitterID];
        emitter->controller->mEmitter = emitter;
        emitter->controller->Reset();
        emitter->groupID    = groupID;

        set->mDrawPathFlag |= 1 << res->drawPath;

        emitter->emitterSetCreateID = set->mEmitterCreateID;;

        InitializeEmitter(emitter, res, eubo, cubo, resourceID, emitterSetID, rndSeed, true);

        if (emitter->res->ptclLife == EFT_INFINIT_LIFE)
            set->mIsHaveInfinityEmitter = true;
    }

    set->mNumEmitterFirst = set->mNumEmitter;

    mNumEmitterSetCalc++;
}

void System::ReCreateEmitterSet(EmitterSet* set, u32 resId, s32 setId, u8 groupID)
{
    ReCreateEmitter(set, resId, setId, groupID);

    set->mResourceID    = resId;
    set->mEmitterSetID  = setId;
}

} } // namespace nw::eft
