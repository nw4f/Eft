#include <nw/eft/cafe/eft_CafeWrapper.h>
#include <nw/eft/eft_Heap.h>

#include <cafe/gfd.h>

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

bool TextureSampler::Initialize(TextureFilterMode textureFilter, TextureWrapMode wrapModeU, TextureWrapMode wrapModeV)
{
    Setup(textureFilter, wrapModeU, wrapModeV);
    return true;
}

bool TextureSampler::SetupLOD(f32 maxMip, f32 bais)
{
    GX2InitSamplerLOD(&mTextureSampler, 0.0f, maxMip, bais);
    return true;
}

VertexBuffer::VertexBuffer()
{
    mVertexBufferSize   = 0;
    mVertexBuffer       = NULL;
}

void* VertexBuffer::AllocateVertexBuffer(Heap* heap, u32 size, u32 element)
{
    mVertexBufferSize   = size;
    mVertexBuffer       = heap->Alloc(mVertexBufferSize, GX2_VERTEX_BUFFER_ALIGNMENT);
    mVertexElement      = element;
    return mVertexBuffer;
}

void VertexBuffer::SetVertexBuffer(void* buffer, u32 size, u32 element)
{
    mVertexBufferSize   = size;
    mVertexBuffer       = buffer;
    mVertexElement      = element;
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
    _DEMOGFDReadVertexShader  (heap, &mpVertexShader,   0, gshBuffer);
    _DEMOGFDReadPixelShader   (heap, &mpPixelShader,    0, gshBuffer);
    _DEMOGFDReadGeometryShader(heap, &mpGeometryShader, 0, gshBuffer);

    return true;
}

u32 Shader::GetFragmentSamplerLocation(const char* name)
{
    return GX2GetPixelSamplerVarLocation(mpPixelShader, name);
}

u32 Shader::GetVertexSamplerLocation(const char* name)
{
    return GX2GetVertexSamplerVarLocation(mpVertexShader, name);
}

u32 Shader::GetAttributeLocation(const char* name)
{
    return GX2GetVertexAttribVarLocation(mpVertexShader, name);
}

u32 Shader::GetAttribute(const char* name, u32 index, VertexFormat fmt, u32 offset, bool instancingAttr, bool endianSwap)
{
    u32 attrLoc = GetAttributeLocation(name);
    if (attrLoc == EFT_INVALID_ATTRIBUTE)
        return EFT_INVALID_ATTRIBUTE;

    mAttribsIndex[attrLoc] = index;
    GX2InitAttribStream(&mAttribs[mAttribsNum], attrLoc, index, offset * sizeof(u32), static_cast<GX2AttribFormat>(fmt));

    if (instancingAttr)
    {
        mAttribs[mAttribsNum].indexType = GX2_ATTRIB_INDEX_INSTANCE_ID;
        mAttribs[mAttribsNum].aluDivisor = 1;
    }

    if (endianSwap)
        mAttribs[mAttribsNum].endianSwap = GX2_ENDIANSWAP_NONE;

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
    if (block == NULL)
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
    if (block == NULL)
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

void UniformBlock::BindUniformBlock(const void* param, s32 size)
{
    if (mBufferSize == 0)
        return;

    s32 bufferSize = mBufferSize;
    if (size != -1)
        bufferSize = size;

    switch (mUniformBlockMode)
    {
    case EFT_UNIFORM_BLOCK_MODE_VERTEX:
        GX2SetVertexUniformBlock(mUniformLoc, bufferSize, param);
        break;
    case EFT_UNIFORM_BLOCK_MODE_FRAGMENT:
        GX2SetPixelUniformBlock(mUniformLoc, bufferSize, param);
        break;
    case EFT_UNIFORM_BLOCK_MODE_GEOMETRY:
        GX2SetGeometryUniformBlock(mUniformLoc, bufferSize, param);
        break;
    }
}

} } // namespace nw::eft
