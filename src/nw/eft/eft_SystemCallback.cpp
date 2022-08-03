#include <nw/eft/eft_Data.h>
#include <nw/eft/eft_Emitter.h>
#include <nw/eft/eft_System.h>

namespace nw { namespace eft {

void System::SetUserDataEmitterPreCalcCallback(UserDataCallBackID id, UserDataEmitterPreCalcCallback callback)
{
    if (id < EFT_USER_DATA_CALLBACK_ID_MAX)
        mUserDataEmitterPreCalcCallback[id] = callback;
}

void System::SetUserDataEmitterPostCalcCallback(UserDataCallBackID id, UserDataEmitterPostCalcCallback callback)
{
    if (id < EFT_USER_DATA_CALLBACK_ID_MAX)
        mUserDataEmitterPostCalcCallback[id] = callback;
}

void System::SetUserDataParticleEmitCallback(UserDataCallBackID id, UserDataParticleEmitCallback callback)
{
    if (id < EFT_USER_DATA_CALLBACK_ID_MAX)
        mUserDataParticleEmitCallback[id] = callback;
}

void System::SetUserDataParticleRemoveCallback(UserDataCallBackID id, UserDataParticleRemoveCallback callback)
{
    if (id < EFT_USER_DATA_CALLBACK_ID_MAX)
        mUserDataParticleRemoveCallback[id] = callback;
}

void System::SetUserDataParticleCalcCallback(UserDataCallBackID id, UserDataParticleCalcCallback callback)
{
    if (id < EFT_USER_DATA_CALLBACK_ID_MAX)
        mUserDataParticleCalcCallback[id] = callback;
}

void System::SetUserDataParticleMakeAttributeCallback(UserDataCallBackID id, UserDataParticleMakeAttributeCallback callback)
{
    if (id < EFT_USER_DATA_CALLBACK_ID_MAX)
        mUserDataParticleMakeAttrCallback[id] = callback;
}

void System::SetUserDataEmitterDrawOverrideCallback(UserDataCallBackID id, UserDataEmitterDrawOverrideCallback callback)
{
    if (id < EFT_USER_DATA_CALLBACK_ID_MAX)
        mUserDataEmitterDrawOverrideCallback[id] = callback;
}

UserDataEmitterPreCalcCallback System::GetCurrentUserDataEmitterPreCalcCallback(const EmitterInstance* emitter)
{
    if (mEnableCallbackID != EFT_USER_DATA_CALLBACK_ID_NONE)
        return mUserDataEmitterPreCalcCallback[mEnableCallbackID];

    if (emitter->res->userCallbackID != -1)
        return mUserDataEmitterPreCalcCallback[emitter->res->userCallbackID];

    return NULL;
}

UserDataEmitterPostCalcCallback System::GetCurrentUserDataEmitterPostCalcCallback(const EmitterInstance* emitter)
{
    if (mEnableCallbackID != EFT_USER_DATA_CALLBACK_ID_NONE)
        return mUserDataEmitterPostCalcCallback[mEnableCallbackID];

    if (emitter->res->userCallbackID != -1)
        return mUserDataEmitterPostCalcCallback[emitter->res->userCallbackID];

    return NULL;
}

UserDataParticleEmitCallback System::GetCurrentUserDataParticleEmitCallback(const EmitterInstance* emitter)
{
    if (mEnableCallbackID != EFT_USER_DATA_CALLBACK_ID_NONE)
        return mUserDataParticleEmitCallback[mEnableCallbackID];

    if (emitter->res->userCallbackID != -1)
        return mUserDataParticleEmitCallback[emitter->res->userCallbackID];

    return NULL;
}

UserDataParticleRemoveCallback System::GetCurrentUserDataParticleRemoveCallback(const EmitterInstance* emitter)
{
    if (mEnableCallbackID != EFT_USER_DATA_CALLBACK_ID_NONE)
        return mUserDataParticleRemoveCallback[mEnableCallbackID];

    if (emitter->res->userCallbackID != -1)
        return mUserDataParticleRemoveCallback[emitter->res->userCallbackID];

    return NULL;
}

UserDataParticleCalcCallback System::GetCurrentUserDataParticleCalcCallback(const EmitterInstance* emitter)
{
    if (mEnableCallbackID != EFT_USER_DATA_CALLBACK_ID_NONE)
        return mUserDataParticleCalcCallback[mEnableCallbackID];

    if (emitter->res->userCallbackID != -1)
        return mUserDataParticleCalcCallback[emitter->res->userCallbackID];

    return NULL;
}

UserDataParticleMakeAttributeCallback System::GetCurrentUserDataParticleMakeAttributeCallback(const EmitterInstance* emitter)
{
    if (mEnableCallbackID != EFT_USER_DATA_CALLBACK_ID_NONE)
        return mUserDataParticleMakeAttrCallback[mEnableCallbackID];

    if (emitter->res->userCallbackID != -1)
        return mUserDataParticleMakeAttrCallback[emitter->res->userCallbackID];

    return NULL;
}

UserDataEmitterDrawOverrideCallback System::GetCurrentUserDataEmitterDrawOverrideCallback(const EmitterInstance* emitter)
{
    if (mEnableCallbackID != EFT_USER_DATA_CALLBACK_ID_NONE)
        return mUserDataEmitterDrawOverrideCallback[mEnableCallbackID];

    if (emitter->res->userCallbackID != -1)
        return mUserDataEmitterDrawOverrideCallback[emitter->res->userCallbackID];

    return NULL;
}

UserShaderEmitterPostCalcCallback System::GetUserShaderEmitterPostCalcCallback(UserShaderCallBackID id)
{
    if (id > EFT_USER_SHADER_CALLBACK_MAX) // No idea why not >=
        return NULL;

    return mUserShaderEmitterCalcPostCallback[id];
}

UserShaderDrawOverrideCallback System::GetUserShaderDrawOverrideCallback(UserShaderCallBackID id)
{
    if (id > EFT_USER_SHADER_CALLBACK_MAX) // No idea why not >=
        return NULL;

    return mUserShaderDrawOverrideCallback[id];
}

UserShaderRenderStateSetCallback System::GetUserShaderRenderStateSetCallback(UserShaderCallBackID id)
{
    if (id > EFT_USER_SHADER_CALLBACK_MAX) // No idea why not >=
        return NULL;

    return mUserShaderRenderStateSetCallback[id];
}

void System::SetUserShaderEmitterPostCalcCallback(UserShaderCallBackID id, UserShaderEmitterPostCalcCallback callback)
{
    if (id < EFT_USER_SHADER_CALLBACK_MAX)
        mUserShaderEmitterCalcPostCallback[id] = callback;
}

void System::SetUserShaderDrawOverrideCallback(UserShaderCallBackID id, UserShaderDrawOverrideCallback callback)
{
    if (id < EFT_USER_SHADER_CALLBACK_MAX)
        mUserShaderDrawOverrideCallback[id] = callback;
}

void System::SetUserShaderRenderStateSetCallback(UserShaderCallBackID id, UserShaderRenderStateSetCallback callback)
{
    if (id < EFT_USER_SHADER_CALLBACK_MAX)
        mUserShaderRenderStateSetCallback[id] = callback;
}

} } // namespace nw::eft
