#include <nw/eft/eft_Resource.h>
#include <nw/eft/eft_Shader.h>

#include <nw/eft/eft_Data.hpp>

#include <new>

#if EFT_IS_CAFE_WUT || !EFT_IS_CAFE
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#endif

namespace nw { namespace eft {

Resource::Resource(Heap* heap, void* bin, s32 resourceID, System* ptclSys)
{
    Initialize(heap, bin, resourceID, ptclSys);
}

Resource::~Resource()
{
    Finalize(NULL);
}



#if EFT_IS_WIN

static const GX2SurfaceFormat TextureFormat2GX2Tbl[] = {
    GX2_SURFACE_FORMAT_UNORM_RGBA8,
    GX2_SURFACE_FORMAT_UNORM_RGBA8,
    GX2_SURFACE_FORMAT_UNORM_RGBA8,
    GX2_SURFACE_FORMAT_UNORM_BC1,
    GX2_SURFACE_FORMAT_SRGB_BC1,
    GX2_SURFACE_FORMAT_UNORM_BC2,
    GX2_SURFACE_FORMAT_SRGB_BC2,
    GX2_SURFACE_FORMAT_UNORM_BC3,
    GX2_SURFACE_FORMAT_SRGB_BC3,
    GX2_SURFACE_FORMAT_UNORM_BC4,
    GX2_SURFACE_FORMAT_SNORM_BC4,
    GX2_SURFACE_FORMAT_UNORM_BC5,
    GX2_SURFACE_FORMAT_SNORM_BC5,
    GX2_SURFACE_FORMAT_UNORM_R8,
    GX2_SURFACE_FORMAT_UNORM_RG8,
    GX2_SURFACE_FORMAT_SRGB_RGBA8
};

struct GLTexFormat
{
    GLint internalformat;
    GLenum format;
    GLenum type;
};

static const GLTexFormat TextureFormat2GLTbl[] = {
    { GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE },
    { GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE },
    { GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE },
    { GL_COMPRESSED_RGBA_S3TC_DXT1_EXT },
    { GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT },
    { GL_COMPRESSED_RGBA_S3TC_DXT3_EXT },
    { GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT },
    { GL_COMPRESSED_RGBA_S3TC_DXT5_EXT },
    { GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT },
    { GL_COMPRESSED_RED_RGTC1 },
    { GL_COMPRESSED_SIGNED_RED_RGTC1 },
    { GL_COMPRESSED_RG_RGTC2 },
    { GL_COMPRESSED_SIGNED_RG_RGTC2 },
    { GL_R8, GL_RED, GL_UNSIGNED_BYTE },
    { GL_RG8, GL_RG, GL_UNSIGNED_BYTE },
    { GL_SRGB8_ALPHA8, GL_RGBA, GL_UNSIGNED_BYTE }
};

static const GLint GX2CompSel2GLTbl[6] = {
    GL_RED, GL_GREEN, GL_BLUE,
    GL_ALPHA, GL_ZERO, GL_ONE
};

void Resource::CreateFtexbTextureHandle(Heap* heap, void* texture_data, TextureRes& texRes)
{
    GX2SurfaceFormat texformat = GX2_SURFACE_FORMAT_UNORM_RGBA8;
    if (texRes.nativeDataFormat <= EFT_TEXTURE_FORMAT_SRGB_8_8_8_8 && texRes.nativeDataFormat > EFT_TEXTURE_FORMAT_NONE)
        texformat = TextureFormat2GX2Tbl[texRes.nativeDataFormat];

    GX2Surface surface;
    surface.dim = GX2_SURFACE_DIM_2D;
    surface.width = texRes.width;
    surface.height = texRes.height;
    surface.depth = 1;
    surface.numMips = texRes.mipLevel;
    surface.format = texformat;
    surface.aa = GX2_AA_MODE_1X;
    surface.use = GX2_SURFACE_USE_TEXTURE;
    surface.tileMode = texRes.tileMode;
    surface.swizzle = texRes.swizzle << 8;
    GX2CalcSurfaceSizeAndAlignment(&surface);

    surface.imagePtr = texture_data;
    surface.mipPtr = (surface.numMips > 1) ? (u8*)texture_data + surface.imageSize : NULL;

    for (s32 i = 0; i < 13; i++)
        surface.mipOffset[i] = texRes.mipOffset[i];

    GLint compSel[4] = {
        GX2CompSel2GLTbl[texRes.compSel >> 24 & 0xFF],
        GX2CompSel2GLTbl[texRes.compSel >> 16 & 0xFF],
        GX2CompSel2GLTbl[texRes.compSel >>  8 & 0xFF],
        GX2CompSel2GLTbl[texRes.compSel >>  0 & 0xFF]
    };

    glGenTextures(1, &texRes.handle);
    glBindTexture(GL_TEXTURE_2D, texRes.handle);
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, compSel);

