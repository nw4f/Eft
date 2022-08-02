#ifndef EFT_SYSTEM_H_
#define EFT_SYSTEM_H_

#include <nw/math/math_Matrix34.h>
#include <nw/math/math_Matrix44.h>
#include <nw/eft/eft_Enum.h>

namespace nw { namespace eft {

struct AlphaAnim;
struct ComplexEmitterParam;
class Config;
class EmitterCalc;
class EmitterSet;
struct EmitterStaticUniformBlock;
class Handle;
class Heap;
class PtclStripe;
class Resource;
struct ScaleAnim; // ?
struct SimpleEmitterData;

class System
{
public:
    System(const Config& config);
    virtual ~System(); // deleted

    virtual void Initialize(Heap* heap, Heap* dynamicHeap, const Config& config);

    void RemoveStripe(PtclStripe* stripe);
    void RemoveStripe_();
    EmitterSet* RemoveEmitterSetFromDrawList(EmitterSet* emitterSet);
    void RemovePtcl();
    void AddStripeRemoveList(PtclStripe* stripe, CpuCore core);
    void EmitChildParticle();
    void AddPtclAdditionList(PtclInstance* ptcl, CpuCore core);
    PtclStripe* AllocAndConnectStripe(EmitterInstance* emitter);
    PtclInstance* AllocPtcl();
    inline PtclInstance* AllocPtcl(EmitterInstance* emitter);
    void UpdateEmitterResInfo();
    void UpdateEmitterSetResInfo();
    EmitterSet* AllocEmitterSet(Handle* handle);
    EmitterInstance* AllocEmitter(u8 groupID);
    void AddEmitterSetToDrawList(EmitterSet* emitterSet, u8 groupID);

    void InitializeEmitter(EmitterInstance* emitter, const SimpleEmitterData* data, EmitterStaticUniformBlock* esub, EmitterStaticUniformBlock* cesub, u32 resourceID, s32 emitterSetID, u32 seed, bool keepCreateID);
    bool CreateEmitterSetID(Handle* handle, const math::MTX34& matrixRT, s32 emitterSetID, u32 resourceID, u8 groupID, u32 emitterEnableMask = 0xFFFFFFFF);

    void BeginFrame();
    void SwapDoubleBuffer();
    void CalcEmitter(u8 groupID, f32 emissionSpeed = 1.0f);
    void CalcParticle(EmitterInstance* emitter, CpuCore core);
    void CalcChildParticle(EmitterInstance* emitter, CpuCore core);
    void FlushCache();
    void FlushGpuCache();
    void CalcEmitter(EmitterInstance* emitter, f32 emissionSpeed);
    void CalcParticle(bool flushCache);
    void BeginRender(const math::MTX44& proj, const math::MTX34& view, const math::VEC3& cameraWorldPos, f32 zNear, f32 zFar);
    void RenderEmitter(const EmitterInstance* emitter, void* argData);
    void RenderSortBuffer(void* argData, RenderEmitterProfilerCallback callback);
    void AddSortBuffer(u8 groupID, u32 flag = 0xFFFFFFFF);
    void EndRender();
    void SwapStreamOutBuffer();
    void CalcStreamOutEmittter();

    void ClearResource(Heap* heap, u32 resourceID);
    void EntryResource(Heap* heap, void* resource, u32 resourceID, bool);
    void KillEmitter(EmitterInstance* emitter);
    void KillEmitterGroup(u8 groupID);
    void KillEmitterSet(EmitterSet* emitterSet);

    CustomActionEmitterMatrixSetCallback GetCurrentCustomActionEmitterMatrixSetCallback(const EmitterInstance* emitter);
    CustomActionEmitterPreCalcCallback GetCurrentCustomActionEmitterPreCalcCallback(const EmitterInstance* emitter);
    CustomActionEmitterPostCalcCallback GetCurrentCustomActionEmitterPostCalcCallback(const EmitterInstance* emitter);
    CustomActionParticleEmitCallback GetCurrentCustomActionParticleEmitCallback(const EmitterInstance* emitter);
    CustomActionParticleRemoveCallback GetCurrentCustomActionParticleRemoveCallback(const EmitterInstance* emitter);
    CustomActionParticleCalcCallback GetCurrentCustomActionParticleCalcCallback(const EmitterInstance* emitter);
    CustomActionParticleMakeAttributeCallback GetCurrentCustomActionParticleMakeAttributeCallback(const EmitterInstance* emitter);
    CustomActionEmitterDrawOverrideCallback GetCurrentCustomActionEmitterDrawOverrideCallback(const EmitterInstance* emitter);
    CustomShaderEmitterInitializeCallback GetCustomShaderEmitterInitializeCallback(CustomShaderCallBackID callbackID);
    CustomShaderEmitterFinalizeCallback GetCustomShaderEmitterFinalizeCallback(CustomShaderCallBackID callbackID);
    CustomShaderEmitterPreCalcCallback GetCustomShaderEmitterPreCalcCallback(CustomShaderCallBackID callbackID);
    CustomShaderEmitterPostCalcCallback GetCustomShaderEmitterPostCalcCallback(CustomShaderCallBackID callbackID);
    CustomShaderDrawOverrideCallback GetCustomShaderDrawOverrideCallback(CustomShaderCallBackID callbackID);
    CustomShaderRenderStateSetCallback GetCustomShaderRenderStateSetCallback(CustomShaderCallBackID callbackID);
    DrawPathRenderStateSetCallback GetDrawPathRenderStateSetCallback(DrawPathFlag flag);

    struct PtclViewZ // Actual name not known
    {
        EmitterSet* emitterSet;
        u32 z;
    };
    static_assert(sizeof(PtclViewZ) == 8, "PtclViewZ size mismatch");

