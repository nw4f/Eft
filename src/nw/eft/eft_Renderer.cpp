#include <nw/eft/eft_Config.h>
#include <nw/eft/eft_EmitterSet.h>
#include <nw/eft/eft_Misc.h>
#include <nw/eft/eft_Particle.h>
#include <nw/eft/eft_Primitive.h>
#include <nw/eft/eft_Renderer.h>
#include <nw/eft/eft_Shader.h>
#include <nw/eft/eft_System.h>
#include <nw/eft/eft_UniformBlock.h>

#include <cstdlib>

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

Renderer::Renderer(System* system, const Config& config)
    : mSystem(system)
{
    mDepthTextureOffset.x               = 0.0f;
    mDepthTextureOffset.y               = 0.0f;
    mDepthTextureScale.x                = 1.0f;
    mDepthTextureScale.y                = 1.0f;
    mFrameBufferTextureOffset.x         = 0.0f;
    mFrameBufferTextureOffset.y         = 0.0f;
    mFrameBufferTextureScale.x          = 1.0f;
    mFrameBufferTextureScale.y          = 1.0f;
    mCurrentDrawingType                 = EFT_PTCL_TYPE_MAX;
    mCurrentShaderType                  = EFT_SHADER_TYPE_NORMAL;
    mDrawViewFlag                       = EFT_DRAW_VIEW_FLAG_ALL;
    mRenderingEmitterNum                = 0;
    mRenderingParticleNum               = 0;
    mCurrentPrimitive                   = NULL;
    mDrawPrimitiveType                  = Draw::PRIM_TYPE_QUADS;
    mViewDirtyFlag                      = true;

    for (u32 i = 0; i < EFT_TEXTURE_SLOT_MAX; i++)
        mExtensionTexture[i] = Texture(0);

    const f32 particleWH = 0.5f;
    nw::math::VEC3* pos = static_cast<nw::math::VEC3*>(AllocFromStaticHeap(sizeof(nw::math::VEC3) * 4));
    {
        pos[0].x = -particleWH;
        pos[0].y =  particleWH;
        pos[0].z =  0.0f;
        pos[1].x = -particleWH;
        pos[1].y = -particleWH;
        pos[1].z =  0.0f;
        pos[2].x =  particleWH;
        pos[2].y = -particleWH;
        pos[2].z =  0.0f;
        pos[3].x =  particleWH;
        pos[3].y =  particleWH;
        pos[3].z =  0.0f;
    }
    mPositionVB.SetVertexBuffer(pos, sizeof(nw::math::VEC3) * 4, 3);
    DCFlushRange(pos, sizeof(nw::math::VEC3) * 4);

    u32* index = static_cast<u32*>(AllocFromStaticHeap(sizeof(u32) * 4));
    {
        index[0] = 0;
        index[1] = 1;
        index[2] = 2;
        index[3] = 3;
    }
    mIndexVB.SetVertexBuffer(index, sizeof(u32) * 4, 1);
    DCFlushRange(index, sizeof(u32) * 4);

    u32 tempBufferSize = config.GetDoubleBufferSize() +
                         config.GetParticleNum() * sizeof(PtclAttributeBuffer) +
                         EFT_GROUP_MAX           * sizeof(ViewUniformBlock) +
                         config.GetEmitterNum()  * sizeof(EmitterDynamicUniformBlock*);

    mTemporaryBuffer.Initialize(tempBufferSize, config.IsEnableTripleBuffer());
}

Renderer::~Renderer()
{
    mTemporaryBuffer.Finalize();
    FreeFromStaticHeap(mPositionVB.GetVertexBuffer());
    FreeFromStaticHeap(mIndexVB.GetVertexBuffer());
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

    // Why? Couldn't you have used mMatView?
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
        mViewUniformBlock->projMat = proj;
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
        mViewUniformBlock->viewParam.z = near * far;
        mViewUniformBlock->viewParam.w = far - near;

        GX2EndianSwap(mViewUniformBlock, sizeof(ViewUniformBlock));
        GX2Invalidate(GX2_INVALIDATE_CPU_UNIFORM_BLOCK, mViewUniformBlock, sizeof(ViewUniformBlock));

        mViewDirtyFlag = true;
    }
}

