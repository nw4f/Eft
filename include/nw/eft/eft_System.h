#ifndef EFT_SYSTEM_H_
#define EFT_SYSTEM_H_

#include <nw/eft/eft_Callback.h>
#include <nw/eft/eft_Data.h>
#include <nw/eft/eft_Renderer.h>
#include <nw/eft/eft_Resource.h>

namespace nw { namespace eft {

struct AlphaAnim;
struct ComplexParam;
class  Config;
class  EmitterCalc;
class  EmitterSet;
struct EmitterStaticUniformBlock;
class  Handle;
class  Heap;
class  PtclStripe;
struct ScaleAnim;
struct SimpleEmitterData;

class System
{
public:
    explicit System(const Config& config);
    virtual ~System(); // deleted

    void EntryResource(Heap* heap, void* resource, u32 resId, bool delayCompile = false);
    void ClearResource(Heap* heap, u32 resId);

    Resource* GetResource(u32 resId)
    {
        return mResource[resId];
    }

    bool CreateEmitterSetID(Handle* handle, const nw::math::VEC3& pos, s32 emitterSetID, u32 resourceID = 0, u8 groupID = 0, u32 emitterMask = 0xffffffff)
    {
        nw::math::MTX34 matrix;
        matrix.SetIdentity();
        matrix._03 = pos.x;
        matrix._13 = pos.y;
        matrix._23 = pos.z;
        return CreateEmitterSetID(handle, matrix, emitterSetID, resourceID, groupID, emitterMask);
    }

    bool CreateEmitterSetID(Handle* handle, const nw::math::MTX34& mtx, s32 emitterSetID, u32 resourceID, u8 groupID, u32 emitterMask = 0xffffffff);

    void KillAllEmitter();
    void KillEmitterGroup(u8 groupID);
    void KillEmitterSet(EmitterSet* emitterSet);
    void KillEmitterSet(const char* emitterSetName, u32 resId);
    void KillEmitter(EmitterInstance* emitter);

    s32 SearchEmitterSetID(const char* emitterSetName, s32 resId = 0) const;
    const char* SearchEmitterSetName(s32 emitterSetId, s32 resId = 0) const;

    void KillParticle(PtclInstance* ptcl);
    void KillChildParticle(PtclInstance* ptcl);

    void BeginFrame();
    void SwapDoubleBuffer();
    void Calc(u8 groupID, f32 frameRate = 1.0f, bool cacheFlush = true, bool forwardFrame = true);
    void CalcEmitterSet(EmitterSet* emitterSet, f32 frameRate, bool forwardFrame);
    void CalcEmitter(u8 groupID, f32 frameRate = 1.0f);

private:
    void CalcEmitter(EmitterInstance* emitter, f32 frameRate = 1.0f);

public:
    void CalcParticle(EmitterInstance* emitter, CpuCore core);
    void CalcChildParticle(EmitterInstance* emitter, CpuCore core);
    void CalcParticle(bool cacheFlush = true);

    void RemovePtcl();

    void UpdateParticlePosition(u8 groupID);
    void UpdateParticlePosition(EmitterInstance* emitter);

    void FlushCache();
    void FlushGpuCache();

    void SetDepthTexture      (const Texture depthTexture,        f32 offsetU = 0.0f, f32 offsetV = 0.0f, f32 scaleU = 1.0f, f32 scaleV = 1.0f);
    void SetFrameBufferTexture(const Texture frameBufferTexture,  f32 offsetU = 0.0f, f32 offsetV = 0.0f, f32 scaleU = 1.0f, f32 scaleV = 1.0f);

    void SetTextureSlot(TextureSlot slot, const Texture texture);

    void SetShaderType(ShaderType type);
    void SetShaderType(ShaderType type, CpuCore core);

    void SwapPreDrawDoubleBuffer();
    void MarkingCurrentDrawDoubleBuffer();

    void BeginRender(const nw::math::MTX44& proj, const nw::math::MTX34& view, const nw::math::VEC3& camPos, f32 nearClip, f32 farClip);
    void RenderParticle(u8 groupID, bool sort = false, void* userParam = NULL, RenderEmitterProfilerCallback profiler = NULL);
    void RenderParticleWithDrawPath(u8 groupID, u32 drawPathFlag, bool sort = false, void* userParam = NULL, RenderEmitterProfilerCallback profiler = NULL);
    void AddSortBuffer(u8 groupID);
    void AddSortBuffer(u8 groupID, u32 drawPathFlag);
    void RenderSortBuffer(void* userParam = NULL, RenderEmitterProfilerCallback profiler = NULL);
    void DrawSortBuffer(void* userParam = NULL, RenderEmitterProfilerCallback profiler = NULL) { RenderSortBuffer( userParam, profiler ); }
    void RenderEmitter(const EmitterInstance* emitter, void* userParam = NULL);
    void EndRender();

