#pragma once

#include <nw/eft/eft_AnimKeyFrame.h>
#include <nw/eft/eft_Data.h>
#include <nw/eft/eft_Shader.h>

#if EFT_IS_WIN

#include <cassert>

static inline void Swap16(void* buf)
{
    *(u16*)buf = __builtin_bswap16(*(u16*)buf);
}

static inline void Swap32(void* buf)
{
    *(u32*)buf = __builtin_bswap32(*(u32*)buf);
}

static inline void Swap32(void* buf, u32 count)
{
    for (u32 i = 0; i < count; i++)
        Swap32((u32*)buf + i);
}

#endif // EFT_IS_WIN

static inline nw::eft::HeaderData* LoadNwEftHeaderData(void* data, bool is_be = true)
{
    nw::eft::HeaderData* header = static_cast<nw::eft::HeaderData*>(data);

#if EFT_IS_WIN
    if (is_be)
    {
        assert(header != NULL);

        Swap32(&header->version);
        Swap32(&header->numEmitterSet);
        Swap32(&header->namePos);
        Swap32(&header->nameTblPos);
        Swap32(&header->textureTblPos);
        Swap32(&header->textureTblSize);
        Swap32(&header->shaderTblPos);
        Swap32(&header->shaderTblSize);
        Swap32(&header->animkeyTblPos);
        Swap32(&header->animkeyTblSize);
        Swap32(&header->primitiveTblPos);
        Swap32(&header->primitiveTblSize);
        Swap32(&header->totalTextureSize);
        Swap32(&header->totalShaderSize);
        Swap32(&header->totalEmitterSize);
    }
#endif // EFT_IS_WIN

    return header;
}

static inline nw::eft::ShaderImageInformation* LoadNwEftShaderImageInformation(void* data, bool is_be = true)
{
    nw::eft::ShaderImageInformation* shaderImageInfo = static_cast<nw::eft::ShaderImageInformation*>(data);

#if EFT_IS_WIN
    if (is_be)
    {
        Swap32(&shaderImageInfo->shaderNum);
        Swap32(&shaderImageInfo->totalSize);
        Swap32(&shaderImageInfo->offsetShaderSrcInfo);
        Swap32(&shaderImageInfo->offsetShaderBinInfo);
    }
#endif // EFT_IS_WIN

    return shaderImageInfo;
}

static inline nw::eft::VertexShaderKey* LoadNwEftVertexShaderKey(void* data, bool is_be = true)
{
    nw::eft::VertexShaderKey* vertexShaderKey = static_cast<nw::eft::VertexShaderKey*>(data);

#if EFT_IS_WIN
    if (is_be)
    {
        Swap32(&vertexShaderKey->mUserShaderFlag);
        Swap32(&vertexShaderKey->mUserShaderSwitchFlag);
    }
#endif // EFT_IS_WIN

    return vertexShaderKey;
}

static inline nw::eft::FragmentShaderKey* LoadNwEftFragmentShaderKey(void* data, bool is_be = true)
{
    nw::eft::FragmentShaderKey* fragmentShaderKey = static_cast<nw::eft::FragmentShaderKey*>(data);

#if EFT_IS_WIN
    if (is_be)
    {
        Swap16(&fragmentShaderKey->_12);
        Swap32(&fragmentShaderKey->mUserShaderFlag);
        Swap32(&fragmentShaderKey->mUserShaderSwitchFlag);
        Swap16(&fragmentShaderKey->_2c);
    }
#endif // EFT_IS_WIN

    return fragmentShaderKey;
}

static inline nw::eft::ShaderInformation* LoadNwEftShaderInformation(void* data, bool is_be = true)
{
    nw::eft::ShaderInformation* shaderInfo = static_cast<nw::eft::ShaderInformation*>(data);

#if EFT_IS_WIN
    (void)LoadNwEftVertexShaderKey(&shaderInfo->vertexShaderKey, is_be);
    (void)LoadNwEftFragmentShaderKey(&shaderInfo->fragmentShaderKey, is_be);
    //(void)LoadNwEftGeometryShaderKey(&shaderInfo->geometryShaderKey, is_be);

    if (is_be)
    {
        Swap32(&shaderInfo->shaderSize);
        Swap32(&shaderInfo->offset);
    }
#endif // EFT_IS_WIN

    return shaderInfo;
}

