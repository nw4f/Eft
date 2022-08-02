#ifndef EFT_RENDERER_H_
#define EFT_RENDERER_H_

#include <nw/eft/eft_Emitter.h>
#include <nw/eft/eft_RenderContext.h>
#include <nw/eft/eft_TemporaryBuffer.h>

namespace nw { namespace eft {

class  Config;
class  ParticleShader;
class  Primitive;
struct PtclAttributeBuffer;
struct PtclAttributeBufferGpu;
struct PtclInstance;
struct PtclStripe;
struct StripeData;
struct StripeVertexBuffer;
class  System;
struct ViewUniformBlock;

class Renderer
{
private:
    struct sortPtcl
    {
        PtclInstance*   ptcl;
        f32             z;
        u32             index;
    };
    static_assert(sizeof(sortPtcl) == 0xC, "nw::Renderer::sortPtcl size mismatch");

    static s32 ComparePtclViewZ(const void* a, const void* b);

public:
    Renderer(System* system, const Config& config);
    ~Renderer();

    void BeginRender(const nw::math::MTX44& proj, const nw::math::MTX34& view, const nw::math::VEC3& camPos, f32 near, f32 far);

    void EntryParticle(const EmitterInstance* emitter, void* userParam);

    bool EntryParticleFromCallback(const EmitterInstance* emitter, void* userParam, bool bDraw = true)
    {
        if (mCurrentDrawingType == EFT_PTCL_TYPE_CHILD)
            return EntryChildParticleSub(emitter, userParam, bDraw);
        else
            return EntryParticleSub(emitter, userParam, bDraw);
    }

    void DirectEntryParticleFromCallback(const EmitterInstance* emitter, void* userParam);

    void EndRender();

    void BeginStremOut();
    void CalcStremOutParticle(const EmitterInstance* emitter, bool bind);
    void EndStremOut();

    void SetDepthTexture(Texture depthTexture, f32 offsetU = 0.0f, f32 offsetV = 0.0f, f32 scaleU = 1.0f, f32 scaleV = 1.0f)
    {
        SetTextureSlot(EFT_TEXTURE_SLOT_DEPTH_BUFFER, depthTexture);
        mDepthTextureOffset.x   = offsetU;
        mDepthTextureOffset.y   = offsetV;
        mDepthTextureScale.x    = scaleU;
        mDepthTextureScale.y    = scaleV;
    }

    void SetFrameBufferTexture(Texture frameBufferTexture, f32 offsetU = 0.0f, f32 offsetV = 0.0f, f32 scaleU = 1.0f, f32 scaleV = 1.0f)
    {
        SetTextureSlot(EFT_TEXTURE_SLOT_FRAME_BUFFER, frameBufferTexture);
        mFrameBufferTextureOffset.x = offsetU;
        mFrameBufferTextureOffset.y = offsetV;
        mFrameBufferTextureScale.x  = scaleU;
        mFrameBufferTextureScale.y  = scaleV;
    }

    void SetTextureSlot(TextureSlot slot, Texture texture)
    {
        mExtensionTexture[slot] = texture;
    }

    void SwapDoubleBuffer();
    void MarkingCurrentDrawDoubleBufferSize();
    void SwapPreDrawDoubleBuffer();
    void* AllocFromDoubleBuffer(u32 size);

    u32 GetDoubleBufferSize() const
    {
        return mTemporaryBuffer.GetSize();
    }

    u32 GetUsedDoubleBufferSize() const
    {
        return mTemporaryBuffer.GetUsedSize();
    }

    void FlushCache();

    ParticleShader* GetShader(const EmitterInstance* emitter)
    {
        if (mCurrentDrawingType == EFT_PTCL_TYPE_CHILD)
            return emitter->childShader[mCurrentShaderType];
        else
            return emitter->shader[mCurrentShaderType];
    }

    const nw::eft::CustomShaderParam* GetCustomShaderParam(const EmitterInstance* emitter)
    {
        if (mCurrentDrawingType == EFT_PTCL_TYPE_CHILD)
        {
            const ComplexEmitterData* res  = reinterpret_cast<const ComplexEmitterData*>(emitter->res);
            const ChildData*          cres = reinterpret_cast<const ChildData*>(res + 1);

            return &cres->childCustomShaderParam;
        }
        else
        {
            return &emitter->res->customShaderParam;
        }
    }

