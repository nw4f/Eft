#include <nw/eft/eft_Emitter.h>
#include <nw/eft/eft_EmitterSet.h>
#include <nw/eft/eft_Particle.h>
#include <nw/eft/eft_Renderer.h>
#include <nw/eft/eft_Shader.h>
#include <nw/eft/eft_System.h>
#include <nw/eft/eft_UniformBlock.h>

#include <algorithm>

namespace nw { namespace eft {

s32 Renderer::MakeStripeAttributeBlockCore(PtclStripe* stripe, StripeVertexBuffer* stripeVertex, s32 wroteVertexNum, const StripeData* stripeData)
{
    stripe->starDrawVertex = 0;
    stripe->numDrawVertex = 0;

    s32 numVertex = stripe->numHistory;

    s32 numLoop = stripeData->stripeHistoryStep;
    if (numLoop >= numVertex)
        numLoop = numVertex;

    if (numLoop < 3)
        return 0;

    s32 numDrawVertex = 0;

    f32 frateScale    = 1.0f / (f32)(numLoop - 2);
    f32 frateScaleUv;
    if (stripeData->stripeTexCoordOpt == EFT_STRIPE_TEXCOORD_OPTION_TYPE_DIVISION &&
        stripe->cnt < stripeData->stripeHistoryStep)
    {
        frateScaleUv = 1.0f / (f32)(stripeData->stripeHistoryStep - 2);
    }
    else
    {
        frateScaleUv = frateScale;
    }

    f32 arate = stripeData->stripeEndAlpha - stripeData->stripeStartAlpha;

    stripe->starDrawVertex = wroteVertexNum;

    StripeVertexBuffer* left  = NULL;
    StripeVertexBuffer* right = NULL;

    for (s32 i = 0; i < numLoop - 1; i++)
    {
        u32 index = wroteVertexNum + numDrawVertex;
        left  = &stripeVertex[index    ];
        right = &stripeVertex[index + 1];

        f32 frate    = (f32)i * frateScale;
        f32 frateUv = (f32)i * frateScaleUv;

        f32 t = frate * (numVertex - 2);
        s32 iT = (s32)(t + 0.5f);

        s32 current = stripe->histQEnd - 1 - iT;
        if (current < 0)
            current = stripeData->stripeNumHistory + current;

        f32 alpha0 = (stripeData->stripeStartAlpha + arate * frate) * stripe->ptcl->alpha0 * stripe->ptcl->emitter->emitterSet->mColor.a * stripe->ptcl->emitter->fadeAlpha;
        f32 alpha1 = (stripeData->stripeStartAlpha + arate * frate) * stripe->ptcl->alpha1 * stripe->ptcl->emitter->emitterSet->mColor.a * stripe->ptcl->emitter->fadeAlpha;

        left->pos.x     = stripe->hist[current].pos.x;
        left->pos.y     = stripe->hist[current].pos.y;
        left->pos.z     = stripe->hist[current].pos.z;
        left->pos.w     = alpha0 * stripe->ptcl->emitter->emitterAnimValue[EFT_ANIM_ALPHA];
        right->pos.x    = left->pos.x;
        right->pos.y    = left->pos.y;
        right->pos.z    = left->pos.z;
        right->pos.w    = left->pos.w;

        left->dir.x     = stripe->hist[current].dir.x;
        left->dir.y     = stripe->hist[current].dir.y;
        left->dir.z     = stripe->hist[current].dir.z;
        left->dir.w     = alpha1 * stripe->ptcl->emitter->emitterAnimValue[EFT_ANIM_ALPHA];
        right->dir.x    = left->dir.x;
        right->dir.y    = left->dir.y;
        right->dir.z    = left->dir.z;
        right->dir.w    = left->dir.w;

        left->outer.x   = stripe->hist[current].outer.x;
        left->outer.y   = stripe->hist[current].outer.y;
        left->outer.z   = stripe->hist[current].outer.z;
        right->outer.x  = left->outer.x;
        right->outer.y  = left->outer.y;
        right->outer.z  = left->outer.z;

        left->outer.w   =  stripe->hist[current].scale;
        right->outer.w  = -stripe->hist[current].scale;

        right->tex.x    = stripe->uvParam[EFT_TEXTURE_SLOT_0].offset.x;
        right->tex.y    = frateUv * stripe->res->textureData[EFT_TEXTURE_SLOT_0].texVScale + stripe->uvParam[EFT_TEXTURE_SLOT_0].offset.y;
        right->tex.z    = stripe->uvParam[EFT_TEXTURE_SLOT_1].offset.x;
        right->tex.w    = frateUv * stripe->res->textureData[EFT_TEXTURE_SLOT_1].texVScale + stripe->uvParam[EFT_TEXTURE_SLOT_1].offset.y;

        left->tex.x     = stripe->res->textureData[EFT_TEXTURE_SLOT_0].texUScale + stripe->uvParam[EFT_TEXTURE_SLOT_0].offset.x;
        left->tex.y     = right->tex.y;
        left->tex.z     = stripe->res->textureData[EFT_TEXTURE_SLOT_1].texUScale + stripe->uvParam[EFT_TEXTURE_SLOT_1].offset.x;
        left->tex.w     = right->tex.w;

        numDrawVertex += 2;
    }

    stripe->numDrawVertex = numDrawVertex;

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

s32 Renderer::MakeStripeAttributeBlockCoreDivide(PtclStripe* stripe, StripeVertexBuffer* stripeVertex, s32 wroteVertexNum, const StripeData* stripeData)
{
    stripe->starDrawVertex = 0;
    stripe->numDrawVertex = 0;

    s32 numVertex = stripe->numHistory;
    if (numVertex < 3)
        return 0;

    numVertex += (stripe->numHistory - 1) * stripeData->stripeDivideNum;

    s32 numLoop = stripeData->stripeHistoryStep;
    numLoop += (stripeData->stripeHistoryStep - 1) * stripeData->stripeDivideNum;

    if (numLoop >= numVertex)
        numLoop = numVertex;

    if (numLoop < 3)
        return 0;

    s32 numDrawVertex = 0;

    f32 frateScale    = 1.0f / (f32)(numLoop - 2);
    f32 frateScaleDiv = 1.0f / (f32)(stripeData->stripeDivideNum + 1);
    f32 frateScaleUv;
    if (stripeData->stripeTexCoordOpt == EFT_STRIPE_TEXCOORD_OPTION_TYPE_DIVISION &&
        stripe->cnt < stripeData->stripeHistoryStep)
    {
        s32 historyStep = (stripeData->stripeHistoryStep - 2) + (stripeData->stripeHistoryStep - 1) * stripeData->stripeDivideNum;
        frateScaleUv = 1.0f / (f32)(historyStep);
    }
    else
    {
        frateScaleUv = frateScale;
    }

    f32 arate = stripeData->stripeEndAlpha - stripeData->stripeStartAlpha;

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

    stripe->starDrawVertex = wroteVertexNum;

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

        f32 frate = (f32)i * frateScale;
        f32 frateUv = (f32)i * frateScaleUv;

        f32 t = frate * frateScaleDiv * (f32)(numVertex - 2);
        s32 iT = (s32)t;

        s32 current = stripe->histQEnd - 1 - iT;
        s32 next = current - 1;

        if (current < 0)
            current = stripeData->stripeNumHistory + current;

        if (next < 0)
            next = stripeData->stripeNumHistory + next;

        s32 prev = current + 1;
        if (prev >= stripeData->stripeNumHistory)
            prev = prev - stripeData->stripeNumHistory;

        s32 next2 = next - 1;
        if (next2 < 0)
            next2 = stripeData->stripeNumHistory + next2;

        f32 posRate = t - (s32)t;

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

        f32 alpha0 = (stripeData->stripeStartAlpha + arate * frate) * stripe->ptcl->alpha0 * stripe->ptcl->emitter->emitterSet->mColor.a * stripe->ptcl->emitter->fadeAlpha;
        f32 alpha1 = (stripeData->stripeStartAlpha + arate * frate) * stripe->ptcl->alpha1 * stripe->ptcl->emitter->emitterSet->mColor.a * stripe->ptcl->emitter->fadeAlpha;

        pos0->x = curVec.x;
        pos0->y = curVec.y;
        pos0->z = curVec.z;
        pos0->w = alpha0 * stripe->ptcl->emitter->emitterAnimValue[EFT_ANIM_ALPHA];

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
        outer1->w = -stripe->hist[ current ].scale;

        dir0->x = stripe->hist[current].dir.x * invRate + stripe->hist[next].dir.x * posRate;
        dir0->y = stripe->hist[current].dir.y * invRate + stripe->hist[next].dir.y * posRate;
        dir0->z = stripe->hist[current].dir.z * invRate + stripe->hist[next].dir.z * posRate;
        dir0->w = alpha1 * stripe->ptcl->emitter->emitterAnimValue[EFT_ANIM_ALPHA];

        dir1->x = dir0->x;
        dir1->y = dir0->y;
        dir1->z = dir0->z;
        dir1->w = dir0->w;

        tex1->x = stripe->uvParam[0].offset.x;
        tex0->y = frateUv * stripe->res->textureData[0].texVScale + stripe->uvParam[0].offset.y;
        tex0->x = stripe->res->textureData[0].texUScale + stripe->uvParam[0].offset.x;
        tex1->y = tex0->y;

        tex1->z = stripe->uvParam[1].offset.x;
        tex0->w = frateUv * stripe->res->textureData[1].texVScale + stripe->uvParam[1].offset.y;
        tex0->z = stripe->res->textureData[1].texUScale + stripe->uvParam[1].offset.x;
        tex1->w = tex0->w;

        numDrawVertex += 2;
    }

    stripe->numDrawVertex = numDrawVertex;

    return numDrawVertex;
}

bool Renderer::MakeStripeAttributeBlock(const EmitterInstance* emitter, PtclInstance* ptcl)
{
    if (ptcl->complexParam->stripe == NULL)
        return false;

    PtclStripe* stripe = ptcl->complexParam->stripe;

    stripe->uniformBlock         = NULL;
    stripe->uniformBlockForCross = NULL;
    stripe->attributeBuffer      = NULL;

    if (stripe->res == NULL || ptcl->res == NULL)
        return false;

    const StripeData* stripeData = emitter->GetStripeData();

    s32 numDivide = stripeData->stripeDivideNum;

    u32 numVertex = stripe->numHistory;
    if (numVertex == 0)
        return false;

    numVertex += (stripe->numHistory - 1) * numDivide;
    numVertex *= 2;

    nw::ut::Color4f setColor = emitter->emitterSet->GetColor();
    setColor.r *= emitter->res->colorScale;
    setColor.g *= emitter->res->colorScale;
    setColor.b *= emitter->res->colorScale;

    nw::ut::Color4f emitterColor0;
    emitterColor0.r = setColor.r * emitter->emitterAnimValue[EFT_ANIM_COLOR0_R];
    emitterColor0.g = setColor.g * emitter->emitterAnimValue[EFT_ANIM_COLOR0_G];
    emitterColor0.b = setColor.b * emitter->emitterAnimValue[EFT_ANIM_COLOR0_B];

    nw::ut::Color4f emitterColor1;
    emitterColor1.r = setColor.r * emitter->emitterAnimValue[EFT_ANIM_COLOR1_R];
    emitterColor1.g = setColor.g * emitter->emitterAnimValue[EFT_ANIM_COLOR1_G];
    emitterColor1.b = setColor.b * emitter->emitterAnimValue[EFT_ANIM_COLOR1_B];

    u32 attributeBufferSize = numVertex * sizeof(StripeVertexBuffer);
    stripe->attributeBuffer = static_cast<StripeVertexBuffer*>(AllocFromDoubleBuffer(attributeBufferSize));
    if (stripe->attributeBuffer == NULL)
        return false;

    if (numDivide == 0)
        stripe->numDrawVertex = MakeStripeAttributeBlockCore(stripe, stripe->attributeBuffer, 0, stripeData);

    else
        stripe->numDrawVertex = MakeStripeAttributeBlockCoreDivide(stripe, stripe->attributeBuffer, 0, stripeData);

    mStripeVertexCalcNum += stripe->numDrawVertex;

    stripe->uniformBlock = static_cast<StripeUniformBlock*>(AllocFromDoubleBuffer(sizeof(StripeUniformBlock)));
    if (stripe->uniformBlock == NULL)
        return false;

    f32 color0R = ptcl->color[EFT_COLOR_KIND_0].r * emitterColor0.r;
    f32 color0G = ptcl->color[EFT_COLOR_KIND_0].g * emitterColor0.g;
    f32 color0B = ptcl->color[EFT_COLOR_KIND_0].b * emitterColor0.b;

    f32 color1R = ptcl->color[EFT_COLOR_KIND_1].r * emitterColor1.r;
    f32 color1G = ptcl->color[EFT_COLOR_KIND_1].g * emitterColor1.g;
    f32 color1B = ptcl->color[EFT_COLOR_KIND_1].b * emitterColor1.b;

    {
        stripe->uniformBlock->stParam.x = 1.0f;
        stripe->uniformBlock->stParam.y = 0.0f;
        stripe->uniformBlock->stParam.z = emitter->res->offsetParam;
        stripe->uniformBlock->stParam.w = ptcl->scale.x;

        stripe->uniformBlock->uvScrollAnim.x = stripe->uvParam[EFT_TEXTURE_SLOT_0].offset.x + stripe->uvParam[EFT_TEXTURE_SLOT_0].scroll.x;
        stripe->uniformBlock->uvScrollAnim.y = stripe->uvParam[EFT_TEXTURE_SLOT_0].offset.y - stripe->uvParam[EFT_TEXTURE_SLOT_0].scroll.y;
        stripe->uniformBlock->uvScrollAnim.z = stripe->uvParam[EFT_TEXTURE_SLOT_1].offset.x + stripe->uvParam[EFT_TEXTURE_SLOT_1].scroll.x;
        stripe->uniformBlock->uvScrollAnim.w = stripe->uvParam[EFT_TEXTURE_SLOT_1].offset.y - stripe->uvParam[EFT_TEXTURE_SLOT_1].scroll.y;

        stripe->uniformBlock->uvScaleRotAnim0.x = stripe->uvParam[EFT_TEXTURE_SLOT_0].scale.x;
        stripe->uniformBlock->uvScaleRotAnim0.y = stripe->uvParam[EFT_TEXTURE_SLOT_0].scale.y;
        stripe->uniformBlock->uvScaleRotAnim0.z = stripe->uvParam[EFT_TEXTURE_SLOT_0].rotateZ;
        stripe->uniformBlock->uvScaleRotAnim0.w = 0.0f;

        stripe->uniformBlock->uvScaleRotAnim1.x = stripe->uvParam[EFT_TEXTURE_SLOT_1].scale.x;
        stripe->uniformBlock->uvScaleRotAnim1.y = stripe->uvParam[EFT_TEXTURE_SLOT_1].scale.y;
        stripe->uniformBlock->uvScaleRotAnim1.z = stripe->uvParam[EFT_TEXTURE_SLOT_1].rotateZ;
        stripe->uniformBlock->uvScaleRotAnim1.w = 0.0f;

        stripe->uniformBlock->vtxColor0.Set(color0R, color0G, color0B, ptcl->emitter->fadeAlpha);
        stripe->uniformBlock->vtxColor1.Set(color1R, color1G, color1B, ptcl->emitter->fadeAlpha);

        stripe->uniformBlock->emitterMat = nw::math::MTX44(ptcl->emitter->emitterSRT);

        GX2EndianSwap(stripe->uniformBlock, sizeof(StripeUniformBlock));
    }

    if (stripeData->stripeOption == EFT_STRIPE_OPTION_TYPE_CROSS)
    {
        stripe->uniformBlockForCross = static_cast<StripeUniformBlock*>(AllocFromDoubleBuffer(sizeof(StripeUniformBlock)));
        if (stripe->uniformBlockForCross == NULL)
        {
            stripe->uniformBlockForCross->stParam.x = 0.0f;
            stripe->uniformBlockForCross->stParam.y = 1.0f;
            stripe->uniformBlockForCross->stParam.z = emitter->res->offsetParam;
            stripe->uniformBlockForCross->stParam.w = ptcl->scale.x;

            stripe->uniformBlockForCross->uvScrollAnim.x = stripe->uvParam[EFT_TEXTURE_SLOT_0].offset.x + stripe->uvParam[EFT_TEXTURE_SLOT_0].scroll.x;
            stripe->uniformBlockForCross->uvScrollAnim.y = stripe->uvParam[EFT_TEXTURE_SLOT_0].offset.y - stripe->uvParam[EFT_TEXTURE_SLOT_0].scroll.y;
            stripe->uniformBlockForCross->uvScrollAnim.z = stripe->uvParam[EFT_TEXTURE_SLOT_1].offset.x + stripe->uvParam[EFT_TEXTURE_SLOT_1].scroll.x;
            stripe->uniformBlockForCross->uvScrollAnim.w = stripe->uvParam[EFT_TEXTURE_SLOT_1].offset.y - stripe->uvParam[EFT_TEXTURE_SLOT_1].scroll.y;

            stripe->uniformBlockForCross->uvScaleRotAnim0.x = stripe->uvParam[EFT_TEXTURE_SLOT_0].scale.x;
            stripe->uniformBlockForCross->uvScaleRotAnim0.y = stripe->uvParam[EFT_TEXTURE_SLOT_0].scale.y;
            stripe->uniformBlockForCross->uvScaleRotAnim0.z = stripe->uvParam[EFT_TEXTURE_SLOT_0].rotateZ;
            stripe->uniformBlockForCross->uvScaleRotAnim0.w = 0.0f;

            stripe->uniformBlockForCross->uvScaleRotAnim1.x = stripe->uvParam[EFT_TEXTURE_SLOT_1].scale.x;
            stripe->uniformBlockForCross->uvScaleRotAnim1.y = stripe->uvParam[EFT_TEXTURE_SLOT_1].scale.y;
            stripe->uniformBlockForCross->uvScaleRotAnim1.z = stripe->uvParam[EFT_TEXTURE_SLOT_1].rotateZ;
            stripe->uniformBlockForCross->uvScaleRotAnim1.w = 0.0f;

            stripe->uniformBlockForCross->vtxColor0.Set(color0R, color0G, color0B, ptcl->emitter->fadeAlpha);
            stripe->uniformBlockForCross->vtxColor1.Set(color1R, color1G, color1B, ptcl->emitter->fadeAlpha);

            stripe->uniformBlockForCross->emitterMat = nw::math::MTX44(ptcl->emitter->emitterSRT);

            GX2EndianSwap(stripe->uniformBlockForCross, sizeof(StripeUniformBlock));
        }
    }

    return true;
}

bool Renderer::SetupStripeDrawSetting(const EmitterInstance* emitter, void* userParam)
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

