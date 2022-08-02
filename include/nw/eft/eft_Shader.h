#ifndef EFT_SHADER_H_
#define EFT_SHADER_H_

#include <nw/eft/cafe/eft_CafeWrapper.h>

namespace nw { namespace eft {

class  Renderer;
struct RenderStateSetArg;

// nw::eft::CombinerType
typedef u8 FragmentCombinerTypeVariation;

// nw::eft::AlphaCombinerType
typedef u8 FragmentAlphaVariation;

enum
{
    EFT_FRAGMENT_SHADER_TYPE_VARIATION_PARTICLE             = 0,
    EFT_FRAGMENT_SHADER_TYPE_VARIATION_REFRACT_PARTICLE     = 1,
    EFT_FRAGMENT_SHADER_TYPE_VARIATION_DISTORTION_PARTICLE  = 2,
    EFT_FRAGMENT_SHADER_TYPE_VARIATION_MAX                  = 3,
};

typedef u8 FragmentShaderVariation;

enum
{
    EFT_FRAGMENT_SHADER_TEXTURE_VARIATION_0 = 0,
    EFT_FRAGMENT_SHADER_TEXTURE_VARIATION_1,
    EFT_FRAGMENT_SHADER_TEXTURE_VARIATION_2,
    EFT_FRAGMENT_SHADER_TEXTURE_VARIATION_MAX
};

typedef u8 FragmentTextureVariation;

enum
{
    EFT_FRAGMENT_SHADER_COMPOSITE_MUL = 0,
    EFT_FRAGMENT_SHADER_COMPOSITE_ADD,
    EFT_FRAGMENT_SHADER_COMPOSITE_SUB,
    EFT_FRAGMENT_SHADER_COMPOSITE_MAX,
};

typedef u8 FragmentCompositeVariation;

enum
{
    EFT_FRAGMENT_SHADER_INPUT_OPT_SRC = 0,
    EFT_FRAGMENT_SHADER_INPUT_OPT_ONE,
    EFT_FRAGMENT_SHADER_INPUT_OPT_RED,
    EFT_FRAGMENT_SHADER_INPUT_OPT_MAX,
};

enum
{
    EFT_FRAGMENT_SHADER_TEXTURE_DIM_2D = 0,
    EFT_FRAGMENT_SHADER_TEXTURE_DIM_2D_ARRAY,
    EFT_FRAGMENT_SHADER_TEXTURE_DIM_MAX,
};

typedef u8 FragmentInputOptVariation;

struct _Flag64
{
    void Init()
    {
        flag0 = 0;
        flag1 = 0;
    }

    u32 getCarryCount(u64 bits)
    {
        u32 num  = 63;
        u64 mask = 0x8000000000000000;

        for (; mask != 0; mask = mask >> 1)
        {
            if (bits & mask)
                break;

            num--;
        }

        return num;
    }

    void Set(u64 bit)
    {
        if (bit > (u64)0x80000000)
            flag1 |= (bit >> (u64)32);
        else
            flag0 |= bit;
    }

    bool IsFlag(u64 bit) const
    {
        if (bit > (u64)0x80000000)
            return (flag1 & (bit >> (u64)32)) ? 1 : 0;
        else
            return (flag0 & bit) ? 1 : 0;
    }

    u32 flag0;
    u32 flag1;
};

struct VertexShaderKey
{
    u8                              mTexture0Dim;
    u8                              mTexture1Dim;
    u8                              mTexture2Dim;
    u8                              mCustomShaderSetting;
    _Flag64                         mFlag;
    s32                             mDrawPath;
    u32                             mCustomShaderFlag;
    u32                             mCustomShaderSwitchFlag;
    char                            mCustomShaderCompileDef[16];

    // TODO

    bool IsStripe() const { return mFlag.IsFlag(u64(0x80)); }
    bool IsUsePrimitive() const { return mFlag.IsFlag(u64(0x200000)); }
    bool IsUseStreamOut() const { return mFlag.IsFlag(u64(0x8000000)); }
    bool IsUseGpuAcceleration() const { return mFlag.IsFlag(u64(0x2000000)); }

    void MakeKeyFromSimpleEmitterData(const SimpleEmitterData* res, const char* userDef);
    void MakeKeyFromComplexEmitterData(const ComplexEmitterData* res, const StripeData* stripeRes, const char* userDef);
    void MakeKeyFromChildData(const ChildData* res, const char* userDef, const SimpleEmitterData* parent);