    f32 GetCustomShaderParam(const EmitterInstance* emitter, u32 paramIdx)
    {
        if (mCurrentDrawingType == EFT_PTCL_TYPE_CHILD)
        {
            const ComplexEmitterData* res  = reinterpret_cast<const ComplexEmitterData*>(emitter->res);
            const ChildData*          cres = reinterpret_cast<const ChildData*>(res + 1);

            if (!cres->childCustomShaderParam.param)
                return 0.0f;

            return cres->childCustomShaderParam.param[paramIdx];
        }
        else
        {
            if (!emitter->res->customShaderParam.param)
                return 0.0f;

            return emitter->res->customShaderParam.param[paramIdx];
        }
    }

    f32 GetParentParticleCustomShaderParam(const EmitterInstance* emitter, u32 paramIdx)
    {
        if (!emitter->res->customShaderParam.param)
            return 0.0f;

        return emitter->res->customShaderParam.param[paramIdx];
    }

    f32 GetChildParticleCustomShaderParam(const EmitterInstance* emitter, u32 paramIdx)
    {
        const ComplexEmitterData* res  = reinterpret_cast<const ComplexEmitterData*>(emitter->res);
        const ChildData*          cres = reinterpret_cast<const ChildData*>(res + 1);

        if (!cres->childCustomShaderParam.param)
            return 0.0f;

        return cres->childCustomShaderParam.param[ paramIdx ];
    }

    u8 GetUserShaderSetting( const EmitterInstance* emitter )
    {
        if (mCurrentDrawingType == EFT_PTCL_TYPE_CHILD)
        {
            const ComplexEmitterData* res  = reinterpret_cast<const ComplexEmitterData*>(emitter->res);
            const ChildData*          cres = reinterpret_cast<const ChildData*>(res + 1);

            return cres->childUserShaderSetting;
        }
        else
        {
            return emitter->res->userShaderSetting;
        }
    }

    u32 GetCustomShaderFlag( const EmitterInstance* emitter )
    {
        if (mCurrentDrawingType == EFT_PTCL_TYPE_CHILD)
        {
            const ComplexEmitterData* res  = reinterpret_cast<const ComplexEmitterData*>(emitter->res);
            const ChildData*          cres = reinterpret_cast<const ChildData*>(res + 1);

            return cres->childUserShaderFlag;
        }
        else
        {
            return emitter->res->userShaderFlag;
        }
    }

    u32 GetUserShaderFlag(const EmitterInstance* emitter)
    {
        return GetCustomShaderFlag(emitter);
    }

    const nw::eft::CustomShaderParam* GetUserShaderParam(const EmitterInstance* emitter)
    {
        return GetCustomShaderParam(emitter);
    }

    f32 GetParentParticleUserShaderParam(const EmitterInstance* emitter, u32 paramIdx)
    {
        return GetParentParticleCustomShaderParam(emitter, paramIdx);
    }

    f32 GetChildParticleUserShaderParam(const EmitterInstance* emitter, u32 paramIdx)
    {
        return GetChildParticleCustomShaderParam(emitter, paramIdx);
    }

    f32 GetUserShaderParam(const EmitterInstance* emitter, u32 paramIdx)
    {
        return GetCustomShaderParam(emitter, paramIdx);
    }

    void SetDrawViewFlag(DrawViewFlag flag)
    {
        mDrawViewFlag = flag;
    }

    DrawViewFlag GetDrawViewFlag() const
    {
        return mDrawViewFlag;
    }

    u32 GetStripeVertexCalcNum() const
    {
        return mStripeVertexCalcNum;
    }

    u32 GetStripeVertexDrawNum() const
    {
        return mStripeVertexDrawNum;
    }

    u32 GetRenderingEmitterNum() const
    {
        return mRenderingEmitterNum;
    }

    u32 GetRenderingParticleNum() const
    {
        return mRenderingParticleNum;
    }

    void ResetRenderingParticleNum()
    {
        mRenderingParticleNum = 0;
    }

    void ResetRenderingEmitterNum()
    {
        mRenderingEmitterNum = 0;
    }

    void ResetRenderingStripeVertexNum()
    {
        mStripeVertexDrawNum = 0;
    }

    const System* GetEffectSystem() const
    {
        return mSystem;
    }

    void SetShaderType(ShaderType type)
    {
        mCurrentShaderType = type;
    }

    bool MakeStripeAttributeBlock(const EmitterInstance* emitter, PtclInstance* ptcl);
    StripeVertexBuffer* MakeConnectionStripeAttributeBlock(EmitterInstance* emitter);

    Primitive* GetCurrentPrimitive() const
    {
        return mCurrentPrimitive;
    }

    Rendercontext& GetRenderContext()
    {
        return mRenderContext;
    }

    PtclType GetPtclType() const
    {
        return mCurrentDrawingType;
    }

    nw::math::MTX44 GetViewMatrix() const
    {
        return mMatView;
    }