        SetupTexture(stripeShader,
                     &res->texRes[EFT_TEXTURE_SLOT_0],
                     &res->texRes[EFT_TEXTURE_SLOT_1],
                     &res->texRes[EFT_TEXTURE_SLOT_2]);
    }

    CustomShaderRenderStateSetCallback callback = mSystem->GetCustomShaderRenderStateSetCallback(static_cast<CustomShaderCallBackID>(res->userShaderSetting));
    if (callback)
    {
        RenderStateSetArg arg;
        arg.emitter = emitter;
        arg.renderer = this;
        arg.cacheFlush = true;
        arg.userParam = userParam;
        callback(arg);
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

    return true;
}

void Renderer::EntryStripe(const EmitterInstance* emitter, void* userParam)
{
    if (!emitter->res->isDisplayParent)
        return;

    ParticleShader* stripeShader = emitter->shader[mCurrentShaderType];
    if (stripeShader == NULL)
        return;

    const StripeData* spData = emitter->GetStripeData();

    mCurrentDrawingType = EFT_PTCL_TYPE_COMPLEX;

    if (emitter->res->billboardType == EFT_BILLBOARD_TYPE_COMPLEX_STRIPE)
    {
        EntryConnectionStripe(emitter, userParam);
        mRenderingEmitterNum++;
        return;
    }

    if (!SetupStripeDrawSetting(emitter, userParam))
        return;

    PtclInstance* ptcl = emitter->ptclHead;
    PtclStripe* stripe = NULL;
    u32 entryNum = 0;

    while (ptcl)
    {
        stripe = ptcl->complexParam->stripe;

        if (stripe == NULL ||
            stripe->res == NULL ||
            stripe->attributeBuffer == NULL ||
            stripe->uniformBlock == NULL ||
            stripe->numDrawVertex < 4)
        {
            goto next;
        }

        VertexBuffer::BindExtBuffer(0, sizeof(StripeVertexBuffer) * stripe->numDrawVertex, 0, sizeof(StripeVertexBuffer), stripe->attributeBuffer);

        if (stripe->uniformBlock && stripe->numDrawVertex > 0)
        {
            stripeShader->mStripeUniformBlock.BindUniformBlock(stripe->uniformBlock);
            Draw::DrawPrimitive(Draw::PRIM_TYPE_TRIANGLE_STRIP, stripe->starDrawVertex, stripe->numDrawVertex);
            mStripeVertexDrawNum += stripe->numDrawVertex - stripe->starDrawVertex;

            entryNum++;
        }

        if (stripe->uniformBlockForCross && stripe->numDrawVertex > 0 && spData->stripeType != EFT_STRIPE_TYPE_BILLBOARD)
        {
            stripeShader->mStripeUniformBlock.BindUniformBlock(stripe->uniformBlockForCross);
            Draw::DrawPrimitive(Draw::PRIM_TYPE_TRIANGLE_STRIP, stripe->starDrawVertex, stripe->numDrawVertex);
            mStripeVertexDrawNum += stripe->numDrawVertex - stripe->starDrawVertex;

            entryNum++;
        }

next:
        ptcl = ptcl->next;
    }

    mRenderingParticleNum += entryNum;
    mRenderingEmitterNum++;
}

} } // namespace nw::eft
