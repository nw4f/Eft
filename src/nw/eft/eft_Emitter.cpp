#include <nw/eft/eft_Emitter.h>
#include <nw/eft/eft_EmitterSet.h>
#include <nw/eft/eft_Shader.h>
#include <nw/eft/eft_System.h>

#if EFT_IS_CAFE_WUT || !EFT_IS_CAFE
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#endif

namespace nw { namespace eft {

System* EmitterCalc::mSys = NULL;

void EmitterInstance::Init(const SimpleEmitterData* resource)
{
    res                = resource;

    cnt                = 0.0f;
    preCnt             = 0.0f;
    emitCnt            = 0.0f;
    preEmitCnt         = 0.0f;
    emitSaving         = 0.0f;
    ptclNum            = 0;
    childPtclNum       = 0;

    emissionInterval   = res->lifeStep - rnd.GetS32(res->lifeStepRnd);
    fadeAlpha          = 1.0f;
    frameRate          = 1.0f;
    followType         = res->ptclFollowType;

    ptclHead           = NULL;
    childHead          = NULL;
    ptclTail           = NULL;
    childTail          = NULL;

    scaleRnd.x         = 0.0f;
    scaleRnd.y         = 0.0f;
    scaleRnd.z         = 0.0f;

    rotatRnd.x         = /* rnd.GetF32Range(-1.0, 1.0f) */ (rnd.GetF32() * 2.0f - 1.0f) * res->rotRnd.x;
    rotatRnd.y         = /* rnd.GetF32Range(-1.0, 1.0f) */ (rnd.GetF32() * 2.0f - 1.0f) * res->rotRnd.y;
    rotatRnd.z         = /* rnd.GetF32Range(-1.0, 1.0f) */ (rnd.GetF32() * 2.0f - 1.0f) * res->rotRnd.z;

    transRnd.x         = /* rnd.GetF32Range(-1.0, 1.0f) */ (rnd.GetF32() * 2.0f - 1.0f) * res->transRnd.x;
    transRnd.y         = /* rnd.GetF32Range(-1.0, 1.0f) */ (rnd.GetF32() * 2.0f - 1.0f) * res->transRnd.y;
    transRnd.z         = /* rnd.GetF32Range(-1.0, 1.0f) */ (rnd.GetF32() * 2.0f - 1.0f) * res->transRnd.z;

    emitterAnimArray   = (KeyFrameAnimArray*)res->animKeyTable.animKeyTable;
    emitVessel         = 0.0f;
    isEmitted          = false;
    isCalculated       = false;
    stopDraw           = false;

    for (u32 i = 0; i < EFT_SHADER_TYPE_MAX; i++)
    {
        shader[i]      = NULL;
        childShader[i] = NULL;
    }

    primitive          = NULL;
    childPrimitive     = NULL;

    animEmitterRT.SetIdentity();
    animEmitterSRT.SetIdentity();

    emitDistPrevPosSet = false;
    emitDistVessel     = 0.0f;
}

void EmitterInstance::UpdateResInfo()
{
    followType                          = res->ptclFollowType;

    emitAnimValue[EFT_ANIM_EM_RATE]     = res->emitRate;
    emitAnimValue[EFT_ANIM_LIFE]        = res->ptclLife;
    emitAnimValue[EFT_ANIM_ALL_DIR_VEL] = res->figureVel;
    emitAnimValue[EFT_ANIM_DIR_VEL]     = res->emitterVel;
    emitAnimValue[EFT_ANIM_ALPHA]       = res->alpha;
    emitAnimValue[EFT_ANIM_COLOR0_R]    = res->color0.r;
    emitAnimValue[EFT_ANIM_COLOR0_G]    = res->color0.g;
    emitAnimValue[EFT_ANIM_COLOR0_B]    = res->color0.b;
    emitAnimValue[EFT_ANIM_COLOR1_R]    = res->color1.r;
    emitAnimValue[EFT_ANIM_COLOR1_G]    = res->color1.g;
    emitAnimValue[EFT_ANIM_COLOR1_B]    = res->color1.b;
    emitAnimValue[EFT_ANIM_EM_FORM_SX]  = res->formScale.x;
    emitAnimValue[EFT_ANIM_EM_FORM_SY]  = res->formScale.y;
    emitAnimValue[EFT_ANIM_EM_FORM_SZ]  = res->formScale.z;
    emitAnimValue[EFT_ANIM_PTCL_SX]     = 1.0f;
    emitAnimValue[EFT_ANIM_PTCL_SY]     = 1.0f;
    // WTF Nintendo
    emitAnimValue[EFT_ANIM_EM_SX]       = res->scale.x;
    emitAnimValue[EFT_ANIM_EM_SY]       = res->scale.y;
    emitAnimValue[EFT_ANIM_EM_SZ]       = res->scale.z;
    emitAnimValue[EFT_ANIM_EM_RX]       = res->rot.x;
    emitAnimValue[EFT_ANIM_EM_RY]       = res->rot.y;
    emitAnimValue[EFT_ANIM_EM_RZ]       = res->rot.z;
    emitAnimValue[EFT_ANIM_EM_TX]       = res->trans.x;
    emitAnimValue[EFT_ANIM_EM_TY]       = res->trans.y;
    emitAnimValue[EFT_ANIM_EM_TZ]       = res->trans.z;
    // Casually sets the values twice...
    emitAnimValue[EFT_ANIM_EM_SX]       = res->scale.x + scaleRnd.x;
    emitAnimValue[EFT_ANIM_EM_SY]       = res->scale.y + scaleRnd.y;
    emitAnimValue[EFT_ANIM_EM_SZ]       = res->scale.z + scaleRnd.z;
    emitAnimValue[EFT_ANIM_EM_RX]       = res->rot.x   + rotatRnd.x;
    emitAnimValue[EFT_ANIM_EM_RY]       = res->rot.y   + rotatRnd.y;
    emitAnimValue[EFT_ANIM_EM_RZ]       = res->rot.z   + rotatRnd.z;
    emitAnimValue[EFT_ANIM_EM_TX]       = res->trans.x + transRnd.x;
    emitAnimValue[EFT_ANIM_EM_TY]       = res->trans.y + transRnd.y;
    emitAnimValue[EFT_ANIM_EM_TZ]       = res->trans.z + transRnd.z;

    behaviorFlag                        = 0;
    shaderArrtFlag                      = 0;
    childShaderArrtFlag                 = 0;

    for (u32 i = 0; i < EFT_SHADER_TYPE_MAX; i++)
    {
        if (shader[i] && shader[i]->GetScaleAttribute() != EFT_INVALID_ATTRIBUTE)
            shaderArrtFlag |= EFT_SHADER_ATTRIBUTE_HAS_SCALE;

        if (shader[i] && shader[i]->GetSubTexAnimAttribute() != EFT_INVALID_ATTRIBUTE) // Checks SubTex instead of Tex... mistake?
            shaderArrtFlag |= EFT_SHADER_ATTRIBUTE_HAS_TEX_ANIM;

        if (shader[i] && shader[i]->GetSubTexAnimAttribute() != EFT_INVALID_ATTRIBUTE)
            shaderArrtFlag |= EFT_SHADER_ATTRIBUTE_HAS_SUB_TEX_ANIM;

        if (shader[i] && shader[i]->GetWolrdPosAttribute() != EFT_INVALID_ATTRIBUTE)
            shaderArrtFlag |= EFT_SHADER_ATTRIBUTE_HAS_WORLD_POS;

        if (shader[i] && shader[i]->GetWolrdPosDiffAttribute() != EFT_INVALID_ATTRIBUTE)
            shaderArrtFlag |= EFT_SHADER_ATTRIBUTE_HAS_WORLD_POS_DIFF;

        if (shader[i] && shader[i]->GetColorAttribute0() != EFT_INVALID_ATTRIBUTE)
            shaderArrtFlag |= EFT_SHADER_ATTRIBUTE_HAS_COLOR0;

        if (shader[i] && shader[i]->GetColorAttribute1() != EFT_INVALID_ATTRIBUTE)
            shaderArrtFlag |= EFT_SHADER_ATTRIBUTE_HAS_COLOR1;

        if (shader[i] && shader[i]->GetRotateAttribute() != EFT_INVALID_ATTRIBUTE)
            shaderArrtFlag |= EFT_SHADER_ATTRIBUTE_HAS_ROTATE;

        if (shader[i] && shader[i]->GetEmitterMatrix0Attribute() != EFT_INVALID_ATTRIBUTE)
            shaderArrtFlag |= EFT_SHADER_ATTRIBUTE_HAS_EMITTER_MATRIX;
    }

    for (u32 i = 0; i < EFT_SHADER_TYPE_MAX; i++)
    {
        if (childShader[i] && childShader[i]->GetScaleAttribute() != EFT_INVALID_ATTRIBUTE)
            childShaderArrtFlag |= EFT_SHADER_ATTRIBUTE_HAS_SCALE;

        if (childShader[i] && childShader[i]->GetSubTexAnimAttribute() != EFT_INVALID_ATTRIBUTE) // Checks SubTex instead of Tex... mistake?
            childShaderArrtFlag |= EFT_SHADER_ATTRIBUTE_HAS_TEX_ANIM;

        if (childShader[i] && childShader[i]->GetSubTexAnimAttribute() != EFT_INVALID_ATTRIBUTE)
            childShaderArrtFlag |= EFT_SHADER_ATTRIBUTE_HAS_SUB_TEX_ANIM;

        if (childShader[i] && childShader[i]->GetWolrdPosAttribute() != EFT_INVALID_ATTRIBUTE)
            childShaderArrtFlag |= EFT_SHADER_ATTRIBUTE_HAS_WORLD_POS;

        if (childShader[i] && childShader[i]->GetWolrdPosDiffAttribute() != EFT_INVALID_ATTRIBUTE)
            childShaderArrtFlag |= EFT_SHADER_ATTRIBUTE_HAS_WORLD_POS_DIFF;

        if (childShader[i] && childShader[i]->GetColorAttribute0() != EFT_INVALID_ATTRIBUTE)
            childShaderArrtFlag |= EFT_SHADER_ATTRIBUTE_HAS_COLOR0;

        if (childShader[i] && childShader[i]->GetColorAttribute1() != EFT_INVALID_ATTRIBUTE)
            childShaderArrtFlag |= EFT_SHADER_ATTRIBUTE_HAS_COLOR1;

        if (childShader[i] && childShader[i]->GetRotateAttribute() != EFT_INVALID_ATTRIBUTE)
            childShaderArrtFlag |= EFT_SHADER_ATTRIBUTE_HAS_ROTATE;

        if (childShader[i] && childShader[i]->GetEmitterMatrix0Attribute() != EFT_INVALID_ATTRIBUTE)
            childShaderArrtFlag |= EFT_SHADER_ATTRIBUTE_HAS_EMITTER_MATRIX;
    }

    if (res->airRegist != 1.0f)
        behaviorFlag |= EFT_EMITTER_BEHAVIOR_FLAG_AIR_REGIST;

    if (res->gravity.Length() != 0.0f)
        behaviorFlag |= EFT_EMITTER_BEHAVIOR_FLAG_GRAVITY;

    if (res->ptclRotType != EFT_ROT_TYPE_NO_WORK)
        behaviorFlag |= EFT_EMITTER_BEHAVIOR_FLAG_ROTATE;

    if (res->rotRegist != 1.0f)
        behaviorFlag |= EFT_EMITTER_BEHAVIOR_FLAG_ROT_REGIST;

    if (shaderArrtFlag & EFT_SHADER_ATTRIBUTE_HAS_WORLD_POS_DIFF)
        behaviorFlag |= EFT_EMITTER_BEHAVIOR_FLAG_WLD_POSDIF;

    if (res->scaleSection1 != -127 || res->scaleSection2 != 100)
        behaviorFlag |= EFT_EMITTER_BEHAVIOR_FLAG_SCALE_ANIM;

    if (res->alphaSection1 != 0 || res->alphaSection2 != 100)
        behaviorFlag |= EFT_EMITTER_BEHAVIOR_FLAG_ALPHA_ANIM;

    if (res->colorCalcType[0] == EFT_COLOR_CALC_TYPE_RANDOM_LINEAR3COLOR)
        behaviorFlag |= EFT_EMITTER_BEHAVIOR_FLAG_COLOR0_ANIM;

    if (res->colorCalcType[1] == EFT_COLOR_CALC_TYPE_RANDOM_LINEAR3COLOR)
        behaviorFlag |= EFT_EMITTER_BEHAVIOR_FLAG_COLOR1_ANIM;

    if (res->textureData[EFT_TEXTURE_SLOT_0].uvShiftAnimMode != EFT_UV_SHIFT_ANIM_NONE)
        behaviorFlag |= EFT_EMITTER_BEHAVIOR_FLAG_TEXTURE0_UV_ANIM;

    if (res->textureData[EFT_TEXTURE_SLOT_1].uvShiftAnimMode != EFT_UV_SHIFT_ANIM_NONE)
        behaviorFlag |= EFT_EMITTER_BEHAVIOR_FLAG_TEXTURE1_UV_ANIM;

    if (res->textureData[EFT_TEXTURE_SLOT_0].isTexPatAnim)
        behaviorFlag |= EFT_EMITTER_BEHAVIOR_FLAG_TEXTURE0_PTN_ANIM;

    if (res->textureData[EFT_TEXTURE_SLOT_1].isTexPatAnim)
        behaviorFlag |= EFT_EMITTER_BEHAVIOR_FLAG_TEXTURE1_PTN_ANIM;

    if (res->texRes[EFT_TEXTURE_SLOT_1].handle)
        behaviorFlag |= EFT_EMITTER_BEHAVIOR_FLAG_TEXTURE1_CALC;

    emitterStaticUniformBlock->uvScaleInit.x = res->textureData[EFT_TEXTURE_SLOT_0].texUScale;
    emitterStaticUniformBlock->uvScaleInit.y = res->textureData[EFT_TEXTURE_SLOT_0].texVScale;
    emitterStaticUniformBlock->uvScaleInit.z = res->textureData[EFT_TEXTURE_SLOT_1].texUScale;
    emitterStaticUniformBlock->uvScaleInit.w = res->textureData[EFT_TEXTURE_SLOT_1].texVScale;

    emitterStaticUniformBlock->rotBasis.x = res->rotBasis.x;
    emitterStaticUniformBlock->rotBasis.y = res->rotBasis.y;
    emitterStaticUniformBlock->rotBasis.z = 0.0f;
    emitterStaticUniformBlock->rotBasis.w = 0.0f;

    emitterStaticUniformBlock->shaderParam.x = res->shaderParam0;
    emitterStaticUniformBlock->shaderParam.y = res->shaderParam1;
    emitterStaticUniformBlock->shaderParam.z = res->softFadeDistance;
    emitterStaticUniformBlock->shaderParam.w = res->softVolumeParam;

#if EFT_IS_CAFE
    GX2EndianSwap(emitterStaticUniformBlock, sizeof(EmitterStaticUniformBlock));
    DCFlushRange(emitterStaticUniformBlock, sizeof(EmitterStaticUniformBlock));
#endif // EFT_IS_CAFE

    if (IsHasChildParticle())
    {
        const ChildData* cres = GetChildData();

        childEmitterStaticUniformBlock->uvScaleInit.x = cres->childTexUScale;
        childEmitterStaticUniformBlock->uvScaleInit.y = cres->childTexVScale;
        childEmitterStaticUniformBlock->uvScaleInit.z = 0.0f;
        childEmitterStaticUniformBlock->uvScaleInit.w = 0.0f;

        childEmitterStaticUniformBlock->rotBasis.x = cres->childRotBasis.x;
        childEmitterStaticUniformBlock->rotBasis.y = cres->childRotBasis.y;
        childEmitterStaticUniformBlock->rotBasis.z = 0.0f;
        childEmitterStaticUniformBlock->rotBasis.w = 0.0f;

        childEmitterStaticUniformBlock->shaderParam.x = cres->childShaderParam0;
        childEmitterStaticUniformBlock->shaderParam.y = cres->childShaderParam1;
        childEmitterStaticUniformBlock->shaderParam.z = cres->childSoftFadeDistance;
        childEmitterStaticUniformBlock->shaderParam.w = cres->childSoftVolumeParam;

#if EFT_IS_CAFE
        GX2EndianSwap(childEmitterStaticUniformBlock, sizeof(EmitterStaticUniformBlock));
        DCFlushRange(childEmitterStaticUniformBlock, sizeof(EmitterStaticUniformBlock));
#endif // EFT_IS_CAFE
    }
}

void EmitterCalc::RemoveParticle(EmitterInstance* e, PtclInstance* ptcl, CpuCore core)
{
    mSys->AddPtclRemoveList(ptcl, core);
}

static inline bool _isExistKeyFrameAnim(EmitterInstance* __restrict e, AnimKeyFrameApplyType animType)
{
    KeyFrameAnimArray* keyAnimArray = e->emitterAnimArray;
    if (keyAnimArray == NULL)
        return false;

    KeyFrameAnim* info = reinterpret_cast<KeyFrameAnim*>(keyAnimArray + 1);
    if (info == NULL)
        return false;

    for (u32 i = 0; i < keyAnimArray->numAnims; ++i)
    {
        if (i != 0)
            info = reinterpret_cast<KeyFrameAnim*>((u32)info + info->offset);

        if (info->target == animType)
            return true;
    }

    return false;
}

inline f32 _sqrtSafe(f32 v)
{
    if (v <= 0.0f)
        return 0.0f;

    return nw::math::FSqrt(v);
}

void EmitterCalc::EmitCommon(EmitterInstance* __restrict e, PtclInstance* __restrict ptcl)
{
    const SimpleEmitterData* __restrict res = e->res;
    const EmitterSet*        __restrict set = e->emitterSet;

    register f32 velRand = 1.0f - e->rnd.GetF32() * res->initVelRnd * set->mRandomVel;
    register f32 dirVel  = e->emitAnimValue[EFT_ANIM_DIR_VEL] * set->mDirectionalVel;

    if (res->initPosRand != 0.0f)
    {
        nw::math::VEC3 rndVec3 = e->rnd.GetNormalizedVec3();
        ptcl->pos.x = rndVec3.x * res->initPosRand + ptcl->pos.x;
        ptcl->pos.y = rndVec3.y * res->initPosRand + ptcl->pos.y;
        ptcl->pos.z = rndVec3.z * res->initPosRand + ptcl->pos.z;
    }

    if (set->mIsSetDirectional != 0)
    {
        ptcl->vel.x = (ptcl->vel.x + set->mDirectional.x * dirVel) * velRand;
        ptcl->vel.y = (ptcl->vel.y + set->mDirectional.y * dirVel) * velRand;
        ptcl->vel.z = (ptcl->vel.z + set->mDirectional.z * dirVel) * velRand;
    }
    else
    {
        f32 angle = res->emitterVelDirAngle;
        if (angle == 0.0f)
        {
            ptcl->vel.x = (ptcl->vel.x + res->emitterVelDir.x * dirVel) * velRand;
            ptcl->vel.y = (ptcl->vel.y + res->emitterVelDir.y * dirVel) * velRand;
            ptcl->vel.z = (ptcl->vel.z + res->emitterVelDir.z * dirVel) * velRand;
        }
        else
        {
            angle = angle / 90.0f;
            angle = 1.0f - angle;

            f32 rot = e->rnd.GetF32() * 2.0f * nw::math::F_PI;
            f32 sinV;
            f32 cosV;
            nw::math::SinCosRad(&sinV, &cosV, rot);

            f32 y = e->rnd.GetF32() * (1.0f - angle) + angle;

            f32 r = _sqrtSafe(1.0f - y * y);

            nw::math::VEC3 velocity;
            velocity.x = r * cosV;
            velocity.z = r * sinV;
            velocity.y = y;

            nw::math::QUAT q;
            nw::math::QUATMakeVectorRotation(&q, nw::math::VEC3(0.0f, 1.0f, 0.0f), res->emitterVelDir);
            nw::math::MTX34 qmat;
            nw::math::QUATToMTX34(&qmat, q);
            nw::math::VEC3 srcVelocity;
            srcVelocity.Set(velocity.x, velocity.y, velocity.z);
            nw::math::VEC3Transform(&velocity, &qmat, &srcVelocity);

            ptcl->vel.x = (ptcl->vel.x + velocity.x * dirVel) * velRand;
            ptcl->vel.y = (ptcl->vel.y + velocity.y * dirVel) * velRand;
            ptcl->vel.z = (ptcl->vel.z + velocity.z * dirVel) * velRand;
        }
    }

    const nw::math::VEC3& rndVec3 = e->rnd.GetVec3();
    ptcl->vel.x += rndVec3.x * res->spreadVec.x;
    ptcl->vel.y += rndVec3.y * res->spreadVec.y;
    ptcl->vel.z += rndVec3.z * res->spreadVec.z;

    ptcl->vel.x += set->mVelAdd.x * set->mRT.m[0][0] + set->mVelAdd.y * set->mRT.m[1][0] + set->mVelAdd.z * set->mRT.m[2][0];
    ptcl->vel.y += set->mVelAdd.x * set->mRT.m[0][1] + set->mVelAdd.y * set->mRT.m[1][1] + set->mVelAdd.z * set->mRT.m[2][1];
    ptcl->vel.z += set->mVelAdd.x * set->mRT.m[0][2] + set->mVelAdd.y * set->mRT.m[1][2] + set->mVelAdd.z * set->mRT.m[2][2];

    ptcl->posDiff = ptcl->vel;

    ptcl->cnt = 0.0f;
    ptcl->rnd = e->rnd.GetU32Direct();

    if (res->ptclLife == EFT_INFINIT_LIFE)
        ptcl->life = EFT_INFINIT_LIFE;

    else
        ptcl->life = (s32)((e->emitAnimValue[EFT_ANIM_LIFE] - e->rnd.GetS32(res->ptclLifeRnd)) * e->controller->mLife);

    f32 scaleRnd   = 1.0f - res->scaleRand * e->rnd.GetF32();
    f32 initScaleX = scaleRnd * set->mEmissionParticleScale.x;
    f32 initScaleY = scaleRnd * set->mEmissionParticleScale.y;

    register s32 maxTime = ptcl->life - 1;

    if (maxTime == 0)
    {
        ptcl->alphaAnim->alphaSec1      = -1;
        ptcl->alphaAnim->alphaSec2      = 0x7fffffff;
        ptcl->alphaAnim->alphaAddSec1   = 0.0f;
        ptcl->alphaAnim->alphaAddSec2   = 0.0f;
        ptcl->alpha                     = res->initAlpha;

        ptcl->scaleAnim->scaleSec1     = -1;
        ptcl->scaleAnim->scaleSec2     = 0x7fffffff;
        ptcl->scaleAnim->scaleAddSec1  = nw::math::VEC2::Zero();
        ptcl->scaleAnim->scaleAddSec2  = nw::math::VEC2::Zero();

        if (_isExistKeyFrameAnim(e, EFT_ANIM_PTCL_SX))
        {
            ptcl->scale.x = res->initScale.x * initScaleX * e->emitAnimValue[EFT_ANIM_PTCL_SX];
            ptcl->scale.y = res->initScale.y * initScaleY * e->emitAnimValue[EFT_ANIM_PTCL_SY];
        }
        else
        {
            ptcl->scale.x = res->initScale.x * initScaleX * res->baseScale.x;
            ptcl->scale.y = res->initScale.y * initScaleY * res->baseScale.y;
        }
    }
    else
    {
        ptcl->alphaAnim->alphaSec1 = (res->alphaSection1 * maxTime) / 100;
        ptcl->alphaAnim->alphaSec2 = (res->alphaSection2 * maxTime) / 100;

        if (ptcl->alphaAnim->alphaSec1 == 0)
            ptcl->alphaAnim->alphaAddSec1 = 0.0f;
        else
            ptcl->alphaAnim->alphaAddSec1 = res->diffAlpha21 / (f32)ptcl->alphaAnim->alphaSec1;

        if (res->alphaSection2 == 100)
            ptcl->alphaAnim->alphaAddSec2 = 0.0f;
        else
            ptcl->alphaAnim->alphaAddSec2 = res->diffAlpha32 / (f32)(maxTime - ptcl->alphaAnim->alphaSec2);

        ptcl->alpha = res->initAlpha - ptcl->alphaAnim->alphaAddSec1;

        register f32 scaleX = 1.0f;
        register f32 scaleY = 1.0f;
        register f32 scaleAddSec1X = 0.0f;
        register f32 scaleAddSec1Y = 0.0f;
        register f32 scaleAddSec2X = 0.0f;
        register f32 scaleAddSec2Y = 0.0f;

        ptcl->scaleAnim->scaleSec1 = 0; // ???
        ptcl->scaleAnim->scaleSec2 = 0; // ^^^

        ptcl->scaleAnim->scaleSec1 = (res->scaleSection1 * maxTime) / 100;
        ptcl->scaleAnim->scaleSec2 = (res->scaleSection2 * maxTime) / 100;

        f32 invScaleSec1 = 1.0f / (f32)ptcl->scaleAnim->scaleSec1;
        f32 invScaleSec2 = 1.0f / (f32)(maxTime - ptcl->scaleAnim->scaleSec2);

        scaleAddSec1X = res->diffScale21.x * invScaleSec1;
        scaleAddSec1Y = res->diffScale21.y * invScaleSec1;

        scaleAddSec2X = res->diffScale32.x * invScaleSec2;
        scaleAddSec2Y = res->diffScale32.y * invScaleSec2;

        scaleX = res->initScale.x - scaleAddSec1X;
        scaleY = res->initScale.y - scaleAddSec1Y;

        if (_isExistKeyFrameAnim(e, EFT_ANIM_PTCL_SX))
        {
            ptcl->scaleAnim->scaleAddSec1.x = scaleAddSec1X * e->emitAnimValue[EFT_ANIM_PTCL_SX] * initScaleX;
            ptcl->scaleAnim->scaleAddSec1.y = scaleAddSec1Y * e->emitAnimValue[EFT_ANIM_PTCL_SY] * initScaleY;
            ptcl->scaleAnim->scaleAddSec2.x = scaleAddSec2X * e->emitAnimValue[EFT_ANIM_PTCL_SX] * initScaleX;
            ptcl->scaleAnim->scaleAddSec2.y = scaleAddSec2Y * e->emitAnimValue[EFT_ANIM_PTCL_SY] * initScaleY;
            ptcl->scale.x                   = scaleX * e->emitAnimValue[EFT_ANIM_PTCL_SX] * initScaleX;
            ptcl->scale.y                   = scaleY * e->emitAnimValue[EFT_ANIM_PTCL_SY] * initScaleY;
        }
        else
        {
            ptcl->scaleAnim->scaleAddSec1.x = scaleAddSec1X * res->baseScale.x * initScaleX;
            ptcl->scaleAnim->scaleAddSec1.y = scaleAddSec1Y * res->baseScale.y * initScaleY;
            ptcl->scaleAnim->scaleAddSec2.x = scaleAddSec2X * res->baseScale.x * initScaleX;
            ptcl->scaleAnim->scaleAddSec2.y = scaleAddSec2Y * res->baseScale.y * initScaleY;
            ptcl->scale.x                   = scaleX * res->baseScale.x * initScaleX;
            ptcl->scale.y                   = scaleY * res->baseScale.y * initScaleY;
        }
    }

    ptcl->rot.x = res->initRot.x + e->rnd.GetF32() * res->initRotRand.x + set->mInitialRoate.x;
    ptcl->rot.y = res->initRot.y + e->rnd.GetF32() * res->initRotRand.y + set->mInitialRoate.y;
    ptcl->rot.z = res->initRot.z + e->rnd.GetF32() * res->initRotRand.z + set->mInitialRoate.z;

    ptcl->rotVel.x = 0; // ???
    ptcl->rotVel.y = 0; // ^^^
    ptcl->rotVel.z = 0; // ^^^

    ptcl->rotVel.x = res->rotVel.x + e->rnd.GetF32() * res->rotVelRand.x;
    ptcl->rotVel.y = res->rotVel.y + e->rnd.GetF32() * res->rotVelRand.y;
    ptcl->rotVel.z = res->rotVel.z + e->rnd.GetF32() * res->rotVelRand.z;

    if (res->colorCalcType[EFT_COLOR_KIND_0] == EFT_COLOR_CALC_TYPE_RANDOM)
    {
        u32 id = ptcl->rnd % 3;
        ptcl->color[EFT_COLOR_KIND_0].r = res->color[EFT_COLOR_KIND_0][id].r;
        ptcl->color[EFT_COLOR_KIND_0].g = res->color[EFT_COLOR_KIND_0][id].g;
        ptcl->color[EFT_COLOR_KIND_0].b = res->color[EFT_COLOR_KIND_0][id].b;
    }
    else
    {
        ptcl->color[EFT_COLOR_KIND_0].r = res->color[EFT_COLOR_KIND_0][0].r;
        ptcl->color[EFT_COLOR_KIND_0].g = res->color[EFT_COLOR_KIND_0][0].g;
        ptcl->color[EFT_COLOR_KIND_0].b = res->color[EFT_COLOR_KIND_0][0].b;
    }

    if (res->colorCalcType[EFT_COLOR_KIND_1] == EFT_COLOR_CALC_TYPE_RANDOM)
    {
        u32 id = ptcl->rnd % 3;
        ptcl->color[EFT_COLOR_KIND_1].r = res->color[EFT_COLOR_KIND_1][id].r;
        ptcl->color[EFT_COLOR_KIND_1].g = res->color[EFT_COLOR_KIND_1][id].g;
        ptcl->color[EFT_COLOR_KIND_1].b = res->color[EFT_COLOR_KIND_1][id].b;
    }
    else
    {
        ptcl->color[EFT_COLOR_KIND_1].r = res->color[EFT_COLOR_KIND_1][0].r;
        ptcl->color[EFT_COLOR_KIND_1].g = res->color[EFT_COLOR_KIND_1][0].g;
        ptcl->color[EFT_COLOR_KIND_1].b = res->color[EFT_COLOR_KIND_1][0].b;
    }

    ptcl->uvScroll.x        = res->textureData[0].uvScrollInit.x + res->textureData[0].uvScrollInitRand.x - e->rnd.GetF32() * res->textureData[0].uvScrollInitRand.x * 2.0f;
    ptcl->uvScroll.y        = res->textureData[0].uvScrollInit.y + res->textureData[0].uvScrollInitRand.y - e->rnd.GetF32() * res->textureData[0].uvScrollInitRand.y * 2.0f;
    ptcl->uvScale.x         = res->textureData[0].uvScaleInit.x  + res->textureData[0].uvScaleInitRand.x  - e->rnd.GetF32() * res->textureData[0].uvScaleInitRand.x  * 2.0f;
    ptcl->uvScale.y         = res->textureData[0].uvScaleInit.y  + res->textureData[0].uvScaleInitRand.y  - e->rnd.GetF32() * res->textureData[0].uvScaleInitRand.y  * 2.0f;
    ptcl->uvRotateZ         = res->textureData[0].uvRotInit      + res->textureData[0].uvRotInitRand      - e->rnd.GetF32() * res->textureData[0].uvRotInitRand      * 2.0f;

    ptcl->uvSubScroll.x     = res->textureData[1].uvScrollInit.x + res->textureData[1].uvScrollInitRand.x - e->rnd.GetF32() * res->textureData[1].uvScrollInitRand.x * 2.0f;
    ptcl->uvSubScroll.y     = res->textureData[1].uvScrollInit.y + res->textureData[1].uvScrollInitRand.y - e->rnd.GetF32() * res->textureData[1].uvScrollInitRand.y * 2.0f;
    ptcl->uvSubScale.x      = res->textureData[1].uvScaleInit.x  + res->textureData[1].uvScaleInitRand.x  - e->rnd.GetF32() * res->textureData[1].uvScaleInitRand.x  * 2.0f;
    ptcl->uvSubScale.y      = res->textureData[1].uvScaleInit.y  + res->textureData[1].uvScaleInitRand.y  - e->rnd.GetF32() * res->textureData[1].uvScaleInitRand.y  * 2.0f;
    ptcl->uvSubRotateZ      = res->textureData[1].uvRotInit      + res->textureData[1].uvRotInitRand      - e->rnd.GetF32() * res->textureData[1].uvRotInitRand      * 2.0f;

    if (e->followType != EFT_FOLLOW_TYPE_ALL)
    {
        nw::math::MTX34Copy(&ptcl->emitterRT, &e->emitterRT);
        nw::math::MTX34Copy(&ptcl->emitterSRT, &e->emitterSRT);

        ptcl->coordinateEmitterRT = &ptcl->emitterRT;
        ptcl->coordinateEmitterSRT = &ptcl->emitterSRT;
    }
    else
    {
        ptcl->coordinateEmitterRT = &e->emitterRT;
        ptcl->coordinateEmitterSRT = &e->emitterSRT;
    }

    if (res->textureData[0].numTexPat <= 1)
    {
        ptcl->uvOffset.x = 0.0f;
        ptcl->uvOffset.y = 0.0f;
    }
    else
    {
        s32 no   = e->rnd.GetS32(res->textureData[0].numTexPat);
        s32 no_x = no % res->textureData[0].numTexDivX;
        s32 no_y = no / res->textureData[0].numTexDivX;

        register f32 no_x_f = (f32)no_x;
        register f32 no_y_f = (f32)no_y;

        ptcl->uvOffset.x = res->textureData[0].texUScale * no_x_f;
        ptcl->uvOffset.y = res->textureData[0].texVScale * no_y_f;
    }

    if (res->textureData[1].numTexPat <= 1)
    {
        ptcl->uvSubOffset.x = 0.0f;
        ptcl->uvSubOffset.y = 0.0f;
    }
    else
    {
        s32 no   = e->rnd.GetS32(res->textureData[1].numTexPat);
        s32 no_x = no % res->textureData[1].numTexDivX;
        s32 no_y = no / res->textureData[1].numTexDivX;

        register f32 no_x_f = (f32)no_x;
        register f32 no_y_f = (f32)no_y;

        ptcl->uvSubOffset.x = res->textureData[1].texUScale * no_x_f;
        ptcl->uvSubOffset.y = res->textureData[1].texVScale * no_y_f;
    }

    if (res->billboardType == EFT_BILLBOARD_TYPE_STRIPE)
        ptcl->stripe = mSys->AllocAndConnectStripe(e, ptcl);

    ptcl->fluctuationAlpha = 1.0f;
    ptcl->fluctuationScale = 1.0f;
    ptcl->runtimeUserData  = 0;
    ptcl->childEmitCnt     = 1000000.0f;
    ptcl->childPreEmitCnt  = 0.0f;
    ptcl->childEmitSaving  = 0.0f;
    ptcl->dynamicsRnd      = 1.0f + res->dynamicsRandom - res->dynamicsRandom * e->rnd.GetF32() * 2.0f;

    ptcl->emitter          = e;

    UserDataParticleEmitCallback particleEmitCB = mSys->GetCurrentUserDataParticleEmitCallback(e);
    if (particleEmitCB)
    {
        ParticleEmitArg arg;
        arg.particle = ptcl;

        if (!particleEmitCB(arg))
            return RemoveParticle(e, ptcl, EFT_CPU_CORE_1);
    }

    AddParticle(e, ptcl);
}

} } // namespace nw::eft

#if EFT_IS_CAFE_WUT || !EFT_IS_CAFE
#pragma GCC diagnostic pop
#endif