    bool IsEqual(const VertexShaderKey* key) const;

    bool GetCompileSetting(char* compileSetting, u32 compileSettingBufferSize, bool targetCafe);
};
static_assert(sizeof(VertexShaderKey) == 0x28, "nw::eft::VertexShaderKey size mismatch");

struct FragmentShaderKey
{
    FragmentShaderVariation             mShaderVariation;
    FragmentTextureVariation            mTextureVariation;
    FragmentCombinerTypeVariation       mColorVariation;
    FragmentAlphaVariation              mAlphaVariation;

    FragmentCompositeVariation          mTexture1ColorComposite;
    FragmentCompositeVariation          mTexture1AlphaComposite;
    FragmentCompositeVariation          mTexture2ColorComposite;
    FragmentCompositeVariation          mTexture2AlphaComposite;

    FragmentCompositeVariation          mPrimitiveColorComposite;
    FragmentCompositeVariation          mPrimitiveAlphaComposite;
    FragmentInputOptVariation           mTexture0ColorOpt;
    FragmentInputOptVariation           mTexture1ColorOpt;

    FragmentInputOptVariation           mTexture2ColorOpt;
    FragmentInputOptVariation           mPrimitiveColorOpt;
    FragmentInputOptVariation           mTexture0AlphaOpt;
    FragmentInputOptVariation           mTexture1AlphaOpt;

    FragmentInputOptVariation           mTexture2AlphaOpt;
    FragmentInputOptVariation           mPrimitiveAlphaOpt;
    u8                                  mCustomShaderSetting;
    u8                                  mTexture0Dim;

    u8                                  mTexture1Dim;
    u8                                  mTexture2Dim;
    u8                                  dummy[2];

    u32                                 mCustomShaderFlag;
    u32                                 mCustomShaderSwitchFlag;
    char                                mCustomShaderCompileDef[16];

    u32                                 mFlag;
    u32                                 mShaderToolGenCodeNo;
    s32                                 mDrawPath;

    // TODO

    bool IsUseSoftEdge() const;
    bool IsUseSoftEdgeVolume() const;
    bool IsUsePrimitive() const;
    bool IsApplyAlpha() const;
    bool IsUseFresnelAlpha() const;
    bool IsUseDecal() const;

    bool IsEqual(const FragmentShaderKey* key) const;

    void MakeKeyFromSimpleEmitterData(const SimpleEmitterData* res, const char* userDef);
    void MakeKeyFromChildData(const ChildData* res, const char* userDef, u32 childFlag, const SimpleEmitterData* parent);

    bool GetCompileSetting(char* compileSetting, u32 compileSettingBufferSize, bool targetCafe);
};
static_assert(sizeof(FragmentShaderKey) == 0x3C, "nw::eft::FragmentShaderKey size mismatch");

struct GeometryShaderKey
{
    u8       mStripeType;
    u8       mDummy[3];

    // TODO

    void MakeKeyFromComplexEmitterData(const ComplexEmitterData* res, const StripeData* stripe);

    bool IsEqual(const GeometryShaderKey* key);

    bool GetCompileSetting(char* compileSetting, u32 compileSettingBufferSize, bool targetCafe);
};
static_assert(sizeof(GeometryShaderKey) == 4, "nw::eft::GeometryShaderKey size mismatch");

struct ShaderImageInformation
{
    u32                 shaderNum;
    u32                 totalSize;
    u32                 offsetShaderSrcInfo;
    u32                 offsetGenShaderSrcInfo;
    u32                 offsetShaderBinary;
    u32                 offsetShaderBinInfo;
};
static_assert(sizeof(ShaderImageInformation) == 0x18, "nw::eft::ShaderImageInformation size mismatch");

struct sourceCodeTable
{
    u32                 size;
    u32                 offset;
};

struct ShaderSrcInformation
{
    u32                 shaderSourceNum;
    u32                 shaderSourceTotalSize;