bool Renderer::SetupParticleShaderAndVertex(ParticleShader* shader, MeshType meshType, Primitive* primitive)
{
    shader->Bind();
    shader->BindViewUniformBlock(mViewUniformBlock);

    mViewDirtyFlag = false;

    if (meshType == EFT_MESH_TYPE_PARTICLE)
    {
        mPositionVB.BindBuffer(shader->GetPositionAttribute(), sizeof(nw::math::VEC3) * 4, sizeof(nw::math::VEC3));
        if (shader->GetIndexAttribute() != EFT_INVALID_ATTRIBUTE)
            mIndexVB.BindBuffer(shader->GetIndexAttribute(), sizeof(u32) * 4, sizeof(u32));

        mDrawPrimitiveType = Draw::PRIM_TYPE_QUADS;
        mCurrentPrimitive = NULL;
    }
    else
    {
        if (primitive == NULL || primitive->GetIndexNum() == 0)
            return false;

        mCurrentPrimitive = primitive;
        if (!mCurrentPrimitive->IsInitialized())
            return false;

        mCurrentPrimitive->BindPosionBuffer    (shader->GetPositionAttribute());
        mCurrentPrimitive->BindTextureCrdBuffer(shader->GetTextureCoordAttribute());
        mCurrentPrimitive->BindColorBuffer     (shader->GetColorAttribute());
        if (shader->GetNormalAttribute() != EFT_INVALID_ATTRIBUTE)
            mCurrentPrimitive->BindNormalBuffer(shader->GetNormalAttribute());

        mDrawPrimitiveType = Draw::PRIM_TYPE_TRIANGLES;
    }

    return true;
}

void Renderer::SetupTexture(ParticleShader* shader, const TextureRes* texture0, const TextureRes* texture1, const TextureRes* texture2)
{
    if (texture0 && texture0->handle)
        mRenderContext.SetupTexture(texture0, EFT_TEXTURE_SLOT_0, shader->GetTextureSamplerLocation(EFT_TEXTURE_SLOT_0));

    if (texture1 && texture1->handle)
        mRenderContext.SetupTexture(texture1, EFT_TEXTURE_SLOT_1, shader->GetTextureSamplerLocation(EFT_TEXTURE_SLOT_1));

    if (texture2 && texture2->handle)
        mRenderContext.SetupTexture(texture2, EFT_TEXTURE_SLOT_2, shader->GetTextureSamplerLocation(EFT_TEXTURE_SLOT_2));

    for (u32 i = EFT_TEXTURE_SLOT_FRAME_BUFFER; i < EFT_TEXTURE_SLOT_MAX; i++)
    {
        TextureSlot slot = static_cast<TextureSlot>(i);

        if (mExtensionTexture[slot] &&
            shader->GetTextureSamplerLocation(slot).IsValid())
        {
            if (slot == EFT_TEXTURE_SLOT_CURL_NOISE)
                mRenderContext.SetupUserFragment2DArrayTexture(mExtensionTexture[slot], slot, shader->GetTextureSamplerLocation(slot));

            else
                mRenderContext.SetupTexture(mExtensionTexture[slot], slot, shader->GetTextureSamplerLocation(slot));
        }

        if (mExtensionTexture[slot] &&
            shader->GetVertexTextureSamplerLocation(slot).IsValid())
        {
            if (slot == EFT_TEXTURE_SLOT_CURL_NOISE)
                mRenderContext.SetupVertexArrayTexture(mExtensionTexture[slot], slot, shader->GetVertexTextureSamplerLocation(slot));

            else
                mRenderContext.SetupUserVertexTexture(mExtensionTexture[slot], slot, shader->GetVertexTextureSamplerLocation(slot));
        }
    }
}