    nw::math::MTX44 GetViewProjMatrix() const
    {
        return mMatViewProj;
    }

private:
    bool EntryParticleSub(const EmitterInstance* emitter, void* userParam, bool bDraw = true);
    bool EntryChildParticleSub(const EmitterInstance* emitter, void* userParam, bool bDraw = true);

    void RequestParticle(const EmitterInstance* e,
                         ParticleShader*        shader,
                         bool                   bChild,
                         void*                  userParam,
                         bool                   bDraw = true);

    bool SetupParticleShaderAndVertex(ParticleShader* shader, MeshType meshType, Primitive* primitive);

    void EntryStripe(const EmitterInstance* emitter, void* userParam);
    void EntryConnectionStripe(const EmitterInstance* emitter, void* userParam);
    bool SetupStripeDrawSetting(const EmitterInstance* emitter, void* userParam);

    s32 MakeStripeAttributeBlockCore(PtclStripe* __restrict stripe, StripeVertexBuffer* stripeVertex, s32 wroteVertexNum, const StripeData* stripeData);
    s32 MakeStripeAttributeBlockCoreDivide(PtclStripe* stripe, StripeVertexBuffer* stripeVertex, s32 wroteVertexNum, const StripeData* stripeData);

    s32 MakeConnectionStripeAttributeBlockCore(EmitterInstance* emitter, s32 numPtcl, PtclInstance* pTailPtcl, PtclInstance* pTail2ndPtcl, s32 tailType, StripeVertexBuffer* stripeVertex);
    s32 MakeConnectionStripeAttributeBlockCoreDivide(EmitterInstance* emitter, s32 numPtcl, PtclInstance* pTailPtcl, PtclInstance* pTail2ndPtcl, s32 tailType, StripeVertexBuffer* stripeVertex);

    bool ConnectionStripeUvScaleCalc(f32& frateScaleUv, f32& uvStartOfs, const EmitterInstance* emitter, s32 numLoop, f32 frateScale, s32 tailType);

    void GetPositionOnCubic(nw::math::VEC3*         result,
                            const nw::math::VEC3&   startPos,
                            const nw::math::VEC3&   startVel,
                            const nw::math::VEC3&   endPos,
                            const nw::math::VEC3&   endVel,
                            float                   time);

    void SetupTexture(ParticleShader* shader, const TextureRes* texture0, const TextureRes* texture1, const nw::eft::TextureRes* texture2);

private:
    void DrawCpuEntry(ParticleShader* shader, Draw::PrimitiveType drawType, u32 startNum, u32 entryNum, PtclAttributeBuffer*    buffer, Primitive* primitive);
    void DrawGpuEntry(ParticleShader* shader, Draw::PrimitiveType drawType, u32 startNum, u32 entryNum, PtclAttributeBufferGpu* buffer, Primitive* primitive);

    void BindParticleAttributeBlock   (PtclAttributeBuffer*    ptclAttributeBuffer, ParticleShader* shader, u32 startNum, u32 entryNum);
    void BindGpuParticleAttributeBlock(PtclAttributeBufferGpu* ptclAttributeBuffer, ParticleShader* shader, u32 startNum, u32 entryNum);

private:
    System*                         mSystem;
    Heap*                           mHeap;
    Rendercontext                   mRenderContext;
    Draw::PrimitiveType             mDrawPrimitiveType;
    nw::math::MTX44                 mMatView;
    nw::math::MTX44                 mMatViewProj;
    nw::math::MTX44                 mMatBillboard;
    nw::math::VEC3                  mEyeVec;
    nw::math::VEC3                  mEyePos;
    VertexBuffer                    mPositionVB;
    VertexBuffer                    mIndexVB;
    Primitive*                      mCurrentPrimitive;
    ViewUniformBlock*               mViewUniformBlock;
    Texture                         mExtensionTexture[EFT_TEXTURE_SLOT_MAX];
    nw::math::VEC2                  mDepthTextureOffset;
    nw::math::VEC2                  mDepthTextureScale;
    nw::math::VEC2                  mFrameBufferTextureOffset;
    nw::math::VEC2                  mFrameBufferTextureScale;
    TemporaryBuffer                 mTemporaryBuffer;
    u32                             mStripeVertexCalcNum;
    u32                             mStripeVertexDrawNum;
    PtclType                        mCurrentDrawingType;
    ShaderType                      mCurrentShaderType;
    DrawViewFlag                    mDrawViewFlag;
    u32                             mRenderingEmitterNum;
    u32                             mRenderingParticleNum;
    bool                            mViewDirtyFlag;
};
static_assert(sizeof(Renderer) == 0x27C, "nw::eft::Renderer size mismatch");

} } // namespace nw::eft

#endif // EFT_RENDERER_H_