static inline nw::eft::PrimitiveImageInformation* LoadNwEftPrimitiveImageInformation(void* data, bool is_be = true)
{
    nw::eft::PrimitiveImageInformation* imageInfo = static_cast<nw::eft::PrimitiveImageInformation*>(data);

#if EFT_IS_WIN
    if (is_be)
    {
        Swap32(&imageInfo->primitiveNum);
        Swap32(&imageInfo->totalSize);
        Swap32(&imageInfo->offsetPrimitiveTableInfo);
    }
#endif // EFT_IS_WIN

    return imageInfo;
}

static inline nw::eft::PrimitiveTableInfo::PrimDataTable* LoadNwEftPrimitiveTableInfoPrimDataTable(void* data, bool is_be = true)
{
    nw::eft::PrimitiveTableInfo::PrimDataTable* primitiveAttrib = static_cast<nw::eft::PrimitiveTableInfo::PrimDataTable*>(data);

#if EFT_IS_WIN
    if (is_be)
    {
        Swap32(&primitiveAttrib->count);
        Swap32(&primitiveAttrib->column);
        Swap32(&primitiveAttrib->offset);
        Swap32(&primitiveAttrib->size);
    }
#endif // EFT_IS_WIN

    return primitiveAttrib;
}

static inline nw::eft::PrimitiveTableInfo* LoadNwEftPrimitiveTableInfo(void* data, bool is_be = true)
{
    nw::eft::PrimitiveTableInfo* info = static_cast<nw::eft::PrimitiveTableInfo*>(data);

#if EFT_IS_WIN
    (void)LoadNwEftPrimitiveTableInfoPrimDataTable(&info->pos, is_be);
    (void)LoadNwEftPrimitiveTableInfoPrimDataTable(&info->normal, is_be);
    (void)LoadNwEftPrimitiveTableInfoPrimDataTable(&info->color, is_be);
    (void)LoadNwEftPrimitiveTableInfoPrimDataTable(&info->texCoord, is_be);
    (void)LoadNwEftPrimitiveTableInfoPrimDataTable(&info->index, is_be);
#endif // EFT_IS_WIN

    return info;
}

static inline nw::eft::EmitterSetData* LoadNwEftEmitterSetData(void* data, bool is_be = true)
{
    nw::eft::EmitterSetData* emitterSetData = static_cast<nw::eft::EmitterSetData*>(data);

#if EFT_IS_WIN
    if (is_be)
    {
        Swap32(&emitterSetData->userData);
        Swap32(&emitterSetData->lastUpdateDate);
        Swap32(&emitterSetData->namePos);
        Swap32(&emitterSetData->name);
        Swap32(&emitterSetData->numEmitter);
        Swap32(&emitterSetData->emitterTblPos);
        Swap32(&emitterSetData->emitterTbl);
    }
#endif // EFT_IS_WIN

    return emitterSetData;
}

static inline nw::eft::EmitterTblData* LoadNwEftEmitterTblData(void* data, bool is_be = true)
{
    nw::eft::EmitterTblData* emitterTblData = static_cast<nw::eft::EmitterTblData*>(data);

#if EFT_IS_WIN
    if (is_be)
    {
        Swap32(&emitterTblData->emitterPos);
        Swap32(&emitterTblData->emitter);
    }
#endif // EFT_IS_WIN

    return emitterTblData;
}

static inline nw::eft::TextureRes* LoadNwEftTextureRes(void* data, bool is_be = true)
{
    nw::eft::TextureRes* texRes = static_cast<nw::eft::TextureRes*>(data);

#if EFT_IS_WIN
    if (is_be)
    {
        Swap16(&texRes->width);
        Swap16(&texRes->height);
        Swap32(&texRes->tileMode);
        Swap32(&texRes->swizzle);
        Swap32(&texRes->alignment);
        Swap32(&texRes->pitch);
        Swap32(&texRes->mipLevel);
        Swap32(&texRes->compSel);
        Swap32(&texRes->mipOffset, 13);
        Swap32(&texRes->enableMipLevel);
        Swap32(&texRes->mipMapBias);
        Swap32(&texRes->originalDataFormat);
        Swap32(&texRes->originalDataPos);
        Swap32(&texRes->originalDataSize);
        Swap32(&texRes->nativeDataFormat);
        Swap32(&texRes->nativeDataSize);
        Swap32(&texRes->nativeDataPos);
        Swap32(&texRes->handle);
    }
#endif // EFT_IS_WIN

    return texRes;
}