    Renderer* GetRenderer()
    {
        return mRenderer[GetCurrentCore()];
    }

    Renderer* GetRenderer(nw::eft::CpuCore core)
    {
        return mRenderer[core];
    }

    void SwapStreamOutBuffer();
    void CalcStreamOutEmittter();

    void SetCustomActionEmitterMatrixSetCallback(CustomActionCallBackID id, CustomActionEmitterMatrixSetCallback callback);
    void SetCustomActionEmitterPreCalcCallback(CustomActionCallBackID id, CustomActionEmitterPreCalcCallback callback);
    void SetCustomActionEmitterPostCalcCallback(CustomActionCallBackID id, CustomActionEmitterPostCalcCallback callback);
    void SetCustomActionParticleEmitCallback(CustomActionCallBackID id, CustomActionParticleEmitCallback callback);
    void SetCustomActionParticleRemoveCallback(CustomActionCallBackID id, CustomActionParticleRemoveCallback callback);
    void SetCustomActionParticleCalcCallback(CustomActionCallBackID id, CustomActionParticleCalcCallback callback);
    void SetCustomActionParticleMakeAttributeCallback(CustomActionCallBackID id, CustomActionParticleMakeAttributeCallback callback);
    void SetCustomActionEmitterDrawOverrideCallback(CustomActionCallBackID id, CustomActionEmitterDrawOverrideCallback callback);

    CustomActionEmitterMatrixSetCallback GetCurrentCustomActionEmitterMatrixSetCallback(const EmitterInstance* emitter);
    CustomActionEmitterPreCalcCallback GetCurrentCustomActionEmitterPreCalcCallback(const EmitterInstance* emitter);
    CustomActionEmitterPostCalcCallback GetCurrentCustomActionEmitterPostCalcCallback(const EmitterInstance* emitter);
    CustomActionParticleEmitCallback GetCurrentCustomActionParticleEmitCallback(const EmitterInstance* emitter);
    CustomActionParticleRemoveCallback GetCurrentCustomActionParticleRemoveCallback(const EmitterInstance* emitter);
    CustomActionParticleCalcCallback GetCurrentCustomActionParticleCalcCallback(const EmitterInstance* emitter);
    CustomActionParticleMakeAttributeCallback GetCurrentCustomActionParticleMakeAttributeCallback(const EmitterInstance* emitter);
    CustomActionEmitterDrawOverrideCallback GetCurrentCustomActionEmitterDrawOverrideCallback(const EmitterInstance* emitter);

    bool SetCurrentCustomActionCallback(CustomActionCallBackID id);

    CustomShaderEmitterInitializeCallback GetCustomShaderEmitterInitializeCallback(CustomShaderCallBackID id);
    CustomShaderEmitterFinalizeCallback GetCustomShaderEmitterFinalizeCallback(CustomShaderCallBackID id);
    CustomShaderEmitterPreCalcCallback GetCustomShaderEmitterPreCalcCallback(CustomShaderCallBackID id);
    CustomShaderEmitterPostCalcCallback GetCustomShaderEmitterPostCalcCallback(CustomShaderCallBackID id);
    CustomShaderDrawOverrideCallback GetCustomShaderDrawOverrideCallback(CustomShaderCallBackID id);
    CustomShaderRenderStateSetCallback GetCustomShaderRenderStateSetCallback(CustomShaderCallBackID id);

    void SetCustomShaderEmitterInitialzieCallback(CustomShaderCallBackID id, CustomShaderEmitterInitializeCallback callback);
    void SetCustomShaderEmitterFinalzieCallback(CustomShaderCallBackID id, CustomShaderEmitterFinalizeCallback callback);
    void SetCustomShaderEmitterPreCalcCallback(CustomShaderCallBackID id, CustomShaderEmitterPreCalcCallback callback);
    void SetCustomShaderEmitterPostCalcCallback(CustomShaderCallBackID id, CustomShaderEmitterPostCalcCallback callback);
    void SetCustomShaderDrawOverrideCallback(CustomShaderCallBackID id, CustomShaderDrawOverrideCallback callback);
    void SetCustomShaderRenderStateSetCallback(CustomShaderCallBackID id, CustomShaderRenderStateSetCallback callback);