void Renderer::DrawCpuEntry(ParticleShader* shader, Draw::PrimitiveType drawType, u32 startNum, u32 entryNum, PtclAttributeBuffer* buffer, Primitive* primitive)
{
    if (buffer)
        BindParticleAttributeBlock(buffer, shader, startNum, entryNum);

    if (primitive)
        Draw::DrawInstanceIndexedPrimitive(drawType, primitive->GetIndexNum(), primitive->GetIndexBuffer(), entryNum);
    else
        Draw::DrawInstancePrimitive(drawType, 4, entryNum);
}

void Renderer::DrawGpuEntry(ParticleShader* shader, Draw::PrimitiveType drawType, u32 startNum, u32 entryNum, PtclAttributeBufferGpu* buffer, Primitive* primitive)
{
    if (buffer)
        BindGpuParticleAttributeBlock(buffer, shader, startNum, entryNum);

    if (primitive)
        Draw::DrawInstanceIndexedPrimitive(drawType, primitive->GetIndexNum(), primitive->GetIndexBuffer(), entryNum);
    else
        Draw::DrawInstancePrimitive(drawType, 4, entryNum);
}

void Renderer::RequestParticle(const EmitterInstance* emitter, ParticleShader* shader, bool bChild, void* userParam, bool bDraw)
{
    const SimpleEmitterData* res = emitter->GetSimpleEmitterData();

    ZBufATestType           zBuffATest      = res->zBufATestType;
    BlendType               blendType       = res->blendType;
    CustomShaderCallBackID  shaderCallback  = static_cast<CustomShaderCallBackID>(res->userShaderSetting);
    u32                     numPtcl         = emitter->ptclNum;
    MeshType                meshType        = res->meshType;
    DisplaySideType         dispSideType    = res->displaySide;
    bool                    isGpu           = shader->IsGpuAcceleration();

    if (bChild)
    {
        const ChildData* cdata = emitter->GetChildData();

        zBuffATest      = cdata->childZBufATestType;
        blendType       = cdata->childBlendType;
        numPtcl         = emitter->childPtclNum;
        shaderCallback  = static_cast<CustomShaderCallBackID>(cdata->childUserShaderSetting);
        meshType        = cdata->childMeshType;
        dispSideType    = cdata->childDisplaySide;
    }

    {
        mRenderContext.SetupZBufATest(zBuffATest);
        mRenderContext.SetupBlendType(blendType);
        mRenderContext.SetupDisplaySideType(dispSideType);
    }

    if (mSystem->GetCustomShaderRenderStateSetCallback(shaderCallback))
    {
        RenderStateSetArg arg;
        arg.emitter = emitter;
        arg.renderer = this;
        arg.cacheFlush = true;
        arg.userParam = userParam;
        mSystem->GetCustomShaderRenderStateSetCallback(shaderCallback)(arg);
    }
    else if (shaderCallback != EFT_CUSTOM_SHADER_CALLBACK_NONE)
    {
        RenderStateSetArg arg;
        arg.emitter = emitter;
        arg.renderer = this;
        arg.cacheFlush = true;
        arg.userParam = userParam;
        shader->SetUserUniformBlockFromData(arg, ParticleShader::UNIFORM_BLOCK_USER_VERTEX_ID_0);
    }

    if (mSystem->GetDrawPathRenderStateSetCallback(static_cast<DrawPathFlag>(1 << emitter->res->drawPath)))
    {
        RenderStateSetArg arg;
        arg.emitter = emitter;
        arg.renderer = this;
        arg.cacheFlush = true;
        arg.userParam = userParam;
        mSystem->GetDrawPathRenderStateSetCallback(static_cast<DrawPathFlag>(1 << emitter->res->drawPath))(arg);
    }

    if (!bDraw)
        return;

    if (!isGpu)
    {
        if (!bChild && res->flg & EFT_EMITTER_FLAG_ENABLE_SORTPARTICLE)
        {
            u32 cnt = 0;

            sortPtcl* sortPtcls = static_cast<sortPtcl*>(AllocFromDoubleBuffer(sizeof(sortPtcl) * numPtcl));
            if (sortPtcls)
            {
                PtclInstance* p = emitter->ptclHead;
                if (p == NULL)
                    return;

                while (p)
                {
                    sortPtcls[cnt].ptcl = p;
                    sortPtcls[cnt].z     = mMatView.f._20 * p->worldPos.x +
                                           mMatView.f._21 * p->worldPos.y +
                                           mMatView.f._22 * p->worldPos.z +
                                           mMatView.f._23;
                    sortPtcls[cnt].index = cnt;
                    p = p->next;
                    cnt++;
                }

                qsort(sortPtcls, cnt, sizeof(sortPtcl), ComparePtclViewZ);

                for (u32 i = 0; i < cnt; i++)
                {
                    BindParticleAttributeBlock(&emitter->ptclAttributeBuffer[sortPtcls[i].index], shader, 0, 1);

                    if (meshType == EFT_MESH_TYPE_PRIMITIVE && mCurrentPrimitive)
                        Draw::DrawInstanceIndexedPrimitive(mDrawPrimitiveType, mCurrentPrimitive->GetIndexNum(), mCurrentPrimitive->GetIndexBuffer(), 1);
                    else
                        Draw::DrawInstancePrimitive(mDrawPrimitiveType, 4, 1);
                }

                mRenderingParticleNum += cnt;
            }
        }
        else
        {
            PtclAttributeBuffer* ptclAttributeBuffer = emitter->ptclAttributeBuffer;
            u32 entryNum = emitter->entryNum;

            if (bChild)
            {
                ptclAttributeBuffer = emitter->childPtclAttributeBuffer;
                entryNum = emitter->childEntryNum;
            }

            DrawCpuEntry(shader, mDrawPrimitiveType, 0, entryNum, ptclAttributeBuffer, mCurrentPrimitive);

            mRenderingParticleNum += entryNum;
        }
    }
    else
    {
        if (shader->IsUseStreamOut())
        {
            EmitterInstance* ccemt = const_cast<EmitterInstance*>(emitter);

            BindGpuParticleAttributeBlock(emitter->ptclAttributeBufferGpu, shader, 0, emitter->entryNum);

            bool ret0 = ccemt->positionStreamOutBuffer.Bind(shader->mPositionInput, 0, ccemt->streamOutFlip, false);
            bool ret1 = ccemt->vectorStreamOutBuffer  .Bind(shader->mVectorInput,   1, ccemt->streamOutFlip, false);

            if (!ret0 || !ret1)
                return;

            DrawGpuEntry(shader, mDrawPrimitiveType, 0, emitter->entryNum, NULL, mCurrentPrimitive);
        }
        else
        {
            if (emitter->controller->mEmissionRatio < 1.0f && !emitter->controller->mEmissionRatioChanged)
            {
                u32 emitMax = (u32)(emitter->gpuParticleBufferNum * emitter->controller->mEmissionRatio);

                if (emitter->gpuParticleBufferFillNum < emitter->entryNum && emitter->gpuParticleBufferFillNum < emitMax)
                {
                    s32 deleteNum = emitMax - emitter->gpuParticleBufferFillNum;
                    DrawGpuEntry(shader, mDrawPrimitiveType, emitter->gpuParticleBufferNum - deleteNum, deleteNum, emitter->ptclAttributeBufferGpu, mCurrentPrimitive);
                }

                DrawGpuEntry(shader, mDrawPrimitiveType, 0, emitter->gpuParticleBufferFillNum, emitter->ptclAttributeBufferGpu, mCurrentPrimitive);
            }
            else
            {
                DrawGpuEntry(shader, mDrawPrimitiveType, 0, emitter->entryNum, emitter->ptclAttributeBufferGpu, mCurrentPrimitive);
            }
        }

        mRenderingParticleNum += emitter->entryNum;
    }

    mRenderingEmitterNum++;
}