static inline nw::eft::AnimKeyTable* LoadNwEftAnimKeyTable(void* data, bool is_be = true)
{
    nw::eft::AnimKeyTable* animKeyTable = static_cast<nw::eft::AnimKeyTable*>(data);

#if EFT_IS_WIN
    if (is_be)
    {
        Swap32(&animKeyTable->animKeyTable);
        Swap32(&animKeyTable->animPos);
        Swap32(&animKeyTable->dataSize);
    }
#endif // EFT_IS_WIN

    return animKeyTable;
}

static inline nw::eft::PrimitiveFigure* LoadNwEftPrimitiveFigure(void* data, bool is_be = true)
{
    nw::eft::PrimitiveFigure* primitiveFigure = static_cast<nw::eft::PrimitiveFigure*>(data);

#if EFT_IS_WIN
    if (is_be)
    {
        Swap32(&primitiveFigure->primitiveTableInfo);
        Swap32(&primitiveFigure->dataSize);
        Swap32(&primitiveFigure->index);
    }
#endif // EFT_IS_WIN

    return primitiveFigure;
}

static inline nw::eft::TextureEmitterData* LoadNwEftTextureEmitterData(void* data, bool is_be = true)
{
    nw::eft::TextureEmitterData* textureData = static_cast<nw::eft::TextureEmitterData*>(data);

#if EFT_IS_WIN
    if (is_be)
    {
        Swap16(&textureData->texPatFreq);
        Swap16(&textureData->texPatTblUse);
        Swap32(&textureData->texAddressingMode);
        Swap32(&textureData->texUScale);
        Swap32(&textureData->texVScale);
        Swap32(&textureData->uvShiftAnimMode);
        Swap32(&textureData->uvScroll, 2);
        Swap32(&textureData->uvScrollInit, 2);
        Swap32(&textureData->uvScrollInitRand, 2);
        Swap32(&textureData->uvScale, 2);
        Swap32(&textureData->uvScaleInit, 2);
        Swap32(&textureData->uvScaleInitRand, 2);
        Swap32(&textureData->uvRot);
        Swap32(&textureData->uvRotInit);
        Swap32(&textureData->uvRotInitRand);
    }
#endif // EFT_IS_WIN

    return textureData;
}

static inline nw::eft::UserShaderParam* LoadNwEftUserShaderParam(void* data, bool is_be = true)
{
    nw::eft::UserShaderParam* userShaderParam = static_cast<nw::eft::UserShaderParam*>(data);

#if EFT_IS_WIN
    if (is_be)
    {
        Swap32(&userShaderParam->param, 32);
    }
#endif // EFT_IS_WIN

    return userShaderParam;
}

static inline nw::eft::ChildData* LoadNwEftChildData(void* data, bool is_be = true)
{
    nw::eft::ChildData* childData = static_cast<nw::eft::ChildData*>(data);

#if EFT_IS_WIN
    (void)LoadNwEftPrimitiveFigure(&childData->childPrimitiveFigure, is_be);
    (void)LoadNwEftTextureRes(&childData->childTex, is_be);
    (void)LoadNwEftUserShaderParam(&childData->childUserShaderParam, is_be);

    if (is_be)
    {
        Swap32(&childData->childEmitRate);
        Swap32(&childData->childEmitTiming);
        Swap32(&childData->childLife);
        Swap32(&childData->childEmitStep);
        Swap32(&childData->childVelInheritRate);
        Swap32(&childData->childFigureVel);
        Swap32(&childData->childInitPosRand, 3);
        Swap32(&childData->childInitPosRand);
        Swap32(&childData->childDynamicsRandom);
        Swap32(&childData->childBlendType);
        Swap32(&childData->childMeshType);
        Swap32(&childData->childBillboardType);
        Swap32(&childData->childZBufATestType);
        Swap32(&childData->childDisplaySide);
        Swap32(&childData->childColor0, 3);
        Swap32(&childData->childColor1, 3);
        Swap32(&childData->primitiveColorBlend);
        Swap32(&childData->primitiveAlphaBlend);
        Swap32(&childData->childAlpha);
        Swap32(&childData->childAlphaTarget);
        Swap32(&childData->childAlphaInit);
        Swap32(&childData->childScaleInheritRate);
        Swap32(&childData->childScale, 2);
        Swap32(&childData->childScaleRand);
        Swap32(&childData->childRotType);
        Swap32(&childData->childInitRot, 3);
        Swap32(&childData->childInitRotRand, 3);
        Swap32(&childData->childRotVel, 3);
        Swap32(&childData->childRotVelRand, 3);
        Swap32(&childData->childRotRegist);
        Swap32(&childData->childRotBasis, 2);
        Swap32(&childData->childGravity, 3);
        Swap32(&childData->childAlphaStartFrame);
        Swap32(&childData->childAlphaBaseFrame);
        Swap32(&childData->childScaleStartFrame);
        Swap32(&childData->childScaleTarget, 2);
        Swap16(&childData->childNumTexPat);
        Swap32(&childData->childTexUScale);
        Swap32(&childData->childTexVScale);
        Swap16(&childData->childTexPatFreq);
        Swap16(&childData->childTexPatTblUse);
        Swap32(&childData->childCombinerType);
        Swap32(&childData->childAlphaCombinerType);
        Swap32(&childData->childAirRegist);
        Swap32(&childData->childShaderParam0);
        Swap32(&childData->childShaderParam1);
        Swap32(&childData->childSoftFadeDistance);
        Swap32(&childData->childSoftVolumeParam);
        Swap32(&childData->childUserShaderFlag);
        Swap32(&childData->childUserShaderSwitchFlag);
    }
#endif // EFT_IS_WIN

    return childData;
}