    void SetCustomShaderCallback(CustomShaderRenderStateSetCallback callback,
                                 CustomShaderCallBackID id = EFT_CUSTOM_SHADER_CALLBACK_1);

    void SetCustomShaderCallback(CustomShaderDrawOverrideCallback    drawOverrideCallback,
                                 CustomShaderRenderStateSetCallback  renderStateSetCallback,
                                 CustomShaderCallBackID              id = EFT_CUSTOM_SHADER_CALLBACK_1 );

    DrawPathRenderStateSetCallback GetDrawPathRenderStateSetCallback(DrawPathFlag flag);

    void SetDrawPathRenderStateSetCallback(DrawPathCallBackID  id,
                                           DrawPathFlag        flag,
                                           DrawPathRenderStateSetCallback callback);

    void SetFieldCollisionCommonPlaneEnable(bool flag)
    {
        mFieldCommonPlaneEnable = flag;
    }

    bool IsFieldCollisionCommonPlaneEnable() const
    {
        return mFieldCommonPlaneEnable;
    }

    void SetFieldCollisionCommonPlane(f32 minX, f32 maxX, f32 y, f32 minZ, f32 maxZ)
    {
        mFieldCommonPlaneMinX = minX;
        mFieldCommonPlaneMaxX = maxX;
        mFieldCommonPlaneY    = y;
        mFieldCommonPlaneMinZ = minZ;
        mFieldCommonPlaneMaxZ = maxZ;
    }

    void GetFieldCollisionCommonPlane(f32* minX, f32* maxX, f32* y, f32* minZ, f32* maxZ)
    {
        *minX = mFieldCommonPlaneMinX;
        *maxX = mFieldCommonPlaneMaxX;
        *y    = mFieldCommonPlaneY;
        *minZ = mFieldCommonPlaneMinZ;
        *maxZ = mFieldCommonPlaneMaxZ;
    }

    bool IsValidStripe(u8 groupID) const
    {
        return mStripeHead[groupID] != NULL;
    }

    bool HasEnableEmitter(u8 groupID);

    s32 GetNumResource() const
    {
        return mNumResource;
    }

    s32 GetNumFreeEmitter() const
    {
        return mNumFreeEmitter;
    }

    s32 GetNumEmitterCalc() const
    {
        return mNumEmitterCalc;
    }

    s32 GetNumPtclCalc() const
    {
        return mNumPtclCalc;
    }

    s32 GetNumGpuPtclCalc() const
    {
        return mNumGpuPtclCalc;
    }

    s32 GetNumPtclCalc(u8 groupID) const;
    s32 GetNumPtclRender() const;
    s32 GetNumEmitterRender() const;

    s32 GetNumEmitterSetCalc() const
    {
        return mNumEmitterSetCalc;
    }

    s32 GetNumStripeCalc() const
    {
        return mNumStripeCalc;
    }

    s32 GetNumEmittedPtcl() const
    {
        return mNumEmittedPtcl;
    }

    s32 GetNumStripeVertexCalc() const;
    s32 GetNumStripeVertexDraw() const;

    u32 GetUsedDoubleBufferSize(CpuCore core = EFT_CPU_CORE_1) const;
    u32 GetDoubleBufferSize(CpuCore core = EFT_CPU_CORE_1) const;

    const EmitterInstance* GetEmitterHead(u8 groupID) const
    {
        return mEmitterHead[groupID];
    }

    EmitterInstance* GetEmitterHead(u8 groupID)
    {
        return mEmitterHead[groupID];
    }

    u32 GetNumEmitter(u8 groupID);
    const EmitterInstance* GetEmitter(u8 groupID, u32 idx) const;
    EmitterInstance* GetEmitter(u8 groupID, u32 idx);

    const EmitterSet* GetEmitterSetHead(u8 groupID) const
    {
        return mEmitterSetHead[groupID];
    }

    EmitterSet* GetEmitterSetHead(u8 groupID)
    {
        return mEmitterSetHead[groupID];
    }

    const PtclStripe* GetStripeHead(u8 groupID) const
    {
        return mStripeHead[groupID];
    }

    bool IsHasRenderingEmitter(u8 groupID, u32 drawPath) const
    {
        for (u32 core = 0; core < EFT_CPU_CORE_MAX; core++)
            if (mEnableRenderPath[core][groupID] & drawPath)
                return true;

        return false;
    }

    u64 GetEnableGroupID() const
    {
        return mEnableGroupID;
    }

    static CpuCore GetCurrentCore()
    {
        return (CpuCore)OSGetCoreId();
    }