    GX2Surface& linear_surface = texRes.gx2Texture.surface;
    linear_surface.dim = GX2_SURFACE_DIM_2D;
    linear_surface.width = texRes.width;
    linear_surface.height = texRes.height;
    linear_surface.depth = 1;
    linear_surface.numMips = 1;
    linear_surface.format = texformat;
    linear_surface.aa = GX2_AA_MODE_1X;
    linear_surface.use = GX2_SURFACE_USE_TEXTURE;
    linear_surface.tileMode = GX2_TILE_MODE_LINEAR_SPECIAL;
    linear_surface.swizzle = 0;
    GX2CalcSurfaceSizeAndAlignment(&linear_surface);

    linear_surface.imagePtr = heap->Alloc(linear_surface.imageSize);
    linear_surface.mipPtr = NULL;

    GX2CopySurface(&surface, 0, 0, &linear_surface, 0, 0);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    const GLTexFormat* glFormat = &TextureFormat2GLTbl[0];
    if (texRes.nativeDataFormat <= EFT_TEXTURE_FORMAT_SRGB_8_8_8_8 && texRes.nativeDataFormat > EFT_TEXTURE_FORMAT_NONE)
        glFormat = &TextureFormat2GLTbl[texRes.nativeDataFormat];

    if (EFT_TEXTURE_FORMAT_UNORM_BC1 <= texRes.nativeDataFormat && texRes.nativeDataFormat <= EFT_TEXTURE_FORMAT_SRGB_BC3)
        glCompressedTexImage2DARB(GL_TEXTURE_2D, 0, glFormat->internalformat, texRes.width, texRes.height, 0, linear_surface.imageSize, linear_surface.imagePtr);

    else if (EFT_TEXTURE_FORMAT_UNORM_BC4 <= texRes.nativeDataFormat && texRes.nativeDataFormat <= EFT_TEXTURE_FORMAT_SNORM_BC5)
        glCompressedTexImage2D(GL_TEXTURE_2D, 0, glFormat->internalformat, texRes.width, texRes.height, 0, linear_surface.imageSize, linear_surface.imagePtr);

    else
        glTexImage2D(GL_TEXTURE_2D, 0, glFormat->internalformat, texRes.width, texRes.height, 0, glFormat->format, glFormat->type, linear_surface.imagePtr);
}

void Resource::CreateOriginalTextureHandle(Heap* heap, void* texture_data, TextureRes& texRes)
{
    GX2Surface& linear_surface = texRes.gx2Texture.surface;
    linear_surface.dim = GX2_SURFACE_DIM_2D;
    linear_surface.width = texRes.width;
    linear_surface.height = texRes.height;
    linear_surface.depth = 1;
    linear_surface.numMips = 1;
    linear_surface.format = GX2_SURFACE_FORMAT_UNORM_RGBA8;
    linear_surface.aa = GX2_AA_MODE_1X;
    linear_surface.use = GX2_SURFACE_USE_TEXTURE;
    linear_surface.tileMode = GX2_TILE_MODE_LINEAR_SPECIAL;
    linear_surface.swizzle = 0;
    GX2CalcSurfaceSizeAndAlignment(&linear_surface);

    linear_surface.imagePtr = heap->Alloc(linear_surface.imageSize);
    linear_surface.mipPtr = NULL;

    if (texRes.originalDataFormat == EFT_TEXTURE_FORMAT_32BIT_COLOR)
        std::memcpy(linear_surface.imagePtr, texture_data, linear_surface.imageSize);

    else
    {
        u8* const texAddr = (u8*)linear_surface.imagePtr;

        for (u32 i = 0; i < texRes.width * texRes.height; i++)
        {
            texAddr[i * 4 + 0] = ((u8*)texture_data)[i * 3 + 0];
            texAddr[i * 4 + 1] = ((u8*)texture_data)[i * 3 + 1];
            texAddr[i * 4 + 2] = ((u8*)texture_data)[i * 3 + 2];
            texAddr[i * 4 + 3] = 0xFF;
        }
    }

    GLint compSel[4] = {
        GX2CompSel2GLTbl[texRes.compSel >> 24 & 0xFF],
        GX2CompSel2GLTbl[texRes.compSel >> 16 & 0xFF],
        GX2CompSel2GLTbl[texRes.compSel >>  8 & 0xFF],
        GX2CompSel2GLTbl[texRes.compSel >>  0 & 0xFF]
    };

    glGenTextures(1, &texRes.handle);
    glBindTexture(GL_TEXTURE_2D, texRes.handle);
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, compSel);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texRes.width, texRes.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, linear_surface.imagePtr);
}