static inline nw::eft::FieldRandomData* LoadNwEftFieldRandomData(void* data, bool is_be = true)
{
    nw::eft::FieldRandomData* dat = static_cast<nw::eft::FieldRandomData*>(data);

#if EFT_IS_WIN
    if (is_be)
    {
        Swap32(&dat->fieldRandomBlank);
        Swap32(&dat->fieldRandomVelAdd, 3);
    }
#endif // EFT_IS_WIN

    return dat;
}

static inline nw::eft::FieldMagnetData* LoadNwEftFieldMagnetData(void* data, bool is_be = true)
{
    nw::eft::FieldMagnetData* dat = static_cast<nw::eft::FieldMagnetData*>(data);

#if EFT_IS_WIN
    if (is_be)
    {
        Swap32(&dat->fieldMagnetPower);
        Swap32(&dat->fieldMagnetPos, 3);
        Swap32(&dat->fieldMagnetFlg);
    }
#endif // EFT_IS_WIN

    return dat;
}

static inline nw::eft::FieldSpinData* LoadNwEftFieldSpinData(void* data, bool is_be = true)
{
    nw::eft::FieldSpinData* dat = static_cast<nw::eft::FieldSpinData*>(data);

#if EFT_IS_WIN
    if (is_be)
    {
        Swap32(&dat->fieldSpinRotate);
        Swap32(&dat->fieldSpinAxis);
        Swap32(&dat->fieldSpinOuter);
    }
#endif // EFT_IS_WIN

    return dat;
}

static inline nw::eft::FieldCollisionData* LoadNwEftFieldCollisionData(void* data, bool is_be = true)
{
    nw::eft::FieldCollisionData* dat = static_cast<nw::eft::FieldCollisionData*>(data);

#if EFT_IS_WIN
    if (is_be)
    {
        Swap16(&dat->fieldCollisionType);
        Swap16(&dat->fieldCollisionIsWorld);
        Swap32(&dat->fieldCollisionCoord);
        Swap32(&dat->fieldCollisionCoef);
    }
#endif // EFT_IS_WIN

    return dat;
}

static inline nw::eft::FieldConvergenceData* LoadNwEftFieldConvergenceData(void* data, bool is_be = true)
{
    nw::eft::FieldConvergenceData* dat = static_cast<nw::eft::FieldConvergenceData*>(data);

#if EFT_IS_WIN
    if (is_be)
    {
        Swap32(&dat->fieldConvergencePos, 3);
        Swap32(&dat->fieldConvergenceRatio);
    }
#endif // EFT_IS_WIN

    return dat;
}

static inline nw::eft::FieldPosAddData* LoadNwEftFieldPosAddData(void* data, bool is_be = true)
{
    nw::eft::FieldPosAddData* dat = static_cast<nw::eft::FieldPosAddData*>(data);

#if EFT_IS_WIN
    if (is_be)
    {
        Swap32(&dat->fieldPosAdd, 3);
    }
#endif // EFT_IS_WIN

    return dat;
}