bool Renderer::EntryChildParticleSub(const EmitterInstance* emitter, void* userParam, bool bDraw)
{
    if (emitter->childEntryNum == 0 || emitter->childEmitterDynamicUniformBlock == NULL || emitter->childPtclAttributeBuffer == NULL || !bDraw)
        return false;

    ParticleShader* shader = emitter->childShader[mCurrentShaderType];
    if (shader == NULL)
        return false;

    const ChildData* cres = emitter->GetChildData();

    if (!SetupParticleShaderAndVertex(shader, cres->childMeshType, emitter->childPrimitive))
        return false;

    shader->BindEmitterStaticUniformBlock (emitter->childEmitterStaticUniformBlock);
    shader->BindEmitterDynamicUniformBlock(emitter->childEmitterDynamicUniformBlock);

    const TextureRes* texture0 = &cres->childTex;
    SetupTexture(shader, texture0, NULL, NULL);

    shader->EnableInstanced();
    RequestParticle(emitter, shader, true, userParam, bDraw);
    shader->DisableInstanced();

    return true;
}

bool Renderer::EntryParticleSub(const EmitterInstance* emitter, void* userParam, bool bDraw)
{
    if (emitter->entryNum == 0 || emitter->emitterDynamicUniformBlock == NULL || !bDraw)
        return false;

    ParticleShader* shader = emitter->shader[mCurrentShaderType];
    if (shader == NULL)
        return false;

    if (!shader->IsGpuAcceleration())
    {
        if (emitter->ptclAttributeBuffer == NULL)
            return false;
    }
    else
    {
        if (emitter->ptclAttributeBufferGpu == NULL)
            return false;
    }

    const SimpleEmitterData* res = emitter->GetSimpleEmitterData();

    if (!SetupParticleShaderAndVertex(shader, res->meshType, emitter->primitive))
        return false;

    shader->BindEmitterStaticUniformBlock (emitter->emitterStaticUniformBlock);
    shader->BindEmitterDynamicUniformBlock(emitter->emitterDynamicUniformBlock);

    const TextureRes* texture0 = &res->texRes[EFT_TEXTURE_SLOT_0];
    const TextureRes* texture1 = &res->texRes[EFT_TEXTURE_SLOT_1];
    const TextureRes* texture2 = &res->texRes[EFT_TEXTURE_SLOT_2];
    SetupTexture(shader, texture0, texture1, texture2);

    shader->EnableInstanced();
    RequestParticle(emitter, shader, false, userParam, bDraw);
    shader->DisableInstanced();

    return true;
}