#endif // EFT_IS_WIN

#if EFT_IS_CAFE

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
    };

    GX2SurfaceFormat texformat = GX2_SURFACE_FORMAT_TCS_R8_G8_B8_A8_UNORM;
    if (texRes.nativeDataFormat <= EFT_TEXTURE_FORMAT_SRGB_8_8_8_8 && texRes.nativeDataFormat > EFT_TEXTURE_FORMAT_NONE)
        texformat = texture_format_table[texRes.nativeDataFormat];

    GX2InitTexture(&texRes.gx2Texture, texRes.width, texRes.height, 1, texRes.mipLevel, texformat, GX2_SURFACE_DIM_2D);
    texRes.gx2Texture.surface.tileMode = texRes.tileMode;

    GX2CalcSurfaceSizeAndAlignment(&texRes.gx2Texture.surface);
    GX2SetSurfaceSwizzle(&texRes.gx2Texture.surface, texRes.swizzle);
    GX2InitTexturePtrs(&texRes.gx2Texture, texture_data, NULL);

    for (s32 i = 0; i < 13; i++)
        texRes.gx2Texture.surface.mipOffset[i] = texRes.mipOffset[i];

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

    u8* col8 = static_cast<u8*>(texture_data);
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
                r = *col8; col8++;
                g = *col8; col8++;
                b = *col8; col8++;
                a = *col8; col8++;

                ((u32*)texAddr)[y * texRes.gx2Texture.surface.pitch + x] = (r << 24 |
                                                                            g << 16 |
                                                                            b <<  8 |
                                                                            a);
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
                r = *col8; col8++;
                g = *col8; col8++;
                b = *col8; col8++;

                ((u32*)texAddr)[y * texRes.gx2Texture.surface.pitch + x] = (r << 24 |
                                                                            g << 16 |
                                                                            b <<  8 |
                                                                            a);
            }
        }
    }

    DCFlushRange(texRes.gx2Texture.surface.imagePtr, texRes.gx2Texture.surface.imageSize);

    texRes.handle = 1;
}

#endif // EFT_IS_CAFE

