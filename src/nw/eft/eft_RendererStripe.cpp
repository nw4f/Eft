#include <nw/eft/eft_Emitter.h>
#include <nw/eft/eft_EmitterSet.h>
#include <nw/eft/eft_Particle.h>
#include <nw/eft/eft_Renderer.h>
#include <nw/eft/eft_Shader.h>
#include <nw/eft/eft_System.h>
#include <nw/eft/eft_UniformBlock.h>

#include <algorithm>

namespace nw { namespace eft {

u32 Renderer::MakeStripeAttributeBlockCore(PtclStripe* stripe, StripeVertexBuffer* stripeVertexBuffer, s32 firstVertex, const StripeData* stripeData)
{
    stripe->drawFirstVertex = 0;
    stripe->numDraw = 0;

    s32 histQueueCount = stripe->queueCount;
    s32 sliceHistInterval = stripeData->sliceHistInterval;

    s32 numSliceHistory = std::min( sliceHistInterval,
                                    histQueueCount );
    if (numSliceHistory < 3)
        return 0;

    u32 numDrawStripe = 0;

    f32 invRatio    = 1.0f / (f32)(numSliceHistory - 2);
    f32 invTexRatio;

    if (stripeData->textureType == 1 && stripe->counter < sliceHistInterval)
        invTexRatio = 1.0f / (f32)(sliceHistInterval - 2);
    else
        invTexRatio = invRatio;

    f32 alphaRange = stripeData->alphaEnd - stripeData->alphaStart;
    stripe->drawFirstVertex = firstVertex;

    for (s32 i = 0; i < numSliceHistory - 1; i++)
    {
        u32 idx = firstVertex + numDrawStripe;
        StripeVertexBuffer* buffer0 = &stripeVertexBuffer[idx + 0];
        StripeVertexBuffer* buffer1 = &stripeVertexBuffer[idx + 1];

        f32 ratio    = (f32)i * invRatio;
        f32 texRatio = (f32)i * invTexRatio;

        f32 v0 = ratio * (f32)(histQueueCount - 2) + 0.5f;
        s32 v1 = (s32)v0;

        s32 sliceHistIdx = stripe->queueRear - v1 - 1;
        if (sliceHistIdx < 0)
            sliceHistIdx += stripeData->numSliceHistory;

        f32 alpha0 = (stripeData->alphaStart + alphaRange * ratio) * stripe->particle->alpha0 * stripe->particle->emitter->emitterSet->color.a * stripe->particle->emitter->fadeAlpha;
        f32 alpha1 = (stripeData->alphaStart + alphaRange * ratio) * stripe->particle->alpha1 * stripe->particle->emitter->emitterSet->color.a * stripe->particle->emitter->fadeAlpha;

        buffer0->pos.xyz() = stripe->queue[sliceHistIdx].pos;
        buffer0->pos.w     = alpha0 * stripe->particle->emitter->anim[14];
        buffer1->pos.xyz() = buffer0->pos.xyz();
        buffer1->pos.w     = buffer0->pos.w;

        buffer0->dir.xyz() = stripe->queue[sliceHistIdx].dir;
        buffer0->dir.w     = alpha1 * stripe->particle->emitter->anim[14];
        buffer1->dir.xyz() = buffer0->dir.xyz();
        buffer1->dir.w     = buffer0->dir.w;

        buffer0->outer.xyz() = stripe->queue[sliceHistIdx].outer;
        buffer1->outer.xyz() = buffer0->outer.xyz();

        buffer0->outer.w =  stripe->queue[sliceHistIdx].scale;
        buffer1->outer.w = -stripe->queue[sliceHistIdx].scale;

        buffer1->texCoord.x = stripe->texAnimParam[0].offset.x;
        buffer1->texCoord.y = stripe->texAnimParam[0].offset.y + texRatio * stripe->data->texAnimParam[0].uvScaleInit.y;
        buffer1->texCoord.z = stripe->texAnimParam[1].offset.x;
        buffer1->texCoord.w = stripe->texAnimParam[1].offset.y + texRatio * stripe->data->texAnimParam[1].uvScaleInit.y;

        buffer0->texCoord.x = stripe->texAnimParam[0].offset.x +            stripe->data->texAnimParam[0].uvScaleInit.x;
        buffer0->texCoord.y = buffer1->texCoord.y;
        buffer0->texCoord.z = stripe->texAnimParam[1].offset.x +            stripe->data->texAnimParam[1].uvScaleInit.x;
        buffer0->texCoord.w = buffer1->texCoord.w;

        numDrawStripe += 2;
    }

    stripe->numDraw = numDrawStripe;

    return numDrawStripe;
}

void Renderer::GetPositionOnCubic(math::VEC3* result, const math::VEC3& startPos, const math::VEC3& startVel, const math::VEC3& endPos, const math::VEC3& endVel, f32 time)
{
    // http://old.zaynar.co.uk/cppdoc/latest/projects/maths/NUSpline.cpp.html

    static const math::MTX44 hermite(
         2.0f, -3.0f, 0.0f, 1.0f,
        -2.0f,  3.0f, 0.0f, 0.0f,
         1.0f, -2.0f, 1.0f, 0.0f,
         1.0f, -1.0f, 0.0f, 0.0f
    );

    math::MTX44 mtx(
        startPos.x, endPos.x, startVel.x, endVel.x,
        startPos.y, endPos.y, startVel.y, endVel.y,
        startPos.z, endPos.z, startVel.z, endVel.z,
        0.0f, 0.0f, 0.0f, 1.0f
    );

    math::MTX44::Concat(&mtx, &mtx, &hermite);

    math::VEC3 timeVector = (math::VEC3){ time * time * time, time * time, time };
    math::MTX34::MultVec(result, (const math::MTX34*)&mtx, &timeVector);
}

u32 Renderer::MakeStripeAttributeBlockCoreDivide(PtclStripe* stripe, StripeVertexBuffer* stripeVertexBuffer, s32 firstVertex, const StripeData* stripeData)
{
    s32 numDivisions = stripeData->numDivisions;

    stripe->drawFirstVertex = 0;
    stripe->numDraw = 0;

    s32 histQueueCount = stripe->queueCount; if (histQueueCount < 3) return 0;
    s32 sliceHistInterval = stripeData->sliceHistInterval;

    s32 numSliceHistory = std::min( sliceHistInterval +  (sliceHistInterval - 1) * numDivisions,
                                    histQueueCount    += (histQueueCount    - 1) * numDivisions );
    if (numSliceHistory < 3)
        return 0;

    u32 numDrawStripe = 0;

    f32 invRatio    = 1.0f / (f32)(numSliceHistory - 2);
    f32 invDivRatio = 1.0f / (f32)(numDivisions    + 1);
    f32 invTexRatio;

    if (stripeData->textureType == 1 && stripe->counter < sliceHistInterval)
        invTexRatio = 1.0f / (f32)((sliceHistInterval - 2) + (sliceHistInterval - 1) * numDivisions);
    else
        invTexRatio = invRatio;

    f32 alphaRange = stripeData->alphaEnd - stripeData->alphaStart;
    stripe->drawFirstVertex = firstVertex;

    for (s32 i = 0; i < numSliceHistory - 1; i++)
    {
        u32 idx = firstVertex + numDrawStripe;
        StripeVertexBuffer* buffer0 = &stripeVertexBuffer[idx + 0];
        StripeVertexBuffer* buffer1 = &stripeVertexBuffer[idx + 1];

        f32 ratio    = (f32)i * invRatio;
        f32 texRatio = (f32)i * invTexRatio;
        f32 divRatio = ratio  * invDivRatio;

        f32 v0 = divRatio * (f32)(histQueueCount - 2);
        s32 v1 = (s32)v0;

        s32 sliceHistIdx = stripe->queueRear - v1 - 1;
        s32 nextSliceHistIdx = sliceHistIdx - 1;

        if (sliceHistIdx < 0)
            sliceHistIdx += stripeData->numSliceHistory;

        if (nextSliceHistIdx < 0)
            nextSliceHistIdx += stripeData->numSliceHistory;

        s32 prevSliceHistIdx = sliceHistIdx + 1;
        if (prevSliceHistIdx >= stripeData->numSliceHistory)
            prevSliceHistIdx -= stripeData->numSliceHistory;

        s32 nextSliceHist2Idx = nextSliceHistIdx - 1;
        if (nextSliceHist2Idx < 0)
            nextSliceHist2Idx += stripeData->numSliceHistory;

        f32 delta = v0 - (f32)v1;

        u32 idx0 = prevSliceHistIdx;
        u32 idx1 = sliceHistIdx;
        u32 idx2 = nextSliceHistIdx;
        u32 idx3 = nextSliceHist2Idx;

        if (v1 == 0)
        {
            idx0 = sliceHistIdx;
            idx1 = nextSliceHistIdx;
        }

        if (v1 >= stripe->queueCount - 2)
        {
            idx2 = sliceHistIdx;
            idx3 = nextSliceHistIdx;
        }

        math::VEC3 diff0 = stripe->queue[idx0].pos - stripe->queue[idx1].pos;
        math::VEC3 diff1 = stripe->queue[idx2].pos - stripe->queue[idx1].pos;
        math::VEC3 diff2 = stripe->queue[idx1].pos - stripe->queue[idx2].pos;
        math::VEC3 diff3 = stripe->queue[idx3].pos - stripe->queue[idx2].pos;

        math::VEC3 startVel = (diff1 - diff0) * 0.5f;
        math::VEC3 endVel = (diff3 - diff2) * 0.5f;

        math::VEC3 pos;
        GetPositionOnCubic(&pos, stripe->queue[sliceHistIdx].pos, startVel, stripe->queue[nextSliceHistIdx].pos, endVel, delta);

        f32 alpha0 = (stripeData->alphaStart + alphaRange * ratio) * stripe->particle->alpha0 * stripe->particle->emitter->emitterSet->color.a * stripe->particle->emitter->fadeAlpha;
        f32 alpha1 = (stripeData->alphaStart + alphaRange * ratio) * stripe->particle->alpha1 * stripe->particle->emitter->emitterSet->color.a * stripe->particle->emitter->fadeAlpha;

        buffer0->pos.xyz() = pos;
        buffer0->pos.w     = alpha0 * stripe->particle->emitter->anim[14];
        buffer1->pos.xyz() = pos;
        buffer1->pos.w     = buffer0->pos.w;

        f32 invDelta = 1.0f - delta;

        buffer0->outer.xyz() =  stripe->queue[sliceHistIdx].outer * invDelta + stripe->queue[nextSliceHistIdx].outer * delta;
        buffer0->outer.w     =  stripe->queue[sliceHistIdx].scale;
        buffer1->outer.xyz() =  buffer0->outer.xyz();
        buffer1->outer.w     = -stripe->queue[sliceHistIdx].scale;

        buffer0->dir.xyz() = stripe->queue[sliceHistIdx].dir * invDelta + stripe->queue[nextSliceHistIdx].dir * delta;
        buffer0->dir.w     = alpha1 * stripe->particle->emitter->anim[14];
        buffer1->dir.xyz() = buffer0->dir.xyz();
        buffer1->dir.w     = buffer0->dir.w;

        buffer1->texCoord.x = stripe->texAnimParam[0].offset.x;
        buffer0->texCoord.y = stripe->texAnimParam[0].offset.y + texRatio * stripe->data->texAnimParam[0].uvScaleInit.y;
        buffer0->texCoord.x = stripe->texAnimParam[0].offset.x +            stripe->data->texAnimParam[0].uvScaleInit.x;
        buffer1->texCoord.y = buffer0->texCoord.y;

        buffer1->texCoord.z = stripe->texAnimParam[1].offset.x;
        buffer0->texCoord.w = stripe->texAnimParam[1].offset.y + texRatio * stripe->data->texAnimParam[1].uvScaleInit.y;
        buffer0->texCoord.z = stripe->texAnimParam[1].offset.x +            stripe->data->texAnimParam[1].uvScaleInit.x;
        buffer1->texCoord.w = buffer0->texCoord.w;

        numDrawStripe += 2;
    }

    stripe->numDraw = numDrawStripe;

    return numDrawStripe;
}

bool Renderer::MakeStripeAttributeBlock(const EmitterInstance* emitter, PtclInstance* ptcl)
{
    PtclStripe* stripe = ptcl->complexParam->stripe;
    if (stripe == NULL)
        return false;

    stripe->stripeUniformBlock = NULL;
    stripe->stripeUniformBlockCross = NULL;
    stripe->stripeVertexBuffer = NULL;

    if (stripe->data == NULL || ptcl->data == NULL)
        return false;

    const StripeData* stripeData = emitter->GetStripeData();
    u32 numDivisions = stripeData->numDivisions;

    u32 numVertex = stripe->queueCount;
    if (numVertex == 0)
        return false;

    numVertex += (stripe->queueCount - 1) * numDivisions;
    numVertex *= 2;

    math::VEC3 emitterSetColor = emitter->emitterSet->color.rgb();
    emitterSetColor.x *= emitter->data->colorScaleFactor;
    emitterSetColor.y *= emitter->data->colorScaleFactor;
    emitterSetColor.z *= emitter->data->colorScaleFactor;

    math::VEC3 emitterColor0;
    emitterColor0.x = emitterSetColor.x * emitter->anim[11];
    emitterColor0.y = emitterSetColor.y * emitter->anim[12];
    emitterColor0.z = emitterSetColor.z * emitter->anim[13];

    math::VEC3 emitterColor1;
    emitterColor1.x = emitterSetColor.x * emitter->anim[19];
    emitterColor1.y = emitterSetColor.y * emitter->anim[20];
    emitterColor1.z = emitterSetColor.z * emitter->anim[21];

    stripe->stripeVertexBuffer = static_cast<StripeVertexBuffer*>(AllocFromDoubleBuffer(sizeof(StripeVertexBuffer) * numVertex));
    if (stripe->stripeVertexBuffer == NULL)
        return false;

    if (numDivisions == 0)
        stripe->numDraw = MakeStripeAttributeBlockCore(stripe, stripe->stripeVertexBuffer, 0, stripeData);

    else
        stripe->numDraw = MakeStripeAttributeBlockCoreDivide(stripe, stripe->stripeVertexBuffer, 0, stripeData);

    stripeNumCalcVertex += stripe->numDraw;

    stripe->stripeUniformBlock = static_cast<StripeUniformBlock*>(AllocFromDoubleBuffer(sizeof(StripeUniformBlock)));
    if (stripe->stripeUniformBlock == NULL)
        return false;

    math::VEC3 ptclColor0 = ptcl->color0.rgb();
    ptclColor0.x *= emitterColor0.x;
    ptclColor0.y *= emitterColor0.y;
    ptclColor0.z *= emitterColor0.z;

    math::VEC3 ptclColor1 = ptcl->color1.rgb();
    ptclColor1.x *= emitterColor1.x;
    ptclColor1.y *= emitterColor1.y;
    ptclColor1.z *= emitterColor1.z;

    {
        StripeUniformBlock* uniformBlock = stripe->stripeUniformBlock;

        uniformBlock->stParam.x = 1.0f;
        uniformBlock->stParam.y = 0.0f;
        uniformBlock->stParam.z = emitter->data->cameraOffset;
        uniformBlock->stParam.w = ptcl->scale.x;

        uniformBlock->uvScrollAnim.x = stripe->texAnimParam[0].offset.x + stripe->texAnimParam[0].scroll.x;
        uniformBlock->uvScrollAnim.y = stripe->texAnimParam[0].offset.y - stripe->texAnimParam[0].scroll.y;
        uniformBlock->uvScrollAnim.z = stripe->texAnimParam[1].offset.x + stripe->texAnimParam[1].scroll.x;
        uniformBlock->uvScrollAnim.w = stripe->texAnimParam[1].offset.y - stripe->texAnimParam[1].scroll.y;

        uniformBlock->uvScaleRotateAnim0.x = stripe->texAnimParam[0].scale.x;
        uniformBlock->uvScaleRotateAnim0.y = stripe->texAnimParam[0].scale.y;
        uniformBlock->uvScaleRotateAnim0.z = stripe->texAnimParam[0].rotate;
        uniformBlock->uvScaleRotateAnim0.w = 0.0f;

        uniformBlock->uvScaleRotateAnim1.x = stripe->texAnimParam[1].scale.x;
        uniformBlock->uvScaleRotateAnim1.y = stripe->texAnimParam[1].scale.y;
        uniformBlock->uvScaleRotateAnim1.z = stripe->texAnimParam[1].rotate;
        uniformBlock->uvScaleRotateAnim1.w = 0.0f;

        uniformBlock->vtxColor0.xyz() = ptclColor0;
        uniformBlock->vtxColor0.w = ptcl->emitter->fadeAlpha;

        uniformBlock->vtxColor1.xyz() = ptclColor1;
        uniformBlock->vtxColor1.w = ptcl->emitter->fadeAlpha;

        uniformBlock->emitterMat = math::MTX44(ptcl->emitter->matrixSRT);

        GX2EndianSwap(uniformBlock, sizeof(StripeUniformBlock));
    }

    if (stripeData->crossType == 1)
    {
        stripe->stripeUniformBlockCross = static_cast<StripeUniformBlock*>(AllocFromDoubleBuffer(sizeof(StripeUniformBlock)));
        if (stripe->stripeUniformBlockCross == NULL)
            return true;

        StripeUniformBlock* uniformBlock = stripe->stripeUniformBlockCross;

        uniformBlock->stParam.x = 0.0f;
        uniformBlock->stParam.y = 1.0f;
        uniformBlock->stParam.z = emitter->data->cameraOffset;
        uniformBlock->stParam.w = ptcl->scale.x;

        uniformBlock->uvScrollAnim.x = stripe->texAnimParam[0].offset.x + stripe->texAnimParam[0].scroll.x;
        uniformBlock->uvScrollAnim.y = stripe->texAnimParam[0].offset.y - stripe->texAnimParam[0].scroll.y;
        uniformBlock->uvScrollAnim.z = stripe->texAnimParam[1].offset.x + stripe->texAnimParam[1].scroll.x;
        uniformBlock->uvScrollAnim.w = stripe->texAnimParam[1].offset.y - stripe->texAnimParam[1].scroll.y;

        uniformBlock->uvScaleRotateAnim0.x = stripe->texAnimParam[0].scale.x;
        uniformBlock->uvScaleRotateAnim0.y = stripe->texAnimParam[0].scale.y;
        uniformBlock->uvScaleRotateAnim0.z = stripe->texAnimParam[0].rotate;
        uniformBlock->uvScaleRotateAnim0.w = 0.0f;

        uniformBlock->uvScaleRotateAnim1.x = stripe->texAnimParam[1].scale.x;
        uniformBlock->uvScaleRotateAnim1.y = stripe->texAnimParam[1].scale.y;
        uniformBlock->uvScaleRotateAnim1.z = stripe->texAnimParam[1].rotate;
        uniformBlock->uvScaleRotateAnim1.w = 0.0f;

        uniformBlock->vtxColor0.xyz() = ptclColor0;
        uniformBlock->vtxColor0.w = ptcl->emitter->fadeAlpha;

        uniformBlock->vtxColor1.xyz() = ptclColor1;
        uniformBlock->vtxColor1.w = ptcl->emitter->fadeAlpha;

        uniformBlock->emitterMat = math::MTX44(ptcl->emitter->matrixSRT);

        GX2EndianSwap(uniformBlock, sizeof(StripeUniformBlock));
    }

    return true;
}

bool Renderer::SetupStripeDrawSetting(const EmitterInstance* emitter, void* argData)
{
    if (emitter->shader == NULL)
        return false;

    const SimpleEmitterData* data = emitter->data;

    ParticleShader* shader = emitter->shader[shaderType];
    if (shader == NULL)
        return false;

    shader->Bind();

    shader->vertexViewUniformBlock.BindUniformBlock(viewUniformBlock);
    shader->fragmentViewUniformBlock.BindUniformBlock(viewUniformBlock);
    shader->fragmentEmitterStaticUniformBlock.BindUniformBlock(emitter->emitterStaticUniformBlock);

    renderContext.SetupZBufATest(data->zBufATestType);
    renderContext.SetupBlendType(data->blendType);
    renderContext.SetupDisplaySideType(data->displaySideType);

    SetupTexture(shader, &data->textures[0], &data->textures[1], &data->textures[2]);

    CustomShaderRenderStateSetCallback callback = system->GetCustomShaderRenderStateSetCallback(static_cast<CustomShaderCallBackID>(data->shaderUserSetting));
    if (callback != NULL)
    {
        RenderStateSetArg arg = {
            .emitter = emitter,
            .renderer = this,
            .flushCache = true,
            .argData = argData,
        };
        callback(arg);
    }

    if (system->GetDrawPathRenderStateSetCallback(static_cast<DrawPathFlag>(1 << emitter->data->_bitForUnusedFlag)) != NULL)
    {
        RenderStateSetArg arg = {
            .emitter = emitter,
            .renderer = this,
            .flushCache = true,
            .argData = argData,
        };
        system->GetDrawPathRenderStateSetCallback(static_cast<DrawPathFlag>(1 << emitter->data->_bitForUnusedFlag))(arg);
    }

    return true;
}

void Renderer::EntryStripe(const EmitterInstance* emitter, void* argData)
{
    if (emitter->data->displayParent == 0)
        return;

    ParticleShader* shader = emitter->shader[shaderType];
    if (shader == NULL)
        return;

    const StripeData* stripeData = emitter->GetStripeData();

    currentParticleType = PtclType_Complex;

    if (emitter->data->vertexTransformMode == VertexTransformMode_Complex_Stripe)
    {
        EntryConnectionStripe(emitter, argData);
        numDrawEmitter++;
        return;
    }

    if (!SetupStripeDrawSetting(emitter, argData))
        return;

    u32 numDraw = 0;

    for (PtclInstance* ptcl = emitter->particleHead; ptcl != NULL; ptcl = ptcl->next)
    {
        PtclStripe* stripe = ptcl->complexParam->stripe;

        if (stripe == NULL
            || stripe->data == NULL
            || stripe->stripeVertexBuffer == NULL
            || stripe->stripeUniformBlock == NULL
            || stripe->numDraw < 4)
        {
            continue;
        }

        VertexBuffer::BindExtBuffer(0, sizeof(StripeVertexBuffer) * stripe->numDraw, 0, sizeof(StripeVertexBuffer), stripe->stripeVertexBuffer);

        if (stripe->stripeUniformBlock != NULL && stripe->numDraw != 0)
        {
            StripeUniformBlock* uniformBlock = stripe->stripeUniformBlock;
            shader->stripeUniformBlock.BindUniformBlock(uniformBlock);
            GX2DrawEx(GX2_PRIMITIVE_TRIANGLE_STRIP, stripe->numDraw, stripe->drawFirstVertex, 1);

            stripeNumDrawVertex += stripe->numDraw - stripe->drawFirstVertex;
            numDraw++;
        }

        if (stripe->stripeUniformBlockCross != NULL && stripe->numDraw != 0 && stripeData->type != 0)
        {
            StripeUniformBlock* uniformBlock = stripe->stripeUniformBlockCross;
            shader->stripeUniformBlock.BindUniformBlock(uniformBlock);
            GX2DrawEx(GX2_PRIMITIVE_TRIANGLE_STRIP, stripe->numDraw, stripe->drawFirstVertex, 1);

            stripeNumDrawVertex += stripe->numDraw - stripe->drawFirstVertex;
            numDraw++;
        }
    }

    numDrawParticle += numDraw;
    numDrawEmitter++;
}

} } // namespace nw::eft
