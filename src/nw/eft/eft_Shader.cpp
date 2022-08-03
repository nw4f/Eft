#include <nw/eft/eft_Data.h>
#include <nw/eft/eft_Shader.h>

#if EFT_IS_CAFE_WUT || !EFT_IS_CAFE
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#endif

namespace nw { namespace eft {

ParticleShader::ParticleShader()
{
    mPositionAttr   = EFT_INVALID_ATTRIBUTE;
    mNormalAttr     = EFT_INVALID_ATTRIBUTE;
    mColorAttr      = EFT_INVALID_ATTRIBUTE;
    mTexCoordAttr0  = EFT_INVALID_ATTRIBUTE;
    mTexCoordAttr1  = EFT_INVALID_ATTRIBUTE;
    mIndexAttr      = EFT_INVALID_ATTRIBUTE;
    mOuterAttr      = EFT_INVALID_ATTRIBUTE;
    mDirAttr        = EFT_INVALID_ATTRIBUTE;

    mSclAttr        = EFT_INVALID_ATTRIBUTE;
    mTexAnimAttr    = EFT_INVALID_ATTRIBUTE;
  //mSubTexAnimAttr = EFT_INVALID_ATTRIBUTE; <-- Nintendo forgot to do this
    mWldPosAttr     = EFT_INVALID_ATTRIBUTE;
    mWldPosDfAttr   = EFT_INVALID_ATTRIBUTE;
    mColor0Attr     = EFT_INVALID_ATTRIBUTE;
    mColor1Attr     = EFT_INVALID_ATTRIBUTE;
    vRotAttr        = EFT_INVALID_ATTRIBUTE;
    mEmtMatAttr0    = EFT_INVALID_ATTRIBUTE;
    mEmtMatAttr1    = EFT_INVALID_ATTRIBUTE;
    mEmtMatAttr2    = EFT_INVALID_ATTRIBUTE;

    for (u32 i = 0; i < EFT_TEXTURE_SLOT_BIN_MAX; i++)
        mFragmentTextureSampler[i].loc                          = EFT_INVALID_SAMPLER;

    mFragmentTextureSampler[EFT_TEXTURE_SLOT_DEPTH_BUFFER].loc  = EFT_INVALID_SAMPLER;
    mFragmentTextureSampler[EFT_TEXTURE_SLOT_FRAME_BUFFER].loc  = EFT_INVALID_SAMPLER;

    for (u32 i = 0; i < USER_SAMPLER_SLOT_MAX; i++)
    {
        mUserVertexSamplerLocation[i].loc                       = EFT_INVALID_SAMPLER;
        mUserFragmentSamplerLocation[i].loc                     = EFT_INVALID_SAMPLER;
    }
}

void ParticleShader::Finalize(Heap* heap)
{
    return mShader.Finalize(heap);
}

void ParticleShader::InitializeVertexShaderLocation()
{
    mVertexViewUniformBlock          .InitializeVertexUniformBlock(&mShader, "viewUniformBlock",           UNIFORM_BLOCK_VERTEX_VIEW_BLOCK);
    mVertexEmitterStaticUniformBlock .InitializeVertexUniformBlock(&mShader, "emitterStaticUniformBlock",  UNIFORM_BLOCK_VERTEX_EMITTER_STATIC_BLOCK);
    mVertexEmitterDynamicUniformBlock.InitializeVertexUniformBlock(&mShader, "emitterDynamicUniformBlock", UNIFORM_BLOCK_VERTEX_EMITTER_DYNAMIC_BLOCK);
}

void ParticleShader::InitializeFragmentShaderLocation()
{
    mFragmentViewUniformBlock         .InitializePixelUniformBlock(&mShader, "viewUniformBlock",           UNIFORM_BLOCK_FRAGMENTVIEW_BLOCK);
    mFragmentEmitterStaticUniformBlock.InitializePixelUniformBlock(&mShader, "emitterStaticUniformBlock",  UNIFORM_BLOCK_FRAGMENT_EMITTER_STATIC_BLOCK);

    mFragmentTextureSampler[EFT_TEXTURE_SLOT_0].loc = mShader.GetFragmentSamplerLocation("s_firstTexture");
    if (mFragmentShaderKey.mTextureVariation == EFT_FRAGMENT_SHADER_TEXTURE_VARIATION_1)
        mFragmentTextureSampler[EFT_TEXTURE_SLOT_1].loc = mShader.GetFragmentSamplerLocation("s_secondTexture");

    mFragmentTextureSampler[EFT_TEXTURE_SLOT_DEPTH_BUFFER].loc = mShader.GetFragmentSamplerLocation("s_depthBufferTexture");
    mFragmentTextureSampler[EFT_TEXTURE_SLOT_FRAME_BUFFER].loc = mShader.GetFragmentSamplerLocation("s_frameBufferTexture");
}

void ParticleShader::InitializeAttribute()
{
    mPositionAttr   = mShader.GetAttribute("v_inPos",        0, Shader::FORMAT_32_32_32_32_FLOAT,  0, false);
    if (!mVertexShaderKey.IsUsePrimitive())
        mIndexAttr  = mShader.GetAttribute("v_inIndex",      1, Shader::FORMAT_32_UINT,            0, false);
    mTexCoordAttr0  = mShader.GetAttribute("v_inTexCoord",   2, Shader::FORMAT_32_32_32_32_FLOAT,  0, false);
    mNormalAttr     = mShader.GetAttribute("v_inNormal",     3, Shader::FORMAT_32_32_32_FLOAT,     0, false);
    mColorAttr      = mShader.GetAttribute("v_inColor",      4, Shader::FORMAT_32_32_32_32_FLOAT,  0, false);

    mWldPosAttr     = mShader.GetAttribute("v_inWldPos",     5, Shader::FORMAT_32_32_32_32_FLOAT,  0, true);
    mSclAttr        = mShader.GetAttribute("v_inScl",        5, Shader::FORMAT_32_32_32_32_FLOAT,  4, true);
    mColor0Attr     = mShader.GetAttribute("v_inColor0",     5, Shader::FORMAT_32_32_32_32_FLOAT,  8, true);
    mColor1Attr     = mShader.GetAttribute("v_inColor1",     5, Shader::FORMAT_32_32_32_32_FLOAT, 12, true);
    mTexAnimAttr    = mShader.GetAttribute("v_inTexAnim",    5, Shader::FORMAT_32_32_32_32_FLOAT, 16, true);
    mWldPosDfAttr   = mShader.GetAttribute("v_inWldPosDf",   5, Shader::FORMAT_32_32_32_32_FLOAT, 20, true);
    vRotAttr        = mShader.GetAttribute("v_inRot",        5, Shader::FORMAT_32_32_32_32_FLOAT, 24, true);
    mSubTexAnimAttr = mShader.GetAttribute("v_inSubTexAnim", 5, Shader::FORMAT_32_32_32_32_FLOAT, 28, true);
    mEmtMatAttr0    = mShader.GetAttribute("v_inEmtMat0",    5, Shader::FORMAT_32_32_32_32_FLOAT, 32, true);
    mEmtMatAttr1    = mShader.GetAttribute("v_inEmtMat1",    5, Shader::FORMAT_32_32_32_32_FLOAT, 36, true);
    mEmtMatAttr2    = mShader.GetAttribute("v_inEmtMat2",    5, Shader::FORMAT_32_32_32_32_FLOAT, 40, true);
}

void ParticleShader::InitializeStripeVertexShaderLocation()
{
    mVertexViewUniformBlock.InitializeVertexUniformBlock(&mShader, "viewUniformBlock",   UNIFORM_BLOCK_VERTEX_VIEW_BLOCK);
    mStripeUniformBlock    .InitializeVertexUniformBlock(&mShader, "stripeUniformBlock", UNIFORM_BLOCK_VERTEX_STRIPE_BLOCK);
}

void ParticleShader::InitializeStripeAttribute()
{
    mPositionAttr  = mShader.GetAttribute("v_inPos",      0, Shader::FORMAT_32_32_32_32_FLOAT,  0, false);
    mOuterAttr     = mShader.GetAttribute("v_inOuter",    0, Shader::FORMAT_32_32_32_32_FLOAT,  4, false);
    mTexCoordAttr0 = mShader.GetAttribute("v_inTexCoord", 0, Shader::FORMAT_32_32_32_32_FLOAT,  8, false);
    mDirAttr       = mShader.GetAttribute("v_inDir",      0, Shader::FORMAT_32_32_32_32_FLOAT, 12, false);
}

bool ParticleShader::SetupShaderResource(Heap* heap, void* shaderResource, u32 shaderResourceSize)
{
    mShader.CreateShader(heap, shaderResource, shaderResourceSize);

    if (!IsStripe())
    {
        InitializeVertexShaderLocation();
        InitializeFragmentShaderLocation();
        InitializeAttribute();
    }
    else
    {
        InitializeStripeVertexShaderLocation();
        InitializeFragmentShaderLocation();
        InitializeStripeAttribute();
    }

    Setup(heap);

#if EFT_IS_CAFE
    memset(mDisplayListBuffer, 0, DISPLAY_LSIT_SIZE);
    DCFlushRange(mDisplayListBuffer, DISPLAY_LSIT_SIZE);
    DCInvalidateRange(mDisplayListBuffer, DISPLAY_LSIT_SIZE);

    GX2BeginDisplayList(mDisplayListBuffer, DISPLAY_LSIT_SIZE);
    {
        mShader.BindShader();
    }
    mDisplayListBufferUsed = GX2EndDisplayList(mDisplayListBuffer);
#endif // EFT_IS_CAFE

    return true;
}

void ParticleShader::Bind()
{
#if EFT_IS_CAFE
    GX2CallDisplayList(mDisplayListBuffer, mDisplayListBufferUsed);
#else
    mShader.BindShader();
#endif
}

bool ParticleShader::SetUserVertexUniformBlock(UserUniformBlockID uniformBlockID, const char* name, void* param)
{
    u32 index = uniformBlockID - UNIFORM_BLOCK_USER_VERTEX_ID_0;

    if (!mUserVertexUniformBlock[index].IsFailed() && !mUserVertexUniformBlock[index].IsInitialized())
    {
        if (!mUserVertexUniformBlock[index].InitializeVertexUniformBlock(&mShader, name, uniformBlockID))
            return false;
    }

    if (mUserVertexUniformBlock[index].IsInitialized())
        mUserVertexUniformBlock[index].BindUniformBlock(param);

    return true;
}

void ParticleShader::EnableInstanced()
{
#if EFT_IS_WIN
    if (mWldPosAttr != EFT_INVALID_ATTRIBUTE)
        glVertexAttribDivisor(mWldPosAttr, 1);

    if (mSclAttr != EFT_INVALID_ATTRIBUTE)
        glVertexAttribDivisor(mSclAttr, 1);

    if (mColor0Attr != EFT_INVALID_ATTRIBUTE)
        glVertexAttribDivisor(mColor0Attr, 1);

    if (mColor1Attr != EFT_INVALID_ATTRIBUTE)
        glVertexAttribDivisor(mColor1Attr, 1);

    if (mTexAnimAttr != EFT_INVALID_ATTRIBUTE)
        glVertexAttribDivisor(mTexAnimAttr, 1);

    if (mWldPosDfAttr != EFT_INVALID_ATTRIBUTE)
        glVertexAttribDivisor(mWldPosDfAttr, 1);

    if (vRotAttr != EFT_INVALID_ATTRIBUTE)
        glVertexAttribDivisor(vRotAttr, 1);

    if (mSubTexAnimAttr != EFT_INVALID_ATTRIBUTE)
        glVertexAttribDivisor(mSubTexAnimAttr, 1);

    if (mEmtMatAttr0 != EFT_INVALID_ATTRIBUTE)
        glVertexAttribDivisor(mEmtMatAttr0, 1);

    if (mEmtMatAttr1 != EFT_INVALID_ATTRIBUTE)
        glVertexAttribDivisor(mEmtMatAttr1, 1);

    if (mEmtMatAttr2 != EFT_INVALID_ATTRIBUTE)
        glVertexAttribDivisor(mEmtMatAttr2, 1);
#endif
}

void ParticleShader::DisableInstanced()
{
#if EFT_IS_WIN
    if (mWldPosAttr != EFT_INVALID_ATTRIBUTE)
        glVertexAttribDivisor(mWldPosAttr, 0);

    if (mSclAttr != EFT_INVALID_ATTRIBUTE)
        glVertexAttribDivisor(mSclAttr, 0);

    if (mColor0Attr != EFT_INVALID_ATTRIBUTE)
        glVertexAttribDivisor(mColor0Attr, 0);

    if (mColor1Attr != EFT_INVALID_ATTRIBUTE)
        glVertexAttribDivisor(mColor1Attr, 0);

    if (mTexAnimAttr != EFT_INVALID_ATTRIBUTE)
        glVertexAttribDivisor(mTexAnimAttr, 0);

    if (mWldPosDfAttr != EFT_INVALID_ATTRIBUTE)
        glVertexAttribDivisor(mWldPosDfAttr, 0);

    if (vRotAttr != EFT_INVALID_ATTRIBUTE)
        glVertexAttribDivisor(vRotAttr, 0);

    if (mSubTexAnimAttr != EFT_INVALID_ATTRIBUTE)
        glVertexAttribDivisor(mSubTexAnimAttr, 0);

    if (mEmtMatAttr0 != EFT_INVALID_ATTRIBUTE)
        glVertexAttribDivisor(mEmtMatAttr0, 0);

    if (mEmtMatAttr1 != EFT_INVALID_ATTRIBUTE)
        glVertexAttribDivisor(mEmtMatAttr1, 0);

    if (mEmtMatAttr2 != EFT_INVALID_ATTRIBUTE)
        glVertexAttribDivisor(mEmtMatAttr2, 0);
#endif
}

} } // namespace nw::eft

#if EFT_IS_CAFE_WUT || !EFT_IS_CAFE
#pragma GCC diagnostic pop
#endif
