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

// This function was moved to eft_RendererConnectionStripe.cpp
bool Renderer::ConnectionStripeUvScaleCalc(f32& invTexRatio, f32& texRatioSub, const EmitterInstance* emitter, s32 numParticles, f32 invRatio, s32 connectionType)
{
    const ComplexEmitterData* cdata = reinterpret_cast<const ComplexEmitterData*>(emitter->particleHead->data);
    const StripeData* stripeData = reinterpret_cast<const StripeData*>((u32)cdata + cdata->stripeDataOffs);

    texRatioSub = 0.0f;

    if (!(stripeData->textureType == 1 && emitter->counter < emitter->data->ptclMaxLifespan))
    {
        invTexRatio = invRatio;
        return false;
    }

    s32 emissionDuration, numEmit;
    if (emitter->data->endFrame != 0x7FFFFFFF && (emitter->data->endFrame - emitter->data->startFrame) < emitter->data->ptclMaxLifespan)
    {
        emissionDuration = emitter->data->endFrame - emitter->data->startFrame + emitter->data->emitInterval;
        numEmit = (emissionDuration / (emitter->data->emitInterval + 1)) * (s32)emitter->data->emissionRate;
    }
    else
    {
        emissionDuration = emitter->data->ptclMaxLifespan;
        numEmit = (emissionDuration / (emitter->data->emitInterval + 1) + 1) * (s32)emitter->data->emissionRate;
    }

    if (connectionType != 0)
    {
        numEmit++;
        numParticles++;
    }

    invTexRatio = 1.0f / (f32)(numEmit - 1);
    texRatioSub = 1.0f - invTexRatio * (numParticles - 1);

    return true;

}