static inline void* LoadNwEftFieldData(void* data, u16 fieldFlg, bool is_be = true)
{
#if EFT_IS_WIN
    void* fieldData = data;

    if ( fieldFlg & nw::eft::EFT_FIELD_MASK_RANDOM      ) fieldData = LoadNwEftFieldRandomData     (fieldData, is_be) + 1;
    if ( fieldFlg & nw::eft::EFT_FIELD_MASK_MAGNET      ) fieldData = LoadNwEftFieldMagnetData     (fieldData, is_be) + 1;
    if ( fieldFlg & nw::eft::EFT_FIELD_MASK_SPIN        ) fieldData = LoadNwEftFieldSpinData       (fieldData, is_be) + 1;
    if ( fieldFlg & nw::eft::EFT_FIELD_MASK_COLLISION   ) fieldData = LoadNwEftFieldCollisionData  (fieldData, is_be) + 1;
    if ( fieldFlg & nw::eft::EFT_FIELD_MASK_CONVERGENCE ) fieldData = LoadNwEftFieldConvergenceData(fieldData, is_be) + 1;
    if ( fieldFlg & nw::eft::EFT_FIELD_MASK_POSADD      ) fieldData = LoadNwEftFieldPosAddData     (fieldData, is_be) + 1;
#endif // EFT_IS_WIN

    return data;
}

static inline nw::eft::FluctuationData* LoadNwEftFluctuationData(void* data, bool is_be = true)
{
    nw::eft::FluctuationData* fluctuationData = static_cast<nw::eft::FluctuationData*>(data);

#if EFT_IS_WIN
    if (is_be)
    {
        Swap32(&fluctuationData->fluctuationScale);
        Swap32(&fluctuationData->fluctuationFreq);
        Swap32(&fluctuationData->fluctuationPhaseRnd);
    }
#endif // EFT_IS_WIN

    return fluctuationData;
}

static inline nw::eft::StripeData* LoadNwEftStripeData(void* data, bool is_be = true)
{
    nw::eft::StripeData* stripeData = static_cast<nw::eft::StripeData*>(data);

#if EFT_IS_WIN
    if (is_be)
    {
        Swap32(&stripeData->stripeType);
        Swap32(&stripeData->stripeOption);
        Swap32(&stripeData->stripeConnectOpt);
        Swap32(&stripeData->stripeTexCoordOpt);
        Swap32(&stripeData->stripeNumHistory);
        Swap32(&stripeData->stripeDivideNum);
        Swap32(&stripeData->stripeStartAlpha);
        Swap32(&stripeData->stripeEndAlpha);
        Swap32(&stripeData->stripeUVScroll, 2);
        Swap32(&stripeData->stripeHistoryStep);
        Swap32(&stripeData->stripeHistoryInterpolate);
        Swap32(&stripeData->stripeDirInterpolate);
    }
#endif // EFT_IS_WIN

    return stripeData;
}

