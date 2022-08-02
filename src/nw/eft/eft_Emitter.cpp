#include <nw/eft/eft_Emitter.h>
#include <nw/eft/eft_EmitterSet.h>
#include <nw/eft/eft_Misc.h>
#include <nw/eft/eft_Primitive.h>
#include <nw/eft/eft_Shader.h>

namespace nw { namespace eft {

System* EmitterCalc::mSys = NULL;

void EmitterInstance::Init(const SimpleEmitterData* resource)
{
    res                     = resource;
    emissionInterval        = resource->emitStep - rnd.GetS32(resource->emitStepRnd);
    fadeRequestFrame        = -1.0f;
    emitterInitialRandom    = rnd.GetF32();
    fadeAlpha               = 1.0f;
    frameRate               = 1.0f;
    streamOutCounter        = 0;
    emitDistPrevPos.x       = emitterSet->GetRTMatrix()._03;
    emitDistPrevPos.y       = emitterSet->GetRTMatrix()._13;
    emitDistPrevPos.z       = emitterSet->GetRTMatrix()._23;
    followType              = res->ptclFollowType;

    animValEmitterRT.SetIdentity();
    animValEmitterSRT.SetIdentity();

    animArrayEmitter        = (KeyFrameAnimArray*)res->animKeyTable.animKeyTable;

    if (animArrayEmitter)
    {
        KeyFrameAnim* anim = _getFirstKeyFrameAnim(animArrayEmitter);
        for (u32 i = 0; i < animArrayEmitter->numAnims; i++)
        {
            if (anim->target < EFT_ANIM_EMITTER_MAX)
            {
                emitterAnimKey[emitterAnimNum++] = anim;

                if (anim->target == EFT_ANIM_EM_TX)
                    emitterBehaviorFlag |= EFT_EMITTER_BEHAVIOR_FLAG_MATRIX_TRANS_ANIM;

                if (anim->target == EFT_ANIM_EM_RX)
                    emitterBehaviorFlag |= EFT_EMITTER_BEHAVIOR_FLAG_MATRIX_ROTATE_ANIM;

                if (anim->target == EFT_ANIM_EM_TX || anim->target == EFT_ANIM_EM_RX || anim->target == EFT_ANIM_EM_SX)
                    emitterBehaviorFlag |= EFT_EMITTER_BEHAVIOR_FLAG_MATRIX_ANIM;
            }

            if (anim->target > EFT_ANIM_EMITTER_MAX)
                particleAnimKey[EFT_ANIM_8KEY_OFFSET(anim->target)] = anim;

            anim = _getNextKeyFrameAnim(anim);
        }
    }
}

#define TEXTURE0_PTN_START_RANDAM               0x00000001
#define TEXTURE1_PTN_START_RANDAM               0x00000002
#define TEXTURE0_PTN_ENABLE_LIFE_FIT            0x00000004
#define TEXTURE0_PTN_ENABLE_CLAMP               0x00000008
#define TEXTURE0_PTN_ENABLE_LOOP                0x00000010
#define TEXTURE0_PTN_ENABLE_RANDOM              0x00000020
#define TEXTURE1_PTN_ENABLE_LIFE_FIT            0x00000040
#define TEXTURE1_PTN_ENABLE_CLAMP               0x00000080
#define TEXTURE1_PTN_ENABLE_LOOP                0x00000100
#define TEXTURE1_PTN_ENABLE_RANDOM              0x00000200

#define TEXTURE0_INV_U_RANDOM                   0x00000400
#define TEXTURE0_INV_V_RANDOM                   0x00000800
#define TEXTURE1_INV_U_RANDOM                   0x00001000
#define TEXTURE1_INV_V_RANDOM                   0x00002000
#define TEXTURE2_INV_U_RANDOM                   0x00004000
#define TEXTURE2_INV_V_RANDOM                   0x00008000

#define TEXTURE0_SURFACE_NO_CROSSFADE           0x00010000
#define TEXTURE1_SURFACE_NO_CROSSFADE           0x00020000

#define ROTATE_DIR_RANDOM_X                     0x00040000
#define ROTATE_DIR_RANDOM_Y                     0x00080000
#define ROTATE_DIR_RANDOM_Z                     0x00100000

#define FLUCTUATION_X_TYPE_SIN                  0x00200000
#define FLUCTUATION_X_TYPE_SAWTOOTH             0x00400000
#define FLUCTUATION_X_TYPE_RECT                 0x00800000
#define FLUCTUATION_Y_TYPE_SIN                  0x01000000
#define FLUCTUATION_Y_TYPE_SAWTOOTH             0x02000000
#define FLUCTUATION_Y_TYPE_RECT                 0x04000000

#define PRIMITIVE_SCALE_Y_TO_Z                  0x08000000

#define TEXTURE0_PTN_ENABLE_SURFACE             0x10000000
#define TEXTURE1_PTN_ENABLE_SURFACE             0x20000000

void _copyTextureShiftAnimParm(TexUvShiftAnimUbo* ubo, u32* flag, const TextureEmitterData* data, TextureSlot slot, u8 fixedRandom)
{
    ubo->scrollInit.x       = data->uvScrollInit.x;
    ubo->scrollInit.y       = data->uvScrollInit.y;
    ubo->scrollInitRand.x   = data->uvScrollInitRand.x;
    ubo->scrollInitRand.y   = data->uvScrollInitRand.y;
    ubo->scrollAdd.x        = data->uvScroll.x;
    ubo->scrollAdd.y        = data->uvScroll.y;
    ubo->rotInit            = data->uvRotInit;
    ubo->rotInitRand        = data->uvRotInitRand;
    ubo->rotAdd             = data->uvRot;
    ubo->scaleInit.x        = data->uvScaleInit.x;
    ubo->scaleInit.y        = data->uvScaleInit.y;
    ubo->scaleInitRand.x    = data->uvScaleInitRand.x;
    ubo->scaleInitRand.y    = data->uvScaleInitRand.y;
    ubo->scaleAdd.x         = data->uvScale.x;
    ubo->scaleAdd.y         = data->uvScale.y;
    ubo->uvScale.x          = data->texUScale;
    ubo->uvScale.y          = data->texVScale;
    ubo->uvDiv.x            = data->numTexDivX;
    ubo->uvDiv.y            = data->numTexDivY;

    if (slot == EFT_TEXTURE_SLOT_0)
    {
        if (data->isTexURandomInv)
            *flag |= TEXTURE0_INV_U_RANDOM;

        if (data->isTexVRandomInv)
            *flag |= TEXTURE0_INV_V_RANDOM;

        ubo->randomType = 0.0f;
    }

    if (slot == EFT_TEXTURE_SLOT_1)
    {
        if (data->isTexURandomInv)
            *flag |= TEXTURE1_INV_U_RANDOM;

        if (data->isTexVRandomInv)
            *flag |= TEXTURE1_INV_V_RANDOM;

        ubo->randomType = 1.0f;
    }

    if (slot == EFT_TEXTURE_SLOT_2)
    {
        if (data->isTexURandomInv)
            *flag |= TEXTURE2_INV_U_RANDOM;

        if (data->isTexVRandomInv)
            *flag |= TEXTURE2_INV_V_RANDOM;

        ubo->randomType = 2.0f;
    }

    if (fixedRandom)
        ubo->randomType = 0.0f;
}

void _copyTexturePtnAnimParm(TexPtnAnimUbo* ubo, u32* flag, const TextureEmitterData* data, TextureSlot slot)
{
    u32 bitPtnStartRandom       = TEXTURE0_PTN_START_RANDAM;
    u32 bitPtnEnableLifeFit     = TEXTURE0_PTN_ENABLE_LIFE_FIT;
    u32 bitPtnEnableClamp       = TEXTURE0_PTN_ENABLE_CLAMP;
    u32 bitPtnEnableLoop        = TEXTURE0_PTN_ENABLE_LOOP;
    u32 bitPtnEnableRandom      = TEXTURE0_PTN_ENABLE_RANDOM;
    u32 bitSurDisableCrossFade  = TEXTURE0_SURFACE_NO_CROSSFADE;
    u32 bitPtnEnableSurface     = TEXTURE0_PTN_ENABLE_SURFACE;

    if (slot == EFT_TEXTURE_SLOT_1)
    {
        bitPtnStartRandom       = TEXTURE1_PTN_START_RANDAM;
        bitPtnEnableLifeFit     = TEXTURE1_PTN_ENABLE_LIFE_FIT;
        bitPtnEnableClamp       = TEXTURE1_PTN_ENABLE_CLAMP;
        bitPtnEnableLoop        = TEXTURE1_PTN_ENABLE_LOOP;
        bitPtnEnableRandom      = TEXTURE1_PTN_ENABLE_RANDOM;
        bitSurDisableCrossFade  = TEXTURE1_SURFACE_NO_CROSSFADE;
        bitPtnEnableSurface     = TEXTURE1_PTN_ENABLE_SURFACE;
    }

    if (data->isTexPatAnimRand)
        *flag |= bitPtnStartRandom;

    switch (data->texPtnAnimMode)
    {
    case EFT_TEX_PTN_ANIM_LIFEFIT: *flag |= bitPtnEnableLifeFit; break;
    case EFT_TEX_PTN_ANIM_CLAMP:   *flag |= bitPtnEnableClamp;   break;
    case EFT_TEX_PTN_ANIM_LOOP:    *flag |= bitPtnEnableLoop;    break;
    case EFT_TEX_PTN_ANIM_RANDOM:  *flag |= bitPtnEnableRandom;  break;
    case EFT_TEX_PTN_ANIM_SURFACE: *flag |= bitPtnEnableSurface; break;
    }

    if (data->isCrossFade)
        *flag |= bitSurDisableCrossFade;

    ubo->ptnFreq      = data->texPatFreq;
    ubo->ptnTableNum  = data->texPatTblUse;
    ubo->ptnNum       = data->numTexPat;

    for (u32 i = 0; i < EFT_TEXTURE_PATTERN_NUM; i++)
        ubo->ptnTable[i] = data->texPatTbl[i];
}

void EmitterInstance::UpdateEmitterStaticUniformBlock(EmitterStaticUniformBlock* emitterStaticUniformBlock, const SimpleEmitterData* emitterData, const ComplexEmitterData* complexEmitterData)
{
    DCZeroRange(emitterStaticUniformBlock, sizeof(EmitterStaticUniformBlock));

    EmitterStaticUniformBlock* ubo = emitterStaticUniformBlock;
    u32 flag = 0;

    _copyTextureShiftAnimParm(&ubo->texShiftAnim0, &flag, &emitterData->textureData[EFT_TEXTURE_SLOT_0], EFT_TEXTURE_SLOT_0, emitterData->useFixedUVRandomAnimation);
    _copyTextureShiftAnimParm(&ubo->texShiftAnim1, &flag, &emitterData->textureData[EFT_TEXTURE_SLOT_1], EFT_TEXTURE_SLOT_1, emitterData->useFixedUVRandomAnimation);
    _copyTextureShiftAnimParm(&ubo->texShiftAnim2, &flag, &emitterData->textureData[EFT_TEXTURE_SLOT_2], EFT_TEXTURE_SLOT_2, emitterData->useFixedUVRandomAnimation);

    _copyTexturePtnAnimParm(&ubo->texPtnAnim0, &flag, &emitterData->textureData[EFT_TEXTURE_SLOT_0], EFT_TEXTURE_SLOT_0);
    _copyTexturePtnAnimParm(&ubo->texPtnAnim1, &flag, &emitterData->textureData[EFT_TEXTURE_SLOT_1], EFT_TEXTURE_SLOT_1);

    if (emitterData->isRotDirRand[0])
        flag |= ROTATE_DIR_RANDOM_X;

    if (emitterData->isRotDirRand[1])
        flag |= ROTATE_DIR_RANDOM_Y;

    if (emitterData->isRotDirRand[2])
        flag |= ROTATE_DIR_RANDOM_Z;

    if (emitterData->primitiveScaleY2Z)
        flag |= PRIMITIVE_SCALE_Y_TO_Z;

    ubo->rotBasis.x = emitterData->rotBasis.x;
    ubo->rotBasis.y = emitterData->rotBasis.y;
    ubo->rotBasis.z = emitterData->offsetParam;
    ubo->rotBasis.w = 0.0f;

    ubo->fresnelMinMax.x = emitterData->fresnelAlphaMin;
    ubo->fresnelMinMax.y = emitterData->fresnelAlphaMax;
    ubo->fresnelMinMax.z = emitterData->softAlphaOffset;
    ubo->fresnelMinMax.w = emitterData->decalVolume;

    ubo->nearAlpha.x = emitterData->nearAlphaMin;
    ubo->nearAlpha.y = emitterData->nearAlphaMax;
    ubo->nearAlpha.z = emitterData->farAlphaMin;
    ubo->nearAlpha.w = emitterData->farAlphaMax;

    ubo->rotateVel.x = emitterData->rotVel.x;
    ubo->rotateVel.y = emitterData->rotVel.y;
    ubo->rotateVel.z = emitterData->rotVel.z;
    ubo->rotateVel.w = emitterData->rotRegist;

    ubo->rotateVelRand.x = emitterData->rotVelRand.x;
    ubo->rotateVelRand.y = emitterData->rotVelRand.y;
    ubo->rotateVelRand.z = emitterData->rotVelRand.z;
    ubo->rotateVelRand.w = 0.0f;

    ubo->alphaAnim0.x = emitterData->alpha3v4kAnim[EFT_ALPHA_KIND_0].init;
    ubo->alphaAnim0.y = emitterData->alpha3v4kAnim[EFT_ALPHA_KIND_0].init + emitterData->alpha3v4kAnim[EFT_ALPHA_KIND_0].diff21;
    ubo->alphaAnim0.z = emitterData->alpha3v4kAnim[EFT_ALPHA_KIND_0].section1;
    ubo->alphaAnim0.w = 0.0f;
    ubo->alphaAnim1.x = emitterData->alpha3v4kAnim[EFT_ALPHA_KIND_0].init + emitterData->alpha3v4kAnim[EFT_ALPHA_KIND_0].diff21 + emitterData->alpha3v4kAnim[EFT_ALPHA_KIND_0].diff32;
    ubo->alphaAnim1.y = emitterData->alpha3v4kAnim[EFT_ALPHA_KIND_0].section2;

    ubo->alphaAnim1.z = emitterData->alpha3v4kAnim[EFT_ALPHA_KIND_1].init;
    ubo->alphaAnim1.w = emitterData->alpha3v4kAnim[EFT_ALPHA_KIND_1].init + emitterData->alpha3v4kAnim[EFT_ALPHA_KIND_1].diff21;
    ubo->alphaAnim2.x = emitterData->alpha3v4kAnim[EFT_ALPHA_KIND_1].section1;
  //ubo->alphaAnim2.  = 0.0f;
    ubo->alphaAnim2.y = emitterData->alpha3v4kAnim[EFT_ALPHA_KIND_1].init + emitterData->alpha3v4kAnim[EFT_ALPHA_KIND_1].diff21 + emitterData->alpha3v4kAnim[EFT_ALPHA_KIND_1].diff32;
    ubo->alphaAnim2.z = emitterData->alpha3v4kAnim[EFT_ALPHA_KIND_1].section2;

    ubo->alphaAnim2.w = (f32)emitterData->colorNumRepeat[EFT_COLOR_KIND_0];

    ubo->scaleAnim0.x = emitterData->baseScale.x;
    ubo->scaleAnim0.y = emitterData->baseScale.y;
    ubo->scaleAnim0.z = emitterData->initScale.x;
    ubo->scaleAnim0.w = emitterData->initScale.y;

    ubo->scaleAnim1.x = (f32)emitterData->scaleSection1 / 100.0f;
    ubo->scaleAnim1.y = emitterData->initScale.x + emitterData->diffScale21.x;
    ubo->scaleAnim1.z = emitterData->initScale.y + emitterData->diffScale21.y;
    ubo->scaleAnim1.w = emitterData->scaleRand.x;

    ubo->scaleAnim2.x = (f32)emitterData->scaleSection2 / 100.0f;
    ubo->scaleAnim2.y = emitterData->initScale.x + emitterData->diffScale21.x + emitterData->diffScale32.x;
    ubo->scaleAnim2.z = emitterData->initScale.y + emitterData->diffScale21.y + emitterData->diffScale32.y;
    ubo->scaleAnim2.w = emitterData->scaleRand.y;

    for (u32 i = 0; i < 8; i++)
    {
        ubo->color0[i].x = emitterData->color[EFT_COLOR_KIND_0][i].r;
        ubo->color0[i].y = emitterData->color[EFT_COLOR_KIND_0][i].g;
        ubo->color0[i].z = emitterData->color[EFT_COLOR_KIND_0][i].b;
        ubo->color0[i].w = emitterData->color[EFT_COLOR_KIND_0][i].a;
        ubo->color1[i].x = emitterData->color[EFT_COLOR_KIND_1][i].r;
        ubo->color1[i].y = emitterData->color[EFT_COLOR_KIND_1][i].g;
        ubo->color1[i].z = emitterData->color[EFT_COLOR_KIND_1][i].b;
        ubo->color1[i].w = emitterData->color[EFT_COLOR_KIND_1][i].a;
    }

    ubo->color0Anim.x = (f32)emitterData->colorSection1[EFT_COLOR_KIND_0] / 100.0f;
    ubo->color0Anim.y = (f32)emitterData->colorSection2[EFT_COLOR_KIND_0] / 100.0f;
    ubo->color0Anim.z = (f32)emitterData->colorSection3[EFT_COLOR_KIND_0] / 100.0f;

    ubo->color0Anim.w = emitterData->colorScale;

    ubo->color1Anim.x = (f32)emitterData->colorSection1[EFT_COLOR_KIND_1] / 100.0f;
    ubo->color1Anim.y = (f32)emitterData->colorSection2[EFT_COLOR_KIND_1] / 100.0f;
    ubo->color1Anim.z = (f32)emitterData->colorSection3[EFT_COLOR_KIND_1] / 100.0f;

    ubo->color1Anim.w = (f32)emitterData->colorNumRepeat[EFT_COLOR_KIND_1];

    ubo->vectorParam.x = emitterData->emitAirRegist;
    ubo->vectorParam.y = emitterData->emitVelDir.x;
    ubo->vectorParam.z = emitterData->emitVelDir.y;
    ubo->vectorParam.w = emitterData->emitVelDir.z;

    ubo->gravityParam.x = emitterData->gravity.x;
    ubo->gravityParam.y = emitterData->gravity.y;
    ubo->gravityParam.z = emitterData->gravity.z;
    ubo->gravityParam.w = emitterData->scaleVelAddY;

    if (complexEmitterData && complexEmitterData->fluctuationFlg & EFT_FLUCTUATION_FALG_ENABLE)
    {
        const FluctuationData* fres = reinterpret_cast<const FluctuationData*>((u32)complexEmitterData + complexEmitterData->fluctuationDataOffset);

        ubo->fluctuationX.scale         = fres->fluctuationScaleX;
        ubo->fluctuationX.freq          = EFT_FLUCTUATION_TABLE_NUM / fres->fluctuationFreqX;
        ubo->fluctuationX.phase         = fres->fluctuationPhaseInitX;
        ubo->fluctuationX.phaseRandom   = (f32)fres->fluctuationPhaseRndX;

        ubo->fluctuationY.scale         = fres->fluctuationScaleY;
        ubo->fluctuationY.freq          = EFT_FLUCTUATION_TABLE_NUM / fres->fluctuationFreqY;
        ubo->fluctuationY.phase         = fres->fluctuationPhaseInitY;
        ubo->fluctuationY.phaseRandom   = (f32)fres->fluctuationPhaseRndY;

        if (complexEmitterData->fluctuationFlg & EFT_FLUCTUATION_FALG_USE_SIN_WAVE)
            flag |= FLUCTUATION_X_TYPE_SIN;

        if (complexEmitterData->fluctuationFlg & EFT_FLUCTUATION_FALG_USE_SIN_WAVE)
            flag |= FLUCTUATION_Y_TYPE_SIN;

        if (complexEmitterData->fluctuationFlg & EFT_FLUCTUATION_FALG_USE_SAW_TOOTH_WAVE)
            flag |= FLUCTUATION_X_TYPE_SAWTOOTH;

        if (complexEmitterData->fluctuationFlg & EFT_FLUCTUATION_FALG_USE_SAW_TOOTH_WAVE)
            flag |= FLUCTUATION_Y_TYPE_SAWTOOTH;

        if (complexEmitterData->fluctuationFlg & EFT_FLUCTUATION_FALG_USE_RECT_WAVE)
            flag |= FLUCTUATION_X_TYPE_RECT;

        if (complexEmitterData->fluctuationFlg & EFT_FLUCTUATION_FALG_USE_RECT_WAVE)
            flag |= FLUCTUATION_Y_TYPE_RECT;
    }

    if (complexEmitterData && complexEmitterData->fieldFlg != 0)
    {
        const void* fres = reinterpret_cast<const void*>((u32)complexEmitterData + complexEmitterData->fieldDataOffset);

        if (complexEmitterData->fieldFlg & EFT_FIELD_MASK_RANDOM)
        {
            const FieldRandomData* dat = reinterpret_cast<const FieldRandomData*>(fres);

            ubo->fieldRandom.x = dat->fieldRandomVelAdd.x;
            ubo->fieldRandom.y = dat->fieldRandomVelAdd.y;
            ubo->fieldRandom.z = dat->fieldRandomVelAdd.z;
            ubo->fieldRandom.w = (f32)dat->fieldRandomBlank;

            fres = dat + 1;
        }

        if (complexEmitterData->fieldFlg & EFT_FIELD_MASK_MAGNET)
        {
            const FieldMagnetData* dat = reinterpret_cast<const FieldMagnetData*>(fres);

            ubo->fieldMagnet.x = dat->fieldMagnetPos.x;
            ubo->fieldMagnet.y = dat->fieldMagnetPos.y;
            ubo->fieldMagnet.z = dat->fieldMagnetPos.z;
            ubo->fieldMagnet.w = dat->fieldMagnetPower;

            if (dat->isFollowEmitter)
                ubo->fieldCoordEmitter.y  = 1.0f;

            fres = dat + 1;
        }

        if (complexEmitterData->fieldFlg & EFT_FIELD_MASK_SPIN)
        {
            const FieldSpinData* dat = reinterpret_cast<const FieldSpinData*>(fres);

            ubo->fieldSpin.x = NW_MATH_IDX_TO_RAD(dat->fieldSpinRotate);
            ubo->fieldSpin.y = dat->fieldSpinOuter;
            ubo->fieldSpin.z = (f32)dat->fieldSpinAxis;

            fres = dat + 1;
        }

        if (complexEmitterData->fieldFlg & 8)
        {
            const FieldCollisionData* dat = static_cast<const FieldCollisionData*>(fres);

            // The ultimate code
            u32 type = 0;
            if (dat->fieldCollisionType == 1)
                type = 2;

            if (dat->fieldCollisionIsWorld)
                type++;

            if (dat->fieldCollisionType == 2)
            {
                type = 4;
                if (dat->fieldCollisionIsWorld)
                    type++;
            }

            // fieldCollisionCnt is unused?
            ubo->fieldCollision.x = (f32)type;
            ubo->fieldCollision.y = dat->fieldCollisionCoef;
            ubo->fieldCollision.z = dat->fieldCollisionRegist;
            ubo->fieldCollision.w = dat->fieldCollisionCoord;

            fres = dat + 1;
        }

        if (complexEmitterData->fieldFlg & EFT_FIELD_MASK_CONVERGENCE)
        {
            const FieldConvergenceData* dat = static_cast<const FieldConvergenceData*>(fres);

            ubo->fieldConvergence.x = dat->fieldConvergencePos.x;
            ubo->fieldConvergence.y = dat->fieldConvergencePos.y;
            ubo->fieldConvergence.z = dat->fieldConvergencePos.z;
            ubo->fieldConvergence.w = dat->fieldConvergenceRatio;

            fres = dat + 1;

            if (dat->fieldConvergenceType == EFT_FIELD_CONVERGENCE_TYPE_EMITTER_POSITION)
                ubo->fieldCoordEmitter.x = 1.0f;
        }

        if (complexEmitterData->fieldFlg & EFT_FIELD_MASK_POSADD)
        {
            const FieldPosAddData* dat = static_cast<const FieldPosAddData*>(fres);

            ubo->fieldPosAdd.x = dat->fieldPosAdd.x;
            ubo->fieldPosAdd.y = dat->fieldPosAdd.y;
            ubo->fieldPosAdd.z = dat->fieldPosAdd.z;
            ubo->fieldPosAdd.w = (f32)dat->isFieldPosAddGlobal;

            fres = dat + 1;
        }

        if (complexEmitterData->fieldFlg & EFT_FIELD_MASK_CURL_NOISE)
        {
            const FieldCurlNoiseData* dat = static_cast<const FieldCurlNoiseData*>(fres);

            ubo->fieldCurlNoise0.x = dat->fieldCurlNoiseInfluence.x;
            ubo->fieldCurlNoise0.y = dat->fieldCurlNoiseInfluence.y;
            ubo->fieldCurlNoise0.z = dat->fieldCurlNoiseInfluence.z;
            ubo->fieldCurlNoise0.w = dat->fieldCurlNoiseScale;

            ubo->fieldCurlNoise1.x = dat->fieldCurlNoiseSpeed.x;
            ubo->fieldCurlNoise1.y = dat->fieldCurlNoiseSpeed.y;
            ubo->fieldCurlNoise1.z = dat->fieldCurlNoiseSpeed.z;
            ubo->fieldCurlNoise1.w = dat->fieldCurlNoiseBase;
        }
    }

    ubo->shaderParam.x = emitterData->shaderParam0;
    ubo->shaderParam.y = emitterData->shaderParam1;
    ubo->shaderParam.z = emitterData->softFadeDistance;
    ubo->shaderParam.w = emitterData->softVolumeParam;

    ubo->shaderParamAnim0.x = emitterData->shaderParam3v4kAnim.init;
    ubo->shaderParamAnim0.y = emitterData->shaderParam3v4kAnim.init + emitterData->shaderParam3v4kAnim.diff21;
    ubo->shaderParamAnim0.z = emitterData->shaderParam3v4kAnim.section1;
    ubo->shaderParamAnim0.w = (f32)emitterData->shaderUseParamAnim;
    ubo->shaderParamAnim1.x = emitterData->shaderParam3v4kAnim.init + emitterData->shaderParam3v4kAnim.diff21 + emitterData->shaderParam3v4kAnim.diff32;
    ubo->shaderParamAnim1.y = emitterData->shaderParam3v4kAnim.section2;
    ubo->shaderParamAnim1.z = 0.0f;
    ubo->shaderParamAnim1.w = 0.0f;

    ubo->flag[0] = flag;

    GX2EndianSwap(ubo, sizeof(EmitterStaticUniformBlock));
    DCFlushRange(ubo, sizeof(EmitterStaticUniformBlock));
}

void EmitterInstance::UpdateChildStaticUniformBlock(EmitterStaticUniformBlock* childEmitterStaticUniformBlock, const ChildData* childData)
{
    DCZeroRange(childEmitterStaticUniformBlock, sizeof(EmitterStaticUniformBlock));

    EmitterStaticUniformBlock* ubo = childEmitterStaticUniformBlock;

    ubo->rotBasis.x = childData->childRotBasis.x;
    ubo->rotBasis.y = childData->childRotBasis.y;
    ubo->rotBasis.z = 0.0f;
    ubo->rotBasis.w = 0.0f;

    ubo->fresnelMinMax.x = childData->childFresnelAlphaMin;
    ubo->fresnelMinMax.y = childData->childFresnelAlphaMax;
    ubo->fresnelMinMax.z = childData->childSoftAlphaOffset;
    ubo->fresnelMinMax.w = childData->childDecalVolume;

    ubo->nearAlpha.x = childData->childNearAlphaMin;
    ubo->nearAlpha.y = childData->childNearAlphaMax;
    ubo->nearAlpha.z = childData->childFarAlphaMin;
    ubo->nearAlpha.w = childData->childFarAlphaMax;

    u32 flag = 0;

    _copyTextureShiftAnimParm(&ubo->texShiftAnim0, &flag, &childData->textureData, EFT_TEXTURE_SLOT_0, 0);
    ubo->texShiftAnim0.scaleInit.x = 1.0f;
    ubo->texShiftAnim0.scaleInit.y = 1.0f;

    _copyTexturePtnAnimParm(&ubo->texPtnAnim0, &flag, &childData->textureData, EFT_TEXTURE_SLOT_0);

    if (childData->primitiveScaleY2Z)
        flag |= PRIMITIVE_SCALE_Y_TO_Z;

    ubo->flag[0] = flag;

    ubo->shaderParam.x = childData->childShaderParam0;
    ubo->shaderParam.y = childData->childShaderParam1;
    ubo->shaderParam.z = childData->childSoftFadeDistance;
    ubo->shaderParam.w = childData->childSoftVolumeParam;

    ubo->shaderParamAnim0.x = childData->shaderParam3v4kAnim.init;
    ubo->shaderParamAnim0.y = childData->shaderParam3v4kAnim.init + childData->shaderParam3v4kAnim.diff21;
    ubo->shaderParamAnim0.z = childData->shaderParam3v4kAnim.section1;
    ubo->shaderParamAnim0.w = (f32)childData->shaderUseParamAnim;
    ubo->shaderParamAnim1.x = childData->shaderParam3v4kAnim.init + childData->shaderParam3v4kAnim.diff21 + childData->shaderParam3v4kAnim.diff32;
    ubo->shaderParamAnim1.y = childData->shaderParam3v4kAnim.section2;
    ubo->shaderParamAnim1.z = 0.0f;
    ubo->shaderParamAnim1.w = 0.0f;

    GX2EndianSwap(ubo, sizeof(EmitterStaticUniformBlock));
    DCFlushRange(ubo, sizeof(EmitterStaticUniformBlock));
}

void EmitterInstance::UpdateEmitterInfoByEmit()
{
    scaleRnd.x = 0.0f;
    scaleRnd.y = 0.0f;
    scaleRnd.z = 0.0f;

    if (emitterBehaviorFlag & EFT_EMITTER_BEHAVIOR_FLAG_MATRIX_ROTATE_ANIM)
    {
        rotatRnd.x = 0.0f;
        rotatRnd.y = 0.0f;
        rotatRnd.z = 0.0f;
    }
    else
    {
        rotatRnd.x = /* rnd.GetF32Range(-1.0, 1.0f) */ (rnd.GetF32() * 2.0f - 1.0f) * res->emitterRotateRnd.x;
        rotatRnd.y = /* rnd.GetF32Range(-1.0, 1.0f) */ (rnd.GetF32() * 2.0f - 1.0f) * res->emitterRotateRnd.y;
        rotatRnd.z = /* rnd.GetF32Range(-1.0, 1.0f) */ (rnd.GetF32() * 2.0f - 1.0f) * res->emitterRotateRnd.z;
    }

    if (emitterBehaviorFlag & EFT_EMITTER_BEHAVIOR_FLAG_MATRIX_TRANS_ANIM)
    {
        transRnd.x = 0.0f;
        transRnd.y = 0.0f;
        transRnd.z = 0.0f;
    }
    else
    {
        transRnd.x = /* rnd.GetF32Range(-1.0, 1.0f) */ (rnd.GetF32() * 2.0f - 1.0f) * res->emitterTransRnd.x;
        transRnd.y = /* rnd.GetF32Range(-1.0, 1.0f) */ (rnd.GetF32() * 2.0f - 1.0f) * res->emitterTransRnd.y;
        transRnd.z = /* rnd.GetF32Range(-1.0, 1.0f) */ (rnd.GetF32() * 2.0f - 1.0f) * res->emitterTransRnd.z;
    }

    nw::math::VEC3 scale;
    nw::math::VEC3 rotat;
    nw::math::VEC3 trans;
    nw::math::VEC3Add(&scale, &res->emitterScale,  &scaleRnd);
    nw::math::VEC3Add(&rotat, &res->emitterRotate, &rotatRnd);
    nw::math::VEC3Add(&trans, &res->emitterTrans,  &transRnd);
    nw::math::VEC3 scaleOne;
    scaleOne.Set(1.0f, 1.0f, 1.0f);
    nw::math::MTX34MakeSRT(&animValEmitterSRT, scale,    rotat, trans);
    nw::math::MTX34MakeSRT(&animValEmitterRT,  scaleOne, rotat, trans);

    emitterRT .SetMult(emitterSet->GetRTMatrix(),  animValEmitterRT );
    emitterSRT.SetMult(emitterSet->GetSRTMatrix(), animValEmitterSRT);
}

void EmitterInstance::UpdateResInfo()
{
    const ComplexEmitterData* complex = GetComplexEmitterData();

    bool isUseGpuParticle = shader[EFT_SHADER_TYPE_NORMAL]->IsGpuAcceleration();
    bool isUseStreamOut   = shader[EFT_SHADER_TYPE_NORMAL]->IsUseStreamOut();

    ptclAttributeBuffer = NULL;
    childPtclAttributeBuffer = NULL;

    if (isUseGpuParticle)
    {
        u32 particleNum = 0;
        f32 emitRate = res->emitRate;

        if (res->volumeType == EFT_VOLUME_TYPE_PRIMITIVE)
            if (emitVolumePrimitive && res->volumePrimEmitType == EFT_VOLUME_PRIM_EMIT_TYPE_NORMAL)
                emitRate = (f32)(emitVolumePrimitive->GetPositionVertexBuffer().GetVertexBufferSize() / (sizeof(f32) * 3));

        if (res->isOnetime)
        {
            s32 time  = res->emitEndFrame - res->emitStartFrame;
            s32 times = time / (res->emitStep + 1) + 1;
            particleNum = (s32)(times * emitRate);
        }
        else
        {
            particleNum =  (s32)(res->ptclLife / (res->emitStep + 1) * emitRate);
            particleNum += (s32)emitRate;
        }

        if (particleNum != 0)
        {
            if (ptclAttributeBufferGpu)
            {
                if (gpuParticleBufferNum < particleNum)
                {
                    FreeFromDynamicHeap(ptclAttributeBufferGpu);
                    ptclAttributeBufferGpu = static_cast<PtclAttributeBufferGpu*>(AllocFromDynamicHeap(sizeof(PtclAttributeBufferGpu) * particleNum));
                }
            }
            else
            {
                ptclAttributeBufferGpu = static_cast<PtclAttributeBufferGpu*>(AllocFromDynamicHeap(sizeof(PtclAttributeBufferGpu) * particleNum));
            }

            if (isUseStreamOut && gpuParticleBufferNum != particleNum)
            {
                positionStreamOutBuffer.Finalize();
                vectorStreamOutBuffer  .Finalize();
                positionStreamOutBuffer.Initialize(sizeof(nw::math::VEC4) * particleNum);
                vectorStreamOutBuffer  .Initialize(sizeof(nw::math::VEC4) * particleNum);
            }
        }

        gpuParticleBufferNum = particleNum;
        gpuParticleBufferFillNum = 0;
        entryNum = 0;
    }
    else
    {
        ptclAttributeBufferGpu = NULL;
        gpuParticleBufferNum = 0;
    }

    if (complex && complex->fieldFlg & EFT_FIELD_MASK_COLLISION)
    {
        const void* __restrict fres = reinterpret_cast<const void*>((u32)complex + complex->fieldDataOffset);

        if (complex->fieldFlg & EFT_FIELD_MASK_RANDOM)
        {
            const FieldRandomData* dat = static_cast<const FieldRandomData*>(fres);
            fres = dat + 1;
        }

        if (complex->fieldFlg & EFT_FIELD_MASK_MAGNET)
        {
            const FieldMagnetData* dat = static_cast<const FieldMagnetData*>(fres);
            fres = dat + 1;
        }

        if (complex->fieldFlg & EFT_FIELD_MASK_SPIN)
        {
            const FieldSpinData* dat = static_cast<const FieldSpinData*>(fres);
            fres = dat + 1;
        }

        const FieldCollisionData* dat = static_cast<const FieldCollisionData*>(fres);
        filedCollisionY = dat->fieldCollisionCoord;
    }
    else
    {
        filedCollisionY = 0.0f;
    }

    followType                                  = res->ptclFollowType;
    particleBehaviorFlag                        = res->behaviorFlag;

    emitterAnimValue[EFT_ANIM_EM_RATE]          = res->emitRate;
    emitterAnimValue[EFT_ANIM_LIFE]             = (f32)res->ptclLife;
    emitterAnimValue[EFT_ANIM_ALL_DIR_VEL]      = res->emitFigureVel;
    emitterAnimValue[EFT_ANIM_DIR_VEL]          = res->emitVel;
    emitterAnimValue[EFT_ANIM_ALPHA]            = res->emitterAlpha;
    emitterAnimValue[EFT_ANIM_COLOR0_R]         = res->emitterColor0.r;
    emitterAnimValue[EFT_ANIM_COLOR0_G]         = res->emitterColor0.g;
    emitterAnimValue[EFT_ANIM_COLOR0_B]         = res->emitterColor0.b;
    emitterAnimValue[EFT_ANIM_COLOR1_R]         = res->emitterColor1.r;
    emitterAnimValue[EFT_ANIM_COLOR1_G]         = res->emitterColor1.g;
    emitterAnimValue[EFT_ANIM_COLOR1_B]         = res->emitterColor1.b;
    emitterAnimValue[EFT_ANIM_EM_FORM_SX]       = res->volumeFormScale.x;
    emitterAnimValue[EFT_ANIM_EM_FORM_SY]       = res->volumeFormScale.y;
    emitterAnimValue[EFT_ANIM_EM_FORM_SZ]       = res->volumeFormScale.z;
    emitterAnimValue[EFT_ANIM_PTCL_SX]          = 1.0f;
    emitterAnimValue[EFT_ANIM_PTCL_SY]          = 1.0f;
    emitterAnimValue[EFT_ANIM_EM_SX]            = res->emitterScale.x  + scaleRnd.x;
    emitterAnimValue[EFT_ANIM_EM_SY]            = res->emitterScale.y  + scaleRnd.y;
    emitterAnimValue[EFT_ANIM_EM_SZ]            = res->emitterScale.z  + scaleRnd.z;
    emitterAnimValue[EFT_ANIM_EM_RX]            = res->emitterRotate.x + rotatRnd.x;
    emitterAnimValue[EFT_ANIM_EM_RY]            = res->emitterRotate.y + rotatRnd.y;
    emitterAnimValue[EFT_ANIM_EM_RZ]            = res->emitterRotate.z + rotatRnd.z;
    emitterAnimValue[EFT_ANIM_EM_TX]            = res->emitterTrans.x  + transRnd.x;
    emitterAnimValue[EFT_ANIM_EM_TY]            = res->emitterTrans.y  + transRnd.y;
    emitterAnimValue[EFT_ANIM_EM_TZ]            = res->emitterTrans.z  + transRnd.z;
    emitterAnimValue[EFT_ANIM_GRAVITY]          = 1.0f;
    emitterAnimValue[EFT_ANIM_GRAVITY_CHILD]    = 1.0f;

    UpdateEmitterStaticUniformBlock(emitterStaticUniformBlock, res, complex);

    if (IsHasChildParticle())
    {
        const ChildData* childData = GetChildData();

        particleBehaviorFlag |= EFT_PARTICLE_BEHAVIOR_FLAG_WLD_POSDIF;
        childBehaviorFlag = childData->behaviorFlag;

        UpdateChildStaticUniformBlock(childEmitterStaticUniformBlock, childData);
    }

    UpdateEmitterInfoByEmit();
}

bool EmitterInstance::IsRequestFrameBufferTexture() const
{
    bool req = false;

    for (u32 i = 0; i < EFT_SHADER_TYPE_MAX; i++)
        if (shader[i] && shader[i]->GetFrameBufferTextureSamplerLocation().loc != EFT_INVALID_LOCATION)
            req = true;

    for (u32 i = 0; i < EFT_SHADER_TYPE_MAX; i++)
        if (childShader[i] && childShader[i]->GetFrameBufferTextureSamplerLocation().loc != EFT_INVALID_LOCATION)
            req = true;

    return req;
}

void EmitterCalc::RemoveParticle(PtclInstance* ptcl, CpuCore core)
{
    if (ptcl->complexParam && ptcl->complexParam->stripe)
        mSys->AddStripeRemoveList(ptcl->complexParam->stripe, core);

    {
        EmitterInstance* emitter = ptcl->emitter;

        if (ptcl->type == EFT_PTCL_TYPE_CHILD)
        {
            emitter->childPtclNum--;

            if (ptcl->prev && ptcl->next)
            {
                ptcl->prev->next = ptcl->next;
                ptcl->next->prev = ptcl->prev;
            }
            else if (emitter->childHead == ptcl)
            {
                emitter->childHead = ptcl->next;

                if (ptcl->next)
                    ptcl->next->prev = ptcl->prev;

                else
                    emitter->childTail = NULL;
            }
            else if (emitter->childTail == ptcl)
            {
                emitter->childTail = ptcl->prev;

                if (ptcl->prev)
                    ptcl->prev->next = ptcl->next;

                else
                    emitter->childHead = NULL;
            }
        }
        else
        {
            emitter->ptclNum--;

            if (ptcl->prev && ptcl->next)
            {
                ptcl->prev->next = ptcl->next;
                ptcl->next->prev = ptcl->prev;
            }
            else if (emitter->ptclHead == ptcl)
            {
                emitter->ptclHead = ptcl->next;

                if (ptcl->next)
                    ptcl->next->prev = ptcl->prev;

                else
                    emitter->ptclTail = NULL;
            }
            else if (emitter->ptclTail == ptcl)
            {
                emitter->ptclTail = ptcl->prev;

                if (ptcl->prev)
                    ptcl->prev->next = ptcl->next;

                else
                    emitter->ptclHead = NULL;
            }
        }

        CustomActionParticleRemoveCallback cb = mSys->GetCurrentCustomActionParticleRemoveCallback(ptcl->emitter);
        if (cb)
        {
            ParticleRemoveArg arg;
            arg.particle = ptcl;
            cb(arg);
        }
    }

    ptcl->life = 0;
    ptcl->res = NULL;
}

void EmitterCalc::AddPtclToList(EmitterInstance* emitter, PtclInstance* ptcl)
{
    if (emitter->res->flg & EFT_EMITTER_FLAG_REVERSE_ORDER_PARTICLE)
    {
        if (emitter->ptclTail == NULL)
        {
            emitter->ptclTail       = ptcl;
            ptcl->prev              = NULL;
            ptcl->next              = NULL;
        }
        else
        {
            emitter->ptclTail->next = ptcl;
            ptcl->prev              = emitter->ptclTail;
            emitter->ptclTail       = ptcl;
            ptcl->next              = NULL;
        }

        if (emitter->ptclHead == NULL)
            emitter->ptclHead = ptcl;
    }
    else
    {
        if (emitter->ptclHead == NULL)
        {
            emitter->ptclHead       = ptcl;
            ptcl->prev              = NULL;
            ptcl->next              = NULL;
        }
        else
        {
            emitter->ptclHead->prev = ptcl;
            ptcl->next              = emitter->ptclHead;
            emitter->ptclHead       = ptcl;
            ptcl->prev              = NULL;
        }

        if (emitter->ptclTail == NULL)
            emitter->ptclTail = ptcl;
    }

    emitter->ptclNum++;
}

static inline f32 _sqrtSafe(f32 v)
{
    if (v <= 0.0f)
        return 0.0f;

    return nw::math::FSqrt(v);
}

f32 _initialize3v4kAnim(AlphaAnim* anim, const anim3v4Key* key, s32 maxTime)
{
    anim->alphaSec1 = (s32)(key->section1 * maxTime);
    anim->alphaSec2 = (s32)(key->section2 * maxTime);

    if (anim->alphaSec1 == 0)
        anim->alphaAddSec1 = 0.0f;
    else
        anim->alphaAddSec1 = key->diff21 / (f32)anim->alphaSec1;

    if (key->section2 == 1.0f)
        anim->alphaAddSec2 = 0.0f;
    else
        anim->alphaAddSec2 = key->diff32 / (f32)(maxTime - anim->alphaSec2);

    return key->init - anim->alphaAddSec1;
}

void EmitterCalc::EmitCommon(EmitterInstance* __restrict e , PtclInstance*__restrict ptcl)
{
    const SimpleEmitterData*  __restrict res  = e->GetSimpleEmitterData();
    const ComplexEmitterData* __restrict cres = e->GetComplexEmitterData();

    ptcl->res  = res;
    ptcl->cres = cres;

    const EmitterSet* __restrict set = e->emitterSet;

    ptcl->emitter           = e;
    ptcl->emitTime          = e->cnt;
    ptcl->cnt               = 0.0f;
    ptcl->cntS              = 0;
    ptcl->rnd               = e->rnd.GetU32Direct();
    ptcl->fieldCollisionCnt = 0;
    ptcl->dynamicsRnd       = 1.0f + res->dynamicsRandom - res->dynamicsRandom * e->rnd.GetF32() * 2.0f;
    ptcl->runtimeUserData   = 0;

    ptcl->random[0]         = e->rnd.GetF32();
    ptcl->random[1]         = e->rnd.GetF32();
    ptcl->random[2]         = e->rnd.GetF32();
    ptcl->random[3]         = e->rnd.GetF32();

    ptcl->fluctuationAlpha  = 1.0f;
    ptcl->fluctuationScaleX = 1.0f;
    ptcl->fluctuationScaleY = 1.0f;

    ptcl->complexParam->childEmitcnt    = 1000000.0f;
    ptcl->complexParam->childPreEmitcnt = 0.0f;
    ptcl->complexParam->childEmitSaving = 0.0f;

    if (res->ptclLife == EFT_INFINIT_LIFE)
        ptcl->life = EFT_INFINIT_LIFE;
    else
        ptcl->life = (s32)((e->emitterAnimValue[EFT_ANIM_LIFE] - e->rnd.GetS32( res->ptclLifeRnd)) * e->controller->mLife);

    register f32 velRand = 1.0f - e->rnd.GetF32() * res->emitInitVelRnd * set->mRandomVel;
    register f32 dirVel  = e->emitterAnimValue[EFT_ANIM_DIR_VEL] * set->mDirectionalVel;

    if (e->followType != EFT_FOLLOW_TYPE_ALL)
    {
        nw::math::MTX34Copy(&ptcl->emitterRT,  e->emitterRT);
        nw::math::MTX34Copy(&ptcl->emitterSRT, e->emitterSRT);

        ptcl->coordinateEmitterRT  = &ptcl->emitterRT;
        ptcl->coordinateEmitterSRT = &ptcl->emitterSRT;
    }
    else
    {
        ptcl->coordinateEmitterRT  = &e->emitterRT;
        ptcl->coordinateEmitterSRT = &e->emitterSRT;
    }

    if (res->emitInitPosRand != 0.0f)
    {
        nw::math::VEC3 rndVec3 = e->rnd.GetNormalizedVec3();
        ptcl->pos.x = rndVec3.x * res->emitInitPosRand + ptcl->pos.x;
        ptcl->pos.y = rndVec3.y * res->emitInitPosRand + ptcl->pos.y;
        ptcl->pos.z = rndVec3.z * res->emitInitPosRand + ptcl->pos.z;
    }

    if (set->mIsSetDirectional)
    {
        ptcl->vel.x = (ptcl->vel.x + set->mDirectional.x * dirVel) * velRand;
        ptcl->vel.y = (ptcl->vel.y + set->mDirectional.y * dirVel) * velRand;
        ptcl->vel.z = (ptcl->vel.z + set->mDirectional.z * dirVel) * velRand;
    }
    else
    {
        f32 angle = res->emitVelDirAngle;
        if (angle == 0.0f)
        {
            ptcl->vel.x = (ptcl->vel.x + res->emitVelDir.x * dirVel) * velRand;
            ptcl->vel.y = (ptcl->vel.y + res->emitVelDir.y * dirVel) * velRand;
            ptcl->vel.z = (ptcl->vel.z + res->emitVelDir.z * dirVel) * velRand;
        }
        else
        {
            angle = angle / 90.0f;
            angle = 1.0f - angle;

            f32 rot = e->rnd.GetF32() * 2.0f * nw::math::F_PI;
            f32 sinV;
            f32 cosV;
            nw::math::SinCosRad( &sinV, &cosV, rot );

            f32 y = e->rnd.GetF32() * (1.0f - angle) + angle;

            f32 r = _sqrtSafe(1.0f - y * y);

            nw::math::VEC3 velocity;
            velocity.x = r * cosV;
            velocity.z = r * sinV;
            velocity.y = y;

            nw::math::QUAT q;
            nw::math::VEC3 dir;
            dir.Set(res->emitVelDir);
            nw::math::VEC3 unitY(0.0f, 1.0f, 0.0f);
            nw::math::QUATMakeVectorRotation(&q, &unitY, &dir);
            nw::math::MTX34 qmat;
            nw::math::QUATToMTX34(&qmat, q);
            nw::math::VEC3 srcVelocity;
            srcVelocity.Set(velocity.x, velocity.y, velocity.z);
            nw::math::VEC3Transform(&velocity,& qmat, &srcVelocity);

            ptcl->vel.x = (ptcl->vel.x + velocity.x * dirVel) * velRand;
            ptcl->vel.y = (ptcl->vel.y + velocity.y * dirVel) * velRand;
            ptcl->vel.z = (ptcl->vel.z + velocity.z * dirVel) * velRand;
        }
    }

    const nw::math::VEC3& rndVec3 = e->rnd.GetVec3();
    ptcl->vel.x += rndVec3.x * res->emitSpreadVec.x;
    ptcl->vel.y += rndVec3.y * res->emitSpreadVec.y;
    ptcl->vel.z += rndVec3.z * res->emitSpreadVec.z;

    if (res->emitEmitterVelInherit != 0.0f)
    {
        nw::math::VEC3 localVec;
        nw::math::VEC3 emitterVec;
        nw::math::VEC3 emitterDisVec;
        nw::math::VEC3 emitterDstVec;
        nw::math::VEC3 emitterDstDisVec;

        emitterVec.x = e->emitterRT._03;
        emitterVec.y = e->emitterRT._13;
        emitterVec.z = e->emitterRT._23;
        e->TransformLocalVec(&emitterDstVec, &emitterVec, ptcl);
        e->TransformLocalVec(&emitterDstDisVec, &e->emitDistPrevPos, ptcl);

        localVec.x = (emitterDstVec.x - emitterDstDisVec.x) * res->emitEmitterVelInherit;
        localVec.y = (emitterDstVec.y - emitterDstDisVec.y) * res->emitEmitterVelInherit;
        localVec.z = (emitterDstVec.z - emitterDstDisVec.z) * res->emitEmitterVelInherit;

        ptcl->vel.x += localVec.x;
        ptcl->vel.y += localVec.y;
        ptcl->vel.z += localVec.z;

        ptcl->pos.x -= localVec.x;
        ptcl->pos.y -= localVec.y;
        ptcl->pos.z -= localVec.z;
    }

    ptcl->vel.x += set->mVelAdd.x * set->mRT.m[0][0] + set->mVelAdd.y * set->mRT.m[1][0] + set->mVelAdd.z * set->mRT.m[2][0];
    ptcl->vel.y += set->mVelAdd.x * set->mRT.m[0][1] + set->mVelAdd.y * set->mRT.m[1][1] + set->mVelAdd.z * set->mRT.m[2][1];
    ptcl->vel.z += set->mVelAdd.x * set->mRT.m[0][2] + set->mVelAdd.y * set->mRT.m[1][2] + set->mVelAdd.z * set->mRT.m[2][2];

    if (!ptcl->vel.IsZero())
    {
        ptcl->posDiff.x = ptcl->vel.x;
        ptcl->posDiff.y = ptcl->vel.y;
        ptcl->posDiff.z = ptcl->vel.z;
    }
    else
    {
        #define EMITTER_UP_MIN_VEL 0.0001f

        ptcl->posDiff.x = 0.0f;
        ptcl->posDiff.y = EMITTER_UP_MIN_VEL;
        ptcl->posDiff.z = 0.0f;
    }

    s32 maxTime    = ptcl->life - 1;
    f32 initScaleX = 1.0f;
    f32 initScaleY = 1.0f;

    if (res->scaleRand.x != res->scaleRand.y)
    {
        f32 scaleRndX = (1.0f - res->scaleRand.x * e->rnd.GetF32());
        f32 scaleRndY = (1.0f - res->scaleRand.y * e->rnd.GetF32());
        initScaleX = scaleRndX * set->mEmissionParticleScale.x;
        initScaleY = scaleRndY * set->mEmissionParticleScale.y;
    }
    else
    {
        f32 scaleRnd  = (1.0f - res->scaleRand.x * e->rnd.GetF32());
        initScaleX = scaleRnd * set->mEmissionParticleScale.x;
        initScaleY = scaleRnd * set->mEmissionParticleScale.y;
    }


    if (e->shader[EFT_SHADER_TYPE_NORMAL]->IsGpuAcceleration())
    {
        ptcl->scale.x = e->emitterAnimValue[EFT_ANIM_PTCL_SX] * initScaleX;
        ptcl->scale.y = e->emitterAnimValue[EFT_ANIM_PTCL_SY] * initScaleY;
    }
    else
    {
        if (maxTime == 0)
        {
            ptcl->scaleAnim->scaleSec1 = -1;
            ptcl->scaleAnim->scaleSec2 = 0x7fffffff;
            ptcl->scaleAnim->scaleAddSec1 = nw::math::VEC2::Zero();
            ptcl->scaleAnim->scaleAddSec2 = nw::math::VEC2::Zero();

            ptcl->scale.x = res->initScale.x * res->baseScale.x * initScaleX * e->emitterAnimValue[EFT_ANIM_PTCL_SX];
            ptcl->scale.y = res->initScale.y * res->baseScale.y * initScaleY * e->emitterAnimValue[EFT_ANIM_PTCL_SY];
        }
        else
        {
            register f32 scaleX = 1.0f;
            register f32 scaleY = 1.0f;
            register f32 scaleAddSec1X = 0.0f;
            register f32 scaleAddSec1Y = 0.0f;
            register f32 scaleAddSec2X = 0.0f;
            register f32 scaleAddSec2Y = 0.0f;

            ptcl->scaleAnim->scaleSec1 = 0;
            ptcl->scaleAnim->scaleSec2 = 0;

            ptcl->scaleAnim->scaleSec1 = (res->scaleSection1 * maxTime) / 100;
            ptcl->scaleAnim->scaleSec2 = (res->scaleSection2 * maxTime) / 100;

            if (ptcl->scaleAnim->scaleSec1 == 0)
            {
                scaleAddSec1X = 0.0f;
                scaleAddSec1Y = 0.0f;
            }
            else
            {
                f32 invScaleSec1 = 1.0f / (f32)ptcl->scaleAnim->scaleSec1;
                scaleAddSec1X = res->diffScale21.x * invScaleSec1;
                scaleAddSec1Y = res->diffScale21.y * invScaleSec1;
            }

            if (ptcl->scaleAnim->scaleSec2 == maxTime)
            {
                scaleAddSec2X = 0.0f;
                scaleAddSec2Y = 0.0f;
            }
            else
            {
                f32 invScaleSec2 = 1.0f / (f32)(maxTime - ptcl->scaleAnim->scaleSec2);
                scaleAddSec2X = res->diffScale32.x * invScaleSec2;
                scaleAddSec2Y = res->diffScale32.y * invScaleSec2;
            }

            scaleX = res->initScale.x - scaleAddSec1X;
            scaleY = res->initScale.y - scaleAddSec1Y;

            ptcl->scaleAnim->scaleAddSec1.x = scaleAddSec1X * res->baseScale.x * e->emitterAnimValue[EFT_ANIM_PTCL_SX] * initScaleX;
            ptcl->scaleAnim->scaleAddSec1.y = scaleAddSec1Y * res->baseScale.y * e->emitterAnimValue[EFT_ANIM_PTCL_SY] * initScaleY;
            ptcl->scaleAnim->scaleAddSec2.x = scaleAddSec2X * res->baseScale.x * e->emitterAnimValue[EFT_ANIM_PTCL_SX] * initScaleX;
            ptcl->scaleAnim->scaleAddSec2.y = scaleAddSec2Y * res->baseScale.y * e->emitterAnimValue[EFT_ANIM_PTCL_SY] * initScaleY;

            ptcl->scale.x = scaleX * res->baseScale.x * e->emitterAnimValue[EFT_ANIM_PTCL_SX] * initScaleX;
            ptcl->scale.y = scaleY * res->baseScale.y * e->emitterAnimValue[EFT_ANIM_PTCL_SY] * initScaleY;
        }
    }

    ptcl->scaleVelY = 0.0f;


    if (res->alphaCalcType[EFT_ALPHA_KIND_0] == EFT_ALPHA_CALC_TYPE_FIXED ||
        res->alphaCalcType[EFT_ALPHA_KIND_0] == EFT_ALPHA_CALC_TYPE_8KEY)
    {
        ptcl->alpha0 = res->alpha3v4kAnim[EFT_ALPHA_KIND_0].init + res->alpha3v4kAnim[EFT_ALPHA_KIND_0].diff21;
    }
    else if (res->alphaCalcType[EFT_ALPHA_KIND_0] == EFT_ALPHA_CALC_TYPE_3V4KEY)
    {
        ptcl->alpha0 = _initialize3v4kAnim( ptcl->alphaAnim[EFT_ALPHA_KIND_0], &res->alpha3v4kAnim[EFT_ALPHA_KIND_0], maxTime );
    }

    if (res->alphaCalcType[EFT_ALPHA_KIND_1] == EFT_ALPHA_CALC_TYPE_FIXED ||
        res->alphaCalcType[EFT_ALPHA_KIND_1] == EFT_ALPHA_CALC_TYPE_8KEY)
    {
        ptcl->alpha1 = res->alpha3v4kAnim[EFT_ALPHA_KIND_1].init + res->alpha3v4kAnim[EFT_ALPHA_KIND_1].diff21;
    }
    else if (res->alphaCalcType[EFT_ALPHA_KIND_1] == EFT_ALPHA_CALC_TYPE_3V4KEY)
    {
        ptcl->alpha1 = _initialize3v4kAnim( ptcl->alphaAnim[EFT_ALPHA_KIND_1], &res->alpha3v4kAnim[EFT_ALPHA_KIND_1], maxTime );
    }

    ptcl->rot.x = res->initRot.x + e->rnd.GetF32() * res->initRotRand.x + set->mInitialRoate.x;
    ptcl->rot.y = res->initRot.y + e->rnd.GetF32() * res->initRotRand.y + set->mInitialRoate.y;
    ptcl->rot.z = res->initRot.z + e->rnd.GetF32() * res->initRotRand.z + set->mInitialRoate.z;

    ptcl->rotVel.x = res->rotVel.x + e->rnd.GetF32() * res->rotVelRand.x;
    ptcl->rotVel.y = res->rotVel.y + e->rnd.GetF32() * res->rotVelRand.y;
    ptcl->rotVel.z = res->rotVel.z + e->rnd.GetF32() * res->rotVelRand.z;

    if (res->isRotDirRand[0] && e->rnd.GetF32() > 0.5f)
    {
        ptcl->rot.x *= -1.0f;
        ptcl->rotVel.x *= -1.0f;
    }
    if (res->isRotDirRand[1] && e->rnd.GetF32() > 0.5f)
    {
        ptcl->rot.y *= -1.0f;
        ptcl->rotVel.y *= -1.0f;
    }
    if (res->isRotDirRand[2] && e->rnd.GetF32() > 0.5f)
    {
        ptcl->rot.z *= -1.0f;
        ptcl->rotVel.z *= -1.0f;
    }

    if (res->colorCalcType[EFT_COLOR_KIND_0] == EFT_COLOR_CALC_TYPE_RANDOM)
    {
        u32 num = (u32)res->colorNumRepeat[EFT_COLOR_KIND_0];
        u32 id = (u32)(ptcl->random[0] * num);
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
        u32 num = (u32)res->colorNumRepeat[EFT_COLOR_KIND_1];
        u32 id = (u32)( ptcl->random[0] * num );
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

    ptcl->color[EFT_COLOR_KIND_1].a = 1.0f;

    AddPtclToList(e, ptcl);

    CustomActionParticleEmitCallback particleEmitCB = mSys->GetCurrentCustomActionParticleEmitCallback(e);
    if (particleEmitCB)
    {
        ParticleEmitArg arg;
        arg.particle = ptcl;

        if (!particleEmitCB(arg))
        {
            RemoveParticle(ptcl, EFT_CPU_CORE_1);
            return;
        }
    }
}

u32 _getUnifiedAnimID(u32 id)
{
    switch(id)
    {
    case EFT_ANIM_8KEY_PARTICLE_COLOR0_R:
    case EFT_ANIM_8KEY_PARTICLE_COLOR0_G:
    case EFT_ANIM_8KEY_PARTICLE_COLOR0_B:
    case EFT_ANIM_8KEY_PARTICLE_COLOR1_R:
    case EFT_ANIM_8KEY_PARTICLE_COLOR1_G:
    case EFT_ANIM_8KEY_PARTICLE_COLOR1_B:
        return EFT_ANIM_8KEY_PARTICLE_COLOR1_R;

    case EFT_ANIM_8KEY_PARTICLE_SCALE_X:
    case EFT_ANIM_8KEY_PARTICLE_SCALE_Y:
        return EFT_ANIM_8KEY_PARTICLE_SCALE_X;
    }

    return id;
}

f32 _calcParticleAnimTime(EmitterInstance* __restrict e, PtclInstance* __restrict ptcl, u32 id)
{
    KeyFrameAnim* anim = e->particleAnimKey[EFT_ANIM_8KEY_OFFSET(id)];
    f32 time;

    if (anim->isLoop)
    {
        time = ptcl->cntS % (anim->loopFrame + 1) * 100.0f / anim->loopFrame + (ptcl->cnt - ptcl->cntS);

        if (anim->isStartRandom)
        {
            time += ptcl->rnd * _getUnifiedAnimID(id) * EFT_RANDOM_MAX_INVERSE_F32 * 100.0f;
            if (time > 100.0f)
                time -= 100.0f;
        }
    }
    else
    {
        time = 100.0f * ptcl->cnt / ptcl->life;

        if (anim->isStartRandom)
        {
            time += ptcl->rnd * _getUnifiedAnimID(id) * EFT_RANDOM_MAX_INVERSE_F32 * 100.0f;
            if (time > 100.0f)
                time -= 100.0f;
        }
    }

    return time;
}

f32 _calcParticleAnimTime(KeyFrameAnim* anim, PtclInstance* __restrict ptcl, u32 id)
{
    f32 time;

    if (anim->isLoop)
    {
        time = ptcl->cntS % (anim->loopFrame + 1) * 100.0f / anim->loopFrame + (ptcl->cnt - ptcl->cntS);

        if (anim->isStartRandom)
        {
            time += ptcl->rnd * _getUnifiedAnimID(id) * EFT_RANDOM_MAX_INVERSE_F32 * 100.0f;
            if (time > 100.0f)
                time -= 100.0f;
        }
    }
    else
    {
        time = 100.0f * ptcl->cnt / ptcl->life;

        if (anim->isStartRandom)
        {
            time += ptcl->rnd * _getUnifiedAnimID(id) * EFT_RANDOM_MAX_INVERSE_F32 * 100.0f;
            if (time > 100.0f)
                time -= 100.0f;
        }
    }

    return time;
}

} } // namespace nw::eft
