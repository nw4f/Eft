#include <nw/eft/eft_Emitter.h>
#include <nw/eft/eft_EmitterSet.h>
#include <nw/eft/eft_Particle.h>
#include <nw/eft/eft_Renderer.h>
#include <nw/eft/eft_Shader.h>
#include <nw/eft/eft_System.h>
#include <nw/eft/eft_UniformBlock.h>

#include <algorithm>

namespace nw { namespace eft {

bool Renderer::ConnectionStripeUvScaleCalc(f32& frateScaleUv, f32& uvStartOfs, const EmitterInstance* emitter, s32 numLoop, f32 frateScale, s32 tailType)
{
    const ComplexEmitterData* res  = reinterpret_cast<const ComplexEmitterData*>(emitter->ptclHead->res);
    const StripeData        * sres = reinterpret_cast<const StripeData*>((u32)res + res->stripeDataOffset);

    bool ret = false;
    uvStartOfs = 0.0f;

    if (sres->stripeTexCoordOpt == EFT_STRIPE_TEXCOORD_OPTION_TYPE_DIVISION &&
        emitter->cnt < emitter->res->ptclLife)
    {
        s32 maxLife, maxNum;
        if (emitter->res->emitEndFrame != EFT_INFINIT_LIFE && (emitter->res->emitEndFrame - emitter->res->emitStartFrame) < emitter->res->ptclLife)
        {
            maxLife = emitter->res->emitEndFrame - emitter->res->emitStartFrame + emitter->res->emitStep;
            maxNum = maxLife / (emitter->res->emitStep + 1);
        }
        else
        {
            maxLife = emitter->res->ptclLife;
            maxNum = maxLife / (emitter->res->emitStep + 1) + 1;
        }

        maxNum *= (s32)(emitter->res->emitRate);

        if (tailType != 0)
        {
            maxNum++;
            numLoop++;
        }

        frateScaleUv = 1.0f / (f32)(maxNum - 1);
        uvStartOfs = 1.0f - frateScaleUv * (numLoop - 1);
        ret = true;
    }
    else
    {
        frateScaleUv = frateScale;
    }
    return ret;
}

s32 Renderer::MakeConnectionStripeAttributeBlockCore(EmitterInstance* emitter, s32 numPtcl, PtclInstance* pTailPtcl, PtclInstance* pTail2ndPtcl, s32 tailType, StripeVertexBuffer* stripeVertex)
{
    if (numPtcl < 2)
        return 0;

    const StripeData* stripeData = emitter->GetStripeData();

    s32 numLoop = numPtcl;
    bool bTailConnect = false;

    if (tailType == 1 || tailType == 2)
    {
        bTailConnect = true;
        numLoop++;
    }

    s32 numDrawVertex = 0;

    f32 frateScale = 1.0f / (f32)(numLoop - 1);

    f32 frateScaleUv, uvStartOfs;
    ConnectionStripeUvScaleCalc(frateScaleUv, uvStartOfs, emitter, numPtcl, frateScale, tailType);

    f32 arate = stripeData->stripeEndAlpha - stripeData->stripeStartAlpha;

    nw::math::VEC4* pos0   = NULL;
    nw::math::VEC4* pos1   = NULL;
    nw::math::VEC4* outer0 = NULL;
    nw::math::VEC4* outer1 = NULL;
    nw::math::VEC4* tex0   = NULL;
    nw::math::VEC4* tex1   = NULL;
    nw::math::VEC4* dir0   = NULL;
    nw::math::VEC4* dir1   = NULL;

    PtclInstance* pPtcl     = emitter->ptclHead;
    PtclInstance* pPtclTop  = pPtcl;
    PtclInstance* pPtclNext = pPtcl->next;
    nw::math::VEC3 curVec, nowPos, nextPos;
    nw::math::VEC3 interpolateNextDir;

    for (s32 i = 0; i < numLoop; i++)
    {
        pos0   = &stripeVertex[numDrawVertex    ].pos;
        pos1   = &stripeVertex[numDrawVertex + 1].pos;
        outer0 = &stripeVertex[numDrawVertex    ].outer;
        outer1 = &stripeVertex[numDrawVertex + 1].outer;
        tex0   = &stripeVertex[numDrawVertex    ].tex;
        tex1   = &stripeVertex[numDrawVertex + 1].tex;
        dir0   = &stripeVertex[numDrawVertex    ].dir;
        dir1   = &stripeVertex[numDrawVertex + 1].dir;

        f32 frate = (f32)i * frateScale;
        f32 frateUv = 1.0f - ((f32)i * frateScaleUv);
        frateUv -= uvStartOfs;

        f32 t = frate * (numLoop - 1);
        s32 iT = (s32)(t + 0.5f);

        f32 posRate = t - iT;
        if (posRate < 0.0f)
            posRate = 0.0f;

        nw::math::VEC3 dir;

        if (tailType == 1 && i == 0)
        {
            nowPos  = pTailPtcl->worldPos;
            nextPos = pPtcl->worldPos;
        }
        else if (tailType == 1 && pPtcl == pTailPtcl)
        {
            nowPos  = pTailPtcl->worldPos;
            nextPos = pPtclTop->worldPos;
        }
        else if (tailType == 2 && i == 0)
        {
            nowPos.x = emitter->emitterSRT.m[0][3];
            nowPos.y = emitter->emitterSRT.m[1][3];
            nowPos.z = emitter->emitterSRT.m[2][3];
            nextPos = pPtcl->worldPos;
        }
        else
        {
            if (pPtclNext != NULL)
            {
                nextPos = pPtclNext->worldPos;
                nowPos  = pPtcl->worldPos;
            }
            else
            {
                nextPos += nextPos - nowPos;
                nowPos  = pPtcl->worldPos;
            }
        }

        curVec = nowPos + (nextPos - nowPos) * posRate;
        dir.Set(nextPos.x - nowPos.x, nextPos.y - nowPos.y, nextPos.z - nowPos.z);

        f32 alpha = (stripeData->stripeStartAlpha + arate * frate) * pPtcl->alpha0;

        if (stripeData->stripeDirInterpolate != 1.0f)
        {
            if (i == 0)
                interpolateNextDir = dir;

            else
            {
                interpolateNextDir += (dir - interpolateNextDir) * stripeData->stripeDirInterpolate;
                if (interpolateNextDir.Length() > 0.0f)
                    interpolateNextDir.Normalize();

                dir = interpolateNextDir;
            }
        }

        *pos0 = nw::math::VEC4(curVec);
        pos0->w = alpha * pPtcl->emitter->emitterAnimValue[EFT_ANIM_ALPHA] * pPtcl->emitter->emitterSet->mColor.a;
        *pos1 = nw::math::VEC4(curVec);
        pos1->w = pos0->w;

        *dir0 = nw::math::VEC4(dir);
        *dir1 = nw::math::VEC4(dir);

        if (stripeData->stripeType == EFT_STRIPE_TYPE_BILLBOARD)
        {
            *outer0 = nw::math::VEC4(mEyeVec);
            *outer1 = nw::math::VEC4(mEyeVec);

            outer1->w   = pPtcl->scale.x * emitter->emitterSet->mParticlScaleForCalc.x;
            outer0->w   = -outer1->w;
        }
        else if (stripeData->stripeType == EFT_STRIPE_TYPE_EMITTER_UP_DOWN)
        {
            f32 upX, upY, upZ;
            if (emitter->followType == EFT_FOLLOW_TYPE_ALL)
            {
                upX = emitter->emitterSRT.m[0][1];
                upY = emitter->emitterSRT.m[1][1];
                upZ = emitter->emitterSRT.m[2][1];
            }
            else
            {
                upX = pPtcl->emitterSRT.m[0][1];
                upY = pPtcl->emitterSRT.m[1][1];
                upZ = pPtcl->emitterSRT.m[2][1];
            }

            outer0->x = upX;
            outer0->y = upY;
            outer0->z = upZ;
            outer1->x = upX;
            outer1->y = upY;
            outer1->z = upZ;

            outer1->w = pPtcl->scale.x * emitter->emitterSet->mParticlScaleForCalc.x;
            outer0->w = -outer1->w;
        }
        else if (stripeData->stripeType == EFT_STRIPE_TYPE_EMITTER_MATRIX)
        {
            nw::math::VEC3 basis;
            if (emitter->followType == EFT_FOLLOW_TYPE_ALL)
            {
                basis.x = emitter->emitterSRT.m[0][1];
                basis.y = emitter->emitterSRT.m[1][1];
                basis.z = emitter->emitterSRT.m[2][1];
            }
            else
            {
                basis.x = pPtcl->emitterSRT.m[0][1];
                basis.y = pPtcl->emitterSRT.m[1][1];
                basis.z = pPtcl->emitterSRT.m[2][1];
            }

            nw::math::VEC3 outer;
            outer.SetCross(basis, dir);
            if (outer.Length() > 0.0f)
                outer.Normalize();

            outer0->x = outer.x;
            outer0->y = outer.y;
            outer0->z = outer.z;
            outer1->x = outer.x;
            outer1->y = outer.y;
            outer1->z = outer.z;

            outer1->w = pPtcl->scale.x * emitter->emitterSet->mParticlScaleForCalc.x;
            outer0->w = -outer1->w;
        }

        tex0->x = emitter->res->textureData[EFT_TEXTURE_SLOT_0].texUScale;
        tex0->y = 1.0f - frateUv * emitter->res->textureData[EFT_TEXTURE_SLOT_0].texVScale;
        tex1->x = 0.0f;
        tex1->y = 1.0f - frateUv * emitter->res->textureData[EFT_TEXTURE_SLOT_0].texVScale;

        tex0->z = emitter->res->textureData[EFT_TEXTURE_SLOT_1].texUScale;
        tex0->w = 1.0f - frateUv * emitter->res->textureData[EFT_TEXTURE_SLOT_1].texVScale;
        tex1->z = 0.0f;
        tex1->w = 1.0f - frateUv * emitter->res->textureData[EFT_TEXTURE_SLOT_1].texVScale;

        numDrawVertex += 2;

        if (!(bTailConnect && i == 0))
        {
            if (pPtclNext != NULL)
            {
                pPtcl = pPtclNext;
                pPtclNext = pPtcl->next;
            }
        }
    }

    mStripeVertexCalcNum += numDrawVertex;

    return numDrawVertex;
}

s32 Renderer::MakeConnectionStripeAttributeBlockCoreDivide(EmitterInstance* emitter, s32 numPtcl, PtclInstance* pTailPtcl, PtclInstance* pTail2ndPtcl, s32 tailType, StripeVertexBuffer* stripeVertex)
{
    const ComplexEmitterData* res  = reinterpret_cast<const ComplexEmitterData*>(emitter->ptclHead->res);
    const StripeData        * sres = reinterpret_cast<const StripeData*>((u32)res + res->stripeDataOffset);

    s32 numDivide = sres->stripeDivideNum;

    s32 numVertex = numPtcl;
    numVertex += (numVertex - 1) * numDivide;

    if (numPtcl < 2)
        return 0;

    s32 numLoop = numPtcl;

    if (tailType == 1 || tailType == 2)
    {
        numLoop++;
        numVertex += numDivide;
    }

    s32 numDrawVertex = 0;

    f32 frateScale = 1.0f / (f32)(numVertex - 1);

    f32 frateScaleUv, uvStartOfs;
    if (ConnectionStripeUvScaleCalc(frateScaleUv, uvStartOfs, emitter, numPtcl, frateScale, tailType) == true)
        frateScaleUv /= numDivide + 1;

    f32 arate = sres->stripeEndAlpha - sres->stripeStartAlpha;

    nw::math::VEC4* pos0   = NULL;
    nw::math::VEC4* pos1   = NULL;
    nw::math::VEC4* outer0 = NULL;
    nw::math::VEC4* outer1 = NULL;
    nw::math::VEC4* tex0   = NULL;
    nw::math::VEC4* tex1   = NULL;
    nw::math::VEC4* dir0   = NULL;
    nw::math::VEC4* dir1   = NULL;

    PtclInstance* pPtclTop  = emitter->ptclHead;
    PtclInstance* pPtclPrev = pPtclTop->prev;
    PtclInstance* pPtcl     = pPtclTop;
    PtclInstance* pPtclNext = pPtclTop->next;
    PtclInstance* pPtclNext2= NULL;
    if (pPtclNext != NULL)
        pPtclNext2 = pPtclNext->next;

    nw::math::VEC3 curVec, prevCurVec, nowPos, nextPos;
    nw::math::VEC3 prevPos, next2Pos;
    nw::math::VEC3 dir, firstDir;
    nw::math::VEC3 interpolateNextDir;

    f32 divRateAdd = 1.0f / (f32)(numDivide + 1);

    s32 vtxCnt = 0;

    for (s32 i = 0; i < numLoop; i++)
    {
        if (tailType == 1 && i == 0)
        {
            nowPos  = pTailPtcl->worldPos;
            nextPos = pPtcl->worldPos;

            pPtclPrev  = pTail2ndPtcl;
            pPtcl      = pTailPtcl;
            pPtclNext  = pPtclTop;
            pPtclNext2 = pPtclTop->next;
        }
        else if (tailType == 1 && pPtcl == pTailPtcl)
        {
            nowPos  = pTailPtcl->worldPos;
            nextPos = pPtclTop->worldPos;
        }
        else if (tailType == 2 && i == 0)
        {
            nowPos.x = emitter->emitterSRT.m[0][3];
            nowPos.y = emitter->emitterSRT.m[1][3];
            nowPos.z = emitter->emitterSRT.m[2][3];
            nextPos = pPtcl->worldPos;

            pPtclPrev  = NULL;
            pPtcl      = pPtclTop;
            pPtclNext  = pPtclTop;
            pPtclNext2 = pPtclTop->next;
        }
        else
        {
            if (pPtclNext != NULL)
            {
                nextPos = pPtclNext->worldPos;
                nowPos  = pPtcl->worldPos;
            }
            else
            {
                nextPos += nextPos - nowPos;
                nowPos  = pPtcl->worldPos;
            }
        }

        if (pPtclPrev != NULL)
            prevPos = pPtclPrev->worldPos;
        else
            prevPos = nowPos + (nowPos - nextPos);

        if (pPtclNext2 != NULL)
            next2Pos = pPtclNext2->worldPos;

        else
        {
            if (tailType == 1 && i == numLoop - 2)
                next2Pos = pPtclTop->worldPos;
            else
                next2Pos = nextPos + (nextPos - nowPos);
        }

        nw::math::VEC3 v0 = prevPos - nowPos;
        nw::math::VEC3 v1 = nextPos - nowPos;
        nw::math::VEC3 startVel = (v1 - v0) * 0.5;

        nw::math::VEC3 v2 = nowPos   - nextPos;
        nw::math::VEC3 v3 = next2Pos - nextPos;
        nw::math::VEC3 endVel = (v3 - v2) * 0.5;

        bool bDivLinear;
        f32 len = v2.Length();
        if (len < pPtcl->scale.x * 0.25f)
            bDivLinear = true;
        else
            bDivLinear = false;

        f32 divRate = 0.0f;

        for (s32 j = 0; j < numDivide + 1; j++, divRate += divRateAdd)
        {
            f32 invDivRate = 1.0f - divRate;
            if (i == numLoop - 1 && j != 0)
                break;

            pos0   = &stripeVertex[numDrawVertex    ].pos;
            pos1   = &stripeVertex[numDrawVertex + 1].pos;
            outer0 = &stripeVertex[numDrawVertex    ].outer;
            outer1 = &stripeVertex[numDrawVertex + 1].outer;
            tex0   = &stripeVertex[numDrawVertex    ].tex;
            tex1   = &stripeVertex[numDrawVertex + 1].tex;
            dir0   = &stripeVertex[numDrawVertex    ].dir;
            dir1   = &stripeVertex[numDrawVertex + 1].dir;

            {
                prevCurVec = curVec;

                if (bDivLinear)
                    curVec = nowPos * invDivRate + nextPos * divRate;
                else
                    GetPositionOnCubic(&curVec, nowPos, startVel, nextPos, endVel, divRate);

                if (i == 0 && j == 0)
                    firstDir = (dir = startVel);

                else
                {
                    if (tailType == 1 && i == numLoop - 1)
                        dir = firstDir;
                    else
                        dir = curVec - prevCurVec;
                }

                if (dir.Length() > 0.0f)
                    dir.Normalize();

                if (sres->stripeDirInterpolate != 1.0f)
                {
                    if (i == 0)
                        interpolateNextDir = dir;

                    else
                    {
                        interpolateNextDir += (dir - interpolateNextDir) * sres->stripeDirInterpolate;
                        if (interpolateNextDir.Length() > 0.0f)
                            interpolateNextDir.Normalize();

                        dir = interpolateNextDir;
                    }
                }
            }

            f32 frate = (f32)vtxCnt * frateScale;
            f32 frateUv = 1.0f - ((f32)vtxCnt * frateScaleUv);
            frateUv -= uvStartOfs;

            f32 alpha = (sres->stripeStartAlpha + arate * frate) * pPtcl->alpha0;

            *pos0 = nw::math::VEC4(curVec);
            pos0->w = alpha * pPtcl->emitter->emitterAnimValue[EFT_ANIM_ALPHA] * pPtcl->emitter->emitterSet->mColor.a;
            *pos1 = nw::math::VEC4(curVec);
            pos1->w = pos0->w;

            *dir0 = nw::math::VEC4(dir);
            *dir1 = nw::math::VEC4(dir);

            PtclInstance* pNext = pPtclNext;
            if (pNext == NULL)
                pNext = pPtcl;

            if (sres->stripeType == EFT_STRIPE_TYPE_BILLBOARD)
            {
                *outer0       = nw::math::VEC4(mEyeVec);
                *outer1       = nw::math::VEC4(mEyeVec);

                f32 w0 = pPtcl->scale.x * emitter->emitterSet->mParticlScaleForCalc.x;
                f32 w1 = pNext->scale.x * emitter->emitterSet->mParticlScaleForCalc.x;
                outer1->w = w0 * invDivRate + w1 * divRate;
                outer0->w = -outer1->w;
            }
            else if (sres->stripeType == EFT_STRIPE_TYPE_EMITTER_UP_DOWN)
            {
                f32 upX, upY, upZ;
                if (emitter->followType == EFT_FOLLOW_TYPE_ALL)
                {
                    upX = emitter->emitterSRT.m[0][1];
                    upY = emitter->emitterSRT.m[1][1];
                    upZ = emitter->emitterSRT.m[2][1];
                }
                else
                {
                    upX = pPtcl->emitterSRT.m[0][1];
                    upY = pPtcl->emitterSRT.m[1][1];
                    upZ = pPtcl->emitterSRT.m[2][1];
                }

                outer0->x = upX;
                outer0->y = upY;
                outer0->z = upZ;
                outer1->x = upX;
                outer1->y = upY;
                outer1->z = upZ;

                f32 w0 = pPtcl->scale.x * emitter->emitterSet->mParticlScaleForCalc.x;
                f32 w1 = pNext->scale.x * emitter->emitterSet->mParticlScaleForCalc.x;
                outer1->w = w0 * invDivRate + w1 * divRate;
                outer0->w = -outer1->w;
            }
            else if (sres->stripeType == EFT_STRIPE_TYPE_EMITTER_MATRIX)
            {
                nw::math::VEC3 basis;
                if (emitter->followType == EFT_FOLLOW_TYPE_ALL)
                {
                    basis.x = emitter->emitterSRT.m[0][1];
                    basis.y = emitter->emitterSRT.m[1][1];
                    basis.z = emitter->emitterSRT.m[2][1];
                }
                else
                {
                    basis.x = pPtcl->emitterSRT.m[0][1];
                    basis.y = pPtcl->emitterSRT.m[1][1];
                    basis.z = pPtcl->emitterSRT.m[2][1];
                }

                nw::math::VEC3 outer;
                outer.SetCross(basis, dir);
                if (outer.Length() > 0.0f)
                    outer.Normalize();

                outer0->x = outer.x;
                outer0->y = outer.y;
                outer0->z = outer.z;
                outer1->x = outer.x;
                outer1->y = outer.y;
                outer1->z = outer.z;

                f32 w0 = pPtcl->scale.x * emitter->emitterSet->mParticlScaleForCalc.x;
                f32 w1 = pNext->scale.x * emitter->emitterSet->mParticlScaleForCalc.x;
                outer1->w = w0 * invDivRate + w1 * divRate;
                outer0->w = -outer1->w;
            }

            tex0->x = emitter->res->textureData[EFT_TEXTURE_SLOT_0].texUScale;
            tex0->y = 1.0f - frateUv * emitter->res->textureData[EFT_TEXTURE_SLOT_0].texVScale;
            tex1->x = 0.0f;
            tex1->y = 1.0f - frateUv * emitter->res->textureData[EFT_TEXTURE_SLOT_0].texVScale;

            tex0->z = emitter->res->textureData[EFT_TEXTURE_SLOT_1].texUScale;
            tex0->w = 1.0f - frateUv * emitter->res->textureData[EFT_TEXTURE_SLOT_1].texVScale;
            tex1->z = 0.0f;
            tex1->w = 1.0f - frateUv * emitter->res->textureData[EFT_TEXTURE_SLOT_1].texVScale;

            numDrawVertex += 2;
            vtxCnt++;
        }

        pPtclPrev = pPtcl;
        pPtcl = pPtclNext;
        pPtclNext = pPtclNext2;
        if (pPtclNext2 != NULL)
            pPtclNext2 = pPtclNext2->next;

        if (tailType == 1 && pPtclNext2 == NULL)
            pPtclNext2 = pPtclTop;
    }

    mStripeVertexCalcNum += numDrawVertex;

    return numDrawVertex;
}

StripeVertexBuffer* Renderer::MakeConnectionStripeAttributeBlock(EmitterInstance* emitter)
{
    if (emitter->ptclNum == 0)
        return NULL;

    if (emitter->ptclHead == NULL)
        return NULL;

    u32 allocedVertexNum = 0;
    s32 tailType = 0;

    const StripeData* stripeData = emitter->GetStripeData();

    tailType = stripeData->stripeConnectOpt;

    u32 numPtcl = emitter->ptclNum;

    PtclInstance* pTailPtcl    = emitter->ptclTail;
    PtclInstance* pTail2ndPtcl = emitter->ptclTail->prev;

    s32 numDivide = stripeData->stripeDivideNum;

    allocedVertexNum = numPtcl * 2;
    allocedVertexNum += ((numPtcl - 1) * 2) * numDivide;

    if (tailType == 1 || tailType == 2)
    {
        allocedVertexNum += 2;
        allocedVertexNum += 2 * numDivide;
    }

    if (allocedVertexNum == 0)
        return NULL;

    u32 attributeBufferSize = allocedVertexNum * sizeof(StripeVertexBuffer);
    emitter->stripeBuffer = static_cast<StripeVertexBuffer*>(AllocFromDoubleBuffer(attributeBufferSize));
    if (emitter->stripeBuffer == NULL)
        return NULL;

    emitter->connectionStripeUniformBlock = static_cast<StripeUniformBlock*>(AllocFromDoubleBuffer(sizeof(StripeUniformBlock)));
    if (emitter->connectionStripeUniformBlock == NULL)
        return NULL;

    if (stripeData->stripeOption == EFT_STRIPE_OPTION_TYPE_CROSS)
    {
        emitter->connectionStripeUniformBlockForCross = static_cast<StripeUniformBlock*>(AllocFromDoubleBuffer(sizeof(StripeUniformBlock)));
        if (emitter->connectionStripeUniformBlockForCross == NULL)
            return NULL;
    }

    if (numDivide == 0)
        emitter->stripeVertexNum = MakeConnectionStripeAttributeBlockCore(emitter, numPtcl, pTailPtcl, pTail2ndPtcl, tailType, emitter->stripeBuffer);

    else
        emitter->stripeVertexNum = MakeConnectionStripeAttributeBlockCoreDivide(emitter, numPtcl, pTailPtcl, pTail2ndPtcl, tailType, emitter->stripeBuffer);

    {
        nw::ut::Color4f setColor = emitter->emitterSet->GetColor();
        setColor.r *= emitter->res->colorScale;
        setColor.g *= emitter->res->colorScale;
        setColor.b *= emitter->res->colorScale;

        f32 color0R = emitter->ptclHead->color[EFT_COLOR_KIND_0].r * emitter->emitterAnimValue[EFT_ANIM_COLOR0_R] * setColor.r;
        f32 color0G = emitter->ptclHead->color[EFT_COLOR_KIND_0].g * emitter->emitterAnimValue[EFT_ANIM_COLOR0_G] * setColor.g;
        f32 color0B = emitter->ptclHead->color[EFT_COLOR_KIND_0].b * emitter->emitterAnimValue[EFT_ANIM_COLOR0_B] * setColor.b;

        f32 color1R = emitter->ptclHead->color[EFT_COLOR_KIND_1].r * emitter->emitterAnimValue[EFT_ANIM_COLOR1_R] * setColor.r;
        f32 color1G = emitter->ptclHead->color[EFT_COLOR_KIND_1].g * emitter->emitterAnimValue[EFT_ANIM_COLOR1_G] * setColor.g;
        f32 color1B = emitter->ptclHead->color[EFT_COLOR_KIND_1].b * emitter->emitterAnimValue[EFT_ANIM_COLOR1_B] * setColor.b;

        emitter->connectionStripeUniformBlock->stParam.x = 1.0f;
        emitter->connectionStripeUniformBlock->stParam.y = 0.0f;
        emitter->connectionStripeUniformBlock->stParam.z = emitter->res->offsetParam;
        emitter->connectionStripeUniformBlock->stParam.w = 1.0f;

        emitter->connectionStripeUniformBlock->uvScrollAnim.x = emitter->res->textureData[0].uvScrollInit.x + emitter->cnt * emitter->res->textureData[0].uvScroll.x;
        emitter->connectionStripeUniformBlock->uvScrollAnim.y = emitter->res->textureData[0].uvScrollInit.y - emitter->cnt * emitter->res->textureData[0].uvScroll.y;
        emitter->connectionStripeUniformBlock->uvScrollAnim.z = emitter->res->textureData[1].uvScrollInit.x + emitter->cnt * emitter->res->textureData[1].uvScroll.x;
        emitter->connectionStripeUniformBlock->uvScrollAnim.w = emitter->res->textureData[1].uvScrollInit.y - emitter->cnt * emitter->res->textureData[1].uvScroll.y;

        emitter->connectionStripeUniformBlock->uvScaleRotAnim0.x = emitter->res->textureData[0].uvScaleInit.x + emitter->cnt * emitter->res->textureData[0].uvScale.x;
        emitter->connectionStripeUniformBlock->uvScaleRotAnim0.y = emitter->res->textureData[0].uvScaleInit.y + emitter->cnt * emitter->res->textureData[0].uvScale.y;
        emitter->connectionStripeUniformBlock->uvScaleRotAnim0.z = emitter->cnt * emitter->res->textureData[0].uvRot;
        emitter->connectionStripeUniformBlock->uvScaleRotAnim0.w = 0.0;

        emitter->connectionStripeUniformBlock->uvScaleRotAnim1.x = emitter->res->textureData[1].uvScaleInit.x + emitter->cnt * emitter->res->textureData[1].uvScale.x;
        emitter->connectionStripeUniformBlock->uvScaleRotAnim1.y = emitter->res->textureData[1].uvScaleInit.y + emitter->cnt * emitter->res->textureData[1].uvScale.y;
        emitter->connectionStripeUniformBlock->uvScaleRotAnim1.z = emitter->cnt * emitter->res->textureData[1].uvRot;
        emitter->connectionStripeUniformBlock->uvScaleRotAnim1.w = 0.0;

        emitter->connectionStripeUniformBlock->vtxColor0.Set(color0R, color0G, color0B, emitter->fadeAlpha);
        emitter->connectionStripeUniformBlock->vtxColor1.Set(color1R, color1G, color1B, emitter->fadeAlpha);

        emitter->connectionStripeUniformBlock->emitterMat = nw::math::MTX44(emitter->emitterSRT);

        GX2EndianSwap(emitter->connectionStripeUniformBlock, sizeof(StripeUniformBlock));

        if (stripeData->stripeOption == EFT_STRIPE_OPTION_TYPE_CROSS)
        {
            emitter->connectionStripeUniformBlockForCross->stParam.x = 0.0f;
            emitter->connectionStripeUniformBlockForCross->stParam.y = 1.0f;
            emitter->connectionStripeUniformBlockForCross->stParam.z = emitter->res->offsetParam;
            emitter->connectionStripeUniformBlockForCross->stParam.w = 1.0f;

            emitter->connectionStripeUniformBlockForCross->uvScrollAnim.x = emitter->res->textureData[0].uvScrollInit.x + emitter->cnt * emitter->res->textureData[0].uvScroll.x;
            emitter->connectionStripeUniformBlockForCross->uvScrollAnim.y = emitter->res->textureData[0].uvScrollInit.y - emitter->cnt * emitter->res->textureData[0].uvScroll.y;
            emitter->connectionStripeUniformBlockForCross->uvScrollAnim.z = emitter->res->textureData[1].uvScrollInit.x + emitter->cnt * emitter->res->textureData[1].uvScroll.x;
            emitter->connectionStripeUniformBlockForCross->uvScrollAnim.w = emitter->res->textureData[1].uvScrollInit.y - emitter->cnt * emitter->res->textureData[1].uvScroll.y;

            emitter->connectionStripeUniformBlockForCross->uvScaleRotAnim0.x = emitter->res->textureData[0].uvScaleInit.x + emitter->cnt * emitter->res->textureData[0].uvScale.x;
            emitter->connectionStripeUniformBlockForCross->uvScaleRotAnim0.y = emitter->res->textureData[0].uvScaleInit.y + emitter->cnt * emitter->res->textureData[0].uvScale.y;
            emitter->connectionStripeUniformBlockForCross->uvScaleRotAnim0.z = emitter->cnt * emitter->res->textureData[0].uvRot;
            emitter->connectionStripeUniformBlockForCross->uvScaleRotAnim0.w = 0.0;

            emitter->connectionStripeUniformBlockForCross->uvScaleRotAnim1.x = emitter->res->textureData[1].uvScaleInit.x + emitter->cnt * emitter->res->textureData[1].uvScale.x;
            emitter->connectionStripeUniformBlockForCross->uvScaleRotAnim1.y = emitter->res->textureData[1].uvScaleInit.y + emitter->cnt * emitter->res->textureData[1].uvScale.y;
            emitter->connectionStripeUniformBlockForCross->uvScaleRotAnim1.z = emitter->cnt * emitter->res->textureData[1].uvRot;
            emitter->connectionStripeUniformBlockForCross->uvScaleRotAnim1.w = 0.0;

            emitter->connectionStripeUniformBlockForCross->vtxColor0.Set(color0R, color0G, color0B, emitter->fadeAlpha);
            emitter->connectionStripeUniformBlockForCross->vtxColor1.Set(color1R, color1G, color1B, emitter->fadeAlpha);

            emitter->connectionStripeUniformBlockForCross->emitterMat = nw::math::MTX44(emitter->emitterSRT);

            GX2EndianSwap(emitter->connectionStripeUniformBlockForCross, sizeof(StripeUniformBlock));
        }
        else
        {
            emitter->connectionStripeUniformBlockForCross = NULL;
        }
    }

    return emitter->stripeBuffer;
}

void Renderer::EntryConnectionStripe(const EmitterInstance* emitter, void* userParam)
{
    ParticleShader* stripeShader = emitter->shader[mCurrentShaderType];
    if (stripeShader == NULL)
        return;

    StripeVertexBuffer* stripeVertex = emitter->stripeBuffer;
    if (stripeVertex == NULL || emitter->stripeVertexNum < 4)
        return;

    mCurrentDrawingType = EFT_PTCL_TYPE_COMPLEX;

    if (!SetupStripeDrawSetting(emitter, userParam))
        return;

    VertexBuffer::BindExtBuffer(0, sizeof(StripeVertexBuffer) * emitter->stripeVertexNum, 0, sizeof(StripeVertexBuffer), stripeVertex);

    const StripeData* stripeData = emitter->GetStripeData();

    stripeShader->mStripeUniformBlock.BindUniformBlock(emitter->connectionStripeUniformBlock);
    Draw::DrawPrimitive(Draw::PRIM_TYPE_TRIANGLE_STRIP, 0, emitter->stripeVertexNum);
    mStripeVertexDrawNum += emitter->stripeVertexNum;

    if (stripeData->stripeOption == EFT_STRIPE_OPTION_TYPE_CROSS && emitter->connectionStripeUniformBlockForCross &&
        stripeData->stripeType != EFT_STRIPE_TYPE_BILLBOARD)
    {
        stripeShader->mStripeUniformBlock.BindUniformBlock(emitter->connectionStripeUniformBlockForCross);
        Draw::DrawPrimitive(Draw::PRIM_TYPE_TRIANGLE_STRIP, 0, emitter->stripeVertexNum);
        mStripeVertexDrawNum += emitter->stripeVertexNum;
    }
}

} } // namespace nw::eft
