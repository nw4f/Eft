#include <nw/eft/eft_Data.h>
#include <nw/eft/eft_Emitter.h>
#include <nw/eft/eft_Heap.h>
#include <nw/eft/eft_Misc.h>
#include <nw/eft/eft_Primitive.h>
#include <nw/eft/eft_Resource.h>
#include <nw/eft/eft_Shader.h>
#include <nw/eft/eft_System.h>

#include <new>

namespace nw { namespace eft {

#define EFT_MAKE_MAGIC(a, b, c, d) ((a) << 24 | (b) << 16 | (c) << 8 | (d))

Resource::Resource(Heap* heap, void* bin, u32 resourceID, System* eftSys, bool delayCompile)
{
    Initialize(heap, bin, resourceID, eftSys, delayCompile);
}

Resource::~Resource()
{
}

void Resource::CreateFtexbTextureHandle(Heap* heap, void* texture_data, TextureRes& texRes)
{
    GX2SurfaceFormat texture_format_table[] = {
        GX2_SURFACE_FORMAT_TCS_R8_G8_B8_A8_UNORM,
        GX2_SURFACE_FORMAT_TCS_R8_G8_B8_A8_UNORM,
        GX2_SURFACE_FORMAT_TCS_R8_G8_B8_A8_UNORM,
        GX2_SURFACE_FORMAT_T_BC1_UNORM,
        GX2_SURFACE_FORMAT_T_BC1_SRGB,
        GX2_SURFACE_FORMAT_T_BC2_UNORM,
        GX2_SURFACE_FORMAT_T_BC2_SRGB,
        GX2_SURFACE_FORMAT_T_BC3_UNORM,
        GX2_SURFACE_FORMAT_T_BC3_SRGB,
        GX2_SURFACE_FORMAT_T_BC4_UNORM,
        GX2_SURFACE_FORMAT_T_BC4_SNORM,
        GX2_SURFACE_FORMAT_T_BC5_UNORM,
        GX2_SURFACE_FORMAT_T_BC5_SNORM,
        GX2_SURFACE_FORMAT_TC_R8_UNORM,
        GX2_SURFACE_FORMAT_TC_R8_G8_UNORM,
        GX2_SURFACE_FORMAT_TCS_R8_G8_B8_A8_SRGB,
        GX2_SURFACE_FORMAT_TC_R8_SNORM,
        GX2_SURFACE_FORMAT_T_R4_G4_UNORM,
        GX2_SURFACE_FORMAT_TC_R11_G11_B10_FLOAT,
        GX2_SURFACE_FORMAT_TC_R16_FLOAT,
        GX2_SURFACE_FORMAT_TC_R16_G16_FLOAT,
        GX2_SURFACE_FORMAT_TC_R16_G16_B16_A16_FLOAT,
        GX2_SURFACE_FORMAT_TCD_R32_FLOAT,
        GX2_SURFACE_FORMAT_TC_R32_G32_FLOAT,
        GX2_SURFACE_FORMAT_TC_R32_G32_B32_A32_FLOAT,
        GX2_SURFACE_FORMAT_TCS_R5_G6_B5_UNORM,
        GX2_SURFACE_FORMAT_TC_R5_G5_B5_A1_UNORM,
    };

    GX2SurfaceFormat texformat = GX2_SURFACE_FORMAT_TCS_R8_G8_B8_A8_UNORM;
    if (texRes.nativeDataFormat <= EFT_TEXTURE_FORMAT_UNORM_5_5_5_1 && texRes.nativeDataFormat > EFT_TEXTURE_FORMAT_NONE)
        texformat = texture_format_table[texRes.nativeDataFormat];
    else
        WARNING("Input FTX Texture Format is out of support.\n");

    if (texRes.depth == 1)
        GX2InitTexture(&texRes.gx2Texture, texRes.width, texRes.height, texRes.depth, texRes.mipLevel, texformat, GX2_SURFACE_DIM_2D);
    else
        GX2InitTexture(&texRes.gx2Texture, texRes.width, texRes.height, texRes.depth, texRes.mipLevel, texformat, GX2_SURFACE_DIM_2D_ARRAY);

    texRes.gx2Texture.surface.tileMode = texRes.tileMode;

    GX2CalcSurfaceSizeAndAlignment(&texRes.gx2Texture.surface);
    GX2SetSurfaceSwizzle(&texRes.gx2Texture.surface, texRes.swizzle);
    GX2InitTexturePtrs(&texRes.gx2Texture, texture_data, NULL);
    GX2InitTextureCompSel(&texRes.gx2Texture, texRes.compSel);
    GX2InitTextureRegs(&texRes.gx2Texture);
    DCFlushRange(texRes.gx2Texture.surface.imagePtr, texRes.gx2Texture.surface.imageSize + texRes.gx2Texture.surface.mipSize);

    texRes.handle = 1;
}

void Resource::CreateOriginalTextureHandle(Heap* heap, void* texture_data, TextureRes& texRes)
{
    GX2InitTexture(&texRes.gx2Texture, texRes.width, texRes.height, 1, 0, GX2_SURFACE_FORMAT_TCS_R8_G8_B8_A8_UNORM, GX2_SURFACE_DIM_2D);
    texRes.gx2Texture.surface.tileMode = GX2_TILE_MODE_LINEAR_ALIGNED;

    GX2CalcSurfaceSizeAndAlignment(&texRes.gx2Texture.surface);
    GX2InitTextureRegs(&texRes.gx2Texture);

    void* texAddr = heap->Alloc(texRes.gx2Texture.surface.imageSize, texRes.gx2Texture.surface.alignment);
    GX2InitTexturePtrs(&texRes.gx2Texture, texAddr, NULL);

    u8* col8 = (u8*)texture_data;
    u8 r = 0;
    u8 g = 0;
    u8 b = 0;
    u8 a = 0;

    if (texRes.originalDataFormat == EFT_TEXTURE_FORMAT_32BIT_COLOR)
    {
        for (u32 y = 0; y < texRes.height; y++)
        {
            for (u32 x = 0; x < texRes.width; x++)
            {
                b = *col8; col8++;
                g = *col8; col8++;
                r = *col8; col8++;
                a = *col8; col8++;

                ((u32*)texAddr)[y * texRes.gx2Texture.surface.pitch + x] = ( r << 24 |
                                                                             g << 16 |
                                                                             b <<  8 |
                                                                             a <<  0 );
            }
        }
    }
    else
    {
        for (u32 y = 0; y < texRes.height; y++)
        {
            for (u32 x = 0; x < texRes.width; x++)
            {
                a = 255;
                b = *col8; col8++;
                g = *col8; col8++;
                r = *col8; col8++;

                ((u32*)texAddr)[y * texRes.gx2Texture.surface.pitch + x] = ( r << 24 |
                                                                             g << 16 |
                                                                             b <<  8 |
                                                                             a <<  0 );
            }
        }
    }

    DCFlushRange(texRes.gx2Texture.surface.imagePtr, texRes.gx2Texture.surface.imageSize);

    texRes.handle = 1;
}

void Resource::Initialize(Heap* heap, void* bin, u32 resourceID, System* eftSys, bool delayCompile)
{
    mSystem         = eftSys;
    mHeap           = heap;
    mShader         = NULL;
    mShaderNum      = 0;
    mPrimitive      = NULL;
    mPrimitiveNum   = 0;

    mHeader         = reinterpret_cast<HeaderData*>(bin);
    mNameTbl        = reinterpret_cast<char*>((u32)bin + mHeader->nameTblPos);
    mTextureTbl     = reinterpret_cast<char*>((u32)bin + mHeader->textureTblPos);

    u32 magic = EFT_MAKE_MAGIC(mHeader->magic[0], mHeader->magic[1], mHeader->magic[2], mHeader->magic[3]);
    if (magic != EFT_MAKE_MAGIC('E', 'F', 'T', 'F'))
        ERROR("Binary Target is Windows.\n");

    if (EFT_BINARY_VERSION != mHeader->version)
        ERROR("Binary Version Error. Data Version:%d, Runtime Version:%d, DataName:%s.\n", mHeader->version, EFT_BINARY_VERSION, mNameTbl);

    if (mHeader->numEmitterSet == 0)
    {
        WARNING("EmitterSet is Empty.\n");
        return;
    }

    mResourceID = resourceID;

    {
        ShaderImageInformation* shaderImageInfo = reinterpret_cast<ShaderImageInformation*>((u32)bin + mHeader->shaderTblPos);
        ShaderInformation* shaderInfo = reinterpret_cast<ShaderInformation*>((u32)shaderImageInfo + shaderImageInfo->offsetShaderBinInfo);

        mShaderNum = shaderImageInfo->shaderNum;
        LOG("Resource Setup. Setup Shader Num : %d \n", mShaderNum);
        mShader = static_cast<ParticleShader**>(heap->Alloc(sizeof(ParticleShader*) * mShaderNum));

        for (u32 i = 0; i < mShaderNum; i++)
        {
            mShader[i] = new (heap->Alloc(sizeof(ParticleShader))) ParticleShader();
            mShader[i]->SetVertexShaderKey  (&shaderInfo->vertexShaderKey);
            mShader[i]->SetFragmentShaderKey(&shaderInfo->fragmentShaderKey);
            mShader[i]->SetGeometryShaderKey(&shaderInfo->geometryShaderKey);

            char* shaderBinTop = reinterpret_cast<char*>((u32)shaderImageInfo + shaderImageInfo->offsetShaderBinary);
            mShader[i]->SetupShaderResource(mHeap, shaderBinTop + shaderInfo->offset, shaderInfo->shaderSize);
            shaderInfo++;
        }
    }

    {
        PrimitiveImageInformation* imageInfo = reinterpret_cast<PrimitiveImageInformation *>((u32)bin + mHeader->primitiveTblPos);
        PrimitiveTableInfo* infoTop = reinterpret_cast<PrimitiveTableInfo*>((u32)imageInfo + (u32)imageInfo->offsetPrimitiveTableInfo);
        char* primitiveTableStart = reinterpret_cast<char*>(infoTop + imageInfo->primitiveNum);

        mPrimitiveNum = imageInfo->primitiveNum;
        if (mPrimitiveNum != 0)
        {
            LOG("Resource Setup. Setup Primitive Num : %d \n", mPrimitiveNum);
            mPrimitive = static_cast<Primitive**>(heap->Alloc(sizeof(Primitive*) * mPrimitiveNum));

            for (u32 i = 0; i < mPrimitiveNum; i++)
            {
                mPrimitive[i] = new (heap->Alloc(sizeof(Primitive))) Primitive();

                PrimitiveTableInfo* info = &infoTop[i];

                f32* position   = reinterpret_cast<f32*>(info->pos.offset      + primitiveTableStart);
                f32* texCrd     = reinterpret_cast<f32*>(info->texCoord.offset + primitiveTableStart);
                u32* index      = reinterpret_cast<u32*>(info->index.offset    + primitiveTableStart);

                f32* normal = NULL;
                if (info->normal.offset > 0)
                    normal = reinterpret_cast<f32*>(info->normal.offset + primitiveTableStart);

                f32* color = NULL;
                if (info->color.offset > 0)
                    color = reinterpret_cast<f32*>(info->color.offset + primitiveTableStart);

                mPrimitive[i]->Initialize(heap, info->index.count,
                    position, info->pos.size,
                    normal,   info->normal.size,
                    color,    info->color.size,
                    texCrd,   info->texCoord.size,
                    index,    info->index.size);
            }
        }
    }

    mResEmitterSet = static_cast<ResourceEmitterSet*>(mHeap->Alloc(sizeof(ResourceEmitterSet) * mHeader->numEmitterSet));

    u32 emitterNum = 0;
    for (s32 i = 0; i < mHeader->numEmitterSet; i++)
    {
        ResourceEmitterSet* resSet = &mResEmitterSet[i];
        resSet->setData = reinterpret_cast<EmitterSetData*>((u32)mHeader + sizeof(HeaderData) + sizeof(EmitterSetData) * i);
        emitterNum += resSet->setData->numEmitter;
    }

    mEmitterStaticUbo = static_cast<EmitterStaticUniformBlock*>(mHeap->Alloc(sizeof(EmitterStaticUniformBlock) * emitterNum * 2, 0x100));

    u32 uboIdx = 0;

    for (s32 i = 0; i < mHeader->numEmitterSet; i++)
    {
        ResourceEmitterSet* resSet = &mResEmitterSet[i];
        void* texture_addr = NULL;

        resSet->setData->name   = &mNameTbl[ resSet->setData->namePos ];
        resSet->setName         = resSet->setData->name;
        resSet->numEmitter      = resSet->setData->numEmitter;
        resSet->userData        = resSet->setData->userData;
        resSet->shaderArray     = mShader;
        resSet->shaderNum       = mShaderNum;
        resSet->primitiveArray  = mPrimitive;
        resSet->primitiveNum    = mPrimitiveNum;

        if (resSet->setData->emitterTblPos != 0)
        {
            resSet->tblData = (/*resSet->setData->emitterTbl =*/ reinterpret_cast<EmitterTblData*>((u32)bin + resSet->setData->emitterTblPos));

            for (u32 j = 0; j < resSet->numEmitter; j++)
            {
                EmitterTblData* table = &resSet->tblData[j];

                if (table->emitterPos == 0)
                {
                    table->emitterResource         = NULL;
                    table->staticUniformBlock      = NULL;
                    table->childStaticUniformBlock = NULL;
                }
                else
                {
                    table->emitterResource = reinterpret_cast<CommonEmitterData*>((u32)bin + table->emitterPos);
                    table->emitterResource->name = &mNameTbl[table->emitterResource->namePos];

                    table->staticUniformBlock      = &mEmitterStaticUbo[uboIdx]; uboIdx++;
                    table->childStaticUniformBlock = &mEmitterStaticUbo[uboIdx]; uboIdx++;

                    SimpleEmitterData*  res  = static_cast<SimpleEmitterData*>(table->emitterResource);
                    ComplexEmitterData* cres = NULL;
                    if (table->emitterResource->type == EFT_EMITTER_TYPE_COMPLEX)
                        cres = static_cast<ComplexEmitterData*>(table->emitterResource);

                    EmitterInstance::UpdateEmitterStaticUniformBlock(table->staticUniformBlock, res, cres);

                    for (s32 k = 0; k < EFT_TEXTURE_SLOT_BIN_MAX; k++)
                    {
                        if (table->emitterResource->texRes[k].nativeDataSize > 0)
                        {
                            texture_addr = reinterpret_cast<void*>((u32)mTextureTbl + table->emitterResource->texRes[k].nativeDataPos);
                            CreateFtexbTextureHandle(heap, texture_addr, table->emitterResource->texRes[k]);
                        }
                        else if (table->emitterResource->texRes[k].originalDataSize > 0)
                        {
                            texture_addr = reinterpret_cast<void*>((u32)mTextureTbl + table->emitterResource->texRes[k].originalDataPos);
                            CreateOriginalTextureHandle(heap, texture_addr, table->emitterResource->texRes[k]);
                        }
                        else if (k == 0)
                        {
                            ERROR("Texture Binary is None.\n");
                        }
                    }

                    if (table->emitterResource->type == EFT_EMITTER_TYPE_COMPLEX )
                    {
                        ComplexEmitterData* complex = static_cast<ComplexEmitterData*>(table->emitterResource);
                        if (complex->childFlg & EFT_CHILD_FLAG_ENABLE)
                        {
                            ChildData* cres = reinterpret_cast<ChildData*>(complex + 1);
                            if (cres->childTex.nativeDataSize > 0)
                            {
                                texture_addr = reinterpret_cast<void*>((u32)mTextureTbl + cres->childTex.nativeDataPos);
                                CreateFtexbTextureHandle(heap, texture_addr, cres->childTex);
                            }
                            else if (cres->childTex.originalDataSize > 0)
                            {
                                texture_addr = reinterpret_cast<void*>((u32)mTextureTbl + cres->childTex.originalDataPos);
                                CreateOriginalTextureHandle(heap, texture_addr, cres->childTex);
                            }
                            else
                            {
                                ERROR("Child Texture Binary is None.\n");
                            }
                        }
                    }

                    if (table->emitterResource->animKeyTable.dataSize )
                    {
                        char* animkeyTble = reinterpret_cast<char*>((u32)bin + mHeader->animkeyTblPos);
                        table->emitterResource->animKeyTable.animKeyTable = animkeyTble + table->emitterResource->animKeyTable.animPos;
                    }

                    if (table->emitterResource->customShaderParam.paramNum > 0 )
                    {
                        f32* paramBuf = reinterpret_cast<f32*>((u32)bin + mHeader->shaderParamTblPos + table->emitterResource->customShaderParam.paramPos);
                        table->emitterResource->customShaderParam.param = paramBuf;
                    }

                    if (table->emitterResource->type == EFT_EMITTER_TYPE_COMPLEX )
                    {
                        ComplexEmitterData* complex = static_cast<ComplexEmitterData*>(table->emitterResource);
                        if (complex->childFlg & EFT_CHILD_FLAG_ENABLE)
                        {
                            ChildData* cres = reinterpret_cast<ChildData*>(complex + 1);

                            if (cres->childCustomShaderParam.paramNum > 0)
                            {
                                f32* paramBuf = reinterpret_cast<f32*>((u32)bin + mHeader->shaderParamTblPos + cres->childCustomShaderParam.paramPos);
                                cres->childCustomShaderParam.param = paramBuf;
                            }
                        }
                    }
                }
            }
        }
        else
        {
            resSet->tblData = (/*resSet->setData->emitterTbl =*/ NULL);
        }

        resSet->setDataROM      = resSet->setData;
        resSet->tblDataROM      = resSet->tblData;
        resSet->numEmitterROM   = resSet->numEmitter;
        resSet->userDataROM     = resSet->userData;
    }
}

void Resource::DeleteTextureHandle(Heap* heap, TextureRes& texRes, bool isOriginalTexture)
{
    if (heap && isOriginalTexture)
        heap->Free(texRes.gx2Texture.surface.imagePtr);

    texRes.handle = 0;
}

void Resource::Finalize(Heap* heap)
{
    Heap* heapTemp = NULL;
    if (heap)
        heapTemp = heap;
    else
        heapTemp = mHeap;

    for (s32 i = 0; i < mHeader->numEmitterSet; i++)
    {
        ResourceEmitterSet* resSet = &mResEmitterSet[i];
        for (s32 j = 0; j < resSet->setData->numEmitter; j++)
        {
            EmitterTblData* e = &resSet->tblData[j];

            for (s32 k = 0; k < EFT_TEXTURE_SLOT_BIN_MAX; k++)
            {
                if (e->emitterResource->texRes[k].handle)
                {
                    bool isOriginalTexture = (e->emitterResource->texRes[k].nativeDataSize == 0) ? true : false;
                    DeleteTextureHandle(heapTemp, e->emitterResource->texRes[k], isOriginalTexture);
                }
            }

            if (e->emitterResource->type == EFT_EMITTER_TYPE_COMPLEX)
            {
                ComplexEmitterData* complex = static_cast<ComplexEmitterData*>(e->emitterResource);
                if (complex->childFlg & EFT_CHILD_FLAG_ENABLE)
                {
                    ChildData* cres = reinterpret_cast<ChildData*>(complex + 1);
                    if (cres->childTex.handle)
                    {
                        bool isOriginalTexture = (cres->childTex.nativeDataSize == 0) ? true : false;
                        DeleteTextureHandle(heapTemp, cres->childTex, isOriginalTexture);
                    }
                }
            }
        }
    }

    for (u32 i = 0; i < mShaderNum; i++)
    {
        mShader[i]->Finalize(heapTemp);
        heapTemp->Free(mShader[i]);
    }

    heapTemp->Free(mShader);

    if (mResEmitterSet)
    {
        heapTemp->Free(mResEmitterSet);
        mResEmitterSet = NULL;
    }

    if (mPrimitive)
    {
        for (u32 i = 0; i < mPrimitiveNum; i++)
        {
            mPrimitive[i]->Finalize(heapTemp);
            heapTemp->Free(mPrimitive[i]);
        }

        heapTemp->Free(mPrimitive);
    }

    if (mEmitterStaticUbo)
    {
        heapTemp->Free(mEmitterStaticUbo);
        mEmitterStaticUbo = NULL;
    }
}

s32 Resource::SearchEmitterSetID(const char* name) const
{
    for (s32 i = 0; i < mHeader->numEmitterSet; i++)
    {
        ResourceEmitterSet* resSet = &mResEmitterSet[i];
        if (strcmp(name, &mNameTbl[resSet->setData->namePos]) == 0)
            return i;
    }

    return EFT_INVALID_EMITTER_SET_ID;
}

s32 Resource::SearchEmitterID(s32 emitterSetID, const char* emitterName) const
{
    for (u32 j = 0; j < mResEmitterSet[emitterSetID].numEmitter; ++j)
        if (strcmp(emitterName, mResEmitterSet[emitterSetID].tblDataROM[j].emitterResource->name) == 0)
            return j;

    return EFT_INVALID_EMITTER_ID;
}

s32 Resource::SearchEmitterID(s32 emitterSetID, const char* emitterName, u32 emitterIdx) const
{
    for (u32 j = 0; j < mResEmitterSet[emitterSetID].numEmitter; ++j)
        if (strcmp(emitterName, mResEmitterSet[emitterSetID].tblDataROM[j].emitterResource->name) == 0)
            if (j == emitterIdx)
                return j;

    return EFT_INVALID_EMITTER_ID;
}

bool Resource::BindResource(s32 targetSetID, ResourceBind* bind, EmitterTblData* newTbl, s32 newNumEmitter, const char* newSetName, u32 newUserData, u32 newShaderNum, ParticleShader** newShaderArray, u32 newPrimitiveNum, Primitive** newPrimitiveArray)
{
    ResourceEmitterSet* resSet = &mResEmitterSet[targetSetID];

    void* oldRes[EFT_EMITTER_INSET_NUM];
    s32   numRes = resSet->numEmitter;
    for (s32 i = 0; i < numRes; i++)
        oldRes[i] = resSet->tblData[i].emitterResource;

    bind->source                 = resSet->setData;
    bind->emitterSetID           = targetSetID;
    bind->resourceID             = mResourceID;
    bind->saveName               = resSet->setName;
    bind->saveNumEmitter         = resSet->numEmitter;
    bind->saveTbl                = resSet->tblData;
    bind->saveUserData           = resSet->userData;

    bind->shaderNum              = resSet->shaderNum;
    bind->shaderArray            = resSet->shaderArray;

    bind->primitiveNum           = resSet->primitiveNum;
    bind->primitiveArray         = resSet->primitiveArray;

    resSet->tblData              = newTbl;
    resSet->numEmitter           = newNumEmitter;
    resSet->setName              = newSetName;
    resSet->userData             = newUserData;

    resSet->shaderNum            = newShaderNum;
    resSet->shaderArray          = newShaderArray;

    resSet->primitiveNum         = newPrimitiveNum;
    resSet->primitiveArray       = newPrimitiveArray;

    mSystem->ReCreateEmitter(oldRes, numRes, mResourceID, targetSetID, false );

    return true;
}

bool Resource::UnbindResource(ResourceBind* bind, bool isReBind, bool isKill)
{
    ResourceEmitterSet* resSet = &mResEmitterSet[bind->emitterSetID];

    void* oldRes[EFT_EMITTER_INSET_NUM];
    s32   numRes = resSet->numEmitter;
    for (s32 i = 0; i < numRes; i++)
        oldRes[i] = resSet->tblData[i].emitterResource;

    resSet->tblData    = bind->saveTbl;
    resSet->numEmitter = bind->saveNumEmitter;
    resSet->setName    = bind->saveName;
    resSet->userData   = bind->saveUserData;

    resSet->shaderNum   = bind->shaderNum;
    resSet->shaderArray = bind->shaderArray;

    resSet->primitiveNum   = bind->primitiveNum;
    resSet->primitiveArray = bind->primitiveArray;

    bind->source = NULL;

    if (isReBind)
        mSystem->ReCreateEmitter(oldRes, numRes, mResourceID, bind->emitterSetID, false);

    if (isKill)
        mSystem->ReCreateEmitter(oldRes, numRes, mResourceID, bind->emitterSetID , true);

    return true;
}

ParticleShader* Resource::GetShader(s32 emitterSetID, u32 index)
{
    u32 shaderNum = mResEmitterSet[emitterSetID].shaderNum;
    if (shaderNum <= index)
        return NULL;

    ParticleShader** shaderArray = mResEmitterSet[emitterSetID].shaderArray;

    return shaderArray[index];
}

} } // namespace nw::eft