// This function was moved to eft_RendererConnectionStripe.cpp
u32 Renderer::MakeConnectionStripeAttributeBlockCore(EmitterInstance* emitter, s32 numParticles, PtclInstance* ptclLast, PtclInstance* ptclBeforeLast, s32 connectionType, StripeVertexBuffer* stripeVertexBuffer)
{
    if (numParticles < 2)
        return 0;

    const StripeData* stripeData = emitter->GetStripeData();

    bool edgeConnected = false;
    s32 numSliceHistory = numParticles;

    if (connectionType == 1 || connectionType == 2)
    {
        edgeConnected = true;
        numSliceHistory++;
    }

    u32 numDrawStripe = 0;

    f32 invRatio    = 1.0f / (f32)(numSliceHistory - 1);
    f32 invTexRatio, texRatioSub;
    ConnectionStripeUvScaleCalc(invTexRatio, texRatioSub, emitter, numParticles, invRatio, connectionType);

    f32 alphaRange = stripeData->alphaEnd - stripeData->alphaStart;

    PtclInstance* ptclFirst = emitter->particleHead;
    PtclInstance* ptcl = ptclFirst;
    PtclInstance* ptcl_next = ptclFirst->next;

    math::VEC3 currentSliceDir;
    math::VEC3 currPos, nextPos;

    for (s32 i = 0; i < numSliceHistory; i++)
    {
        u32 idx = 0 + numDrawStripe;
        StripeVertexBuffer* buffer0 = &stripeVertexBuffer[idx + 0];
        StripeVertexBuffer* buffer1 = &stripeVertexBuffer[idx + 1];

        f32 ratio    = (f32)i * invRatio;
        f32 texRatio = 1.0f - ((f32)i * invTexRatio) - texRatioSub;

        f32 v0 = ratio * (f32)(numSliceHistory - 1);
        s32 v1 = (s32)(v0 + 0.5f);

        f32 delta = v0 - (f32)v1;
        if (delta < 0.0f)
            delta = 0.0f;

        if (connectionType == 1 && i == 0)
        {
            currPos = ptclLast->worldPos;
            nextPos = ptcl->worldPos;
        }
        else if (connectionType == 1 && ptcl == ptclLast)
        {
            currPos = ptclLast->worldPos;
            nextPos = ptclFirst->worldPos;
        }
        else if (connectionType == 2 && i == 0)
        {
            currPos.x = emitter->matrixSRT.m[0][3];
            currPos.y = emitter->matrixSRT.m[1][3];
            currPos.z = emitter->matrixSRT.m[2][3];
            nextPos = ptcl->worldPos;
        }
        else if (ptcl_next != NULL)
        {
            nextPos = ptcl_next->worldPos;
            currPos = ptcl->worldPos;
        }
        else
        {
            math::VEC3 posDiff;
            math::VEC3::Subtract(&posDiff, &nextPos, &currPos);
            math::VEC3::Add(&nextPos, &nextPos, &posDiff);
            currPos = ptcl->worldPos;
        }

        math::VEC3 pos;
        math::VEC3::Subtract(&pos, &nextPos, &currPos);
        math::VEC3::Scale(&pos, &pos, delta);
        math::VEC3::Add(&pos, &currPos, &pos);

        math::VEC3 dir = nextPos - currPos;

        f32 alpha = (stripeData->alphaStart + alphaRange * ratio) * ptcl->alpha0;

        if (stripeData->dirInterpolation != 1.0f)
        {
            if (i == 0)
                currentSliceDir = dir;

            else
            {
                math::VEC3 diff;
                math::VEC3::Subtract(&diff, &dir, &currentSliceDir);
                math::VEC3::Scale(&diff, &diff, stripeData->dirInterpolation);
                math::VEC3::Add(&currentSliceDir, &currentSliceDir, &diff);
                if (currentSliceDir.Magnitude() > 0.0f)
                    currentSliceDir.Normalize();

                dir = currentSliceDir;
            }
        }

        buffer0->pos.xyz() = pos;
        buffer0->pos.w     = alpha * ptcl->emitter->anim[14] * ptcl->emitter->emitterSet->color.a;
        buffer1->pos.xyz() = pos;
        buffer1->pos.w     = buffer0->pos.w;

        buffer0->dir.xyz() = dir;
        buffer0->dir.w     = 0.0f;
        buffer1->dir.xyz() = dir;
        buffer1->dir.w     = 0.0f;

        if (stripeData->type == 0)
        {
            buffer0->outer.xyz() = eyeVec;
            buffer1->outer.xyz() = eyeVec;

            buffer1->outer.w     = ptcl->scale.x * emitter->emitterSet->ptclEffectiveScale.x;
            buffer0->outer.w     = -buffer1->outer.w;
        }
        else if (stripeData->type == 2)
        {
            math::VEC3 outer;
            if (emitter->ptclFollowType == PtclFollowType_SRT)
            {
                outer.x = emitter->matrixSRT.m[0][1];
                outer.y = emitter->matrixSRT.m[1][1];
                outer.z = emitter->matrixSRT.m[2][1];
            }
            else
            {
                outer.x = ptcl->matrixSRT.m[0][1];
                outer.y = ptcl->matrixSRT.m[1][1];
                outer.z = ptcl->matrixSRT.m[2][1];
            }

            buffer0->outer.xyz() = outer;
            buffer1->outer.xyz() = outer;

            buffer1->outer.w     = ptcl->scale.x * emitter->emitterSet->ptclEffectiveScale.x;
            buffer0->outer.w     = -buffer1->outer.w;
        }
        else if (stripeData->type == 1)
        {
            math::VEC3 outer;
            if (emitter->ptclFollowType == PtclFollowType_SRT)
            {
                outer.x = emitter->matrixSRT.m[0][1];
                outer.y = emitter->matrixSRT.m[1][1];
                outer.z = emitter->matrixSRT.m[2][1];
            }
            else
            {
                outer.x = ptcl->matrixSRT.m[0][1];
                outer.y = ptcl->matrixSRT.m[1][1];
                outer.z = ptcl->matrixSRT.m[2][1];
            }

            math::VEC3::CrossProduct(&outer, &outer, &dir);
            if (outer.Magnitude() > 0.0f)
                outer.Normalize();

            buffer0->outer.xyz() = outer;
            buffer1->outer.xyz() = outer;

            buffer1->outer.w     = ptcl->scale.x * emitter->emitterSet->ptclEffectiveScale.x;
            buffer0->outer.w     = -buffer1->outer.w;
        }

        buffer0->texCoord.x = emitter->data->texAnimParam[0].uvScaleInit.x;
        buffer0->texCoord.y = 1.0f - texRatio * emitter->data->texAnimParam[0].uvScaleInit.y;
        buffer1->texCoord.x = 0.0f;
        buffer1->texCoord.y = 1.0f - texRatio * emitter->data->texAnimParam[0].uvScaleInit.y;

        buffer0->texCoord.z = emitter->data->texAnimParam[1].uvScaleInit.x;
        buffer0->texCoord.w = 1.0f - texRatio * emitter->data->texAnimParam[1].uvScaleInit.y;
        buffer1->texCoord.z = 0.0f;
        buffer1->texCoord.w = 1.0f - texRatio * emitter->data->texAnimParam[1].uvScaleInit.y;

        numDrawStripe += 2;

        if (!(edgeConnected && i == 0) && ptcl_next != NULL)
        {
            ptcl = ptcl_next;
            ptcl_next = ptcl->next;
        }
    }

    stripeNumCalcVertex += numDrawStripe;
    return numDrawStripe;
}