    sourceCodeTable     vshParticle;
    sourceCodeTable     fshParticle;
    sourceCodeTable     vshStripe;
    sourceCodeTable     gshStripe;
    sourceCodeTable     vshUser[9];
    sourceCodeTable     fshUser[9];
    sourceCodeTable     vshParticleDeclaration;
    sourceCodeTable     fshParticleDeclaration;
};

struct ShaderGenerateSrcInformation
{
    u32                 genShaderSourceNum;
    u32                 headerSize;
};

struct ShaderInformation
{
    VertexShaderKey     vertexShaderKey;
    FragmentShaderKey   fragmentShaderKey;
    GeometryShaderKey   geometryShaderKey;
    u32                 shaderSize;
    u32                 offset;
    s32                 userVertexShaderIndex;
    s32                 userFragmentShaderIndex;
};
static_assert(sizeof(ShaderInformation) == 0x78, "nw::eft::ShaderInformation size mismatch");

class ParticleShader
{
    friend class Renderer;

private:
    enum
    {
        UNIFORM_BLOCK_VERTEX_VIEW_BLOCK                 = 0,
        UNIFORM_BLOCK_VERTEX_EMITTER_STATIC_BLOCK       = 1,
        UNIFORM_BLOCK_VERTEX_EMITTER_DYNAMIC_BLOCK      = 2,
        UNIFORM_BLOCK_VERTEX_STRIPE_BLOCK               = 3,
        UNIFORM_BLOCK_FRAGMENT_SHADER_PARAM_BLOCK       = 4,
        UNIFORM_BLOCK_FRAGMENT_EMITTER_DYNAMIC_BLOCK    = 5,
        UNIFORM_BLOCK_FRAGMENTVIEW_BLOCK                = 6,
        UNIFORM_BLOCK_ID_MAX                            = 9,
    };

public:
    enum UserUniformBlockID
    {
        UNIFORM_BLOCK_USER_VERTEX_ID_0      = UNIFORM_BLOCK_ID_MAX,
        UNIFORM_BLOCK_USER_VERTEX_ID_1      = UNIFORM_BLOCK_ID_MAX + 1,
        UNIFORM_BLOCK_USER_VERTEX_ID_2      = UNIFORM_BLOCK_ID_MAX + 2,
        UNIFORM_BLOCK_USER_VERTEX_ID_3      = UNIFORM_BLOCK_ID_MAX + 3,
        UNIFORM_BLOCK_USER_FRAGMENT_ID_0    = UNIFORM_BLOCK_ID_MAX + 4,
        UNIFORM_BLOCK_USER_FRAGMENT_ID_1    = UNIFORM_BLOCK_ID_MAX + 5,
        UNIFORM_BLOCK_USER_FRAGMENT_ID_2    = UNIFORM_BLOCK_ID_MAX + 6,
        UNIFORM_BLOCK_USER_FRAGMENT_ID_3    = UNIFORM_BLOCK_ID_MAX + 7,
    };

    enum UserSamplerSlot
    {
        USER_SAMPLER_SLOT_0      = 0,
        USER_SAMPLER_SLOT_1      = 1,
        USER_SAMPLER_SLOT_2      = 2,
        USER_SAMPLER_SLOT_3      = 3,
        USER_SAMPLER_SLOT_4      = 4,
        USER_SAMPLER_SLOT_5      = 5,
        USER_SAMPLER_SLOT_6      = 6,
        USER_SAMPLER_SLOT_7      = 7,
        USER_SAMPLER_SLOT_MAX    = 8,
    };

public:
    ParticleShader();

    void Finalize(Heap* heap);

    bool SetupShaderResource(Heap* heap, void* shaderResource, u32 shaderResourceSize);

    void SetVertexShaderKey(VertexShaderKey* key)
    {
        mVertexShaderKey = *key;
    }

    void SetFragmentShaderKey(FragmentShaderKey* key)
    {
        mFragmentShaderKey = *key;
    }

    void SetGeometryShaderKey(GeometryShaderKey* key)
    {
        mGeometryShaderKey = *key;
    }

    const VertexShaderKey& GetVertexShaderKey() const
    {
        return mVertexShaderKey;
    }

    const FragmentShaderKey& GetFragmentShaderKey() const
    {
        return mFragmentShaderKey;
    }

    const GeometryShaderKey& GetGeometryShaderKey() const
    {
        return mGeometryShaderKey;
    }

    bool IsStripe() const
    {
        return mVertexShaderKey.IsStripe();
    }

    bool IsGpuAcceleration() const
    {
        return mVertexShaderKey.IsUseGpuAcceleration();
    }

    bool IsUseStreamOut() const
    {
        return mVertexShaderKey.IsUseStreamOut();
    }

    FragmentShaderVariation GetShaderVariation() const
    {
        return mFragmentShaderKey.mShaderVariation;
    }