    u32 GetGlobalCounter() const
    {
        return mGlobalCounter;
    }

    void SetScreenNum(u8 groupID, u32 screenNum)
    {
        mScreenNum[groupID] = screenNum;
    }

    u32 GetScreenNum(u8 groupID) const
    {
        return mScreenNum[groupID];
    }

    void CheckError()
    {
    }

    RuntimeError GetRuntimeError() const
    {
        return mRuntimeError;
    }

    void SetRuntimeError(RuntimeError code)
    {
        mRuntimeError = code;
    }

    void AddStripeRemoveList(PtclStripe* stripe, CpuCore core);

    void EmitChildParticle();
    void AddPtclAdditionList(PtclInstance* ptcl, CpuCore core);

    PtclStripe* AllocAndConnectStripe(EmitterInstance* e);
    PtclInstance* AllocPtcl();

    void UpdateEmitterResInfo();
    void UpdateEmitterSetResInfo();

    void ResetRendererDebugInfo();

    void ReCreateEmitter(void** resList, s32 numResList, u32 resId, s32 setId, bool killOnly);
    void ReCreateEmitter(EmitterSet* set, u32 resId, s32 setId, u8 groupID);
    void ReCreateEmitterSet(EmitterSet* set, u32 resId, s32 setId, u8 groupID);

    const PtclInstance* GetPtclBuf() const
    {
        return mPtcl;
    }

    u32 GetPtclBufSize() const;

    const EmitterInstance* GetEmitterBuf() const
    {
        return mEmitter;
    }

    const PtclStripe* GetStripeBuf() const
    {
        return mStripe;
    }

protected:
    virtual void Initialize(Heap* heap, Heap* dynamicHeap, const Config& config);

private:
    void RemoveStripe(PtclStripe* stripe);

    EmitterSet* AllocEmitterSet(Handle* handle);
    EmitterInstance* AllocEmitter(u8 groupID);

    void AddEmitterSetToDrawList(EmitterSet* emitterSet, u8 groupID);
    EmitterSet* RemoveEmitterSetFromDrawList(EmitterSet* emitterSet);

    void InitializeEmitter(EmitterInstance* emitter, const SimpleEmitterData* res, EmitterStaticUniformBlock* emitterStaticUbo, EmitterStaticUniformBlock* childStaticUbo, u32 resourceID, s32 emitterSetID, u32 setRndSeed, bool nonUpdateCreateID = false);

    void RemoveStripe_();

    struct sortEmitterSets
    {
        EmitterSet*     emitterSet;
        u32             z;
    };
    static_assert(sizeof(sortEmitterSets) == 8, "sortEmitterSets size mismatch");

    static s32 ComparePtclViewZ(const void* a, const void* b);

private:
    static bool         mInitialized;
    u32                 mGlobalCounter;
    bool                mCalcSwapFlag;
    bool                mMarkingSwapFlag;
    Resource**          mResource;
    u32                 mNumResource;
    EmitterSet*         mEmitterSet;
    s32                 mNumEmitterSet;
    s32                 mCurEmitterSetIx;
    EmitterSet*         mEmitterSetHead[EFT_GROUP_MAX];
    EmitterSet*         mEmitterSetTail[EFT_GROUP_MAX];
    sortEmitterSets*    mSortEmittetSet[EFT_CPU_CORE_MAX];
    u32                 mSortEmitterSetIx[EFT_CPU_CORE_MAX];
    EmitterInstance*    mEmitter;
    s32                 mNumEmitterData;
    s32                 mNumFreeEmitter;
    s32                 mCurEmitterIx;
    EmitterInstance*    mEmitterHead[EFT_GROUP_MAX];
    PtclInstance*       mPtcl;
    s32                 mNumPtclData;
    s32                 mCurPtclIx;
    AlphaAnim*          mAlphaAnim0;
    AlphaAnim*          mAlphaAnim1;
    ScaleAnim*          mScaleAnim;
    ComplexParam*       mComplexParam;
    PtclInstance**      mPtclAdditionArray[EFT_CPU_CORE_MAX];
    s32                 mPtclAdditionIdx[EFT_CPU_CORE_MAX];
    PtclStripe**        mStripeRemoveArray[EFT_CPU_CORE_MAX];
    u32                 mStripeRemoveIdx[EFT_CPU_CORE_MAX];
    PtclStripe*         mStripe;
    u32                 mNumStripeData;
    u32                 mCurStripeIx;
    PtclStripe*         mStripeHead[EFT_GROUP_MAX];
    nw::math::MTX44     mViewMatrix[EFT_CPU_CORE_MAX];
    RuntimeError        mRuntimeError;
    s32                 mNumEmitterSetCalc;
    s32                 mNumEmitterCalc;
    s32                 mNumPtclCalc;
    s32                 mNumGpuPtclCalc;
    s32                 mNumEmittedPtcl;
    s32                 mNumStripeCalc;
    u64                 mEnableGroupID;
    u32                 mEnableRenderPath[EFT_CPU_CORE_MAX][EFT_GROUP_MAX];
    Renderer*           mRenderer[EFT_CPU_CORE_MAX];
    u32                 mEmitterSetCreateID;
    EmitterCalc*        mEmitterCalc[EFT_EMITTER_TYPE_MAX];
    void*               mResourcePtr;
    void*               mEmitterSetPtr;
    void*               mRendererPtr[EFT_CPU_CORE_MAX];
    void*               mEmitterCalcSimplePtr;
    void*               mEmitterCalcComplexPtr;
    void*               mEmitterCalcSimpleGpuPtr;
    bool                mFieldCommonPlaneEnable;
    f32                 mFieldCommonPlaneMinX;
    f32                 mFieldCommonPlaneMaxX;
    f32                 mFieldCommonPlaneY;
    f32                 mFieldCommonPlaneMinZ;
    f32                 mFieldCommonPlaneMaxZ;
    u32                 mScreenNum[EFT_GROUP_MAX];