// This function was moved to eft_RendererConnectionStripe.cpp
u32 Renderer::MakeConnectionStripeAttributeBlockCoreDivide(EmitterInstance* emitter, s32 numParticles, PtclInstance* ptclLast, PtclInstance* ptclBeforeLast, s32 connectionType, StripeVertexBuffer* stripeVertexBuffer)
{
    const ComplexEmitterData* cdata = reinterpret_cast<const ComplexEmitterData*>(emitter->particleHead->data);
    const StripeData* stripeData = reinterpret_cast<const StripeData*>((u32)cdata + cdata->stripeDataOffs);

    s32 numDivisions = stripeData->numDivisions;
    s32 numVertex = numParticles + (numParticles - 1) * numDivisions;

    if (numParticles < 2)
        return 0;

    s32 numSliceHistory = numParticles;
    if (connectionType == 1 || connectionType == 2)
    {
        numSliceHistory++;
        numVertex += numDivisions;
    }

    u32 numDrawStripe = 0;

    f32 invRatio    = 1.0f / (f32)(numVertex - 1);
    f32 invTexRatio, texRatioSub;
    if (ConnectionStripeUvScaleCalc(invTexRatio, texRatioSub, emitter, numParticles, invRatio, connectionType))
        invTexRatio /= numDivisions + 1;

    f32 alphaRange = stripeData->alphaEnd - stripeData->alphaStart;

    PtclInstance* ptclFirst = emitter->particleHead;
    PtclInstance* ptcl_prev = ptclFirst->prev;
    PtclInstance* ptcl = ptclFirst;
    PtclInstance* ptcl_next = ptclFirst->next;
    PtclInstance* ptcl_next2 = NULL;
    if (ptcl_next != NULL)
        ptcl_next2 = ptcl_next->next;

    f32 invDivRatioStep = 1.0f / (f32)(numDivisions + 1);
    s32 currentSliceIdx = 0;

    math::VEC3 dir, dirInit, currentSliceDir;
    math::VEC3 currPos, nextPos, prevPos, next2Pos;
    math::VEC3 vtxPos, prevVtxPos;

    for (s32 i = 0; i < numSliceHistory; i++)
    {
        if (connectionType == 1 && i == 0)
        {
            currPos = ptclLast->worldPos;
            nextPos = ptcl->worldPos;

            ptcl_prev = ptclBeforeLast;
            ptcl = ptclLast;
            ptcl_next = ptclFirst;
            ptcl_next2 = ptclFirst->next;
        }
        else if (connectionType == 1 && ptcl == ptclLast)
        {
            currPos = ptclLast->worldPos;
            nextPos = ptclFirst->worldPos;
        }
        else if (connectionType == 2 && i == 0)
        {
            currPos.x = emitter->matrixSRT.m[0][3];
            currPos.y = emitter->matrixSRT.m[1][3];
            currPos.z = emitter->matrixSRT.m[2][3];
            nextPos = ptcl->worldPos;

            ptcl_prev = NULL;
            ptcl = ptclFirst;
            ptcl_next = ptclFirst;
            ptcl_next2 = ptclFirst->next;
        }
        else if (ptcl_next != NULL)
        {
            nextPos = ptcl_next->worldPos;
            currPos = ptcl->worldPos;
        }
        else
        {
            math::VEC3 posDiff;
            math::VEC3::Subtract(&posDiff, &nextPos, &currPos);
            math::VEC3::Add(&nextPos, &nextPos, &posDiff);
            currPos = ptcl->worldPos;
        }

        if (ptcl_prev != NULL)
            prevPos = ptcl_prev->worldPos;

        else
        {
            math::VEC3 posDiff;
            math::VEC3::Subtract(&posDiff, &currPos, &nextPos);
            math::VEC3::Add(&prevPos, &currPos, &posDiff);
        }

        if (ptcl_next2 != NULL)
            next2Pos = ptcl_next2->worldPos;

        else if (connectionType == 1 && i == numSliceHistory - 2)
            next2Pos = ptclFirst->worldPos;

        else
        {
            math::VEC3 posDiff;
            math::VEC3::Subtract(&posDiff, &nextPos, &currPos);
            math::VEC3::Add(&next2Pos, &nextPos, &posDiff);
        }

        math::VEC3 diff0, diff1, diff2, diff3, startVel, endVel;

        math::VEC3::Subtract(&diff0, &prevPos, &currPos);
        math::VEC3::Subtract(&diff1, &nextPos, &currPos);

        math::VEC3::Subtract(&startVel, &diff1, &diff0);
        math::VEC3::Scale(&startVel, &startVel, 0.5f);

        math::VEC3::Subtract(&diff2, &currPos, &nextPos);
        math::VEC3::Subtract(&diff3, &next2Pos, &nextPos);

        math::VEC3::Subtract(&endVel, &diff3, &diff2);
        math::VEC3::Scale(&endVel, &endVel, 0.5f);

        bool notCubic = diff2.Magnitude() < ptcl->scale.x * 0.25f;

        f32 invDivRatio = 0.0f;

        for (s32 j = 0; j < numDivisions + 1; j++, invDivRatio += invDivRatioStep)
        {
            f32 divRatio = 1.0f - invDivRatio;
            if (i == numSliceHistory - 1 && j != 0)
                break;

            u32 idx = 0 + numDrawStripe;
            StripeVertexBuffer* buffer0 = &stripeVertexBuffer[idx + 0];
            StripeVertexBuffer* buffer1 = &stripeVertexBuffer[idx + 1];

            prevVtxPos = vtxPos;

            if (notCubic)
            {
                math::VEC3 pos0, pos1;
                math::VEC3::Scale(&pos0, &currPos, divRatio);
                math::VEC3::Scale(&pos1, &nextPos, invDivRatio);
                math::VEC3::Add(&vtxPos, &pos0, &pos1);
            }
            else
            {
                GetPositionOnCubic(&vtxPos, currPos, startVel, nextPos, endVel, invDivRatio);
            }

            if (i == 0 && j == 0)
                dirInit = (dir = startVel);
            else if (connectionType == 1 && i == numSliceHistory - 1)
                dir = dirInit;
            else
                math::VEC3::Subtract(&dir, &vtxPos, &prevVtxPos);

            if (dir.Magnitude() > 0.0f)
                dir.Normalize();

            if (stripeData->dirInterpolation != 1.0f)
            {
                if (i == 0)
                    currentSliceDir = dir;

                else
                {
                    math::VEC3 diff;
                    math::VEC3::Subtract(&diff, &dir, &currentSliceDir);
                    math::VEC3::Scale(&diff, &diff, stripeData->dirInterpolation);
                    math::VEC3::Add(&currentSliceDir, &currentSliceDir, &diff);
                    if (currentSliceDir.Magnitude() > 0.0f)
                        currentSliceDir.Normalize();

                    dir = currentSliceDir;
                }
            }

            f32 ratio    = (f32)currentSliceIdx * invRatio;
            f32 texRatio = 1.0f - ((f32)currentSliceIdx * invTexRatio) - texRatioSub;

            f32 alpha = (stripeData->alphaStart + alphaRange * ratio) * ptcl->alpha0;

            buffer0->pos.xyz() = vtxPos;
            buffer0->pos.w     = alpha * ptcl->emitter->anim[14] * ptcl->emitter->emitterSet->color.a;
            buffer1->pos.xyz() = vtxPos;
            buffer1->pos.w     = buffer0->pos.w;

            buffer0->dir.xyz() = dir;
            buffer0->dir.w     = 0.0f;
            buffer1->dir.xyz() = dir;
            buffer1->dir.w     = 0.0f;

            PtclInstance* _ptcl = ptcl_next;
            if (_ptcl == NULL)
                _ptcl = ptcl;

            if (stripeData->type == 0)
            {
                buffer0->outer.xyz() = eyeVec;
                buffer1->outer.xyz() = eyeVec;

                buffer1->outer.w     = ptcl->scale.x * emitter->emitterSet->ptclEffectiveScale.x * divRatio + _ptcl->scale.x * emitter->emitterSet->ptclEffectiveScale.x * invDivRatio;
                buffer0->outer.w     = -buffer1->outer.w;
            }
            else if (stripeData->type == 2)
            {
                math::VEC3 outer;
                if (emitter->ptclFollowType == PtclFollowType_SRT)
                {
                    outer.x = emitter->matrixSRT.m[0][1];
                    outer.y = emitter->matrixSRT.m[1][1];
                    outer.z = emitter->matrixSRT.m[2][1];
                }
                else
                {
                    outer.x = ptcl->matrixSRT.m[0][1];
                    outer.y = ptcl->matrixSRT.m[1][1];
                    outer.z = ptcl->matrixSRT.m[2][1];
                }

                buffer0->outer.xyz() = outer;
                buffer1->outer.xyz() = outer;

                buffer1->outer.w     = ptcl->scale.x * emitter->emitterSet->ptclEffectiveScale.x * divRatio + _ptcl->scale.x * emitter->emitterSet->ptclEffectiveScale.x * invDivRatio;
                buffer0->outer.w     = -buffer1->outer.w;
            }
            else if (stripeData->type == 1)
            {
                math::VEC3 outer;
                if (emitter->ptclFollowType == PtclFollowType_SRT)
                {
                    outer.x = emitter->matrixSRT.m[0][1];
                    outer.y = emitter->matrixSRT.m[1][1];
                    outer.z = emitter->matrixSRT.m[2][1];
                }
                else
                {
                    outer.x = ptcl->matrixSRT.m[0][1];
                    outer.y = ptcl->matrixSRT.m[1][1];
                    outer.z = ptcl->matrixSRT.m[2][1];
                }

                math::VEC3::CrossProduct(&outer, &outer, &dir);
                if (outer.Magnitude() > 0.0f)
                    outer.Normalize();

                buffer0->outer.xyz() = outer;
                buffer1->outer.xyz() = outer;

                buffer1->outer.w     = ptcl->scale.x * emitter->emitterSet->ptclEffectiveScale.x * divRatio + _ptcl->scale.x * emitter->emitterSet->ptclEffectiveScale.x * invDivRatio;
                buffer0->outer.w     = -buffer1->outer.w;
            }

            buffer0->texCoord.x = emitter->data->texAnimParam[0].uvScaleInit.x;
            buffer0->texCoord.y = 1.0f - texRatio * emitter->data->texAnimParam[0].uvScaleInit.y;
            buffer1->texCoord.x = 0.0f;
            buffer1->texCoord.y = 1.0f - texRatio * emitter->data->texAnimParam[0].uvScaleInit.y;

            buffer0->texCoord.z = emitter->data->texAnimParam[1].uvScaleInit.x;
            buffer0->texCoord.w = 1.0f - texRatio * emitter->data->texAnimParam[1].uvScaleInit.y;
            buffer1->texCoord.z = 0.0f;
            buffer1->texCoord.w = 1.0f - texRatio * emitter->data->texAnimParam[1].uvScaleInit.y;

            numDrawStripe += 2;
            currentSliceIdx++;
        }

        ptcl_prev = ptcl;
        ptcl = ptcl_next;
        ptcl_next = ptcl_next2;

        if (ptcl_next != NULL)
            ptcl_next2 = ptcl_next->next;

        if (connectionType == 1 && ptcl_next2 == NULL)
            ptcl_next2 = ptclFirst;
    }

    stripeNumCalcVertex += numDrawStripe;
    return numDrawStripe;
}

