#include <nw/eft/cafe/eft_CafeWrapper.h>

BOOL _DEMOGFDReadVertexShader(nw::eft::Heap* heap, GX2VertexShader** ppShader, u32 index, const void* pData)
{
    GX2VertexShader* pHeader;
    void* pProgram;
    u32 ret;
    u32 headerSize;
    u32 programSize;

    if (pData == NULL || ppShader == NULL || index >= GFDGetVertexShaderCount(pData))
        return FALSE;

    headerSize = GFDGetVertexShaderHeaderSize(index, pData);
    programSize = GFDGetVertexShaderProgramSize(index, pData);

    if (headerSize == 0 || programSize == 0)
        return FALSE;

    pHeader = static_cast<GX2VertexShader*>(heap->Alloc(headerSize, PPC_IO_BUFFER_ALIGN));
    pProgram = heap->Alloc(programSize, GX2_SHADER_ALIGNMENT);

    ret = GFDGetVertexShader(pHeader, pProgram, index, pData);
    if (ret)
    {
        DCFlushRange(pHeader->shaderPtr, pHeader->shaderSize);
        *ppShader = pHeader;
    }
    else
    {
        OSReport("Warning: Invalid Vertex Shader :%d", ret);
        if (pHeader) heap->Free(pHeader);
        if (pProgram) heap->Free(pProgram);
    }

    return ret;
}

BOOL _DEMOGFDReadPixelShader(nw::eft::Heap* heap, GX2PixelShader** ppShader, u32 index, const void* pData)
{
    GX2PixelShader* pHeader;
    void* pProgram;
    u32 ret;
    u32 headerSize;
    u32 programSize;

    if (pData == NULL || ppShader == NULL || index >= GFDGetPixelShaderCount(pData))
        return FALSE;

    headerSize = GFDGetPixelShaderHeaderSize(index, pData);
    programSize = GFDGetPixelShaderProgramSize(index, pData);

    if (headerSize == 0 || programSize == 0)
        return FALSE;

    pHeader = static_cast<GX2PixelShader*>(heap->Alloc(headerSize, PPC_IO_BUFFER_ALIGN));
    pProgram = heap->Alloc(programSize, GX2_SHADER_ALIGNMENT);

    ret = GFDGetPixelShader(pHeader, pProgram, index, pData);
    if (ret)
    {
        DCFlushRange(pHeader->shaderPtr, pHeader->shaderSize);
        *ppShader = pHeader;
    }
    else
    {
        OSReport("Warning: Invalid Pixel Shader :%d", ret);
        if (pHeader) heap->Free(pHeader);
        if (pProgram) heap->Free(pProgram);
    }

    return ret;
}

BOOL _DEMOGFDReadGeometryShader(nw::eft::Heap* heap, GX2GeometryShader** ppShader, u32 index, const void* pData)
{
    GX2GeometryShader* pHeader;
    void* pProgram;
    void* pCopyProgram;
    u32 ret;
    u32 headerSize;
    u32 programSize;
    u32 copyProgramSize;

    if (pData == NULL || ppShader == NULL || index >= GFDGetGeometryShaderCount(pData))
        return FALSE;

    headerSize = GFDGetGeometryShaderHeaderSize(index, pData);
    programSize = GFDGetGeometryShaderProgramSize(index, pData);
    copyProgramSize = GFDGetGeometryShaderCopyProgramSize(index, pData);

    if (headerSize == 0 || programSize == 0)
        return FALSE;

    pHeader = static_cast<GX2GeometryShader*>(heap->Alloc(headerSize, PPC_IO_BUFFER_ALIGN));
    pProgram = heap->Alloc(programSize, GX2_SHADER_ALIGNMENT);
    pCopyProgram = heap->Alloc(copyProgramSize, GX2_SHADER_ALIGNMENT);

    ret = GFDGetGeometryShader(pHeader, pProgram, pCopyProgram, index, pData);
    if (ret)
    {
        DCFlushRange(pHeader->shaderPtr, pHeader->shaderSize);
        DCFlushRange(pHeader->copyShaderPtr, pHeader->copyShaderSize);
        *ppShader = pHeader;
    }
    else
    {
        OSReport("Warning: Invalid Geometry Shader :%d", ret);
        if (pHeader) heap->Free(pHeader);
        if (pProgram) heap->Free(pProgram);
        if (pCopyProgram) heap->Free(pCopyProgram);
    }

    return ret;
}