static inline nw::eft::CommonEmitterData* LoadNwEftEmitterData(void* data, bool is_be = true)
{
    nw::eft::ComplexEmitterData* res = static_cast<nw::eft::ComplexEmitterData*>(data);

#if EFT_IS_WIN
    /* nw::eft::CommonEmitterData */

    (void)LoadNwEftTextureRes(&res->texRes[nw::eft::EFT_TEXTURE_SLOT_0], is_be);
    (void)LoadNwEftTextureRes(&res->texRes[nw::eft::EFT_TEXTURE_SLOT_1], is_be);
    (void)LoadNwEftAnimKeyTable(&res->animKeyTable, is_be);
    (void)LoadNwEftPrimitiveFigure(&res->primitiveFigure, is_be);

    if (is_be)
    {
        Swap32(&res->type);
        Swap32(&res->flg);
        Swap32(&res->randomSeed);
        Swap32(&res->userData);
        Swap32(&res->userData2);
        Swap32(&res->userDataF, nw::eft::EFT_USER_DATA_PARAM_MAX);
        Swap32(&res->userCallbackID);
        Swap32(&res->namePos);
        Swap32(&res->name);

        assert(res->type == nw::eft::EFT_EMITTER_TYPE_SIMPLE ||
               res->type == nw::eft::EFT_EMITTER_TYPE_COMPLEX);
    }

    /* nw::eft::SimpleEmitterData */

    (void)LoadNwEftTextureEmitterData(&res->textureData[nw::eft::EFT_TEXTURE_SLOT_0], is_be);
    (void)LoadNwEftTextureEmitterData(&res->textureData[nw::eft::EFT_TEXTURE_SLOT_1], is_be);
    (void)LoadNwEftUserShaderParam(&res->userShaderParam, is_be);

    if (is_be)
    {
        Swap32(&res->ptclRotType);
        Swap32(&res->ptclFollowType);
        Swap32(&res->colorCombinerType);
        Swap32(&res->alphaCombinerType);
        Swap32(&res->drawPath);
        Swap32(&res->displaySide);
        Swap32(&res->dynamicsRandom);
        Swap32(&res->transformSRT, 3 * 4);
        Swap32(&res->transformRT, 3 * 4);
        Swap32(&res->scale, 3);
        Swap32(&res->rot, 3);
        Swap32(&res->trans, 3);
        Swap32(&res->rotRnd, 3);
        Swap32(&res->transRnd, 3);
        Swap32(&res->blendType);
        Swap32(&res->zBufATestType);
        Swap32(&res->volumeType);
        Swap32(&res->volumeRadius, 3);
        Swap32(&res->volumeSweepStart);
        Swap32(&res->volumeSweepParam);
        Swap32(&res->volumeCaliber);
        Swap32(&res->volumeLatitude);
        Swap32(&res->volumeLatitudeDir, 3);
        Swap32(&res->lineCenter);
        Swap32(&res->formScale, 3);
        Swap32(&res->color0, 4);
        Swap32(&res->color1, 4);
        Swap32(&res->alpha);
        Swap32(&res->emitDistUnit);
        Swap32(&res->emitDistMax);
        Swap32(&res->emitDistMin);
        Swap32(&res->emitDistMargin);
        Swap32(&res->emitRate);
        Swap32(&res->startFrame);
        Swap32(&res->endFrame);
        Swap32(&res->lifeStep);
        Swap32(&res->lifeStepRnd);
        Swap32(&res->figureVel);
        Swap32(&res->emitterVel);
        Swap32(&res->initVelRnd);
        Swap32(&res->emitterVelDir, 3);
        Swap32(&res->emitterVelDirAngle);
        Swap32(&res->spreadVec, 3);
        Swap32(&res->airRegist);
        Swap32(&res->gravity, 3);
        Swap32(&res->xzDiffusionVel);
        Swap32(&res->initPosRand);
        Swap32(&res->ptclLife);
        Swap32(&res->ptclLifeRnd);
        Swap32(&res->meshType);
        Swap32(&res->billboardType);
        Swap32(&res->rotBasis, 2);
        Swap32(&res->toCameraOffset);
        Swap32(&res->colorCalcType, nw::eft::EFT_COLOR_KIND_MAX);
        Swap32(&res->color, nw::eft::EFT_COLOR_KIND_MAX * 3 * 4);
        Swap32(&res->colorSection1, nw::eft::EFT_COLOR_KIND_MAX);
        Swap32(&res->colorSection2, nw::eft::EFT_COLOR_KIND_MAX);
        Swap32(&res->colorSection3, nw::eft::EFT_COLOR_KIND_MAX);
        Swap32(&res->colorNumRepeat, nw::eft::EFT_COLOR_KIND_MAX);
        Swap32(&res->colorRepeatStartRand, nw::eft::EFT_COLOR_KIND_MAX);
        Swap32(&res->colorScale);
        Swap32(&res->initAlpha);
        Swap32(&res->diffAlpha21);
        Swap32(&res->diffAlpha32);
        Swap32(&res->alphaSection1);
        Swap32(&res->alphaSection2);
        Swap32(&res->texture1ColorBlend);
        Swap32(&res->primitiveColorBlend);
        Swap32(&res->texture1AlphaBlend);
        Swap32(&res->primitiveAlphaBlend);
        Swap32(&res->scaleSection1);
        Swap32(&res->scaleSection2);
        Swap32(&res->scaleRand);
        Swap32(&res->baseScale, 2);
        Swap32(&res->initScale, 2);
        Swap32(&res->diffScale21, 2);
        Swap32(&res->diffScale32, 2);
        Swap32(&res->initRot, 3);
        Swap32(&res->initRotRand, 3);
        Swap32(&res->rotVel, 3);
        Swap32(&res->rotVelRand, 3);
        Swap32(&res->rotRegist);
        Swap32(&res->alphaAddInFade);
        Swap32(&res->shaderParam0);
        Swap32(&res->shaderParam1);
        Swap32(&res->softFadeDistance);
        Swap32(&res->softVolumeParam);
        Swap32(&res->userShaderFlag);
        Swap32(&res->userShaderSwitchFlag);
    }

    /* nw::eft::ComplexEmitterData */

    if (res->type == nw::eft::EFT_EMITTER_TYPE_COMPLEX)
    {
        if (is_be)
        {
            Swap32(&res->childFlg);
            Swap16(&res->fieldFlg);
            Swap16(&res->fluctuationFlg);
            Swap16(&res->stripeFlg);
            Swap16(&res->childDataOffset);
            Swap16(&res->fieldDataOffset);
            Swap16(&res->fluctuationDataOffset);
            Swap16(&res->stripeDataOffset);
            Swap32(&res->emitterDataSize);
        }

        if (res->childFlg & nw::eft::EFT_CHILD_FLAG_ENABLE)
        {
            assert(res->childDataOffset == sizeof(nw::eft::ComplexEmitterData));
            (void)LoadNwEftChildData(res + 1);
        }

        if (res->fieldFlg != 0)
        {
            assert(res->fieldDataOffset >= sizeof(nw::eft::ComplexEmitterData));
            (void)LoadNwEftFieldData((void*)((u32)res + res->fieldDataOffset), res->fieldFlg);
        }

        if (res->fluctuationFlg & nw::eft::EFT_FLUCTUATION_FALG_ENABLE)
        {
            assert(res->fluctuationDataOffset >= sizeof(nw::eft::ComplexEmitterData));
            (void)LoadNwEftFluctuationData((void*)((u32)res + res->fluctuationDataOffset));
        }

        if (res->billboardType == nw::eft::EFT_BILLBOARD_TYPE_STRIPE ||
            res->billboardType == nw::eft::EFT_BILLBOARD_TYPE_COMPLEX_STRIPE)
        {
            assert(res->stripeDataOffset >= sizeof(nw::eft::ComplexEmitterData));
            (void)LoadNwEftStripeData((void*)((u32)res + res->stripeDataOffset));
        }
    }
#endif // EFT_IS_WIN

    return res;
}