void Resource::Initialize(Heap* heap, void* bin, s32 resourceID, System* ptclSys)
{
    mSystem       = ptclSys;
    mHeap         = heap;
    mShader       = NULL;
    mShaderNum    = 0;
    mPrimitive    = NULL;
    mPrimitiveNum = 0;

    mResourceID = resourceID;

    mHeader     = LoadNwEftHeaderData(bin);
    mNameTbl    = reinterpret_cast<char*>((u32)bin + mHeader->nameTblPos);
    mTextureTbl = reinterpret_cast<char*>((u32)bin + mHeader->textureTblPos);

    if (mHeader->numEmitterSet == 0)
        return;

    {
        ShaderImageInformation* shaderImageInfo = LoadNwEftShaderImageInformation((void*)((u32)bin + mHeader->shaderTblPos));
        ShaderInformation* shaderInfo = reinterpret_cast<ShaderInformation*>((u32)shaderImageInfo + shaderImageInfo->offsetShaderBinInfo);
        char* binTop = reinterpret_cast<char*>(shaderInfo + shaderImageInfo->shaderNum);

        mShaderNum = shaderImageInfo->shaderNum;
        mShader = static_cast<ParticleShader**>(heap->Alloc(sizeof(ParticleShader*) * mShaderNum));

        for (u32 i = 0; i < mShaderNum; i++)
        {
            (void)LoadNwEftShaderInformation(shaderInfo);

            mShader[i] = new (heap->Alloc(sizeof(ParticleShader))) ParticleShader();
            mShader[i]->SetVertexShaderKey(&shaderInfo->vertexShaderKey);
            mShader[i]->SetFragmentShaderKey(&shaderInfo->fragmentShaderKey);
            mShader[i]->SetGeometryShaderKey(&shaderInfo->geometryShaderKey);
            mShader[i]->SetupShaderResource(mHeap, binTop + shaderInfo->offset, shaderInfo->shaderSize);
            shaderInfo++;
        }
    }

    {
        PrimitiveImageInformation* imageInfo = LoadNwEftPrimitiveImageInformation((void*)((u32)bin + mHeader->primitiveTblPos));
        PrimitiveTableInfo* infoTop = reinterpret_cast<PrimitiveTableInfo*>((u32)imageInfo + imageInfo->offsetPrimitiveTableInfo);
        char* primitiveTableStart = reinterpret_cast<char*>(infoTop + imageInfo->primitiveNum);

        mPrimitiveNum = imageInfo->primitiveNum;
        if (mPrimitiveNum != 0)
        {
            mPrimitive = static_cast<Primitive**>(heap->Alloc(sizeof(Primitive*) * mPrimitiveNum));

            for (u32 i = 0; i < mPrimitiveNum; i++)
            {
                mPrimitive[i] = new (heap->Alloc(sizeof(Primitive))) Primitive();

                PrimitiveTableInfo* info = &infoTop[i];
                (void)LoadNwEftPrimitiveTableInfo(info);

                f32* position = reinterpret_cast<f32*>(info->pos.offset      + primitiveTableStart);
                f32* texCrd   = reinterpret_cast<f32*>(info->texCoord.offset + primitiveTableStart);
                u32* index    = reinterpret_cast<u32*>(info->index.offset    + primitiveTableStart);

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

    for (s32 i = 0; i < mHeader->numEmitterSet; i++)
    {
        ResourceEmitterSet* resSet = &mResEmitterSet[i];
        void* texture_addr = NULL;

        resSet->setData        = LoadNwEftEmitterSetData(&((reinterpret_cast<EmitterSetData*>(mHeader + 1))[i]));
        resSet->setData->name  = &mNameTbl[resSet->setData->namePos];
        resSet->setName        = resSet->setData->name;
        resSet->numEmitter     = resSet->setData->numEmitter;
        resSet->userData       = resSet->setData->userData;
        resSet->isShowDetail   = false;

        resSet->shaderArray    = mShader;
        resSet->shaderNum      = mShaderNum;

        resSet->primitiveArray = mPrimitive;
        resSet->primitiveNum   = mPrimitiveNum;

        if (resSet->setData->emitterTblPos != 0)
        {
            resSet->tblData = (/*resSet->setData->emitterTbl =*/ reinterpret_cast<EmitterTblData*>((u32)bin + resSet->setData->emitterTblPos));

            for (s32 j = 0; j < resSet->numEmitter; j++)
            {
                EmitterTblData* e = LoadNwEftEmitterTblData(&resSet->tblData[j]);

                if (e->emitterPos == 0)
                    e->emitter = NULL;

                else
                {
                    e->emitter = LoadNwEftEmitterData((void*)((u32)bin + e->emitterPos));
                    e->emitter->name = &mNameTbl[e->emitter->namePos];

                    if (e->emitter->texRes[EFT_TEXTURE_SLOT_0].nativeDataSize > 0)
                    {
                        texture_addr = reinterpret_cast<void*>((u32)mTextureTbl + e->emitter->texRes[EFT_TEXTURE_SLOT_0].nativeDataPos);
                        CreateFtexbTextureHandle(heap, texture_addr, e->emitter->texRes[EFT_TEXTURE_SLOT_0]);
                    }
                    else if (e->emitter->texRes[EFT_TEXTURE_SLOT_0].originalDataSize > 0)
                    {
                        texture_addr = reinterpret_cast<void*>((u32)mTextureTbl + e->emitter->texRes[EFT_TEXTURE_SLOT_0].originalDataPos);
                        CreateOriginalTextureHandle(heap, texture_addr, e->emitter->texRes[EFT_TEXTURE_SLOT_0]);
                    }

                    if (e->emitter->texRes[EFT_TEXTURE_SLOT_1].nativeDataSize > 0)
                    {
                        texture_addr = reinterpret_cast<void*>((u32)mTextureTbl + e->emitter->texRes[EFT_TEXTURE_SLOT_1].nativeDataPos);
                        CreateFtexbTextureHandle(heap, texture_addr, e->emitter->texRes[EFT_TEXTURE_SLOT_1]);
                    }
                    else if (e->emitter->texRes[EFT_TEXTURE_SLOT_1].originalDataSize > 0)
                    {
                        texture_addr = reinterpret_cast<void*>((u32)mTextureTbl + e->emitter->texRes[EFT_TEXTURE_SLOT_1].originalDataPos);
                        CreateOriginalTextureHandle(heap, texture_addr, e->emitter->texRes[EFT_TEXTURE_SLOT_1]);
                    }

                    if (e->emitter->type == EFT_EMITTER_TYPE_COMPLEX)
                    {
                        ComplexEmitterData* complex = static_cast<ComplexEmitterData*>(e->emitter);
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
                        }
                    }

                    if (e->emitter->animKeyTable.dataSize)
                    {
                        char* animkeyTble = reinterpret_cast<char*>((u32)bin + mHeader->animkeyTblPos);
                        e->emitter->animKeyTable.animKeyTable = LoadNwEftKeyFrameAnimArray(animkeyTble + e->emitter->animKeyTable.animPos);
                    }
                }
            }
        }
        else
        {
            resSet->tblData = (/*resSet->resSet->emitterTbl =*/ NULL);
        }

        resSet->setDataROM    = resSet->setData;
        resSet->tblDataROM    = resSet->tblData;
        resSet->numEmitterROM = resSet->numEmitter;
        resSet->userDataROM   = resSet->userData;
    }
}

void Resource::DeleteTextureHandle(Heap* heap, TextureRes& texRes, bool isOriginalTexture)
{
#if EFT_IS_WIN
    if (heap != NULL)
        heap->Free(texRes.gx2Texture.surface.imagePtr);

    glDeleteTextures(1, &texRes.handle);
#endif // EFT_IS_WIN

#if EFT_IS_CAFE
    if (heap != NULL && isOriginalTexture)
        heap->Free(texRes.gx2Texture.surface.imagePtr);
#endif // EFT_IS_CAFE

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

            if (e->emitter->texRes[EFT_TEXTURE_SLOT_0].handle)
            {
                bool isOriginalTexture = (e->emitter->texRes[EFT_TEXTURE_SLOT_0].nativeDataSize == 0) ? true : false;
                DeleteTextureHandle(heapTemp, e->emitter->texRes[EFT_TEXTURE_SLOT_0], isOriginalTexture);
            }

            if (e->emitter->texRes[EFT_TEXTURE_SLOT_1].handle)
            {
                bool isOriginalTexture = (e->emitter->texRes[EFT_TEXTURE_SLOT_1].nativeDataSize == 0) ? true : false;
                DeleteTextureHandle(heapTemp, e->emitter->texRes[EFT_TEXTURE_SLOT_1], isOriginalTexture);
            }

            if (e->emitter->type == EFT_EMITTER_TYPE_COMPLEX)
            {
                ComplexEmitterData* complex = static_cast<ComplexEmitterData*>(e->emitter);
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
    for (s32 j = 0; j < mResEmitterSet[emitterSetID].numEmitter; ++j)
        if (strcmp(emitterName, mResEmitterSet[emitterSetID].tblDataROM[j].emitter->name) == 0)
            return j;

    return EFT_INVALID_EMITTER_ID;
}

s32 Resource::SearchEmitterID(s32 emitterSetID, const char* emitterName, u32 emitterIdx) const
{
    for (s32 j = 0; j < mResEmitterSet[emitterSetID].numEmitter; ++j)
        if (strcmp(emitterName, mResEmitterSet[emitterSetID].tblDataROM[j].emitter->name) == 0)
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
        oldRes[i] = resSet->tblData[i].emitter;

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

    mSystem->ReCreateEmitter(oldRes, numRes, mResourceID, targetSetID, false);

    return true;
}

bool Resource::UnbindResource(ResourceBind* bind, bool isReBind, bool isKill)
{
    ResourceEmitterSet* resSet = &mResEmitterSet[bind->emitterSetID];

    void* oldRes[EFT_EMITTER_INSET_NUM];
    s32   numRes = resSet->numEmitter;
    for (s32 i = 0; i < numRes; i++)
        oldRes[i] = resSet->tblData[i].emitter;

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
        mSystem->ReCreateEmitter(oldRes, numRes, mResourceID, bind->emitterSetID, true);

    return true;
}

ParticleShader* Resource::GetShader(s32 emitterSetID, const VertexShaderKey* vertexKey, const FragmentShaderKey* fragmentKey)
{
    u32 shaderNum = mResEmitterSet[emitterSetID].shaderNum;
    ParticleShader** shaderArray = mResEmitterSet[emitterSetID].shaderArray;

    for (u32 i = 0; i < shaderNum; i++)
        if (shaderArray[i]->GetVertexShaderKey().IsEqual(vertexKey) && shaderArray[i]->GetFragmentShaderKey().IsEqual(fragmentKey))
            return shaderArray[i];

    return NULL;
}

} } // namespace nw::eft

#if EFT_IS_CAFE_WUT || !EFT_IS_CAFE
#pragma GCC diagnostic pop
#endif