    bool IsFragmentSoftEdgeVariation() const
    {
        return mFragmentShaderKey.IsUseSoftEdge();
    }

    bool IsFragmentFresnelAlphaVariation() const
    {
        return mFragmentShaderKey.IsUseFresnelAlpha();
    }

    FragmentTextureVariation GetTextureVariation() const
    {
        return mFragmentShaderKey.mTextureVariation;
    }

    FragmentCombinerTypeVariation GetCombinerTyperVariation() const
    {
        return mFragmentShaderKey.mColorVariation;
    }

    FragmentAlphaVariation GetAlphaVariation() const
    {
        return mFragmentShaderKey.mAlphaVariation;
    }

    void InitializeAttribute();
    void InitializeStripeAttribute();
    void InitializeVertexShaderLocation();
    void InitializeStripeVertexShaderLocation();
    void InitializeFragmentShaderLocation();

    void Setup(Heap* heap)
    {
        mShader.SetupShader(heap);
    }

    bool IsInitialized() const
    {
        return mShader.IsInitialized();
    }

    void Bind();

    void EnableInstanced();
    void DisableInstanced();

    Shader* GetShaderInst()
    {
        return &mShader;
    }

    void BindViewUniformBlock(ViewUniformBlock* block)
    {
        mVertexViewUniformBlock.BindUniformBlock(block);
        mFragmentViewUniformBlock.BindUniformBlock(block);
    }

    void BindEmitterStaticUniformBlock(EmitterStaticUniformBlock* block)
    {
        mVertexEmitterStaticUniformBlock.BindUniformBlock(block);
        mFragmentEmitterStaticUniformBlock.BindUniformBlock(block);
    }

    void BindEmitterDynamicUniformBlock(EmitterDynamicUniformBlock* block)
    {
        mVertexEmitterDynamicUniformBlock.BindUniformBlock(block);
        mFragmentEmitterDynamicUniformBlock.BindUniformBlock(block);
    }

    s32 GetPositionAttribute() const { return mPositionAttr; }
    s32 GetNormalAttribute() const { return mNormalAttr; }
    s32 GetColorAttribute() const { return mColorAttr; }
    s32 GetTextureCoordAttribute() const { return mTexCoordAttr0; }
    s32 GetTextureCoordAttribute1() const { return mTexCoordAttr1; }
    s32 GetIndexAttribute() const { return mIndexAttr; }
    s32 GetRotateAttribute() const { return vRotAttr; }
    s32 GetColorAttribute0() const { return mColor0Attr; }
    s32 GetColorAttribute1() const { return mColor1Attr; }
    s32 GetScaleAttribute() const { return mSclAttr; }
    s32 GetWolrdPosAttribute() const { return mWldPosAttr; }
    s32 GetWolrdPosDiffAttribute() const { return mWldPosDfAttr; }
    s32 GetEmitterMatrix0Attribute() const { return mEmtMatAttr0; }
    s32 GetRandomAttribute() const { return mRandomAttr; }
    s32 GetOuterAttribute() const { return mOuterAttr; }
    s32 GetDirAttribute() const { return mDirAttr; }
    s32 GetShaderAttributeFlag() const { return mShaderArrtFlag; }
    s32 GetUserAttribute0() const { return mUserAttr0; }
    s32 GetUserAttribute1() const { return mUserAttr1; }
    s32 GetUserAttribute2() const { return mUserAttr2; }
    s32 GetUserAttribute3() const { return mUserAttr3; }

    FragmentTextureLocation GetTextureSamplerLocation(TextureSlot slot) const
    {
        return mFragmentTextureSampler[slot];
    }

    FragmentTextureLocation GetFragmentTextureSamplerLocation(TextureSlot slot) const
    {
        return GetTextureSamplerLocation(slot);
    }

    VertexTextureLocation GetVertexTextureSamplerLocation(TextureSlot slot) const
    {
        return mVertexTextureSampler[slot];
    }

    FragmentTextureLocation GetFrameBufferTextureSamplerLocation() const
    {
        return GetFrameBufferFragmentTextureSamplerLocation();
    }

    FragmentTextureLocation GetFrameBufferFragmentTextureSamplerLocation() const
    {
        return mFragmentTextureSampler[EFT_TEXTURE_SLOT_FRAME_BUFFER];
    }

    VertexTextureLocation GetFrameBufferVertexTextureSamplerLocation() const
    {
        return mVertexTextureSampler[EFT_TEXTURE_SLOT_FRAME_BUFFER];
    }