static inline nw::eft::AnimKeyFrameKey* LoadNwEftAnimKeyFrameKey(void* data, bool is_be = true)
{
    nw::eft::AnimKeyFrameKey* key = static_cast<nw::eft::AnimKeyFrameKey*>(data);

#if EFT_IS_WIN
    if (is_be)
    {
        Swap32(&key->x);
        Swap32(&key->y);
    }
#endif // EFT_IS_WIN

    return key;
}

static inline nw::eft::KeyFrameAnim* LoadNwEftKeyFrameAnim(void* data, bool is_be = true)
{
    nw::eft::KeyFrameAnim* info = static_cast<nw::eft::KeyFrameAnim*>(data);

#if EFT_IS_WIN
    if (is_be)
    {
        Swap32(&info->keyNum);
        Swap32(&info->interpolation);
        Swap32(&info->target);
        Swap32(&info->isLoop);
        Swap32(&info->offset);
    }

    nw::eft::AnimKeyFrameKey* keys = reinterpret_cast<nw::eft::AnimKeyFrameKey*>(info + 1);
    for (u32 i = 0; i < info->keyNum; i++)
        (void)LoadNwEftAnimKeyFrameKey(&keys[i], is_be);
#endif // EFT_IS_WIN

    return info;
}

static inline nw::eft::KeyFrameAnimArray* LoadNwEftKeyFrameAnimArray(void* data, bool is_be = true)
{
    nw::eft::KeyFrameAnimArray* keyAnimArray = static_cast<nw::eft::KeyFrameAnimArray*>(data);

#if EFT_IS_WIN
    if (is_be)
    {
        Swap32(&keyAnimArray->numAnims);
    }

    nw::eft::KeyFrameAnim* info = reinterpret_cast<nw::eft::KeyFrameAnim*>(keyAnimArray + 1);
    for (u32 i = 0; i < keyAnimArray->numAnims; i++)
    {
        if (i != 0)
            info = reinterpret_cast<nw::eft::KeyFrameAnim*>((u32)info + info->offset);

        (void)LoadNwEftKeyFrameAnim(info, is_be);
    }
#endif // EFT_IS_WIN

    return keyAnimArray;
}
