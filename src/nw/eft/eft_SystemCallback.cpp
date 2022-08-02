#include <nw/eft/eft_Data.h>
#include <nw/eft/eft_Emitter.h>
#include <nw/eft/eft_System.h>

namespace nw { namespace eft {

void System::SetCustomActionEmitterMatrixSetCallback(CustomActionCallBackID id, CustomActionEmitterMatrixSetCallback callback)
{
    mCustomActionEmitterMatrixSetCallback[id] = callback;
}

void System::SetCustomActionEmitterPreCalcCallback(CustomActionCallBackID id, CustomActionEmitterPreCalcCallback callback)
{
    mCustomActionEmitterPreCalcCallback[id] = callback;
}

void System::SetCustomActionEmitterPostCalcCallback(CustomActionCallBackID id, CustomActionEmitterPostCalcCallback callback)
{
    mCustomActionEmitterPostCalcCallback[id] = callback;
}

void System::SetCustomActionParticleEmitCallback(CustomActionCallBackID id, CustomActionParticleEmitCallback callback)
{
    mCustomActionParticleEmitCallback[id] = callback;
}

void System::SetCustomActionParticleRemoveCallback(CustomActionCallBackID id, CustomActionParticleRemoveCallback callback)
{
    mCustomActionParticleRemoveCallback[id] = callback;
}

void System::SetCustomActionParticleCalcCallback(CustomActionCallBackID id, CustomActionParticleCalcCallback callback)
{
    mCustomActionParticleCalcCallback[id] = callback;
}

void System::SetCustomActionParticleMakeAttributeCallback(CustomActionCallBackID id, CustomActionParticleMakeAttributeCallback callback)
{
    mCustomActionParticleMakeAttrCallback[id] = callback;
}

void System::SetCustomActionEmitterDrawOverrideCallback(CustomActionCallBackID id, CustomActionEmitterDrawOverrideCallback callback)
{
    mCustomActionEmitterDrawOverrideCallback[id] = callback;
}

CustomActionEmitterMatrixSetCallback System::GetCurrentCustomActionEmitterMatrixSetCallback(const EmitterInstance* emitter)
{
    if (mEnableCallbackID != EFT_CUSTOM_ACTION_CALLBACK_ID_NONE)
        return mCustomActionEmitterMatrixSetCallback[mEnableCallbackID];

    return mCustomActionEmitterMatrixSetCallback[emitter->res->userCallbackID];
}

CustomActionEmitterPreCalcCallback System::GetCurrentCustomActionEmitterPreCalcCallback(const EmitterInstance* emitter)
{
    if (mEnableCallbackID != EFT_CUSTOM_ACTION_CALLBACK_ID_NONE)
        return mCustomActionEmitterPreCalcCallback[mEnableCallbackID];

    return mCustomActionEmitterPreCalcCallback[emitter->res->userCallbackID];
}

CustomActionEmitterPostCalcCallback System::GetCurrentCustomActionEmitterPostCalcCallback(const EmitterInstance* emitter)
{
    if (mEnableCallbackID != EFT_CUSTOM_ACTION_CALLBACK_ID_NONE)
        return mCustomActionEmitterPostCalcCallback[mEnableCallbackID];

    return mCustomActionEmitterPostCalcCallback[emitter->res->userCallbackID];
}

CustomActionParticleEmitCallback System::GetCurrentCustomActionParticleEmitCallback(const EmitterInstance* emitter)
{
    if (mEnableCallbackID != EFT_CUSTOM_ACTION_CALLBACK_ID_NONE)
        return mCustomActionParticleEmitCallback[mEnableCallbackID];

    return mCustomActionParticleEmitCallback[emitter->res->userCallbackID];
}

CustomActionParticleRemoveCallback System::GetCurrentCustomActionParticleRemoveCallback(const EmitterInstance* emitter)
{
    if (mEnableCallbackID != EFT_CUSTOM_ACTION_CALLBACK_ID_NONE)
        return mCustomActionParticleRemoveCallback[mEnableCallbackID];

    return mCustomActionParticleRemoveCallback[emitter->res->userCallbackID];
}

CustomActionParticleCalcCallback System::GetCurrentCustomActionParticleCalcCallback(const EmitterInstance* emitter)
{
    if (mEnableCallbackID != EFT_CUSTOM_ACTION_CALLBACK_ID_NONE)
        return mCustomActionParticleCalcCallback[mEnableCallbackID];

    return mCustomActionParticleCalcCallback[emitter->res->userCallbackID];
}

CustomActionParticleMakeAttributeCallback System::GetCurrentCustomActionParticleMakeAttributeCallback(const EmitterInstance* emitter)
{
    if (mEnableCallbackID != EFT_CUSTOM_ACTION_CALLBACK_ID_NONE)
        return mCustomActionParticleMakeAttrCallback[mEnableCallbackID];

    return mCustomActionParticleMakeAttrCallback[emitter->res->userCallbackID];
}

CustomActionEmitterDrawOverrideCallback System::GetCurrentCustomActionEmitterDrawOverrideCallback(const EmitterInstance* emitter)
{
    if (mEnableCallbackID != EFT_CUSTOM_ACTION_CALLBACK_ID_NONE)
        return mCustomActionEmitterDrawOverrideCallback[mEnableCallbackID];

    return mCustomActionEmitterDrawOverrideCallback[emitter->res->userCallbackID];
}

bool System::SetCurrentCustomActionCallback(CustomActionCallBackID id)
{
    if (id < EFT_CUSTOM_ACTION_CALLBACK_ID_MAX)
    {
        mEnableCallbackID = id;
        return true;
    }

    return false;
}

CustomShaderEmitterInitializeCallback System::GetCustomShaderEmitterInitializeCallback(CustomShaderCallBackID id)
{
    if (id > EFT_CUSTOM_SHADER_CALLBACK_MAX) // No idea why not >=
        return NULL;

    return mCustomShaderEmitterInitializeCallback[id];
}

CustomShaderEmitterFinalizeCallback System::GetCustomShaderEmitterFinalizeCallback(CustomShaderCallBackID id)
{
    if (id > EFT_CUSTOM_SHADER_CALLBACK_MAX) // No idea why not >=
        return NULL;

    return mCustomShaderEmitterFinalizeCallback[id];
}

CustomShaderEmitterPreCalcCallback System::GetCustomShaderEmitterPreCalcCallback(CustomShaderCallBackID id)
{
    if (id > EFT_CUSTOM_SHADER_CALLBACK_MAX) // No idea why not >=
        return NULL;

    return mCustomShaderEmitterCalcPreCallback[id];
}

CustomShaderEmitterPostCalcCallback System::GetCustomShaderEmitterPostCalcCallback(CustomShaderCallBackID id)
{
    if (id > EFT_CUSTOM_SHADER_CALLBACK_MAX) // No idea why not >=
        return NULL;

    return mCustomShaderEmitterCalcPostCallback[id];
}

CustomShaderDrawOverrideCallback System::GetCustomShaderDrawOverrideCallback(CustomShaderCallBackID id)
{
    if (id > EFT_CUSTOM_SHADER_CALLBACK_MAX) // No idea why not >=
        return NULL;

    return mCustomShaderDrawOverrideCallback[id];
}

CustomShaderRenderStateSetCallback System::GetCustomShaderRenderStateSetCallback(CustomShaderCallBackID id)
{
    if (id > EFT_CUSTOM_SHADER_CALLBACK_MAX) // No idea why not >=
        return NULL;

    return mCustomShaderRenderStateSetCallback[id];
}

void System::SetCustomShaderEmitterInitialzieCallback(CustomShaderCallBackID id, CustomShaderEmitterInitializeCallback callback)
{
    if (id < EFT_CUSTOM_SHADER_CALLBACK_MAX)
        mCustomShaderEmitterInitializeCallback[id] = callback;
}

void System::SetCustomShaderEmitterFinalzieCallback(CustomShaderCallBackID id, CustomShaderEmitterFinalizeCallback callback)
{
    if (id < EFT_CUSTOM_SHADER_CALLBACK_MAX)
        mCustomShaderEmitterFinalizeCallback[id] = callback;
}

void System::SetCustomShaderEmitterPreCalcCallback(CustomShaderCallBackID id, CustomShaderEmitterPreCalcCallback callback)
{
    if (id < EFT_CUSTOM_SHADER_CALLBACK_MAX)
        mCustomShaderEmitterCalcPreCallback[id] = callback;
}

void System::SetCustomShaderEmitterPostCalcCallback(CustomShaderCallBackID id, CustomShaderEmitterPostCalcCallback callback)
{
    if (id < EFT_CUSTOM_SHADER_CALLBACK_MAX)
        mCustomShaderEmitterCalcPostCallback[id] = callback;
}

void System::SetCustomShaderDrawOverrideCallback(CustomShaderCallBackID id, CustomShaderDrawOverrideCallback callback)
{
    if (id < EFT_CUSTOM_SHADER_CALLBACK_MAX)
        mCustomShaderDrawOverrideCallback[id] = callback;
}

void System::SetCustomShaderRenderStateSetCallback(CustomShaderCallBackID id, CustomShaderRenderStateSetCallback callback)
{
    if (id < EFT_CUSTOM_SHADER_CALLBACK_MAX)
        mCustomShaderRenderStateSetCallback[id] = callback;
}

DrawPathRenderStateSetCallback System::GetDrawPathRenderStateSetCallback(DrawPathFlag flag)
{
    if (flag == 0)
        return NULL;

    for (s32 i = 0; i < EFT_DRAW_PATH_CALLBACK_MAX; i++)
        if (flag & mDrawPathCallbackFlag[i])
            return mDrawPathRenderStateSetCallback[i];

    return NULL;
}

void System::SetDrawPathRenderStateSetCallback(DrawPathCallBackID id, DrawPathFlag flag, DrawPathRenderStateSetCallback callback)
{
    if (id < EFT_DRAW_PATH_CALLBACK_MAX)
    {
        mDrawPathCallbackFlag[id] = flag;
        mDrawPathRenderStateSetCallback[id] = callback;
    }
}

void System::SetCustomShaderCallback(CustomShaderRenderStateSetCallback callback, CustomShaderCallBackID id)
{
    if (id > EFT_CUSTOM_SHADER_CALLBACK_MAX) // No idea why not >=
        return;

    mCustomShaderDrawOverrideCallback[id]   = NULL;
    mCustomShaderRenderStateSetCallback[id] = callback;
}

void System::SetCustomShaderCallback(CustomShaderDrawOverrideCallback drawOverrideCallback, CustomShaderRenderStateSetCallback renderStateSetCallback, CustomShaderCallBackID id)
{
    if (id > EFT_CUSTOM_SHADER_CALLBACK_MAX) // No idea why not >=
        return;

    mCustomShaderDrawOverrideCallback[id] = drawOverrideCallback;
    mCustomShaderRenderStateSetCallback[id] = renderStateSetCallback;
}

} } // namespace nw::eft