    EmitterInstance*    mStreamOutEmitter;
    EmitterInstance*    mStreamOutEmitterTail;

    CustomActionCallBackID                      mEnableCallbackID;
    CustomActionEmitterMatrixSetCallback        mCustomActionEmitterMatrixSetCallback[EFT_CUSTOM_ACTION_CALLBACK_ID_MAX];
    CustomActionEmitterPreCalcCallback          mCustomActionEmitterPreCalcCallback[EFT_CUSTOM_ACTION_CALLBACK_ID_MAX];
    CustomActionParticleEmitCallback            mCustomActionParticleEmitCallback[EFT_CUSTOM_ACTION_CALLBACK_ID_MAX];
    CustomActionParticleRemoveCallback          mCustomActionParticleRemoveCallback[EFT_CUSTOM_ACTION_CALLBACK_ID_MAX];
    CustomActionParticleCalcCallback            mCustomActionParticleCalcCallback[EFT_CUSTOM_ACTION_CALLBACK_ID_MAX];
    CustomActionParticleMakeAttributeCallback   mCustomActionParticleMakeAttrCallback[EFT_CUSTOM_ACTION_CALLBACK_ID_MAX];
    CustomActionEmitterPostCalcCallback         mCustomActionEmitterPostCalcCallback[EFT_CUSTOM_ACTION_CALLBACK_ID_MAX];
    CustomActionEmitterDrawOverrideCallback     mCustomActionEmitterDrawOverrideCallback[EFT_CUSTOM_ACTION_CALLBACK_ID_MAX];

    CustomShaderEmitterInitializeCallback       mCustomShaderEmitterInitializeCallback[EFT_CUSTOM_SHADER_CALLBACK_MAX];
    CustomShaderEmitterFinalizeCallback         mCustomShaderEmitterFinalizeCallback[EFT_CUSTOM_SHADER_CALLBACK_MAX];
    CustomShaderEmitterPreCalcCallback          mCustomShaderEmitterCalcPreCallback[EFT_CUSTOM_SHADER_CALLBACK_MAX];
    CustomShaderEmitterPostCalcCallback         mCustomShaderEmitterCalcPostCallback[EFT_CUSTOM_SHADER_CALLBACK_MAX];
    CustomShaderDrawOverrideCallback            mCustomShaderDrawOverrideCallback[EFT_CUSTOM_SHADER_CALLBACK_MAX];
    CustomShaderRenderStateSetCallback          mCustomShaderRenderStateSetCallback[EFT_CUSTOM_SHADER_CALLBACK_MAX];

    u32                                         mDrawPathCallbackFlag[EFT_DRAW_PATH_CALLBACK_MAX];
    DrawPathRenderStateSetCallback              mDrawPathRenderStateSetCallback[EFT_DRAW_PATH_CALLBACK_MAX];

    u32                                         mCurrentDrawPathFlag[EFT_CPU_CORE_MAX];
};
static_assert(sizeof(System) == 0xC28, "nw::eft::System size mismatch");

} } // namespace nw::eft

#endif // EFT_SYSTEM_H_
