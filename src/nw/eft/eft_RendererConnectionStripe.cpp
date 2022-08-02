#include <nw/eft/eft_Emitter.h>
#include <nw/eft/eft_EmitterSet.h>
#include <nw/eft/eft_Particle.h>
#include <nw/eft/eft_Renderer.h>
#include <nw/eft/eft_Shader.h>
#include <nw/eft/eft_System.h>
#include <nw/eft/eft_UniformBlock.h>

#include <algorithm>

namespace nw { namespace eft {

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

} } // namespace nw::eft
