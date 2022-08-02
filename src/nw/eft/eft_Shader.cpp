#include <nw/eft/eft_Emitter.h>
#include <nw/eft/eft_Misc.h>
#include <nw/eft/eft_Renderer.h>
#include <nw/eft/eft_Shader.h>
#include <nw/eft/eft_System.h>

namespace nw { namespace eft {

ParticleShader::ParticleShader()
{
    mPositionAttr       = EFT_INVALID_ATTRIBUTE;
    mNormalAttr         = EFT_INVALID_ATTRIBUTE;
    mColorAttr          = EFT_INVALID_ATTRIBUTE;
    mTexCoordAttr0      = EFT_INVALID_ATTRIBUTE;
    mTexCoordAttr1      = EFT_INVALID_ATTRIBUTE;
    mIndexAttr          = EFT_INVALID_ATTRIBUTE;
    mOuterAttr          = EFT_INVALID_ATTRIBUTE;
    mDirAttr            = EFT_INVALID_ATTRIBUTE;
    mRandomAttr         = EFT_INVALID_ATTRIBUTE;
    mVectorAttr         = EFT_INVALID_ATTRIBUTE;

    mSclAttr            = EFT_INVALID_ATTRIBUTE;
    mWldPosAttr         = EFT_INVALID_ATTRIBUTE;
    mWldPosDfAttr       = EFT_INVALID_ATTRIBUTE;
    mColor0Attr         = EFT_INVALID_ATTRIBUTE;
    mColor1Attr         = EFT_INVALID_ATTRIBUTE;
    vRotAttr            = EFT_INVALID_ATTRIBUTE;
    mEmtMatAttr0        = EFT_INVALID_ATTRIBUTE;
    mEmtMatAttr1        = EFT_INVALID_ATTRIBUTE;
    mEmtMatAttr2        = EFT_INVALID_ATTRIBUTE;

    mPositionInput      = EFT_INVALID_ATTRIBUTE;
    mPositionOut        = EFT_INVALID_ATTRIBUTE;
    mVectorInput        = EFT_INVALID_ATTRIBUTE;
    mVectorOut          = EFT_INVALID_ATTRIBUTE;

    mShaderArrtFlag     = 0;

    for (u32 i = 0; i < EFT_TEXTURE_SLOT_MAX; i++)
    {
        mVertexTextureSampler[i].loc        = EFT_INVALID_SAMPLER;
        mFragmentTextureSampler[i].loc      = EFT_INVALID_SAMPLER;
    }

    for (u32 i = 0; i < USER_SAMPLER_SLOT_MAX; i++)
    {
        mUserFragmentSamplerLocation[i].loc = EFT_INVALID_SAMPLER;
        mUserVertexSamplerLocation[i].loc   = EFT_INVALID_SAMPLER;
    }

    mDisplayListBufferUsed = 0;
}

void ParticleShader::Finalize(Heap* heap)
{
    mShader.Finalize(heap);
}

void ParticleShader::InitializeVertexShaderLocation()
{
    mVertexViewUniformBlock            .InitializeVertexUniformBlock(&mShader, "viewUniformBlock",           UNIFORM_BLOCK_VERTEX_VIEW_BLOCK);
    mVertexEmitterStaticUniformBlock   .InitializeVertexUniformBlock(&mShader, "emitterStaticUniformBlock",  UNIFORM_BLOCK_VERTEX_EMITTER_STATIC_BLOCK);
    mVertexEmitterDynamicUniformBlock  .InitializeVertexUniformBlock(&mShader, "emitterDynamicUniformBlock", UNIFORM_BLOCK_VERTEX_EMITTER_DYNAMIC_BLOCK);
    mFragmentEmitterDynamicUniformBlock.InitializeVertexUniformBlock(&mShader, "emitterDynamicUniformBlock", UNIFORM_BLOCK_FRAGMENT_EMITTER_DYNAMIC_BLOCK);
}

void ParticleShader::InitializeFragmentShaderLocation()
{
    mFragmentViewUniformBlock         .InitializePixelUniformBlock(&mShader, "viewUniformBlock",          UNIFORM_BLOCK_FRAGMENTVIEW_BLOCK);
    mFragmentEmitterStaticUniformBlock.InitializePixelUniformBlock(&mShader, "emitterStaticUniformBlock", UNIFORM_BLOCK_FRAGMENT_SHADER_PARAM_BLOCK);

    if (mFragmentShaderKey.mTexture0Dim == EFT_FRAGMENT_SHADER_TEXTURE_DIM_2D_ARRAY)
        mFragmentTextureSampler[EFT_TEXTURE_SLOT_0].loc = mShader.GetFragmentSamplerLocation("textureArraySampler0");
    else
        mFragmentTextureSampler[EFT_TEXTURE_SLOT_0].loc = mShader.GetFragmentSamplerLocation("textureSampler0");

    if (mFragmentShaderKey.mTextureVariation == EFT_FRAGMENT_SHADER_TEXTURE_VARIATION_1)
    {
        if (mFragmentShaderKey.mTexture1Dim == EFT_FRAGMENT_SHADER_TEXTURE_DIM_2D_ARRAY)
            mFragmentTextureSampler[EFT_TEXTURE_SLOT_1].loc = mShader.GetFragmentSamplerLocation("textureArraySampler1");
        else
            mFragmentTextureSampler[EFT_TEXTURE_SLOT_1].loc = mShader.GetFragmentSamplerLocation("textureSampler1");
    }

    if (mFragmentShaderKey.mTextureVariation == EFT_FRAGMENT_SHADER_TEXTURE_VARIATION_2)
    {
        if (mFragmentShaderKey.mTexture1Dim == EFT_FRAGMENT_SHADER_TEXTURE_DIM_2D_ARRAY)
            mFragmentTextureSampler[EFT_TEXTURE_SLOT_1].loc = mShader.GetFragmentSamplerLocation("textureArraySampler1");
        else
            mFragmentTextureSampler[EFT_TEXTURE_SLOT_1].loc = mShader.GetFragmentSamplerLocation("textureSampler1");

      //if (mFragmentShaderKey.mTexture2Dim == 1)
      //    mFragmentTextureSampler[EFT_TEXTURE_SLOT_2].loc = mShader.GetFragmentSamplerLocation("textureArraySampler2");
      //else
            mFragmentTextureSampler[EFT_TEXTURE_SLOT_2].loc = mShader.GetFragmentSamplerLocation("textureSampler2");
    }

    mFragmentTextureSampler[EFT_TEXTURE_SLOT_FRAME_BUFFER].loc  = mShader.GetFragmentSamplerLocation("frameBufferTexture");
    mFragmentTextureSampler[EFT_TEXTURE_SLOT_DEPTH_BUFFER].loc  = mShader.GetFragmentSamplerLocation("depthBufferTexture");
    mFragmentTextureSampler[EFT_TEXTURE_SLOT_CURL_NOISE].loc    = mShader.GetFragmentSamplerLocation("textureArrayCurlNoiseSampler");

    mFragmentTextureSampler[EFT_USER_TEXTURE_SLOT_0].loc        = mShader.GetFragmentSamplerLocation("userTextureSampler0");
    mFragmentTextureSampler[EFT_USER_TEXTURE_SLOT_1].loc        = mShader.GetFragmentSamplerLocation("userTextureSampler1");
    mFragmentTextureSampler[EFT_USER_TEXTURE_SLOT_2].loc        = mShader.GetFragmentSamplerLocation("userTextureSampler2");
    mFragmentTextureSampler[EFT_USER_TEXTURE_SLOT_3].loc        = mShader.GetFragmentSamplerLocation("userTextureSampler3");

    mVertexTextureSampler[EFT_TEXTURE_SLOT_FRAME_BUFFER].loc    = mShader.GetVertexSamplerLocation("frameBufferTexture");
    mVertexTextureSampler[EFT_TEXTURE_SLOT_DEPTH_BUFFER].loc    = mShader.GetVertexSamplerLocation("depthBufferTexture");
    mVertexTextureSampler[EFT_TEXTURE_SLOT_CURL_NOISE].loc      = mShader.GetVertexSamplerLocation("textureArrayCurlNoiseSampler");

    mVertexTextureSampler[EFT_USER_TEXTURE_SLOT_0].loc          = mShader.GetVertexSamplerLocation("userTextureSampler0");
    mVertexTextureSampler[EFT_USER_TEXTURE_SLOT_1].loc          = mShader.GetVertexSamplerLocation("userTextureSampler1");
    mVertexTextureSampler[EFT_USER_TEXTURE_SLOT_2].loc          = mShader.GetVertexSamplerLocation("userTextureSampler2");
    mVertexTextureSampler[EFT_USER_TEXTURE_SLOT_3].loc          = mShader.GetVertexSamplerLocation("userTextureSampler3");
}

void ParticleShader::InitializeAttribute()
{
    mPositionAttr   = mShader.GetAttribute("v_inPos",           0, Shader::FORMAT_32_32_32_32_FLOAT,  0, false);
    if (!mVertexShaderKey.IsUsePrimitive())
        mIndexAttr  = mShader.GetAttribute("v_inIndex",         1, Shader::FORMAT_32_UINT,            0, false);
    mTexCoordAttr0  = mShader.GetAttribute("v_inTexCoord",      2, Shader::FORMAT_32_32_32_32_FLOAT,  0, false);
    mNormalAttr     = mShader.GetAttribute("v_inNormal",        3, Shader::FORMAT_32_32_32_FLOAT,     0, false);
    mColorAttr      = mShader.GetAttribute("v_inColor",         4, Shader::FORMAT_32_32_32_32_FLOAT,  0, false);

    mWldPosAttr     = mShader.GetAttribute("v_inWldPos",        5, Shader::FORMAT_32_32_32_32_FLOAT,  0, true);
    mSclAttr        = mShader.GetAttribute("v_inScl",           5, Shader::FORMAT_32_32_32_32_FLOAT,  4, true);
    mVectorAttr     = mShader.GetAttribute("v_inVec",           5, Shader::FORMAT_32_32_32_32_FLOAT,  8, true);
    mRandomAttr     = mShader.GetAttribute("v_inRandom",        5, Shader::FORMAT_32_32_32_32_FLOAT, 12, true);
    vRotAttr        = mShader.GetAttribute("v_inRot",           5, Shader::FORMAT_32_32_32_32_FLOAT, 16, true);
    mEmtMatAttr0    = mShader.GetAttribute("v_inEmtMat0",       5, Shader::FORMAT_32_32_32_32_FLOAT, 20, true);
    mEmtMatAttr1    = mShader.GetAttribute("v_inEmtMat1",       5, Shader::FORMAT_32_32_32_32_FLOAT, 24, true);
    mEmtMatAttr2    = mShader.GetAttribute("v_inEmtMat2",       5, Shader::FORMAT_32_32_32_32_FLOAT, 28, true);
    mColor0Attr     = mShader.GetAttribute("v_inColor0",        5, Shader::FORMAT_32_32_32_32_FLOAT, 32, true);
    mColor1Attr     = mShader.GetAttribute("v_inColor1",        5, Shader::FORMAT_32_32_32_32_FLOAT, 36, true);
    mWldPosDfAttr   = mShader.GetAttribute("v_inWldPosDf",      5, Shader::FORMAT_32_32_32_32_FLOAT, 40, true);

    mUserAttr0      = mShader.GetAttribute("v_inUserAttr0",     5, Shader::FORMAT_32_32_32_32_FLOAT,  0, true);
    mUserAttr1      = mShader.GetAttribute("v_inUserAttr1",     6, Shader::FORMAT_32_32_32_32_FLOAT,  0, true);
    mUserAttr2      = mShader.GetAttribute("v_inUserAttr2",     7, Shader::FORMAT_32_32_32_32_FLOAT,  0, true);
    mUserAttr3      = mShader.GetAttribute("v_inUserAttr3",     8, Shader::FORMAT_32_32_32_32_FLOAT,  0, true);

    mPositionInput  = mShader.GetAttribute("v_inStreamOutPos",  9, Shader::FORMAT_32_32_32_32_FLOAT,  0, true, true);
    mVectorInput    = mShader.GetAttribute("v_inStreamOutVec", 10, Shader::FORMAT_32_32_32_32_FLOAT,  0, true, true);

    if (mSclAttr      != EFT_INVALID_ATTRIBUTE) mShaderArrtFlag |= EFT_SHADER_ATTRIBUTE_HAS_SCALE;
    if (mWldPosAttr   != EFT_INVALID_ATTRIBUTE) mShaderArrtFlag |= EFT_SHADER_ATTRIBUTE_HAS_WORLD_POS;
    if (mWldPosDfAttr != EFT_INVALID_ATTRIBUTE) mShaderArrtFlag |= EFT_SHADER_ATTRIBUTE_HAS_WORLD_POS_DIFF;
    if (mColor0Attr   != EFT_INVALID_ATTRIBUTE) mShaderArrtFlag |= EFT_SHADER_ATTRIBUTE_HAS_COLOR0;
    if (mColor1Attr   != EFT_INVALID_ATTRIBUTE) mShaderArrtFlag |= EFT_SHADER_ATTRIBUTE_HAS_COLOR1;
    if (vRotAttr      != EFT_INVALID_ATTRIBUTE) mShaderArrtFlag |= EFT_SHADER_ATTRIBUTE_HAS_ROTATE;
    if (mEmtMatAttr0  != EFT_INVALID_ATTRIBUTE) mShaderArrtFlag |= EFT_SHADER_ATTRIBUTE_HAS_EMITTER_MATRIX;
    if (mRandomAttr   != EFT_INVALID_ATTRIBUTE) mShaderArrtFlag |= EFT_SHADER_ATTRIBUTE_HAS_RANDOM;
    if (mVectorAttr   != EFT_INVALID_ATTRIBUTE) mShaderArrtFlag |= EFT_SHADER_ATTRIBUTE_HAS_VECTOR;
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

    memset(mDisplayListBuffer, 0, DISPLAY_LSIT_SIZE);
    DCFlushRange(mDisplayListBuffer, DISPLAY_LSIT_SIZE);
    DCInvalidateRange(mDisplayListBuffer, DISPLAY_LSIT_SIZE);

    GX2BeginDisplayList(mDisplayListBuffer, DISPLAY_LSIT_SIZE);
    {
        mShader.BindShader();
    }
    mDisplayListBufferUsed = GX2EndDisplayList(mDisplayListBuffer);

    return true;
}

void ParticleShader::Bind()
{
    GX2CallDisplayList(mDisplayListBuffer, mDisplayListBufferUsed);
}

bool ParticleShader::SetUserVertexUniformBlock(UserUniformBlockID uniformBlockID, const char* name, void* param, s32 size, bool suppressWarning)
{
    u32 index = uniformBlockID - UNIFORM_BLOCK_USER_VERTEX_ID_0;

    if (!mUserVertexUniformBlock[index].IsFailed() && !mUserVertexUniformBlock[index].IsInitialized())
    {
        if (!mUserVertexUniformBlock[index].InitializeVertexUniformBlock(&mShader, name, uniformBlockID))
        {
            if (!suppressWarning)
                WARNING("User Vertex UniformBlock Setup Failed !! : %s. \n", name);

            return false;
        }
    }

    if (mUserVertexUniformBlock[index].IsInitialized())
        mUserVertexUniformBlock[index].BindUniformBlock(param, size);

    return true;
}

bool ParticleShader::SetUserFragmentUniformBlock(UserUniformBlockID uniformBlockID, const char* name, void* param, s32 size, bool suppressWarning)
{
    u32 index = uniformBlockID - UNIFORM_BLOCK_USER_FRAGMENT_ID_0;

    if (!mUserFragmentUniformBlock[index].IsFailed() && !mUserFragmentUniformBlock[index].IsInitialized())
    {
        if (!mUserFragmentUniformBlock[index].InitializePixelUniformBlock(&mShader, name, uniformBlockID))
        {
            if (!suppressWarning)
                WARNING("User Fragment UniformBlock Setup Failed !! : %s. \n", name);

            return false;
        }
    }

    if (mUserFragmentUniformBlock[index].IsInitialized())
        mUserFragmentUniformBlock[index].BindUniformBlock(param, size);

    return true;
}

bool ParticleShader::SetUserVertexUniformBlockFromData(RenderStateSetArg& arg, UserUniformBlockID uniformBlockID, const char* name)
{
    const CustomShaderParam* customShaderParam = arg.renderer->GetCustomShaderParam(arg.emitter);

    f32* param = static_cast<f32*>(arg.renderer->AllocFromDoubleBuffer(sizeof(f32) * customShaderParam->paramNum));
    memcpy(param, customShaderParam->param, sizeof(f32) * customShaderParam->paramNum);

    GX2EndianSwap(param, sizeof(f32) * customShaderParam->paramNum);
    GX2Invalidate(GX2_INVALIDATE_CPU_UNIFORM_BLOCK, param, sizeof(f32) * customShaderParam->paramNum);

    return SetUserVertexUniformBlock(uniformBlockID, name, param);
}

bool ParticleShader::SetUserFragmentUniformBlockFromData(RenderStateSetArg& arg, UserUniformBlockID uniformBlockID, const char* name)
{
    const CustomShaderParam* customShaderParam = arg.renderer->GetCustomShaderParam(arg.emitter);

    f32* param = static_cast<f32*>(arg.renderer->AllocFromDoubleBuffer(sizeof(f32) * customShaderParam->paramNum));
    memcpy(param, customShaderParam->param, sizeof(f32) * customShaderParam->paramNum);

    GX2EndianSwap(param, sizeof(f32) * customShaderParam->paramNum);
    GX2Invalidate(GX2_INVALIDATE_CPU_UNIFORM_BLOCK, param, sizeof(f32) * customShaderParam->paramNum);

    return SetUserFragmentUniformBlock(uniformBlockID, name, param);
}

bool ParticleShader::SetUserUniformBlockFromData(RenderStateSetArg& arg, UserUniformBlockID uniformBlockID, const char* name)
{
    bool ret =    SetUserVertexUniformBlockFromData  (arg, uniformBlockID, name);
    return ret && SetUserFragmentUniformBlockFromData(arg, uniformBlockID, name);
}

FragmentTextureLocation ParticleShader::GetUserFragmentSamplerLocation(UserSamplerSlot slot, const char* name)
{
    if (mUserFragmentSamplerLocation[slot].loc == EFT_INVALID_SAMPLER)
        mUserFragmentSamplerLocation[slot].loc = mShader.GetFragmentSamplerLocation(name);

    return mUserFragmentSamplerLocation[slot];
}

VertexTextureLocation ParticleShader::GetUserVertexSamplerLocation(UserSamplerSlot slot, const char* name)
{
    if (mUserVertexSamplerLocation[slot].loc == EFT_INVALID_SAMPLER)
        mUserVertexSamplerLocation[slot].loc = mShader.GetVertexSamplerLocation(name);

    return mUserVertexSamplerLocation[slot];
}

void ParticleShader::EnableInstanced()
{
}

void ParticleShader::DisableInstanced()
{
}

} } // namespace nw::eft