void Renderer::EntryParticle(const EmitterInstance* emitter, void* userParam)
{
    if (mViewUniformBlock == NULL)
        return;

    mCurrentPrimitive = NULL;

    CustomShaderDrawOverrideCallback parentShaderPreDrawCallBack = mSystem->GetCustomShaderDrawOverrideCallback(static_cast<CustomShaderCallBackID>(emitter->res->userShaderSetting));

    ShaderDrawOverrideArg arg;
    arg.emitter = emitter;
    arg.renderer = this;
    arg.cacheFlush = true;
    arg.userParam = userParam;

    if (!(emitter->controller->GetDrawViewFalg() & mDrawViewFlag))
        return;

    bool bStripeDraw = false;
    if (emitter->GetBillboardType() == EFT_BILLBOARD_TYPE_STRIPE ||
        emitter->GetBillboardType() == EFT_BILLBOARD_TYPE_COMPLEX_STRIPE)
    {
        EntryStripe(emitter, userParam);
        bStripeDraw = true;
    }

    if (emitter->GetEmitterType() == EFT_EMITTER_TYPE_COMPLEX)
    {
        bool isChild = emitter->IsHasChildParticle();
        const ComplexEmitterData* cres = emitter->GetComplexEmitterData();

        CustomShaderDrawOverrideCallback childShaderPreDrawCallBack = NULL;

        if (isChild)
        {
            const ChildData* childRes = emitter->GetChildData();
            childShaderPreDrawCallBack = mSystem->GetCustomShaderDrawOverrideCallback(static_cast<CustomShaderCallBackID>(childRes->childUserShaderSetting));
        }

        if (isChild && cres->childFlg & EFT_CHILD_FLAG_PRE_CHILD_DRAW)
        {
            mCurrentDrawingType = EFT_PTCL_TYPE_CHILD;

            if (childShaderPreDrawCallBack)
                childShaderPreDrawCallBack(arg);

            else
                EntryChildParticleSub(emitter, userParam);
        }

        if (cres->isDisplayParent && !bStripeDraw)
        {
            mCurrentDrawingType = EFT_PTCL_TYPE_COMPLEX;

            if (parentShaderPreDrawCallBack)
                parentShaderPreDrawCallBack(arg);

            else
                EntryParticleSub(emitter, userParam);
        }

        if (isChild && !(cres->childFlg & EFT_CHILD_FLAG_PRE_CHILD_DRAW))
        {
            mCurrentDrawingType = EFT_PTCL_TYPE_CHILD;

            if (childShaderPreDrawCallBack)
                childShaderPreDrawCallBack(arg);

            else
                EntryChildParticleSub(emitter, userParam);
        }
    }
    else
    {
        if (emitter->res->isDisplayParent /* && !bStripeDraw */)
        {
            mCurrentDrawingType = EFT_PTCL_TYPE_SIMPLE;

            if (parentShaderPreDrawCallBack)
                parentShaderPreDrawCallBack(arg);

            else
                EntryParticleSub(emitter, userParam);
        }
    }

    mCurrentDrawingType = EFT_PTCL_TYPE_MAX;
}

