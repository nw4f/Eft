#include <nw/eft/eft_Config.h>
#include <nw/eft/eft_Emitter.h>
#include <nw/eft/eft_EmitterSet.h>
#include <nw/eft/eft_Particle.h>
#include <nw/eft/eft_Primitive.h>
#include <nw/eft/eft_Renderer.h>
#include <nw/eft/eft_Shader.h>
#include <nw/eft/eft_System.h>
#include <nw/eft/eft_UniformBlock.h>

#include <cstdlib>

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#endif // __GNUC__

namespace nw { namespace eft {

s32 Renderer::ComparePtclViewZ(const void* a, const void* b)
{
    const sortPtcl* src = static_cast<const sortPtcl*>(a);
    const sortPtcl* dst = static_cast<const sortPtcl*>(b);

    if (src->z < 0.0f && dst->z < 0.0f)
    {
        if (src->z < dst->z)
            return -1;
    }
    else
    {
        if (src->z > dst->z)
            return -1;
    }

    return 1;
}

Renderer::Renderer(Heap* heap, System* system, const Config& config)
    : mSystem(system)
{
    mHeap                       = heap;
    mDepthTextureOffset.x       = 0.0f;
    mDepthTextureOffset.y       = 0.0f;
    mDepthTextureScale.x        = 1.0f;
    mDepthTextureScale.y        = 1.0f;
    mFrameBufferTextureOffset.x = 0.0f;
    mFrameBufferTextureOffset.y = 0.0f;
    mFrameBufferTextureScale.x  = 1.0f;
    mFrameBufferTextureScale.y  = 1.0f;
    mCurrentDrawingType         = EFT_PTCL_TYPE_MAX;
    mCurrentShaderType          = EFT_SHADER_TYPE_NORMAL;
    mDrawViewFlag               = EFT_DRAW_VIEW_FLAG_ALL;

    SetTextureSlot(EFT_TEXTURE_SLOT_DEPTH_BUFFER, Texture(0));
    SetTextureSlot(EFT_TEXTURE_SLOT_FRAME_BUFFER, Texture(0));

    const f32 particleWH = 0.5f;
    nw::math::VEC3* pos = static_cast<nw::math::VEC3*>(mPositionVB.AllocateVertexBuffer(heap, sizeof(nw::math::VEC3) * 4, 3));
    pos[0].x = -particleWH;
    pos[0].y =  particleWH;
    pos[0].z =  0.0f;
    pos[1].x = -particleWH;
    pos[1].y = -particleWH;
    pos[1].z =  0.0f;
#if EFT_IS_WIN
    pos[2].x =  particleWH;
    pos[2].y =  particleWH;
    pos[2].z =  0.0f;
    pos[3].x =  particleWH;
    pos[3].y = -particleWH;
    pos[3].z =  0.0f;
#else
    pos[2].x =  particleWH;
    pos[2].y = -particleWH;
    pos[2].z =  0.0f;
    pos[3].x =  particleWH;
    pos[3].y =  particleWH;
    pos[3].z =  0.0f;
    mPositionVB.Invalidate();
#endif

    u32* index = static_cast<u32*>(mIndexVB.AllocateVertexBuffer(mHeap, sizeof(u32) * 4, 1));
    index[0] = 0;
    index[1] = 1;
#if EFT_IS_WIN
    index[2] = 3;
    index[3] = 2;
#else
    index[2] = 2;
    index[3] = 3;
    mIndexVB.Invalidate();
#endif

    mCurrentPrimitive = NULL;

    u32 tempBufferSize = config.GetDoubleBufferSize();
    tempBufferSize    += config.GetParticleNum() * (sizeof(PtclAttributeBuffer) + 16); // ... ?
    tempBufferSize    += config.GetEmitterNum()  *  sizeof(ViewUniformBlock);
    tempBufferSize    += config.GetEmitterNum()  *  sizeof(EmitterDynamicUniformBlock*);
    tempBufferSize    += config.GetParticleNum() *  sizeof(sortPtcl);

    mTemporaryBuffer.Initialize(mHeap, tempBufferSize);
}

Renderer::~Renderer()
{
    mTemporaryBuffer.Finalize(mHeap);
    mHeap->Free(mPositionVB.GetVertexBuffer());
    mHeap->Free(mIndexVB.GetVertexBuffer());
    mRenderContext.~Rendercontext();
}

void Renderer::BeginRender(const nw::math::MTX44& proj, const nw::math::MTX34& view, const nw::math::VEC3& camPos, f32 near, f32 far)
{
    mRenderContext.SetupCommonState();

    mMatView = nw::math::MTX44(view);

    nw::math::MTX34 billboardMtx;
    MTX34Transpose(&billboardMtx, &view);
    billboardMtx._03 = billboardMtx._13 = billboardMtx._23 = 0.0f;
    mMatBillboard.m[0][0] = billboardMtx.m[0][0]; mMatBillboard.m[0][1] = billboardMtx.m[0][1]; mMatBillboard.m[0][2] = billboardMtx.m[0][2]; mMatBillboard.m[0][3] = billboardMtx.m[0][3];
    mMatBillboard.m[1][0] = billboardMtx.m[1][0]; mMatBillboard.m[1][1] = billboardMtx.m[1][1]; mMatBillboard.m[1][2] = billboardMtx.m[1][2]; mMatBillboard.m[1][3] = billboardMtx.m[1][3];
    mMatBillboard.m[2][0] = billboardMtx.m[2][0]; mMatBillboard.m[2][1] = billboardMtx.m[2][1]; mMatBillboard.m[2][2] = billboardMtx.m[2][2]; mMatBillboard.m[2][3] = billboardMtx.m[2][3];
    mMatBillboard.m[3][0] = 0.0f;                 mMatBillboard.m[3][1] = 0.0f;                 mMatBillboard.m[3][2] = 0.0f;                 mMatBillboard.m[3][3] = 1.0f;

    mEyeVec.x = billboardMtx._02;
    mEyeVec.y = billboardMtx._12;
    mEyeVec.z = billboardMtx._22;

    mEyePos = camPos;

    // Why create a new matrix instead of using mMatView?
    nw::math::MTX44 view44;
    view44.m[0][0] = view.m[0][0]; view44.m[0][1] = view.m[0][1]; view44.m[0][2] = view.m[0][2]; view44.m[0][3] = view.m[0][3];
    view44.m[1][0] = view.m[1][0]; view44.m[1][1] = view.m[1][1]; view44.m[1][2] = view.m[1][2]; view44.m[1][3] = view.m[1][3];
    view44.m[2][0] = view.m[2][0]; view44.m[2][1] = view.m[2][1]; view44.m[2][2] = view.m[2][2]; view44.m[2][3] = view.m[2][3];
    view44.m[3][0] = 0.0f;         view44.m[3][1] = 0.0f;         view44.m[3][2] = 0.0f;         view44.m[3][3] = 1.0f;
    mMatViewProj.SetMult(proj, view44);

    mViewUniformBlock = static_cast<ViewUniformBlock*>(AllocFromDoubleBuffer(sizeof(ViewUniformBlock)));
    if (mViewUniformBlock)
    {
        mViewUniformBlock->viewMat = mMatView;
        mViewUniformBlock->vpMat   = mMatViewProj;
        mViewUniformBlock->bldMat  = mMatBillboard;
        mViewUniformBlock->eyeVec  = nw::math::VEC4(mEyeVec);
        mViewUniformBlock->eyePos  = nw::math::VEC4(mEyePos);

        mViewUniformBlock->depthBufferTexMat.x = mDepthTextureScale.x;
        mViewUniformBlock->depthBufferTexMat.y = mDepthTextureScale.y;
        mViewUniformBlock->depthBufferTexMat.z = mDepthTextureOffset.x;
        mViewUniformBlock->depthBufferTexMat.w = mDepthTextureOffset.y;

        mViewUniformBlock->frameBufferTexMat.x = mFrameBufferTextureScale.x;
        mViewUniformBlock->frameBufferTexMat.y = mFrameBufferTextureScale.y;
        mViewUniformBlock->frameBufferTexMat.z = mFrameBufferTextureOffset.x;
        mViewUniformBlock->frameBufferTexMat.w = mFrameBufferTextureOffset.y;

        mViewUniformBlock->viewParam.x = near;
        mViewUniformBlock->viewParam.y = far;
        mViewUniformBlock->viewParam.z = 0.0f;
        mViewUniformBlock->viewParam.w = 0.0f;

#if EFT_IS_CAFE
        GX2EndianSwap(mViewUniformBlock, sizeof(ViewUniformBlock));
        GX2Invalidate(GX2_INVALIDATE_CPU_UNIFORM_BLOCK, mViewUniformBlock, sizeof(ViewUniformBlock));
#endif // EFT_IS_CAFE
    }
}

bool Renderer::SetupParticleShaderAndVertex(ParticleShader* shader, MeshType meshType, Primitive* primitive)
{
    shader->Bind();
    shader->BindViewUniformBlock(mViewUniformBlock);

    if (meshType == EFT_MESH_TYPE_PARTICLE)
    {
        mPositionVB.BindBuffer(shader->GetPositionAttribute(), sizeof(nw::math::VEC3) * 4, sizeof(nw::math::VEC3));
        if (shader->GetIndexAttribute() != EFT_INVALID_ATTRIBUTE)
            mIndexVB.BindBuffer(shader->GetIndexAttribute(), sizeof(u32) * 4, sizeof(u32));

#if EFT_IS_WIN
        mDrawPrimitiveType = Draw::PRIM_TYPE_TRIANGLE_STRIP;
#else
        mDrawPrimitiveType = Draw::PRIM_TYPE_QUADS;
#endif

        mCurrentPrimitive = NULL;
    }
    else
    {
        if (primitive == NULL || primitive->GetIndexNum() == 0)
            return false;

        mCurrentPrimitive = primitive;
        if (!mCurrentPrimitive->IsInitialized())
            return false;

        mCurrentPrimitive->BindPosionBuffer(shader->GetPositionAttribute());
        mCurrentPrimitive->BindTextureCrdBuffer(shader->GetTextureCoordAttribute());
        mCurrentPrimitive->BindColorBuffer(shader->GetColorAttribute());
        if (shader->GetNormalAttribute() != EFT_INVALID_ATTRIBUTE)
            mCurrentPrimitive->BindNormalBuffer(shader->GetNormalAttribute());

        mDrawPrimitiveType = Draw::PRIM_TYPE_TRIANGLES;
    }

    return true;
}

void Renderer::RequestParticle(const EmitterInstance* emitter, ParticleShader* shader, bool bChild, bool cacheFlush, void* userParam)
{
    const SimpleEmitterData* res = emitter->GetSimpleEmitterData();

    const TextureRes* texture0          = &res->texRes[EFT_TEXTURE_SLOT_0];
    const TextureRes* texture1          = &res->texRes[EFT_TEXTURE_SLOT_1];

    ZBufATestType zBuffATest            = res->zBufATestType;
    BlendType blendType                 = res->blendType;
    UserShaderCallBackID shaderCallback = static_cast<UserShaderCallBackID>(res->userShaderSetting);
    u32 numPtcl                         = emitter->ptclNum;
    MeshType meshType                   = res->meshType;
    DisplaySideType dispSideType        = res->displaySide;

    if (bChild)
    {
        const ChildData* cdata = emitter->GetChildData();

        texture0       = &cdata->childTex;
        texture1       = NULL;

        zBuffATest     = cdata->childZBufATestType;
        blendType      = cdata->childBlendType;
        numPtcl        = emitter->childPtclNum;
        shaderCallback = static_cast<UserShaderCallBackID>(cdata->childUserShaderSetting);
        meshType       = cdata->childMeshType;
        dispSideType   = cdata->childDisplaySide;
    }

    if (numPtcl == 0)
        return;

    {
        mRenderContext.SetupZBufATest(zBuffATest);
        mRenderContext.SetupBlendType(blendType);
        mRenderContext.SetupDisplaySideType(dispSideType);
    }

    {
        mRenderContext.SetupTexture(texture0, EFT_TEXTURE_SLOT_0, shader->GetTextureSamplerLocation(EFT_TEXTURE_SLOT_0));
    }

    if (texture1 && texture1->handle)
    {
        mRenderContext.SetupTexture(texture1, EFT_TEXTURE_SLOT_1, shader->GetTextureSamplerLocation(EFT_TEXTURE_SLOT_1));
    }
    else
    {
        mRenderContext.SetupTexture((const TextureRes*)NULL, EFT_TEXTURE_SLOT_1, (FragmentTextureLocation){ 0u });
    }

    if (mExtensionTexture[EFT_TEXTURE_SLOT_DEPTH_BUFFER - EFT_TEXTURE_SLOT_BIN_MAX] &&
        shader->GetTextureSamplerLocation(EFT_TEXTURE_SLOT_DEPTH_BUFFER).IsValid())
    {
        mRenderContext.SetupTexture(mExtensionTexture[EFT_TEXTURE_SLOT_DEPTH_BUFFER - EFT_TEXTURE_SLOT_BIN_MAX],
                                    EFT_TEXTURE_SLOT_DEPTH_BUFFER, shader->GetTextureSamplerLocation(EFT_TEXTURE_SLOT_DEPTH_BUFFER));
    }

    if (mExtensionTexture[EFT_TEXTURE_SLOT_FRAME_BUFFER - EFT_TEXTURE_SLOT_BIN_MAX] &&
        shader->GetTextureSamplerLocation(EFT_TEXTURE_SLOT_FRAME_BUFFER).IsValid())
    {
        mRenderContext.SetupTexture(mExtensionTexture[EFT_TEXTURE_SLOT_FRAME_BUFFER - EFT_TEXTURE_SLOT_BIN_MAX],
                                    EFT_TEXTURE_SLOT_FRAME_BUFFER, shader->GetTextureSamplerLocation(EFT_TEXTURE_SLOT_FRAME_BUFFER));
    }

    if (mSystem->GetUserShaderRenderStateSetCallback(shaderCallback))
    {
        RenderStateSetArg arg = {
            .emitter = emitter,
            .renderer = this,
            .cacheFlush = cacheFlush,
            .userParam = userParam,
        };
        mSystem->GetUserShaderRenderStateSetCallback(shaderCallback)(arg);
    }

#if EFT_IS_WIN
    GLuint gl_vbo;
    glGenBuffers(1, &gl_vbo);
#endif

    if (!bChild && res->flg & EFT_EMITTER_FLAG_ENABLE_SORTPARTICLE)
    {
        u32 cnt = 0;

        sortPtcl* sortPtcls = static_cast<sortPtcl*>(AllocFromDoubleBuffer(sizeof(sortPtcl) * numPtcl));
        if (sortPtcls)
        {
            PtclInstance* p = emitter->ptclHead;
            while (p)
            {
                sortPtcls[cnt].ptcl  = p;
                sortPtcls[cnt].z     = mMatView.f._20 * p->worldPos.x +
                                       mMatView.f._21 * p->worldPos.y +
                                       mMatView.f._22 * p->worldPos.z +
                                       mMatView.f._23;
                sortPtcls[cnt].index = cnt;
                p = p->next;
                cnt++;
            }

            qsort(sortPtcls, cnt, sizeof(sortPtcl), ComparePtclViewZ);

            PtclAttributeBuffer* ptclAttributeBuffer = emitter->ptclAttributeBuffer;

            for (u32 i = 0; i < cnt; i++)
            {
#if EFT_IS_WIN
                glBindBuffer(GL_ARRAY_BUFFER, gl_vbo);
                glBufferData(GL_ARRAY_BUFFER, sizeof(PtclAttributeBuffer), &ptclAttributeBuffer[sortPtcls[i].index], GL_STATIC_DRAW);
#endif

                BindParticleAttributeBlock(&ptclAttributeBuffer[sortPtcls[i].index], shader, 1);

                if (meshType == EFT_MESH_TYPE_PRIMITIVE && mCurrentPrimitive)
                    Draw::DrawInstanceIndexedPrimitive(mDrawPrimitiveType, mCurrentPrimitive->GetIndexNum(), mCurrentPrimitive->GetIndexBuffer(), 1);

                else
                    Draw::DrawInstancePrimitive(mDrawPrimitiveType, 4, 1);
            }
        }
#if EFT_IS_WIN
        glDeleteBuffers(1, &gl_vbo);
#endif
        return;
    }

    u32 entryNum;

    if (bChild)
    {
        entryNum = emitter->childEntryNum;

#if EFT_IS_WIN
        glBindBuffer(GL_ARRAY_BUFFER, gl_vbo);
        glBufferData(GL_ARRAY_BUFFER, entryNum * sizeof(PtclAttributeBuffer), emitter->childPtclAttributeBuffer, GL_STATIC_DRAW);
#endif

        BindParticleAttributeBlock(emitter->childPtclAttributeBuffer, shader, entryNum);
    }
    else
    {
        entryNum = emitter->entryNum;

#if EFT_IS_WIN
        glBindBuffer(GL_ARRAY_BUFFER, gl_vbo);
        glBufferData(GL_ARRAY_BUFFER, entryNum * sizeof(PtclAttributeBuffer), emitter->ptclAttributeBuffer, GL_STATIC_DRAW);
#endif

        BindParticleAttributeBlock(emitter->ptclAttributeBuffer, shader, entryNum);
    }

    if (meshType == EFT_MESH_TYPE_PRIMITIVE && mCurrentPrimitive)
        Draw::DrawInstanceIndexedPrimitive(mDrawPrimitiveType, mCurrentPrimitive->GetIndexNum(), mCurrentPrimitive->GetIndexBuffer(), entryNum);

    else
        Draw::DrawInstancePrimitive(mDrawPrimitiveType, 4, entryNum);

#if EFT_IS_WIN
    glDeleteBuffers(1, &gl_vbo);
#endif
}

void Renderer::EntryChildParticleSub(const EmitterInstance* emitter, bool cacheFlush, void* userParam)
{
    ParticleShader* shader = emitter->childShader[mCurrentShaderType];
    if (shader == NULL
        || emitter->childPtclAttributeBuffer == NULL
        || emitter->childEmitterDynamicUniformBlock == NULL
        || !SetupParticleShaderAndVertex(shader, emitter->GetChildData()->childMeshType, emitter->childPrimitive))
    {
        return;
    }

    shader->BindEmitterStaticUniformBlock(emitter->childEmitterStaticUniformBlock);
    shader->BindEmitterDynamicUniformBlock(emitter->childEmitterDynamicUniformBlock);

    shader->EnableInstanced();
    RequestParticle(emitter, shader, true, cacheFlush, userParam);
    shader->DisableInstanced();
}

void Renderer::EntryParticleSub(const EmitterInstance* emitter, bool cacheFlush, void* userParam)
{
    ParticleShader* shader = emitter->shader[mCurrentShaderType];
    if (shader == NULL
        || emitter->ptclAttributeBuffer == NULL
        || emitter->emitterDynamicUniformBlock == NULL
        || !SetupParticleShaderAndVertex(shader, emitter->GetSimpleEmitterData()->meshType, emitter->primitive))
    {
        return;
    }

    shader->BindEmitterStaticUniformBlock(emitter->emitterStaticUniformBlock);
    shader->BindEmitterDynamicUniformBlock(emitter->emitterDynamicUniformBlock);

    shader->EnableInstanced();
    RequestParticle(emitter, shader, false, cacheFlush, userParam);
    shader->DisableInstanced();
}

void Renderer::EntryParticle(EmitterInstance* emitter, bool cacheFlush, void* userParam)
{
    if (mViewUniformBlock == NULL)
        return;

    UserShaderDrawOverrideCallback parentShaderPreDrawCallBack = mSystem->GetUserShaderDrawOverrideCallback(static_cast<UserShaderCallBackID>(emitter->res->userShaderSetting));
    ShaderDrawOverrideArg arg = {
        .emitter = emitter,
        .renderer = this,
        .cacheFlush = cacheFlush,
        .userParam = userParam,
    };

    if (!(emitter->controller->GetDrawViewFlag() & mDrawViewFlag))
        return;

    bool bStripeDraw = false;
    if (emitter->GetBillboardType() == EFT_BILLBOARD_TYPE_STRIPE ||
        emitter->GetBillboardType() == EFT_BILLBOARD_TYPE_COMPLEX_STRIPE)
    {
        EntryStripe(emitter, cacheFlush, userParam);
        bStripeDraw = true;
    }

    const SimpleEmitterData* __restrict res = emitter->GetSimpleEmitterData();

    if (emitter->GetEmitterType() == EFT_EMITTER_TYPE_COMPLEX && emitter->IsHasChildParticle())
    {
        const ComplexEmitterData* cres = emitter->GetComplexEmitterData();
        UserShaderDrawOverrideCallback childShaderPreDrawCallBack = mSystem->GetUserShaderDrawOverrideCallback(static_cast<UserShaderCallBackID>(emitter->GetChildData()->childUserShaderSetting));

        if (cres->childFlg & EFT_CHILD_FLAG_PRE_CHILD_DRAW)
        {
            if (emitter->childEntryNum != 0 && emitter->childPtclAttributeBuffer)
            {
                mCurrentDrawingType = EFT_PTCL_TYPE_CHILD;

                if (childShaderPreDrawCallBack)
                    childShaderPreDrawCallBack(arg);

                else
                    EntryChildParticleSub(emitter, cacheFlush, userParam);
            }

            if (cres->isDisplayParent && !bStripeDraw && emitter->entryNum != 0 && emitter->ptclAttributeBuffer)
            {
                mCurrentDrawingType = EFT_PTCL_TYPE_COMPLEX;

                if (parentShaderPreDrawCallBack)
                    parentShaderPreDrawCallBack(arg);

                else
                    EntryParticleSub(emitter, cacheFlush, userParam);
            }
        }
        else
        {
            if (cres->isDisplayParent && !bStripeDraw && emitter->entryNum != 0 && emitter->ptclAttributeBuffer)
            {
                mCurrentDrawingType = EFT_PTCL_TYPE_COMPLEX;

                if (parentShaderPreDrawCallBack)
                    parentShaderPreDrawCallBack(arg);

                else
                    EntryParticleSub(emitter, cacheFlush, userParam);
            }

            if (emitter->childEntryNum != 0 && emitter->childPtclAttributeBuffer)
            {
                mCurrentDrawingType = EFT_PTCL_TYPE_CHILD;

                if (childShaderPreDrawCallBack)
                    childShaderPreDrawCallBack(arg);

                else
                    EntryChildParticleSub(emitter, cacheFlush, userParam);
            }
        }
    }
    else
    {
        PtclType ptcl_type = emitter->GetEmitterType() == EFT_EMITTER_TYPE_SIMPLE
                                    ? EFT_PTCL_TYPE_SIMPLE
                                    : EFT_PTCL_TYPE_COMPLEX;

        if (!bStripeDraw)
        {
            mCurrentDrawingType = ptcl_type;

            if (res->isDisplayParent && emitter->entryNum != 0 && emitter->ptclAttributeBuffer)
            {
                if (parentShaderPreDrawCallBack)
                    parentShaderPreDrawCallBack(arg);

                else
                    EntryParticleSub(emitter, cacheFlush, userParam);
            }
        }
    }

    mCurrentDrawingType = EFT_PTCL_TYPE_MAX;
}

void Renderer::EndRender()
{
}

} } // namespace nw::eft

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif // __GNUC__