    FragmentTextureLocation GetDepthTextureSamplerLocation() const
    {
        return GetDepthFragmentTextureSamplerLocation();
    }

    FragmentTextureLocation GetDepthFragmentTextureSamplerLocation() const
    {
        return mFragmentTextureSampler[EFT_TEXTURE_SLOT_DEPTH_BUFFER];
    }

    VertexTextureLocation GetDepthVertexTextureSamplerLocation() const
    {
        return mVertexTextureSampler[EFT_TEXTURE_SLOT_DEPTH_BUFFER];
    }

    bool SetUserVertexUniformBlock(UserUniformBlockID uniformBlockID, const char* name, void* param, s32 size = -1, bool suppressWarning = false);
    bool SetUserFragmentUniformBlock(UserUniformBlockID uniformBlockID, const char* name, void* param, s32 size = -1, bool suppressWarning = false);

    bool SetUserVertexUniformBlockFromData(RenderStateSetArg& arg, UserUniformBlockID uniformBlockID, const char* name);
    bool SetUserFragmentUniformBlockFromData(RenderStateSetArg& arg, UserUniformBlockID uniformBlockID, const char* name);

    bool SetUserUniformBlockFromData(RenderStateSetArg& arg, UserUniformBlockID uniformBlockID, const char* name = "userUniformBlockParam");

    FragmentTextureLocation GetUserFragmentSamplerLocation(UserSamplerSlot userSamplerSlot, const char* name);
    VertexTextureLocation GetUserVertexSamplerLocation(UserSamplerSlot userSamplerSlot, const char* name);

private:
    enum {  DISPLAY_LSIT_SIZE = 512 };
    u8  mDisplayListBuffer[DISPLAY_LSIT_SIZE];
    u32 mDisplayListBufferUsed;

    Shader                  mShader;
    VertexShaderKey         mVertexShaderKey;
    FragmentShaderKey       mFragmentShaderKey;
    GeometryShaderKey       mGeometryShaderKey;

    s32                     mPositionAttr;
    s32                     mNormalAttr;
    s32                     mColorAttr;
    s32                     mTexCoordAttr0;
    s32                     mTexCoordAttr1;
    s32                     mIndexAttr;
    s32                     mRandomAttr;
    s32                     mOuterAttr;
    s32                     mDirAttr;

    VertexTextureLocation   mVertexTextureSampler[EFT_TEXTURE_SLOT_MAX];
    FragmentTextureLocation mFragmentTextureSampler[EFT_TEXTURE_SLOT_MAX];

    s32                     mSclAttr;
    s32                     mVectorAttr;
    s32                     mWldPosAttr;
    s32                     mWldPosDfAttr;
    s32                     mColor0Attr;
    s32                     mColor1Attr;
    s32                     vRotAttr;
    s32                     mEmtMatAttr0;
    s32                     mEmtMatAttr1;
    s32                     mEmtMatAttr2;
    u32                     mShaderArrtFlag;

    s32                     mUserAttr0;
    s32                     mUserAttr1;
    s32                     mUserAttr2;
    s32                     mUserAttr3;

    s32                     mPositionInput;
    s32                     mPositionOut;
    s32                     mVectorInput;
    s32                     mVectorOut;

    s32                     mUserAttr4_ni;
    s32                     mUserAttr5_ni;
    s32                     mUserAttr6_ni;
    s32                     mUserAttr7_ni;

    UniformBlock            mVertexViewUniformBlock;
    UniformBlock            mFragmentViewUniformBlock;
    UniformBlock            mVertexEmitterStaticUniformBlock;
    UniformBlock            mVertexEmitterDynamicUniformBlock;
    UniformBlock            mFragmentEmitterStaticUniformBlock;
    UniformBlock            mFragmentEmitterDynamicUniformBlock;
    UniformBlock            mStripeUniformBlock;

    UniformBlock            mUserVertexUniformBlock[4];
    UniformBlock            mUserFragmentUniformBlock[4];
    VertexTextureLocation   mUserVertexSamplerLocation[USER_SAMPLER_SLOT_MAX];
    FragmentTextureLocation mUserFragmentSamplerLocation[USER_SAMPLER_SLOT_MAX];
};
static_assert(sizeof(ParticleShader) == 0x704, "nw::eft::ParticleShader size mismatch");

} } // namespace nw::eft

#endif // EFT_SHADER_H_