void Renderer::BeginStremOut()
{
    GX2SetRasterizerClipControl(GX2_DISABLE, GX2_ENABLE);
    GX2SetStreamOutEnable(GX2_ENABLE);
}

void Renderer::CalcStremOutParticle(const EmitterInstance* emitter, bool bind)
{
    if (emitter->entryNum == 0 || emitter->emitterDynamicUniformBlock == NULL)
        return;

    ParticleShader* shader = emitter->shader[EFT_SHADER_TYPE_NORMAL];
    if (shader == NULL)
        return;

    if (!shader->IsGpuAcceleration())
    {
        if (emitter->ptclAttributeBuffer == NULL)
            return;
    }
    else
    {
        if (emitter->ptclAttributeBufferGpu == NULL)
            return;
    }

    const SimpleEmitterData* res = emitter->GetSimpleEmitterData();

    if (!SetupParticleShaderAndVertex(shader, res->meshType, emitter->primitive))
        return;

    shader->BindEmitterStaticUniformBlock (emitter->emitterStaticUniformBlock);
    shader->BindEmitterDynamicUniformBlock(emitter->emitterDynamicUniformBlock);

    shader->EnableInstanced();
    {
        EmitterInstance* ccemt = const_cast<EmitterInstance*>(emitter);

        u32 gCounter = mSystem->GetGlobalCounter(); // <--- Unused, but function literally does not match without it

        //if (emitter->streamOutCounter != gCounter)
        {
            BindGpuParticleAttributeBlock(emitter->ptclAttributeBufferGpu, shader, 0, emitter->entryNum);

            if (bind)
            {
                bool ret_pos = ccemt->positionStreamOutBuffer.Bind(shader->mPositionInput, 0, ccemt->streamOutFlip, true);
                bool ret_vec = ccemt->vectorStreamOutBuffer  .Bind(shader->mVectorInput,   1, ccemt->streamOutFlip, true);

                if (!ret_pos || !ret_vec)
                    return;
            }

            Draw::DrawInstancePrimitive(Draw::PRIM_TYPE_POINTS, 1, emitter->entryNum);

            ccemt->positionStreamOutBuffer.UnBind(0);
            ccemt->vectorStreamOutBuffer  .UnBind(1);

            ccemt->positionStreamOutBuffer.UnBind(0);
            ccemt->vectorStreamOutBuffer  .UnBind(1);
        }
    }

    shader->DisableInstanced();
}

void Renderer::EndStremOut()
{
    GX2SetStreamOutEnable(GX2_DISABLE);
    GX2SetRasterizerClipControl(GX2_ENABLE, GX2_ENABLE);
}

void Renderer::EndRender()
{
}

} } // namespace nw::eft