namespace nw { namespace eft {

TextureSampler::TextureSampler()
{
    GX2InitSampler(&mTextureSampler, GX2_TEX_CLAMP_WRAP, GX2_TEX_XY_FILTER_BILINEAR);
}

TextureSampler::~TextureSampler()
{
}

bool TextureSampler::Setup(TextureFilterMode textureFilter, TextureWrapMode wrapModeU, TextureWrapMode wrapModeV)
{
    GX2TexAnisoRatio ratio = GX2_TEX_ANISO_1_TO_1;

    if (textureFilter == EFT_TEXTURE_FILTER_TYPE_LINEAR)
        GX2InitSamplerXYFilter(&mTextureSampler, GX2_TEX_XY_FILTER_BILINEAR, GX2_TEX_XY_FILTER_BILINEAR, ratio);
    else
        GX2InitSamplerXYFilter(&mTextureSampler, GX2_TEX_XY_FILTER_POINT, GX2_TEX_XY_FILTER_POINT, ratio);

    GX2TexClamp clampX = GX2_TEX_CLAMP_MIRROR;
    GX2TexClamp clampY = GX2_TEX_CLAMP_MIRROR;
    GX2TexClamp clampZ = GX2_TEX_CLAMP_WRAP;

    switch (wrapModeU)
    {
    case EFT_TEXTURE_WRAP_TYPE_MIRROR:              clampX = GX2_TEX_CLAMP_MIRROR;      break;
    case EFT_TEXTURE_WRAP_TYPE_REPEAT:              clampX = GX2_TEX_CLAMP_WRAP;        break;
    case EFT_TEXTURE_WRAP_TYPE_CLAMP:               clampX = GX2_TEX_CLAMP_CLAMP;       break;
    case EFT_TEXTURE_WRAP_TYPE_MIROOR_ONCE:         clampX = GX2_TEX_CLAMP_MIRROR_ONCE; break;
    }

    switch (wrapModeV)
    {
    case EFT_TEXTURE_WRAP_TYPE_MIRROR:              clampY = GX2_TEX_CLAMP_MIRROR;      break;
    case EFT_TEXTURE_WRAP_TYPE_REPEAT:              clampY = GX2_TEX_CLAMP_WRAP;        break;
    case EFT_TEXTURE_WRAP_TYPE_CLAMP:               clampY = GX2_TEX_CLAMP_CLAMP;       break;
    case EFT_TEXTURE_WRAP_TYPE_MIROOR_ONCE:         clampY = GX2_TEX_CLAMP_MIRROR_ONCE; break;
    }

    GX2InitSamplerClamping(&mTextureSampler, clampX, clampY, clampZ);
    return true;
}

bool TextureSampler::SetupLOD(f32 maxMip, f32 bais)
{
    GX2InitSamplerLOD(&mTextureSampler, 0.0f, maxMip, bais);
    return true;
}

Rendercontext::Rendercontext()
{
    mTextureSampler.Setup(EFT_TEXTURE_FILTER_TYPE_LINEAR, EFT_TEXTURE_WRAP_TYPE_REPEAT, EFT_TEXTURE_WRAP_TYPE_REPEAT);
    mDefaultTextureSampler.Setup(EFT_TEXTURE_FILTER_TYPE_LINEAR, EFT_TEXTURE_WRAP_TYPE_MIRROR, EFT_TEXTURE_WRAP_TYPE_MIRROR);
}

void Rendercontext::SetupCommonState()
{
    GX2SetCullOnlyControl(GX2_FRONT_FACE_CCW, GX2_DISABLE, GX2_DISABLE);
    GX2SetColorControl(GX2_LOGIC_OP_COPY, GX2_ENABLE, GX2_DISABLE, GX2_ENABLE);

    mBlendType     = EFT_BLEND_TYPE_MAX;
    mZBufATestType = EFT_ZBUFF_ATEST_TYPE_MAX;
}

void Rendercontext::SetupBlendType(BlendType blendType)
{
    switch (blendType)
    {
    case EFT_BLEND_TYPE_NORMAL:
        GX2SetBlendControl(GX2_RENDER_TARGET_0,
                           GX2_BLEND_SRC_ALPHA,
                           GX2_BLEND_ONE_MINUS_SRC_ALPHA,
                           GX2_BLEND_COMBINE_ADD,
                           GX2_DISABLE,
                           GX2_BLEND_SRC_ALPHA,
                           GX2_BLEND_ONE_MINUS_SRC_ALPHA,
                           GX2_BLEND_COMBINE_ADD);
        break;
    case EFT_BLEND_TYPE_ADD:
        GX2SetBlendControl(GX2_RENDER_TARGET_0,
                           GX2_BLEND_SRC_ALPHA,
                           GX2_BLEND_ONE,
                           GX2_BLEND_COMBINE_ADD,
                           GX2_DISABLE,
                           GX2_BLEND_SRC_ALPHA,
                           GX2_BLEND_ONE,
                           GX2_BLEND_COMBINE_ADD);
        break;
    case EFT_BLEND_TYPE_SUB:
        GX2SetBlendControl(GX2_RENDER_TARGET_0,
                           GX2_BLEND_SRC_ALPHA,
                           GX2_BLEND_ONE,
                           GX2_BLEND_COMBINE_DST_MINUS_SRC,
                           GX2_DISABLE,
                           GX2_BLEND_SRC_ALPHA,
                           GX2_BLEND_ONE,
                           GX2_BLEND_COMBINE_DST_MINUS_SRC);
        break;
    case EFT_BLEND_TYPE_SCREEN:
        GX2SetBlendControl(GX2_RENDER_TARGET_0,
                           GX2_BLEND_ONE_MINUS_DST_COLOR,
                           GX2_BLEND_ONE,
                           GX2_BLEND_COMBINE_ADD,
                           GX2_DISABLE,
                           GX2_BLEND_ONE_MINUS_DST_COLOR,
                           GX2_BLEND_ONE,
                           GX2_BLEND_COMBINE_ADD);
        break;
    case EFT_BLEND_TYPE_MULT:
        GX2SetBlendControl(GX2_RENDER_TARGET_0,
                           GX2_BLEND_ZERO,
                           GX2_BLEND_SRC_COLOR ,
                           GX2_BLEND_COMBINE_ADD,
                           GX2_DISABLE,
                           GX2_BLEND_ONE_MINUS_DST_COLOR,
                           GX2_BLEND_ONE,
                           GX2_BLEND_COMBINE_ADD);
        break;
    }

    mBlendType = blendType;
}

void Rendercontext::SetupZBufATest(ZBufATestType zBufATestType)
{
    switch (zBufATestType)
    {
    case EFT_ZBUFF_ATEST_TYPE_NORMAL:
        GX2SetDepthOnlyControl(GX2_TRUE, GX2_FALSE, GX2_COMPARE_LEQUAL);
        GX2SetAlphaTest(GX2_TRUE, GX2_COMPARE_GREATER, 0.0f);
        GX2SetColorControl(GX2_LOGIC_OP_COPY, GX2_ENABLE, GX2_DISABLE, GX2_ENABLE);
        break;
    case EFT_ZBUFF_ATEST_TYPE_ZIGNORE:
        GX2SetDepthOnlyControl(GX2_FALSE, GX2_FALSE, GX2_COMPARE_LEQUAL);
        GX2SetAlphaTest(GX2_TRUE, GX2_COMPARE_GREATER, 0.0f);
        GX2SetColorControl(GX2_LOGIC_OP_COPY, GX2_ENABLE, GX2_DISABLE, GX2_ENABLE);
        break;
    case EFT_ZBUFF_ATEST_TYPE_ENTITY:
        GX2SetDepthOnlyControl(GX2_TRUE, GX2_TRUE, GX2_COMPARE_LEQUAL);
        GX2SetAlphaTest(GX2_TRUE, GX2_COMPARE_GREATER, 0.5f);
        GX2SetColorControl(GX2_LOGIC_OP_COPY, GX2_DISABLE, GX2_DISABLE, GX2_ENABLE);
        break;
    }

    mZBufATestType = zBufATestType;
}

void Rendercontext::SetupDisplaySideType(DisplaySideType displaySideType) const
{
    switch (displaySideType)
    {
    case EFT_DISPLAYSIDETYPE_BOTH:
        GX2SetCullOnlyControl(GX2_FRONT_FACE_CCW, GX2_FALSE, GX2_FALSE);
        break;
    case EFT_DISPLAYSIDETYPE_FRONT:
        GX2SetCullOnlyControl(GX2_FRONT_FACE_CCW, GX2_FALSE, GX2_TRUE);
        break;
    case EFT_DISPLAYSIDETYPE_BACK:
        GX2SetCullOnlyControl(GX2_FRONT_FACE_CCW, GX2_TRUE, GX2_FALSE);
        break;
    }
}

void Rendercontext::SetupTexture(const TextureRes* texture, TextureSlot slot, FragmentTextureLocation location)
{
    if (texture == NULL || location.loc == EFT_INVALID_LOCATION)
        return;

    GX2SetPixelTexture(&texture->gx2Texture, location.loc);

    mTextureSampler.Setup(static_cast<TextureFilterMode>(texture->filterMode),
                          static_cast<TextureWrapMode>( texture->wrapMode       & 0xF),
                          static_cast<TextureWrapMode>((texture->wrapMode >> 4) & 0xF));
    mTextureSampler.SetupLOD(texture->enableMipLevel, texture->mipMapBias);

    GX2SetPixelSampler(mTextureSampler.GetSampler(), location.loc);
}

void Rendercontext::SetupTexture(const GX2Texture* texture, TextureSlot slot, FragmentTextureLocation location)
{
    GX2SetPixelTexture(texture, location.loc);
    GX2SetPixelSampler(mDefaultTextureSampler.GetSampler(), location.loc);
}

VertexBuffer::VertexBuffer()
{
    mVertexBufferSize   = 0;
    mVertexBuffer       = NULL;
}

void* VertexBuffer::AllocateVertexBuffer(Heap* heap, u32 size, u32 element)
{
    mVertexBufferSize = size;
    mVertexBuffer = heap->Alloc(mVertexBufferSize, GX2_VERTEX_BUFFER_ALIGNMENT);
    mVertexElement = element;
    return mVertexBuffer;
}

void VertexBuffer::Finalize(Heap* heap)
{
    if (mVertexBuffer)
    {
        heap->Free(mVertexBuffer);
        mVertexBuffer = NULL;
    }
}

void VertexBuffer::Invalidate()
{
    DCFlushRange(mVertexBuffer, mVertexBufferSize);
}

void VertexBuffer::BindBuffer(u32 index, u32 size, u32 stride)
{
    GX2SetAttribBuffer(index, size, stride, mVertexBuffer);
}

void VertexBuffer::BindExtBuffer(u32 index, u32 size, u32 element, u32 stride, void* buffer)
{
    GX2SetAttribBuffer(index, size, stride, buffer);
}

Shader::Shader()
{
    mpVertexShader              = NULL;
    mpPixelShader               = NULL;
    mpGeometryShader            = NULL;
    mpFetchShaderBuf            = NULL;
    mAttribsNum                 = 0;
    mInitialized                = false;
}

void Shader::Finalize(Heap* heap)
{
    if (mpVertexShader && mpVertexShader->shaderPtr)
    {
        heap->Free(mpVertexShader->shaderPtr);
        heap->Free(mpVertexShader);
    }

    if (mpPixelShader && mpPixelShader->shaderPtr)
    {
        heap->Free(mpPixelShader->shaderPtr);
        heap->Free(mpPixelShader);
    }

    if (mpGeometryShader && mpGeometryShader->shaderPtr)
    {
        heap->Free(mpGeometryShader->shaderPtr);
        heap->Free(mpGeometryShader);
    }

    if (mpFetchShaderBuf)
    {
        heap->Free(mpFetchShaderBuf);
        mpFetchShaderBuf = NULL;
    }
}

void Shader::BindShader()
{
    if (mpGeometryShader)
        GX2SetShadersEx(&mFetchShader, mpVertexShader, mpGeometryShader, mpPixelShader);
    else
        GX2SetShaders(&mFetchShader, mpVertexShader, mpPixelShader);
}

bool Shader::CreateShader(Heap* heap, const void* gshBuffer, u32 gshBufferSize)
{
    _DEMOGFDReadVertexShader(heap, &mpVertexShader, 0, gshBuffer);
    _DEMOGFDReadPixelShader(heap, &mpPixelShader, 0, gshBuffer);
    _DEMOGFDReadGeometryShader(heap, &mpGeometryShader, 0, gshBuffer);

    return true;
}

u32 Shader::GetFragmentSamplerLocation(const char* name)
{
    return GX2GetPixelSamplerVarLocation(mpPixelShader, name);
}

u32 Shader::GetAttributeLocation(const char* name)
{
    return GX2GetVertexAttribVarLocation(mpVertexShader, name);
}

u32 Shader::GetAttribute(const char* name, u32 index, VertexFormat fmt, u32 offset, bool instancingAttr)
{
    u32 attrLoc = GetAttributeLocation(name);
    if (attrLoc == EFT_INVALID_ATTRIBUTE)
        return EFT_INVALID_ATTRIBUTE;

    mAttribsIndex[attrLoc] = index;
    GX2InitAttribStream(&mAttribs[mAttribsNum], attrLoc, index, offset * sizeof(u32), static_cast<GX2AttribFormat>(fmt));

    if (instancingAttr)
    {
        mAttribs[mAttribsNum].indexType  = GX2_ATTRIB_INDEX_INSTANCE_ID;
        mAttribs[mAttribsNum].aluDivisor = 1;
    }

    mAttribsNum++;
    return index;
}

void Shader::SetupShader(Heap* heap)
{
    mpFetchShaderBuf = heap->Alloc(GX2CalcFetchShaderSize(mAttribsNum), GX2_SHADER_ALIGNMENT);
    GX2InitFetchShader(&mFetchShader, mpFetchShaderBuf, mAttribsNum, mAttribs);
    DCFlushRange(mpFetchShaderBuf, GX2CalcFetchShaderSize(mAttribsNum));
    mInitialized = true;
}

bool UniformBlock::InitializeVertexUniformBlock(Shader* shader, const char* name, u32 bindPoint)
{
    GX2UniformBlock* block = GX2GetVertexUniformBlock(shader->GetVertexShader(), name);
    if (!block)
    {
        mIsFailed = true;
        return false;
    }

    mUniformLoc = block->location;
    mBufferSize = block->size;
    mUniformBlockMode = EFT_UNIFORM_BLOCK_MODE_VERTEX;
    mInitialized = true;

    return true;
}

bool UniformBlock::InitializePixelUniformBlock(Shader* shader, const char* name, u32)
{
    GX2UniformBlock* block = GX2GetPixelUniformBlock(shader->GetPixelShader(), name);
    if (!block)
    {
        mIsFailed = true;
        return false;
    }

    mUniformLoc = block->location;
    mBufferSize = block->size;
    mUniformBlockMode = EFT_UNIFORM_BLOCK_MODE_FRAGMENT;
    mInitialized = true;

    return true;
}

void UniformBlock::BindUniformBlock(const void* param)
{
    if (mBufferSize == 0)
        return;

    switch (mUniformBlockMode)
    {
    case EFT_UNIFORM_BLOCK_MODE_VERTEX:
        GX2SetVertexUniformBlock(mUniformLoc, mBufferSize, param);
        break;
    case EFT_UNIFORM_BLOCK_MODE_FRAGMENT:
        GX2SetPixelUniformBlock(mUniformLoc, mBufferSize, param);
        break;
    case EFT_UNIFORM_BLOCK_MODE_GEOMETRY:
        GX2SetGeometryUniformBlock(mUniformLoc, mBufferSize, param);
        break;
    }
}

} } // namespace nw::eft