    // For qsort
    static s32 ComparePtclViewZ(const void* a, const void* b);

    static bool mInitialized;
    u32 counter;
    u8 doubleBufferSwapped;
    u8 _unused;
    Resource** resources;
    u32 numResourceMax;
    EmitterSet* emitterSets;
    s32 numEmitterSetMax;
    s32 currentEmitterSetIdx;
    EmitterSet* emitterSetGroupHead[64]; // Maximum of 64 groups
    EmitterSet* emitterSetGroupTail[64]; // Maximum of 64 groups
    PtclViewZ* sortedEmitterSets[CpuCore_Max];
    u32 numSortedEmitterSets[CpuCore_Max];
    EmitterInstance* emitters;
    s32 numEmitterMax;
    s32 numUnusedEmitters;
    s32 currentEmitterIdx;
    EmitterInstance* emitterGroups[64];  // Maximum of 64 groups
    PtclInstance* particles;
    s32 numParticleMax;
    s32 currentParticleIdx;
    AlphaAnim* alphaAnim[2];
    ScaleAnim* scaleAnim;
    ComplexEmitterParam* complexParam;
    PtclInstance** childParticles[CpuCore_Max];
    s32 numChildParticle[CpuCore_Max];
    PtclStripe** stripesToRemove[CpuCore_Max];
    s32 numStripeToRemove[CpuCore_Max];
    PtclStripe* stripes;
    u32 numStripeMax;
    u32 currentStripeIdx;
    PtclStripe* stripeGroups[64]; // Maximum of 64 groups
    math::MTX44 view[CpuCore_Max];
    u32 _unused0;
    u32 numCalcEmitterSet;
    u32 numCalcEmitter;
    u32 numCalcParticle;
    u32 numCalcParticleGpu;
    u32 numEmittedParticle;
    u32 numCalcStripe;
    u64 activeGroupsFlg;
    u32 _unusedFlags[CpuCore_Max][64]; // Maximum of 64 groups
    Renderer* renderers[CpuCore_Max];
    u32 currentEmitterSetCreateID;
    EmitterCalc* emitterCalc[EmitterType_Max];
    void* resourceWork;
    void* emitterSetWork;
    void* rendererWork[CpuCore_Max];
    void* emitterSimpleCalcWork;
    void* emitterComplexCalcWork;
    void* emitterSimpleGpuCalcWork;
    u8 isSharedPlaneEnable;
    math::VEC2 sharedPlaneX; // Min / Max
    f32 sharedPlaneY;        //       Max
    math::VEC2 sharedPlaneZ; // Min / Max
    u32 streamOutParam[64]; // Maximum of 64 groups
    EmitterInstance* streamOutEmitterHead;
    EmitterInstance* streamOutEmitterTail;
    CustomActionCallBackID currentCallbackID;
    CustomActionEmitterMatrixSetCallback customActionEmitterMatrixSetCallback[CustomActionCallBackID_Max];
    CustomActionEmitterPreCalcCallback customActionEmitterPreCalcCallback[CustomActionCallBackID_Max];
    CustomActionParticleEmitCallback customActionParticleEmitCallback[CustomActionCallBackID_Max];
    CustomActionParticleRemoveCallback customActionParticleRemoveCallback[CustomActionCallBackID_Max];
    CustomActionParticleCalcCallback customActionParticleCalcCallback[CustomActionCallBackID_Max];
    CustomActionParticleMakeAttributeCallback customActionParticleMakeAttributeCallback[CustomActionCallBackID_Max];
    CustomActionEmitterPostCalcCallback customActionEmitterPostCalcCallback[CustomActionCallBackID_Max];
    CustomActionEmitterDrawOverrideCallback customActionEmitterDrawOverrideCallback[CustomActionCallBackID_Max];
    CustomShaderEmitterInitializeCallback customShaderEmitterInitializeCallback[CustomShaderCallBackID_Max];
    CustomShaderEmitterFinalizeCallback customShaderEmitterFinalizeCallback[CustomShaderCallBackID_Max];
    CustomShaderEmitterPreCalcCallback customShaderEmitterPreCalcCallback[CustomShaderCallBackID_Max];
    CustomShaderEmitterPostCalcCallback customShaderEmitterPostCalcCallback[CustomShaderCallBackID_Max];
    CustomShaderDrawOverrideCallback customShaderDrawOverrideCallback[CustomShaderCallBackID_Max];
    CustomShaderRenderStateSetCallback customShaderRenderStateSetCallback[CustomShaderCallBackID_Max];
    u32 drawPathCallbackFlags[DrawPathCallback_Max];
    DrawPathRenderStateSetCallback drawPathRenderStateSetCallback[DrawPathCallback_Max];
    u32 _unused1[CpuCore_Max];
};
static_assert(sizeof(System) == 0xC28, "System size mismatch");

PtclInstance* System::AllocPtcl(EmitterInstance* emitter)
{
    PtclInstance* ptcl;

    if (emitter->data->vertexTransformMode == VertexTransformMode_Stripe)
    {
        PtclStripe* stripe = AllocAndConnectStripe(emitter);
        if (stripe == NULL)
            return NULL;

        ptcl = AllocPtcl();
        if (ptcl == NULL)
            return NULL;

        ptcl->type = emitter->calc->GetPtclType();
        ptcl->data = emitter->data;
        ptcl->complexParam->stripe = stripe;
        stripe->particle = ptcl;
    }
    else
    {
        ptcl = AllocPtcl();
        if (ptcl == NULL)
            return NULL;

        ptcl->type = emitter->calc->GetPtclType();
        ptcl->data = emitter->data;
        ptcl->complexParam->stripe = NULL;
    }

    return ptcl;
}

} } // namespace nw::eft

#endif // EFT_SYSTEM_H_