// This function was moved to eft_RendererConnectionStripe.cpp
StripeVertexBuffer* Renderer::MakeConnectionStripeAttributeBlock(EmitterInstance* emitter)
{
    u32 numParticles = emitter->numParticles;
    if (numParticles == 0)
        return NULL;

    PtclInstance* ptcl = emitter->particleHead;
    if (ptcl == NULL)
        return NULL;

    const StripeData* stripeData = emitter->GetStripeData();

    u32 numDivisions = stripeData->numDivisions;
    u32 connectionType = stripeData->connectionType;

    PtclInstance* ptclLast = emitter->particleTail;
    PtclInstance* ptclBeforeLast = ptclLast->prev;

    u32 numVertex = numParticles * 2 + ((numParticles - 1) * 2) * numDivisions;
    if (connectionType == 1 || connectionType == 2)
        numVertex += 2 + numDivisions * 2;

    if (numVertex == 0)
        return NULL;

    emitter->stripeVertexBuffer = static_cast<StripeVertexBuffer*>(AllocFromDoubleBuffer(sizeof(StripeVertexBuffer) * numVertex));
    if (emitter->stripeVertexBuffer == NULL)
        return NULL;

    emitter->connectionStripeUniformBlock = static_cast<StripeUniformBlock*>(AllocFromDoubleBuffer(sizeof(StripeUniformBlock)));
    if (emitter->connectionStripeUniformBlock == NULL)
        return NULL;

    if (stripeData->crossType == 1)
    {
        emitter->connectionStripeUniformBlockCross = static_cast<StripeUniformBlock*>(AllocFromDoubleBuffer(sizeof(StripeUniformBlock)));
        if (emitter->connectionStripeUniformBlockCross == NULL)
            return NULL;
    }

    if (numDivisions == 0)
        emitter->numDrawStripe = MakeConnectionStripeAttributeBlockCore(emitter, numParticles, ptclLast, ptclBeforeLast, connectionType, emitter->stripeVertexBuffer);

    else
        emitter->numDrawStripe = MakeConnectionStripeAttributeBlockCoreDivide(emitter, numParticles, ptclLast, ptclBeforeLast, connectionType, emitter->stripeVertexBuffer);

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

    math::VEC3 ptclColor0 = ptcl->color0.rgb();
    ptclColor0.x *= emitterColor0.x;
    ptclColor0.y *= emitterColor0.y;
    ptclColor0.z *= emitterColor0.z;

    math::VEC3 ptclColor1 = ptcl->color1.rgb();
    ptclColor1.x *= emitterColor1.x;
    ptclColor1.y *= emitterColor1.y;
    ptclColor1.z *= emitterColor1.z;

    {
        StripeUniformBlock* uniformBlock = emitter->connectionStripeUniformBlock;

        uniformBlock->stParam.x = 1.0f;
        uniformBlock->stParam.y = 0.0f;
        uniformBlock->stParam.z = emitter->data->cameraOffset;
        uniformBlock->stParam.w = 1.0f;

        uniformBlock->uvScrollAnim.x = emitter->data->texAnimParam[0].texInitScroll.x + emitter->counter * emitter->data->texAnimParam[0].texIncScroll.x;
        uniformBlock->uvScrollAnim.y = emitter->data->texAnimParam[0].texInitScroll.y - emitter->counter * emitter->data->texAnimParam[0].texIncScroll.y;
        uniformBlock->uvScrollAnim.z = emitter->data->texAnimParam[1].texInitScroll.x + emitter->counter * emitter->data->texAnimParam[1].texIncScroll.x;
        uniformBlock->uvScrollAnim.w = emitter->data->texAnimParam[1].texInitScroll.y - emitter->counter * emitter->data->texAnimParam[1].texIncScroll.y;

        uniformBlock->uvScaleRotateAnim0.x = emitter->data->texAnimParam[0].texInitScale.x + emitter->counter * emitter->data->texAnimParam[0].texIncScale.x;
        uniformBlock->uvScaleRotateAnim0.y = emitter->data->texAnimParam[0].texInitScale.y + emitter->counter * emitter->data->texAnimParam[0].texIncScale.y;
        uniformBlock->uvScaleRotateAnim0.z = emitter->counter * emitter->data->texAnimParam[0].texIncRotate;
        uniformBlock->uvScaleRotateAnim0.w = 0.0f;

        uniformBlock->uvScaleRotateAnim1.x = emitter->data->texAnimParam[1].texInitScale.x + emitter->counter * emitter->data->texAnimParam[1].texIncScale.x;
        uniformBlock->uvScaleRotateAnim1.y = emitter->data->texAnimParam[1].texInitScale.y + emitter->counter * emitter->data->texAnimParam[1].texIncScale.y;
        uniformBlock->uvScaleRotateAnim1.z = emitter->counter * emitter->data->texAnimParam[1].texIncRotate;
        uniformBlock->uvScaleRotateAnim1.w = 0.0f;

        uniformBlock->vtxColor0.xyz() = ptclColor0;
        uniformBlock->vtxColor0.w = emitter->fadeAlpha;

        uniformBlock->vtxColor1.xyz() = ptclColor1;
        uniformBlock->vtxColor1.w = emitter->fadeAlpha;

        uniformBlock->emitterMat = math::MTX44(emitter->matrixSRT);

        GX2EndianSwap(uniformBlock, sizeof(StripeUniformBlock));
    }

    if (stripeData->crossType == 1)
    {
        StripeUniformBlock* uniformBlock = emitter->connectionStripeUniformBlockCross;

        uniformBlock->stParam.x = 0.0f;
        uniformBlock->stParam.y = 1.0f;
        uniformBlock->stParam.z = emitter->data->cameraOffset;
        uniformBlock->stParam.w = 1.0f;

        uniformBlock->uvScrollAnim.x = emitter->data->texAnimParam[0].texInitScroll.x + emitter->counter * emitter->data->texAnimParam[0].texIncScroll.x;
        uniformBlock->uvScrollAnim.y = emitter->data->texAnimParam[0].texInitScroll.y - emitter->counter * emitter->data->texAnimParam[0].texIncScroll.y;
        uniformBlock->uvScrollAnim.z = emitter->data->texAnimParam[1].texInitScroll.x + emitter->counter * emitter->data->texAnimParam[1].texIncScroll.x;
        uniformBlock->uvScrollAnim.w = emitter->data->texAnimParam[1].texInitScroll.y - emitter->counter * emitter->data->texAnimParam[1].texIncScroll.y;

        uniformBlock->uvScaleRotateAnim0.x = emitter->data->texAnimParam[0].texInitScale.x + emitter->counter * emitter->data->texAnimParam[0].texIncScale.x;
        uniformBlock->uvScaleRotateAnim0.y = emitter->data->texAnimParam[0].texInitScale.y + emitter->counter * emitter->data->texAnimParam[0].texIncScale.y;
        uniformBlock->uvScaleRotateAnim0.z = emitter->counter * emitter->data->texAnimParam[0].texIncRotate;
        uniformBlock->uvScaleRotateAnim0.w = 0.0f;

        uniformBlock->uvScaleRotateAnim1.x = emitter->data->texAnimParam[1].texInitScale.x + emitter->counter * emitter->data->texAnimParam[1].texIncScale.x;
        uniformBlock->uvScaleRotateAnim1.y = emitter->data->texAnimParam[1].texInitScale.y + emitter->counter * emitter->data->texAnimParam[1].texIncScale.y;
        uniformBlock->uvScaleRotateAnim1.z = emitter->counter * emitter->data->texAnimParam[1].texIncRotate;
        uniformBlock->uvScaleRotateAnim1.w = 0.0f;

        uniformBlock->vtxColor0.xyz() = ptclColor0;
        uniformBlock->vtxColor0.w = emitter->fadeAlpha;

        uniformBlock->vtxColor1.xyz() = ptclColor1;
        uniformBlock->vtxColor1.w = emitter->fadeAlpha;

        uniformBlock->emitterMat = math::MTX44(emitter->matrixSRT);

        GX2EndianSwap(uniformBlock, sizeof(StripeUniformBlock));
    }
    else
    {
        emitter->connectionStripeUniformBlockCross = NULL;
    }

    return emitter->stripeVertexBuffer;
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

// This function was moved to eft_RendererConnectionStripe.cpp
void Renderer::EntryConnectionStripe(const EmitterInstance* emitter, void* argData)
{
    ParticleShader* shader = emitter->shader[shaderType];
    if (shader == NULL)
        return;

    StripeVertexBuffer* stripeVertexBuffer = emitter->stripeVertexBuffer;
    if (stripeVertexBuffer == NULL || emitter->numDrawStripe < 4)
        return;

    currentParticleType = PtclType_Complex;

    if (!SetupStripeDrawSetting(emitter, argData))
        return;

    VertexBuffer::BindExtBuffer(0, sizeof(StripeVertexBuffer) * emitter->numDrawStripe, 0, sizeof(StripeVertexBuffer), stripeVertexBuffer);

    const StripeData* stripeData = emitter->GetStripeData();

    {
        StripeUniformBlock* uniformBlock = emitter->connectionStripeUniformBlock;
        shader->stripeUniformBlock.BindUniformBlock(uniformBlock);
        GX2Draw(GX2_PRIMITIVE_TRIANGLE_STRIP, emitter->numDrawStripe);

        stripeNumDrawVertex += emitter->numDrawStripe;
    }

    if (stripeData->crossType == 1 && emitter->connectionStripeUniformBlockCross != NULL && stripeData->type != 0)
    {
        StripeUniformBlock* uniformBlock = emitter->connectionStripeUniformBlockCross;
        shader->stripeUniformBlock.BindUniformBlock(uniformBlock);
        GX2Draw(GX2_PRIMITIVE_TRIANGLE_STRIP, emitter->numDrawStripe);

        stripeNumDrawVertex += emitter->numDrawStripe;
    }
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
