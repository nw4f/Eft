#include <nw/eft/eft_Emitter.h>
#include <nw/eft/eft_EmitterSet.h>
#include <nw/eft/eft_Particle.h>
#include <nw/eft/eft_Renderer.h>
#include <nw/eft/eft_Shader.h>
#include <nw/eft/eft_System.h>
#include <nw/eft/eft_UniformBlock.h>

#include <algorithm>

namespace nw { namespace eft {

s32 Renderer::MakeStripeAttributeBlockCore(PtclStripe* stripe, StripeVertexBuffer* stripeVertex, s32 wroteVertexNum)
{
    if (stripe == NULL || stripe->res == NULL)
        return 0;

    stripe->numDrawVertex = 0;

    const ComplexEmitterData* res  = stripe->res;
    const StripeData*         sres = reinterpret_cast<const StripeData*>((u32)res + res->stripeDataOffset);

    s32 numVertex = stripe->numHistory;

    s32 numLoop = sres->stripeHistoryStep;
    if (numLoop >= numVertex)
        numLoop = numVertex;

    s32 numDrawVertex = 0;

    if (numLoop < 3)
        return 0;

    f32 frateScale = 1.0f / (f32)(numLoop - 2);
    f32 frateScaleUv;
    if (sres->stripeTexCoordOpt == EFT_STRIPE_TEXCOORD_OPTION_TYPE_DIVISION &&
        stripe->cnt < sres->stripeHistoryStep)
    {
        frateScaleUv = 1.0f / (f32)(sres->stripeHistoryStep - 2);
    }
    else
    {
        frateScaleUv = frateScale;
    }

    f32 arate = sres->stripeEndAlpha - sres->stripeStartAlpha;

    nw::math::VEC4* pos0   = NULL;
    nw::math::VEC4* pos1   = NULL;
    nw::math::VEC4* outer0 = NULL;
    nw::math::VEC4* outer1 = NULL;
    nw::math::VEC4* tex0   = NULL;
    nw::math::VEC4* tex1   = NULL;
    nw::math::VEC4* dir0   = NULL;
    nw::math::VEC4* dir1   = NULL;

    stripe->startDrawVertex = wroteVertexNum;

    for (s32 i = 0; i < numLoop - 1; i++)
    {
        u32 index = wroteVertexNum + numDrawVertex;

        pos0   = &stripeVertex[index    ].pos;
        pos1   = &stripeVertex[index + 1].pos;
        outer0 = &stripeVertex[index    ].outer;
        outer1 = &stripeVertex[index + 1].outer;
        tex0   = &stripeVertex[index    ].tex;
        tex1   = &stripeVertex[index + 1].tex;
        dir0   = &stripeVertex[index    ].dir;
        dir1   = &stripeVertex[index + 1].dir;

        f32 frate   = (f32)i * frateScale;
        f32 frateUv = (f32)i * frateScaleUv;

        f32 t  = frate * (f32)(numVertex - 2);
        s32 iT = (s32)(t + 0.5f);

        s32 current = stripe->histQEnd - 1 - iT;
        if (current < 0)
            current = sres->stripeNumHistory + current;

        f32 alpha = (sres->stripeStartAlpha + arate * frate) *
                        stripe->ptcl->alpha * stripe->ptcl->emitter->emitterSet->mColor.a * stripe->ptcl->emitter->fadeAlpha;

        pos0->x = stripe->hist[current].pos.x;
        pos0->y = stripe->hist[current].pos.y;
        pos0->z = stripe->hist[current].pos.z;
        pos0->w = alpha * stripe->ptcl->emitter->emitAnimValue[EFT_ANIM_ALPHA];
        pos1->x = pos0->x;
        pos1->y = pos0->y;
        pos1->z = pos0->z;
        pos1->w = pos0->w;

        dir0->x = stripe->hist[current].dir.x;
        dir0->y = stripe->hist[current].dir.y;
        dir0->z = stripe->hist[current].dir.z;
        dir1->x = dir0->x;
        dir1->y = dir0->y;
        dir1->z = dir0->z;

        outer0->x = stripe->hist[current].outer.x;
        outer0->y = stripe->hist[current].outer.y;
        outer0->z = stripe->hist[current].outer.z;
        outer1->x = outer0->x;
        outer1->y = outer0->y;
        outer1->z = outer0->z;

        outer0->w =  stripe->hist[current].scale;
        outer1->w = -stripe->hist[current].scale;

        tex0->x = stripe->ptcl->uvOffset.x;
        tex0->y = frateUv * stripe->res->textureData[EFT_TEXTURE_SLOT_0].texVScale + stripe->ptcl->uvOffset.y;
        tex1->x = stripe->res->textureData[EFT_TEXTURE_SLOT_0].texUScale + stripe->ptcl->uvOffset.x;
        tex1->y = tex0->y;

        tex0->z = stripe->ptcl->uvSubOffset.x;
        tex0->w = frateUv * stripe->res->textureData[EFT_TEXTURE_SLOT_1].texVScale + stripe->ptcl->uvSubOffset.y;
        tex1->z = stripe->res->textureData[EFT_TEXTURE_SLOT_1].texUScale + stripe->ptcl->uvSubOffset.x;
        tex1->w = tex0->w;

        numDrawVertex += 2;
    }

    stripe->numDrawVertex = numDrawVertex;
    mStripeVertexCalcNum += numDrawVertex;

    return numDrawVertex;
}

void Renderer::GetPositionOnCubic(nw::math::VEC3* retPos, const nw::math::VEC3& startPos, const nw::math::VEC3& startVel, const nw::math::VEC3& endPos, const nw::math::VEC3& endVel, float time)
{
    // http://old.zaynar.co.uk/cppdoc/latest/projects/maths/NUSpline.cpp.html

    const static nw::math::MTX44 hermite(
         2.0f, -3.0f, 0.0f, 1.0f,
        -2.0f,  3.0f, 0.0f, 0.0f,
         1.0f, -2.0f, 1.0f, 0.0f,
         1.0f, -1.0f, 0.0f, 0.0f
    );

    nw::math::MTX44 m(
        startPos.x, endPos.x, startVel.x, endVel.x,
        startPos.y, endPos.y, startVel.y, endVel.y,
        startPos.z, endPos.z, startVel.z, endVel.z,
        0.0f,       0.0f,     0.0f,       0.0f
    );

    MTX44Mult(&m, &m, &hermite);

    nw::math::VEC3 timeVector(time * time * time, time * time, time);
    retPos->x = m.m[0][0] * timeVector.x + m.m[0][1] * timeVector.y + m.m[0][2] * timeVector.z + m.m[0][3];
    retPos->y = m.m[1][0] * timeVector.x + m.m[1][1] * timeVector.y + m.m[1][2] * timeVector.z + m.m[1][3];
    retPos->z = m.m[2][0] * timeVector.x + m.m[2][1] * timeVector.y + m.m[2][2] * timeVector.z + m.m[2][3];
}

s32 Renderer::MakeStripeAttributeBlockCoreDivide(PtclStripe* stripe, StripeVertexBuffer* stripeVertex, s32 wroteVertexNum, s32 numDivide)
{
    if (stripe == NULL || stripe->res == NULL)
        return 0;

    stripe->numDrawVertex = 0;

    const ComplexEmitterData* res  = stripe->res;
    const StripeData*         sres = reinterpret_cast<const StripeData*>((u32)res + res->stripeDataOffset);

    s32 numVertex = stripe->numHistory;
    if (numVertex < 3)
        return 0;

    numVertex += (stripe->numHistory - 1) * numDivide;

    s32 numLoop = sres->stripeHistoryStep;
    numLoop += (sres->stripeHistoryStep - 1) * numDivide;

    if (numLoop >= numVertex)
        numLoop = numVertex;

    if (numLoop < 3)
        return 0;

    s32 numDrawVertex = 0;

    f32 frateScale    = 1.0f / (f32)(numLoop - 2);
    f32 frateScaleDiv = 1.0f / (f32)(numDivide + 1);
    f32 frateScaleUv;
    if (sres->stripeTexCoordOpt == EFT_STRIPE_TEXCOORD_OPTION_TYPE_DIVISION &&
        stripe->cnt < sres->stripeHistoryStep)
    {
        s32 historyStep = (sres->stripeHistoryStep - 2) + (sres->stripeHistoryStep - 1) * numDivide;
        frateScaleUv = 1.0f / (f32)historyStep;
    }
    else
    {
        frateScaleUv = frateScale;
    }

    f32 arate = sres->stripeEndAlpha - sres->stripeStartAlpha;

    nw::math::VEC4* pos0   = NULL;
    nw::math::VEC4* pos1   = NULL;
    nw::math::VEC4* outer0 = NULL;
    nw::math::VEC4* outer1 = NULL;
    nw::math::VEC4* tex0   = NULL;
    nw::math::VEC4* tex1   = NULL;
    nw::math::VEC4* dir0   = NULL;
    nw::math::VEC4* dir1   = NULL;

    nw::math::VEC3 curVec;
    nw::math::VEC3 startVel, endVel;
    nw::math::VEC3 v0, v1, v2, v3;

    stripe->startDrawVertex = wroteVertexNum;

    for (s32 i = 0; i < numLoop - 1; i++)
    {
        u32 index = wroteVertexNum + numDrawVertex;

        pos0   = &stripeVertex[index    ].pos;
        pos1   = &stripeVertex[index + 1].pos;
        outer0 = &stripeVertex[index    ].outer;
        outer1 = &stripeVertex[index + 1].outer;
        tex0   = &stripeVertex[index    ].tex;
        tex1   = &stripeVertex[index + 1].tex;
        dir0   = &stripeVertex[index    ].dir;
        dir1   = &stripeVertex[index + 1].dir;

        f32 frate   = (f32)i * frateScale;
        f32 frateUv = (f32)i * frateScaleUv;

        f32 t = frate * frateScaleDiv * (f32)(numVertex - 2);
        s32 iT = (s32)t;

        s32 current = stripe->histQEnd - 1 - iT;
        s32 next = current - 1;

        if (current < 0)
            current = sres->stripeNumHistory + current;

        if (next < 0)
            next = sres->stripeNumHistory + next;

        s32 prev = current + 1;
        if (prev >= sres->stripeNumHistory)
            prev -= sres->stripeNumHistory;

        s32 next2 = next - 1;
        if (next2 < 0)
            next2 = sres->stripeNumHistory + next2;

        f32 posRate = t - (f32)iT;

        {
            u32 p0 = prev;
            u32 p1 = current;
            u32 p2 = next;
            u32 p3 = next2;

            if (iT == 0)
            {
                p0 = current;
                p1 = next;
            }

            if (iT >= stripe->numHistory - 2)
            {
                p2 = current;
                p3 = next;
            }

            v0.x = stripe->hist[p0].pos.x - stripe->hist[p1].pos.x;
            v0.y = stripe->hist[p0].pos.y - stripe->hist[p1].pos.y;
            v0.z = stripe->hist[p0].pos.z - stripe->hist[p1].pos.z;

            v1.x = stripe->hist[p2].pos.x - stripe->hist[p1].pos.x;
            v1.y = stripe->hist[p2].pos.y - stripe->hist[p1].pos.y;
            v1.z = stripe->hist[p2].pos.z - stripe->hist[p1].pos.z;

            v2.x = stripe->hist[p1].pos.x - stripe->hist[p2].pos.x;
            v2.y = stripe->hist[p1].pos.y - stripe->hist[p2].pos.y;
            v2.z = stripe->hist[p1].pos.z - stripe->hist[p2].pos.z;

            v3.x = stripe->hist[p3].pos.x - stripe->hist[p2].pos.x;
            v3.y = stripe->hist[p3].pos.y - stripe->hist[p2].pos.y;
            v3.z = stripe->hist[p3].pos.z - stripe->hist[p2].pos.z;

            startVel.x = (v1.x - v0.x) * 0.5f;
            startVel.y = (v1.y - v0.y) * 0.5f;
            startVel.z = (v1.z - v0.z) * 0.5f;

            endVel.x = (v3.x - v2.x) * 0.5f;
            endVel.y = (v3.y - v2.y) * 0.5f;
            endVel.z = (v3.z - v2.z) * 0.5f;

            GetPositionOnCubic(&curVec, stripe->hist[current].pos, startVel, stripe->hist[next].pos, endVel, posRate);
        }

        f32 alpha = (sres->stripeStartAlpha + arate * frate) *
                        stripe->ptcl->alpha * stripe->ptcl->emitter->emitterSet->mColor.a * stripe->ptcl->emitter->fadeAlpha;

        pos0->x = curVec.x;
        pos0->y = curVec.y;
        pos0->z = curVec.z;
        pos0->w = alpha * stripe->ptcl->emitter->emitAnimValue[EFT_ANIM_ALPHA];

        pos1->x = curVec.x;
        pos1->y = curVec.y;
        pos1->z = curVec.z;
        pos1->w = pos0->w;

        f32 invRate = 1.0f - posRate;

        outer0->x =  stripe->hist[current].outer.x * invRate + stripe->hist[next].outer.x * posRate;
        outer0->y =  stripe->hist[current].outer.y * invRate + stripe->hist[next].outer.y * posRate;
        outer0->z =  stripe->hist[current].outer.z * invRate + stripe->hist[next].outer.z * posRate;
        outer0->w =  stripe->hist[current].scale;

        outer1->x =  outer0->x;
        outer1->y =  outer0->y;
        outer1->z =  outer0->z;
        outer1->w = -stripe->hist[current].scale;

        dir0->x = stripe->hist[current].dir.x * invRate + stripe->hist[next].dir.x * posRate;
        dir0->y = stripe->hist[current].dir.y * invRate + stripe->hist[next].dir.y * posRate;
        dir0->z = stripe->hist[current].dir.z * invRate + stripe->hist[next].dir.z * posRate;

        dir1->x = dir0->x;
        dir1->y = dir0->y;
        dir1->z = dir0->z;

        tex0->x = stripe->ptcl->uvOffset.x;
        tex0->y = frateUv * stripe->res->textureData[EFT_TEXTURE_SLOT_0].texVScale + stripe->ptcl->uvOffset.y;
        tex1->x = stripe->res->textureData[EFT_TEXTURE_SLOT_0].texUScale + stripe->ptcl->uvOffset.x;
        tex1->y = tex0->y;

        tex0->z = stripe->ptcl->uvSubOffset.x;
        tex0->w = frateUv * stripe->res->textureData[EFT_TEXTURE_SLOT_1].texVScale + stripe->ptcl->uvSubOffset.y;
        tex1->z = stripe->res->textureData[EFT_TEXTURE_SLOT_1].texUScale + stripe->ptcl->uvSubOffset.x;
        tex1->w = tex0->w;

        numDrawVertex += 2;
    }

    stripe->numDrawVertex = numDrawVertex;
    mStripeVertexCalcNum += numDrawVertex;

    return numDrawVertex;
}

bool Renderer::MakeStripeAttributeBlock(EmitterInstance* emitter)
{
    u32 allocedVertexNum = 0;
    s32 numDrawVertex = 0;

    PtclInstance* ptcl = emitter->ptclHead;
    if (ptcl == NULL)
        return false;

    PtclStripe* stripe = ptcl->stripe;
    if (stripe == NULL || stripe->res == NULL)
        return false;

    const ComplexEmitterData* res  = stripe->res;
    const StripeData*         sres = reinterpret_cast<const StripeData*>((u32)res + res->stripeDataOffset);

    s32 numDivide = sres->stripeDivideNum;

    do
    {
        if (ptcl->life)
        {
            PtclStripe* stripe = ptcl->stripe;
            if (stripe && stripe->res)
            {
                s32 numVertex = stripe->numHistory;
                numVertex += (stripe->numHistory - 1) * numDivide;

                s32 numLoop = sres->stripeHistoryStep;
                numLoop += (sres->stripeHistoryStep - 1) * numDivide;

                if (numLoop >= numVertex)
                    numLoop = numVertex;

                if (numLoop > 1)
                    allocedVertexNum += (numLoop - 1) * 2;
            }
        }
        ptcl = ptcl->next;
    }
    while (ptcl);

    ptcl = emitter->ptclHead;

    if (allocedVertexNum == 0)
        return false;

    u32 attributeBufferSize = allocedVertexNum * sizeof(StripeVertexBuffer);
    emitter->stripeBuffer = static_cast<StripeVertexBuffer*>(AllocFromDoubleBuffer(attributeBufferSize));
    if (emitter->stripeBuffer == NULL)
        return false;

    if (ptcl)
    {
        if (numDivide == 0)
        {
            do
            {
                if (ptcl->life)
                    numDrawVertex += MakeStripeAttributeBlockCore(ptcl->stripe, emitter->stripeBuffer, numDrawVertex);

                ptcl = ptcl->next;
            }
            while (ptcl);
        }
        else
        {
            do
            {
                if (ptcl->life)
                    numDrawVertex += MakeStripeAttributeBlockCoreDivide(ptcl->stripe, emitter->stripeBuffer, numDrawVertex, numDivide);

                ptcl = ptcl->next;
            }
            while (ptcl);
        }
    }

    emitter->stripeVertexNum = numDrawVertex;
    return true;
}

bool Renderer::ConnectionStripeUvScaleCalc(f32& frateScaleUv, f32& uvStartOfs, const EmitterInstance* emitter, s32 numLoop, f32 frateScale, s32 tailType)
{
    const ComplexEmitterData* res  = reinterpret_cast<const ComplexEmitterData*>(emitter->ptclHead->res);
    const StripeData*         sres = reinterpret_cast<const StripeData*>((u32)res + res->stripeDataOffset);

    bool ret = false;
    uvStartOfs = 0.0f;

    if (sres->stripeTexCoordOpt == EFT_STRIPE_TEXCOORD_OPTION_TYPE_DIVISION &&
        emitter->cnt < emitter->res->ptclLife)
    {
        s32 maxLife, maxNum;
        if (emitter->res->endFrame != EFT_INFINIT_LIFE && (emitter->res->endFrame - emitter->res->startFrame) < emitter->res->ptclLife)
        {
            maxLife = emitter->res->endFrame - emitter->res->startFrame + emitter->res->lifeStep;
            maxNum = maxLife / (emitter->res->lifeStep + 1);
        }
        else
        {
            maxLife = emitter->res->ptclLife;
            maxNum = maxLife / (emitter->res->lifeStep + 1) + 1;
        }

        maxNum *= (s32)emitter->res->emitRate;

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

s32 Renderer::MakeConnectionStripeAttributeBlockCore(EmitterInstance* emitter, s32 numPtcl, PtclInstance* pTailPtcl, PtclInstance* pTail2ndPtcl, s32 tailType, StripeVertexBuffer* stripeVertex, s32 wroteVertexNum)
{
    if (numPtcl < 2)
        return 0;

    const ComplexEmitterData* res  = reinterpret_cast<const ComplexEmitterData*>(emitter->ptclHead->res);
    const StripeData*         sres = reinterpret_cast<const StripeData*>((u32)res + res->stripeDataOffset);

    s32 numLoop = numPtcl;
    bool bTailConnect = false;

    if (tailType == 1 || tailType == 2)
    {
        bTailConnect = true;
        numLoop++;
    }

    s32 numDrawVertex = 0;

    f32 frateScale    = 1.0f / (f32)(numLoop - 1);
    f32 frateScaleUv, uvStartOfs;
    ConnectionStripeUvScaleCalc(frateScaleUv, uvStartOfs, emitter, numPtcl, frateScale, tailType);

    f32 arate = sres->stripeEndAlpha - sres->stripeStartAlpha;

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

#if (EFT_IS_CAFE_WUT || !EFT_IS_CAFE) // Fix undefined behavior
    curVec.x = 0.0f;
    curVec.y = 0.0f;
    curVec.z = 0.0f;

    nowPos.x = 0.0f;
    nowPos.y = 0.0f;
    nowPos.z = 0.0f;

    nextPos.x = 0.0f;
    nextPos.y = 0.0f;
    nextPos.z = 0.0f;

    interpolateNextDir.x = 0.0f;
    interpolateNextDir.y = 0.0f;
    interpolateNextDir.z = 0.0f;
#endif

    for (s32 i = 0; i < numLoop; i++)
    {
        u32 index = wroteVertexNum + numDrawVertex;

        pos0   = &stripeVertex[index    ].pos;
        pos1   = &stripeVertex[index + 1].pos;
        outer0 = &stripeVertex[index    ].outer;
        outer1 = &stripeVertex[index + 1].outer;
        tex0   = &stripeVertex[index    ].tex;
        tex1   = &stripeVertex[index + 1].tex;
        dir0   = &stripeVertex[index    ].dir;
        dir1   = &stripeVertex[index + 1].dir;

        f32 frate = (f32)i * frateScale;
        f32 frateUv = 1.0f - ((f32)i * frateScaleUv);
        frateUv -= uvStartOfs;

        f32 t = frate * (f32)(numLoop - 1);
        s32 iT = (s32)(t + 0.5f);

        f32 posRate = t - (f32)iT;
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

        f32 alpha = (sres->stripeStartAlpha + arate * frate) * pPtcl->alpha;

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

        *pos0 = nw::math::VEC4(curVec);
        pos0->w = alpha * pPtcl->emitter->emitAnimValue[EFT_ANIM_ALPHA] * pPtcl->emitter->emitterSet->mColor.a;
        *pos1 = nw::math::VEC4(curVec);
        pos1->w = pos0->w;

        *dir0 = nw::math::VEC4(dir);
        *dir1 = nw::math::VEC4(dir);

        if (sres->stripeType == EFT_STRIPE_TYPE_BILLBOARD)
        {
            *outer0 = nw::math::VEC4(mEyeVec);
            *outer1 = nw::math::VEC4(mEyeVec);

            outer0->w = pPtcl->scale.x * emitter->emitterSet->mParticlScaleForCalc.x;
            outer1->w = -outer0->w;
        }
        else if (sres->stripeType == EFT_STRIPE_TYPE_EMITTER_UP_DOWN)
        {
            f32 scale = pPtcl->scale.x;
            f32 upX, upY, upZ;
            if (emitter->followType == EFT_FOLLOW_TYPE_ALL)
            {
                upX = emitter->emitterSRT.m[0][1] * scale;
                upY = emitter->emitterSRT.m[1][1] * scale;
                upZ = emitter->emitterSRT.m[2][1] * scale;
            }
            else
            {
                upX = pPtcl->emitterSRT.m[0][1] * scale;
                upY = pPtcl->emitterSRT.m[1][1] * scale;
                upZ = pPtcl->emitterSRT.m[2][1] * scale;
            }

            outer0->x = upX;
            outer0->y = upY;
            outer0->z = upZ;
            outer1->x = upX;
            outer1->y = upY;
            outer1->z = upZ;

            outer0->w = pPtcl->scale.x * emitter->emitterSet->mParticlScaleForCalc.x;
            outer1->w = -outer0->w;
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

            outer0->w = pPtcl->scale.x * emitter->emitterSet->mParticlScaleForCalc.x;
            outer1->w = -outer0->w;
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

s32 Renderer::MakeConnectionStripeAttributeBlockCoreDivide(EmitterInstance* emitter, s32 numPtcl, PtclInstance* pTailPtcl, PtclInstance* pTail2ndPtcl, s32 tailType, StripeVertexBuffer* stripeVertex, s32 wroteVertexNum)
{
    const ComplexEmitterData* res  = reinterpret_cast<const ComplexEmitterData*>(emitter->ptclHead->res);
    const StripeData*         sres = reinterpret_cast<const StripeData*>((u32)res + res->stripeDataOffset);

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

    f32 frateScale    = 1.0f / (f32)(numVertex - 1);
    f32 frateScaleUv, uvStartOfs;
    if (ConnectionStripeUvScaleCalc(frateScaleUv, uvStartOfs, emitter, numPtcl, frateScale, tailType))
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

#if (EFT_IS_CAFE_WUT || !EFT_IS_CAFE) // Fix undefined behavior
    curVec.x = 0.0f;
    curVec.y = 0.0f;
    curVec.z = 0.0f;

    prevCurVec.x = 0.0f;
    prevCurVec.y = 0.0f;
    prevCurVec.z = 0.0f;

    nowPos.x = 0.0f;
    nowPos.y = 0.0f;
    nowPos.z = 0.0f;

    nextPos.x = 0.0f;
    nextPos.y = 0.0f;
    nextPos.z = 0.0f;

    prevPos.x = 0.0f;
    prevPos.y = 0.0f;
    prevPos.z = 0.0f;

    next2Pos.x = 0.0f;
    next2Pos.y = 0.0f;
    next2Pos.z = 0.0f;

    dir.x = 0.0f;
    dir.y = 0.0f;
    dir.z = 0.0f;

    firstDir.x = 0.0f;
    firstDir.y = 0.0f;
    firstDir.z = 0.0f;

    interpolateNextDir.x = 0.0f;
    interpolateNextDir.y = 0.0f;
    interpolateNextDir.z = 0.0f;
#endif

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
        nw::math::VEC3 startVel = (v1 - v0) * 0.5f;

        nw::math::VEC3 v2 = nowPos   - nextPos;
        nw::math::VEC3 v3 = next2Pos - nextPos;
        nw::math::VEC3 endVel = (v3 - v2) * 0.5f;

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

            u32 index = wroteVertexNum + numDrawVertex;

            pos0   = &stripeVertex[index    ].pos;
            pos1   = &stripeVertex[index + 1].pos;
            outer0 = &stripeVertex[index    ].outer;
            outer1 = &stripeVertex[index + 1].outer;
            tex0   = &stripeVertex[index    ].tex;
            tex1   = &stripeVertex[index + 1].tex;
            dir0   = &stripeVertex[index    ].dir;
            dir1   = &stripeVertex[index + 1].dir;

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

            f32 frate = (f32)vtxCnt * frateScale;
            f32 frateUv = 1.0f - ((f32)vtxCnt * frateScaleUv);
            frateUv -= uvStartOfs;

            f32 alpha = (sres->stripeStartAlpha + arate * frate) * pPtcl->alpha;

            *pos0 = nw::math::VEC4(curVec);
            pos0->w = alpha * pPtcl->emitter->emitAnimValue[EFT_ANIM_ALPHA] * pPtcl->emitter->emitterSet->mColor.a;
            *pos1 = nw::math::VEC4(curVec);
            pos1->w = pos0->w;

            *dir0 = nw::math::VEC4(dir);
            *dir1 = nw::math::VEC4(dir);

            PtclInstance* pNext = pPtclNext;
            if (pNext == NULL)
                pNext = pPtcl;

            if (sres->stripeType == EFT_STRIPE_TYPE_BILLBOARD)
            {
                *outer0 = nw::math::VEC4(mEyeVec);
                *outer1 = nw::math::VEC4(mEyeVec);

                f32 w0 = pPtcl->scale.x * emitter->emitterSet->mParticlScaleForCalc.x;
                f32 w1 = pNext->scale.x * emitter->emitterSet->mParticlScaleForCalc.x;
                outer0->w = w0 * invDivRate + w1 * divRate;
                outer1->w = -outer0->w;
            }
            else if (sres->stripeType == EFT_STRIPE_TYPE_EMITTER_UP_DOWN)
            {
                f32 scale = pPtcl->scale.x * invDivRate + pNext->scale.x * divRate;
                f32 upX, upY, upZ;
                if (emitter->followType == EFT_FOLLOW_TYPE_ALL)
                {
                    upX = emitter->emitterSRT.m[0][1] * scale;
                    upY = emitter->emitterSRT.m[1][1] * scale;
                    upZ = emitter->emitterSRT.m[2][1] * scale;
                }
                else
                {
                    upX = pPtcl->emitterSRT.m[0][1] * scale;
                    upY = pPtcl->emitterSRT.m[1][1] * scale;
                    upZ = pPtcl->emitterSRT.m[2][1] * scale;
                }

                outer0->x = upX;
                outer0->y = upY;
                outer0->z = upZ;
                outer1->x = upX;
                outer1->y = upY;
                outer1->z = upZ;

                f32 w0 = pPtcl->scale.x * emitter->emitterSet->mParticlScaleForCalc.x;
                f32 w1 = pNext->scale.x * emitter->emitterSet->mParticlScaleForCalc.x;
                outer0->w = w0 * invDivRate + w1 * divRate;
                outer1->w = -outer0->w;
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
                outer0->w = w0 * invDivRate + w1 * divRate;
                outer1->w = -outer0->w;
            }

            tex0->x = 0.0f;
            tex0->y = 1.0f - frateUv * emitter->res->textureData[EFT_TEXTURE_SLOT_0].texVScale;
            tex1->x = emitter->res->textureData[EFT_TEXTURE_SLOT_0].texUScale;
            tex1->y = 1.0f - frateUv * emitter->res->textureData[EFT_TEXTURE_SLOT_0].texVScale;

            tex1->z = 0.0f;
            tex0->w = 1.0f - frateUv * emitter->res->textureData[EFT_TEXTURE_SLOT_1].texVScale;
            tex0->z = emitter->res->textureData[EFT_TEXTURE_SLOT_1].texUScale;
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

StripeVertexBuffer* Renderer::MakeConnectionStripeAttributeBlock(EmitterInstance* emitter, bool cacheFlush)
{
    if (emitter->ptclNum == 0)
        return NULL;

    if (emitter->ptclHead == NULL)
        return NULL;

    u32 allocedVertexNum = 0;
    s32 tailType = 0;

    const ComplexEmitterData* res  = reinterpret_cast<const ComplexEmitterData*>(emitter->ptclHead->res);
    const StripeData*         sres = reinterpret_cast<const StripeData*>((u32)res + res->stripeDataOffset);

    tailType = sres->stripeConnectOpt;

    u32 numPtcl = emitter->ptclNum;

    PtclInstance* pTailPtcl    = emitter->ptclTail;
    PtclInstance* pTail2ndPtcl = emitter->ptclTail->prev;

    s32 numDivide = sres->stripeDivideNum;

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
    StripeVertexBuffer* stripeVertex = static_cast<StripeVertexBuffer*>(AllocFromDoubleBuffer(attributeBufferSize));
    if (stripeVertex == NULL)
        return NULL;

    if (numDivide == 0)
        emitter->stripeVertexNum = MakeConnectionStripeAttributeBlockCore(emitter, numPtcl, pTailPtcl, pTail2ndPtcl, tailType, stripeVertex, 0);

    else
        emitter->stripeVertexNum = MakeConnectionStripeAttributeBlockCoreDivide(emitter, numPtcl, pTailPtcl, pTail2ndPtcl, tailType, stripeVertex, 0);

#if EFT_IS_CAFE
    if (cacheFlush)
        GX2Invalidate(GX2_INVALIDATE_CPU_ATTRIB_BUFFER, stripeVertex, attributeBufferSize);
#endif // EFT_IS_CAFE

    return stripeVertex;
}

bool Renderer::SetupStripeDrawSetting(const EmitterInstance* emitter, bool cacheFlush, void* userParam)
{
    if (emitter->shader == NULL)
        return false;

    const ComplexEmitterData* res = reinterpret_cast<const ComplexEmitterData*>(emitter->res);
    ParticleShader* stripeShader = emitter->shader[mCurrentShaderType];
    if (stripeShader == NULL)
        return false;

    stripeShader->Bind();

    stripeShader->mVertexViewUniformBlock.BindUniformBlock(mViewUniformBlock);
    stripeShader->mFragmentViewUniformBlock.BindUniformBlock(mViewUniformBlock);

    stripeShader->mFragmentEmitterStaticUniformBlock.BindUniformBlock(emitter->emitterStaticUniformBlock);

    {
        mRenderContext.SetupZBufATest(res->zBufATestType);
        mRenderContext.SetupBlendType(res->blendType);
        mRenderContext.SetupDisplaySideType(res->displaySide);
    }

    {
        mRenderContext.SetupTexture(&res->texRes[EFT_TEXTURE_SLOT_0], EFT_TEXTURE_SLOT_0, stripeShader->GetTextureSamplerLocation(EFT_TEXTURE_SLOT_0));
    }

    if (res->texRes[EFT_TEXTURE_SLOT_1].handle)
    {
        mRenderContext.SetupTexture(&res->texRes[EFT_TEXTURE_SLOT_1], EFT_TEXTURE_SLOT_1, stripeShader->GetTextureSamplerLocation(EFT_TEXTURE_SLOT_1));
    }
    else
    {
        mRenderContext.SetupTexture((const TextureRes*)NULL, EFT_TEXTURE_SLOT_1, (FragmentTextureLocation){ 0u });
    }

    if (stripeShader->IsFragmentSoftEdgeVariation() &&
        mExtensionTexture[EFT_TEXTURE_SLOT_DEPTH_BUFFER - EFT_TEXTURE_SLOT_BIN_MAX])
    {
        mRenderContext.SetupTexture(mExtensionTexture[EFT_TEXTURE_SLOT_DEPTH_BUFFER - EFT_TEXTURE_SLOT_BIN_MAX],
                                    EFT_TEXTURE_SLOT_DEPTH_BUFFER, stripeShader->GetTextureSamplerLocation(EFT_TEXTURE_SLOT_DEPTH_BUFFER));
    }

    if (stripeShader->GetShaderVariation() == EFT_FRAGMENT_SHADER_TYPE_VARIATION_REFRACT_PARTICLE &&
        mExtensionTexture[EFT_TEXTURE_SLOT_FRAME_BUFFER - EFT_TEXTURE_SLOT_BIN_MAX])
    {
        mRenderContext.SetupTexture(mExtensionTexture[EFT_TEXTURE_SLOT_FRAME_BUFFER - EFT_TEXTURE_SLOT_BIN_MAX],
                                    EFT_TEXTURE_SLOT_DEPTH_BUFFER, stripeShader->GetTextureSamplerLocation(EFT_TEXTURE_SLOT_FRAME_BUFFER));
    }

    UserShaderRenderStateSetCallback callback = mSystem->GetUserShaderRenderStateSetCallback(static_cast<UserShaderCallBackID>(res->userShaderSetting));
    if (callback)
    {
        RenderStateSetArg arg = {
            .emitter = emitter,
            .renderer = this,
            .cacheFlush = cacheFlush,
            .userParam = userParam,
        };
        callback(arg);
    }

    return true;
}

void Renderer::EntryConnectionStripe(EmitterInstance* emitter, bool cacheFlush, void* userParam)
{
    ParticleShader* stripeShader = emitter->shader[mCurrentShaderType];
    if (stripeShader == NULL)
        return;

    PtclInstance* ptcl = emitter->ptclHead;
    if (ptcl == NULL)
        return;

    mCurrentDrawingType = EFT_PTCL_TYPE_COMPLEX;

    StripeVertexBuffer* stripeVertex = MakeConnectionStripeAttributeBlock(emitter, cacheFlush);
    if (stripeVertex == NULL || emitter->stripeVertexNum < 4)
        return;

    mCurrentDrawingType = EFT_PTCL_TYPE_COMPLEX;

    if (!SetupStripeDrawSetting(emitter, cacheFlush, userParam))
        return;

#if EFT_IS_WIN
    GLuint gl_vbo;
    glGenBuffers(1, &gl_vbo);

    glBindBuffer(GL_ARRAY_BUFFER, gl_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(StripeVertexBuffer) * emitter->stripeVertexNum, stripeVertex, GL_STATIC_DRAW);

    VertexBuffer::BindExtBuffer(stripeShader->GetPositionAttribute(),     4, sizeof(StripeVertexBuffer),  0 * 4);
    VertexBuffer::BindExtBuffer(stripeShader->GetOuterAttribute(),        4, sizeof(StripeVertexBuffer),  4 * 4);
    VertexBuffer::BindExtBuffer(stripeShader->GetTextureCoordAttribute(), 4, sizeof(StripeVertexBuffer),  8 * 4);
    VertexBuffer::BindExtBuffer(stripeShader->GetDirAttribute(),          4, sizeof(StripeVertexBuffer), 12 * 4);
#endif // EFT_IS_WIN
#if EFT_IS_CAFE
    VertexBuffer::BindExtBuffer(0, sizeof(StripeVertexBuffer) * emitter->stripeVertexNum, 0, sizeof(StripeVertexBuffer), stripeVertex);
#endif // EFT_IS_CAFE

    nw::ut::FloatColor setColor = emitter->emitterSet->GetColor();
    setColor.r *= emitter->res->colorScale;
    setColor.g *= emitter->res->colorScale;
    setColor.b *= emitter->res->colorScale;

    f32 color0R = ptcl->color[EFT_COLOR_KIND_0].r * emitter->emitAnimValue[EFT_ANIM_COLOR0_R] * setColor.r;
    f32 color0G = ptcl->color[EFT_COLOR_KIND_0].g * emitter->emitAnimValue[EFT_ANIM_COLOR0_G] * setColor.g;
    f32 color0B = ptcl->color[EFT_COLOR_KIND_0].b * emitter->emitAnimValue[EFT_ANIM_COLOR0_B] * setColor.b;

    f32 color1R = ptcl->color[EFT_COLOR_KIND_1].r * emitter->emitAnimValue[EFT_ANIM_COLOR1_R] * setColor.r;
    f32 color1G = ptcl->color[EFT_COLOR_KIND_1].g * emitter->emitAnimValue[EFT_ANIM_COLOR1_G] * setColor.g;
    f32 color1B = ptcl->color[EFT_COLOR_KIND_1].b * emitter->emitAnimValue[EFT_ANIM_COLOR1_B] * setColor.b;

    const ComplexEmitterData* res  = static_cast<const ComplexEmitterData*>(emitter->res);
    const StripeData*         sres = reinterpret_cast<const StripeData*>((u32)res + res->stripeDataOffset);

    s32 numDrawVertex = emitter->stripeVertexNum;

    StripeUniformBlock* uniformBlock = static_cast<StripeUniformBlock*>(AllocFromDoubleBuffer(sizeof(StripeUniformBlock)));
    if (uniformBlock == NULL)
    {
#if EFT_IS_WIN
        glDeleteBuffers(1, &gl_vbo);
#endif
        return;
    }

    uniformBlock->stParam.x = 1.0f;
    uniformBlock->stParam.y = 0.0f;
    uniformBlock->stParam.z = emitter->res->toCameraOffset;
    uniformBlock->stParam.w = 1.0f;

    uniformBlock->uvScrollAnim.x    = emitter->res->textureData[EFT_TEXTURE_SLOT_0].uvScrollInit.x + emitter->cnt * emitter->res->textureData[EFT_TEXTURE_SLOT_0].uvScroll.x;
    uniformBlock->uvScrollAnim.y    = emitter->res->textureData[EFT_TEXTURE_SLOT_0].uvScrollInit.y - emitter->cnt * emitter->res->textureData[EFT_TEXTURE_SLOT_0].uvScroll.y;
    uniformBlock->uvScrollAnim.z    = emitter->res->textureData[EFT_TEXTURE_SLOT_1].uvScrollInit.x + emitter->cnt * emitter->res->textureData[EFT_TEXTURE_SLOT_1].uvScroll.x;
    uniformBlock->uvScrollAnim.w    = emitter->res->textureData[EFT_TEXTURE_SLOT_1].uvScrollInit.y - emitter->cnt * emitter->res->textureData[EFT_TEXTURE_SLOT_1].uvScroll.y;

    uniformBlock->uvScaleRotAnim0.x = emitter->res->textureData[EFT_TEXTURE_SLOT_0].uvScaleInit.x  + emitter->cnt * emitter->res->textureData[EFT_TEXTURE_SLOT_0].uvScale.x;
    uniformBlock->uvScaleRotAnim0.y = emitter->res->textureData[EFT_TEXTURE_SLOT_0].uvScaleInit.y  + emitter->cnt * emitter->res->textureData[EFT_TEXTURE_SLOT_0].uvScale.y;
    uniformBlock->uvScaleRotAnim0.z =                                                                emitter->cnt * emitter->res->textureData[EFT_TEXTURE_SLOT_0].uvRot;
    uniformBlock->uvScaleRotAnim0.w = 0.0f;

    uniformBlock->uvScaleRotAnim1.x = emitter->res->textureData[EFT_TEXTURE_SLOT_1].uvScaleInit.x  + emitter->cnt * emitter->res->textureData[EFT_TEXTURE_SLOT_1].uvScale.x;
    uniformBlock->uvScaleRotAnim1.y = emitter->res->textureData[EFT_TEXTURE_SLOT_1].uvScaleInit.y  + emitter->cnt * emitter->res->textureData[EFT_TEXTURE_SLOT_1].uvScale.y;
    uniformBlock->uvScaleRotAnim1.z =                                                                emitter->cnt * emitter->res->textureData[EFT_TEXTURE_SLOT_1].uvRot;
    uniformBlock->uvScaleRotAnim1.w = 0.0f;

    uniformBlock->vtxColor0.Set(color0R, color0G, color0B, emitter->fadeAlpha);
    uniformBlock->vtxColor1.Set(color1R, color1G, color1B, emitter->fadeAlpha);

    uniformBlock->emitterMat = nw::math::MTX44(ptcl->emitter->emitterSRT);

#if EFT_IS_CAFE
    GX2EndianSwap(uniformBlock, sizeof(StripeUniformBlock));
    if (cacheFlush)
        GX2Invalidate(GX2_INVALIDATE_CPU_UNIFORM_BLOCK, uniformBlock, sizeof(StripeUniformBlock));
#endif // EFT_IS_CAFE

    stripeShader->mStripeUniformBlock.BindUniformBlock(uniformBlock);
    Draw::DrawPrimitive(Draw::PRIM_TYPE_TRIANGLE_STRIP, 0, numDrawVertex);

    if (sres->stripeOption == EFT_STRIPE_OPTION_TYPE_CROSS)
    {
        StripeUniformBlock* uniformBlock = static_cast<StripeUniformBlock*>(AllocFromDoubleBuffer(sizeof(StripeUniformBlock)));
        if (uniformBlock == NULL)
        {
#if EFT_IS_WIN
            glDeleteBuffers(1, &gl_vbo);
#endif
            return;
        }

        uniformBlock->stParam.x = 0.0f;
        uniformBlock->stParam.y = 1.0f;
        uniformBlock->stParam.z = res->toCameraOffset;
        uniformBlock->stParam.w = 1.0f;

        uniformBlock->uvScrollAnim.x    = emitter->res->textureData[EFT_TEXTURE_SLOT_0].uvScrollInit.x + emitter->cnt * emitter->res->textureData[EFT_TEXTURE_SLOT_0].uvScroll.x;
        uniformBlock->uvScrollAnim.y    = emitter->res->textureData[EFT_TEXTURE_SLOT_0].uvScrollInit.y - emitter->cnt * emitter->res->textureData[EFT_TEXTURE_SLOT_0].uvScroll.y;
        uniformBlock->uvScrollAnim.z    = emitter->res->textureData[EFT_TEXTURE_SLOT_1].uvScrollInit.x + emitter->cnt * emitter->res->textureData[EFT_TEXTURE_SLOT_1].uvScroll.x;
        uniformBlock->uvScrollAnim.w    = emitter->res->textureData[EFT_TEXTURE_SLOT_1].uvScrollInit.y - emitter->cnt * emitter->res->textureData[EFT_TEXTURE_SLOT_1].uvScroll.y;

        uniformBlock->uvScaleRotAnim0.x = emitter->res->textureData[EFT_TEXTURE_SLOT_0].uvScaleInit.x  + emitter->cnt * emitter->res->textureData[EFT_TEXTURE_SLOT_0].uvScale.x;
        uniformBlock->uvScaleRotAnim0.y = emitter->res->textureData[EFT_TEXTURE_SLOT_0].uvScaleInit.y  + emitter->cnt * emitter->res->textureData[EFT_TEXTURE_SLOT_0].uvScale.y;
        uniformBlock->uvScaleRotAnim0.z =                                                                emitter->cnt * emitter->res->textureData[EFT_TEXTURE_SLOT_0].uvRot;
        uniformBlock->uvScaleRotAnim0.w = 0.0f;

        uniformBlock->uvScaleRotAnim1.x = emitter->res->textureData[EFT_TEXTURE_SLOT_1].uvScaleInit.x  + emitter->cnt * emitter->res->textureData[EFT_TEXTURE_SLOT_1].uvScale.x;
        uniformBlock->uvScaleRotAnim1.y = emitter->res->textureData[EFT_TEXTURE_SLOT_1].uvScaleInit.y  + emitter->cnt * emitter->res->textureData[EFT_TEXTURE_SLOT_1].uvScale.y;
        uniformBlock->uvScaleRotAnim1.z =                                                                emitter->cnt * emitter->res->textureData[EFT_TEXTURE_SLOT_1].uvRot;
        uniformBlock->uvScaleRotAnim1.w = 0.0f;

        uniformBlock->vtxColor0.Set(color0R, color0G, color0B, emitter->fadeAlpha);
        uniformBlock->vtxColor1.Set(color1R, color1G, color1B, emitter->fadeAlpha);

        uniformBlock->emitterMat = nw::math::MTX44(ptcl->emitter->emitterSRT);

#if EFT_IS_CAFE
        GX2EndianSwap(uniformBlock, sizeof(StripeUniformBlock));
        if (cacheFlush)
            GX2Invalidate(GX2_INVALIDATE_CPU_UNIFORM_BLOCK, uniformBlock, sizeof(StripeUniformBlock));
#endif // EFT_IS_CAFE

        stripeShader->mStripeUniformBlock.BindUniformBlock(uniformBlock);
        Draw::DrawPrimitive(Draw::PRIM_TYPE_TRIANGLE_STRIP, 0, numDrawVertex);
    }

#if EFT_IS_WIN
    glDeleteBuffers(1, &gl_vbo);
#endif
}

void Renderer::EntryStripe(EmitterInstance* emitter, bool cacheFlush, void* userParam)
{
    if (!emitter->res->isDisplayParent)
        return;

    ParticleShader* stripeShader = emitter->shader[mCurrentShaderType];
    if (stripeShader == NULL)
        return;

    if (stripeShader->mVertexShaderKey.mVertexBillboardTypeVariation == EFT_BILLBOARD_TYPE_COMPLEX_STRIPE)
        return EntryConnectionStripe(emitter, cacheFlush, userParam);

    StripeVertexBuffer* stripeVertex = emitter->stripeBuffer;
    if (stripeVertex == NULL)
        return;

    mCurrentDrawingType = EFT_PTCL_TYPE_COMPLEX;

    if (!SetupStripeDrawSetting(emitter, cacheFlush, userParam))
        return;

#if EFT_IS_WIN
    GLuint gl_vbo;
    glGenBuffers(1, &gl_vbo);

    glBindBuffer(GL_ARRAY_BUFFER, gl_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(StripeVertexBuffer) * emitter->stripeVertexNum, stripeVertex, GL_STATIC_DRAW);

    VertexBuffer::BindExtBuffer(stripeShader->GetPositionAttribute(),     4, sizeof(StripeVertexBuffer),  0 * 4);
    VertexBuffer::BindExtBuffer(stripeShader->GetOuterAttribute(),        4, sizeof(StripeVertexBuffer),  4 * 4);
    VertexBuffer::BindExtBuffer(stripeShader->GetTextureCoordAttribute(), 4, sizeof(StripeVertexBuffer),  8 * 4);
    VertexBuffer::BindExtBuffer(stripeShader->GetDirAttribute(),          4, sizeof(StripeVertexBuffer), 12 * 4);
#endif // EFT_IS_WIN
#if EFT_IS_CAFE
    VertexBuffer::BindExtBuffer(0, sizeof(StripeVertexBuffer) * emitter->stripeVertexNum, 0, sizeof(StripeVertexBuffer), stripeVertex);
#endif // EFT_IS_CAFE

    nw::ut::FloatColor setColor = emitter->emitterSet->GetColor();
    setColor.r *= emitter->res->colorScale;
    setColor.g *= emitter->res->colorScale;
    setColor.b *= emitter->res->colorScale;

    nw::ut::FloatColor emitterColor0;
    emitterColor0.r = setColor.r * emitter->emitAnimValue[EFT_ANIM_COLOR0_R];
    emitterColor0.g = setColor.g * emitter->emitAnimValue[EFT_ANIM_COLOR0_G];
    emitterColor0.b = setColor.b * emitter->emitAnimValue[EFT_ANIM_COLOR0_B];

    nw::ut::FloatColor emitterColor1;
    emitterColor1.r = setColor.r * emitter->emitAnimValue[EFT_ANIM_COLOR1_R];
    emitterColor1.g = setColor.g * emitter->emitAnimValue[EFT_ANIM_COLOR1_G];
    emitterColor1.b = setColor.b * emitter->emitAnimValue[EFT_ANIM_COLOR1_B];

    for (PtclInstance* ptcl = emitter->ptclHead; ptcl != NULL; ptcl = ptcl->next)
    {
        PtclStripe* stripe = ptcl->stripe;
        if (stripe == NULL)
            continue;

        const ComplexEmitterData* res = stripe->res;
        if (res == NULL)
            continue;

        const StripeData* sres = reinterpret_cast<const StripeData*>((u32)res + res->stripeDataOffset);

        if (stripe->numDrawVertex < 4)
            continue;

        f32 color0R = ptcl->color[EFT_COLOR_KIND_0].r * emitterColor0.r;
        f32 color0G = ptcl->color[EFT_COLOR_KIND_0].g * emitterColor0.g;
        f32 color0B = ptcl->color[EFT_COLOR_KIND_0].b * emitterColor0.b;

        f32 color1R = ptcl->color[EFT_COLOR_KIND_1].r * emitterColor1.r;
        f32 color1G = ptcl->color[EFT_COLOR_KIND_1].g * emitterColor1.g;
        f32 color1B = ptcl->color[EFT_COLOR_KIND_1].b * emitterColor1.b;

        StripeUniformBlock* uniformBlock = static_cast<StripeUniformBlock*>(AllocFromDoubleBuffer(sizeof(StripeUniformBlock)));
        if (uniformBlock == NULL)
            break;

        uniformBlock->stParam.x = 1.0f;
        uniformBlock->stParam.y = 0.0f;
        uniformBlock->stParam.z = emitter->res->toCameraOffset;
        uniformBlock->stParam.w = ptcl->scale.x;

        uniformBlock->uvScrollAnim.x    = ptcl->uvOffset.x    + ptcl->uvScroll.x;
        uniformBlock->uvScrollAnim.y    = ptcl->uvOffset.y    - ptcl->uvScroll.y;
        uniformBlock->uvScrollAnim.z    = ptcl->uvSubOffset.x + ptcl->uvSubScroll.x;
        uniformBlock->uvScrollAnim.w    = ptcl->uvSubOffset.y - ptcl->uvSubScroll.y;

        uniformBlock->uvScaleRotAnim0.x = ptcl->uvScale.x;
        uniformBlock->uvScaleRotAnim0.y = ptcl->uvScale.y;
        uniformBlock->uvScaleRotAnim0.z = ptcl->uvRotateZ;
        uniformBlock->uvScaleRotAnim0.w = 0.0f;

        uniformBlock->uvScaleRotAnim1.x = ptcl->uvSubScale.x;
        uniformBlock->uvScaleRotAnim1.y = ptcl->uvSubScale.y;
        uniformBlock->uvScaleRotAnim1.z = ptcl->uvSubRotateZ;
        uniformBlock->uvScaleRotAnim1.w = 0.0f;

        uniformBlock->vtxColor0.Set(color0R, color0G, color0B, ptcl->emitter->fadeAlpha);
        uniformBlock->vtxColor1.Set(color1R, color1G, color1B, ptcl->emitter->fadeAlpha);

        uniformBlock->emitterMat = nw::math::MTX44(ptcl->emitter->emitterSRT);

#if EFT_IS_CAFE
        GX2EndianSwap(uniformBlock, sizeof(StripeUniformBlock));
        if (cacheFlush)
            GX2Invalidate(GX2_INVALIDATE_CPU_UNIFORM_BLOCK, uniformBlock, sizeof(StripeUniformBlock));
#endif // EFT_IS_CAFE

        stripeShader->mStripeUniformBlock.BindUniformBlock(uniformBlock);
        Draw::DrawPrimitive(Draw::PRIM_TYPE_TRIANGLE_STRIP, ptcl->stripe->startDrawVertex, ptcl->stripe->numDrawVertex);

        if (sres->stripeOption == EFT_STRIPE_OPTION_TYPE_CROSS)
        {
            StripeUniformBlock* uniformBlock = static_cast<StripeUniformBlock*>(AllocFromDoubleBuffer(sizeof(StripeUniformBlock)));
            if (uniformBlock == NULL)
                break;

            uniformBlock->stParam.x = 0.0f;
            uniformBlock->stParam.y = 1.0f;
            uniformBlock->stParam.z = emitter->res->toCameraOffset;
            uniformBlock->stParam.w = ptcl->scale.x;

            uniformBlock->uvScrollAnim.x    = ptcl->uvOffset.x    + ptcl->uvScroll.x;
            uniformBlock->uvScrollAnim.y    = ptcl->uvOffset.y    - ptcl->uvScroll.y;
            uniformBlock->uvScrollAnim.z    = ptcl->uvSubOffset.x + ptcl->uvSubScroll.x;
            uniformBlock->uvScrollAnim.w    = ptcl->uvSubOffset.y - ptcl->uvSubScroll.y;

            uniformBlock->uvScaleRotAnim0.x = ptcl->uvScale.x;
            uniformBlock->uvScaleRotAnim0.y = ptcl->uvScale.y;
            uniformBlock->uvScaleRotAnim0.z = ptcl->uvRotateZ;
            uniformBlock->uvScaleRotAnim0.w = 0.0f;

            uniformBlock->uvScaleRotAnim1.x = ptcl->uvSubScale.x;
            uniformBlock->uvScaleRotAnim1.y = ptcl->uvSubScale.y;
            uniformBlock->uvScaleRotAnim1.z = ptcl->uvSubRotateZ;
            uniformBlock->uvScaleRotAnim1.w = 0.0f;

            uniformBlock->vtxColor0.Set(color0R, color0G, color0B, ptcl->emitter->fadeAlpha);
            uniformBlock->vtxColor1.Set(color1R, color1G, color1B, ptcl->emitter->fadeAlpha);

            uniformBlock->emitterMat = nw::math::MTX44(ptcl->emitter->emitterSRT);

#if EFT_IS_CAFE
            GX2EndianSwap(uniformBlock, sizeof(StripeUniformBlock));
            if (cacheFlush)
                GX2Invalidate(GX2_INVALIDATE_CPU_UNIFORM_BLOCK, uniformBlock, sizeof(StripeUniformBlock));
#endif // EFT_IS_CAFE

            stripeShader->mStripeUniformBlock.BindUniformBlock(uniformBlock);
            Draw::DrawPrimitive(Draw::PRIM_TYPE_TRIANGLE_STRIP, ptcl->stripe->startDrawVertex, ptcl->stripe->numDrawVertex);
        }
    }

#if EFT_IS_WIN
    glDeleteBuffers(1, &gl_vbo);
#endif
}

} } // namespace nw::eft
