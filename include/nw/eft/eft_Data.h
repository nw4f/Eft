#ifndef EFT_DATA_H_
#define EFT_DATA_H_

#include <nw/typeDef.h>
#include <nw/eft/eft_Enum.h>
#include <nw/eft/eft_UniformBlock.h>

namespace nw { namespace eft {

struct CommonEmitterData;

struct VertexTextureLocation
{
    u32 loc;

    bool IsValid() const
    {
        return loc != u32(EFT_INVALID_LOCATION);
    }
};
static_assert(sizeof(VertexTextureLocation) == 4, "nw::eft::VertexTextureLocation size mismatch");

struct FragmentTextureLocation
{
    u32 loc;

    bool IsValid() const
    {
        return loc != u32(EFT_INVALID_LOCATION);
    }
};
static_assert(sizeof(FragmentTextureLocation) == 4, "nw::eft::FragmentTextureLocation size mismatch");

struct PtclAttributeBuffer
{
    nw::math::VEC4    pWldPos;
    nw::math::VEC4    pScl;
    nw::math::VEC4    pVector;
    nw::math::VEC4    pRandom;
    nw::math::VEC4    pRot;
    nw::math::VEC4    pEmtMat[3];
    nw::math::VEC4    pColor0;
    nw::math::VEC4    pColor1;
    nw::math::VEC4    pWldPosDf;
    nw::math::VEC4    align32;
};
static_assert(sizeof(PtclAttributeBuffer) == 0xC0, "nw::eft::PtclAttributeBuffer size mismatch");

struct PtclAttributeBufferGpu
{
    nw::math::VEC4    pWldPos;
    nw::math::VEC4    pScl;
    nw::math::VEC4    pVector;
    nw::math::VEC4    pRandom;
    nw::math::VEC4    pRot;
    nw::math::VEC4    pEmtMat[3];
};
static_assert(sizeof(PtclAttributeBufferGpu) == 0x80, "nw::eft::PtclAttributeBufferGpu size mismatch");

struct PrimitiveImageInformation
{
    u32     primitiveNum;
    u32     totalSize;
    u32     offsetPrimitiveTableInfo;
};
static_assert(sizeof(PrimitiveImageInformation) == 0xC, "nw::eft::PrimitiveImageInformation size mismatch");

struct PrimitiveTableInfo
{
    struct PrimDataTable
    {
        u32     count;
        u32     column;
        u32     offset;
        u32     size;
    };
    static_assert(sizeof(PrimDataTable) == 0x10, "nw::eft::PrimitiveTableInfo::PrimDataTable size mismatch");

    PrimDataTable pos;
    PrimDataTable normal;
    PrimDataTable color;
    PrimDataTable texCoord;
    PrimDataTable index;
};
static_assert(sizeof(PrimitiveTableInfo) == 0x50, "nw::eft::PrimitiveTableInfo size mismatch");

struct HeaderData
{
    char    magic[4];
    u32     version;
    s32     numEmitterSet;
    s32     namePos;
    s32     nameTblPos;
    s32     textureTblPos;
    s32     textureTblSize;
    s32     shaderTblPos;
    s32     shaderTblSize;
    s32     animkeyTblPos;
    s32     animkeyTblSize;
    s32     primitiveTblPos;
    s32     primitiveTblSize;
    s32     shaderParamTblPos;
    s32     shaderParamTblSize;
    s32     totalTextureSize;
    s32     totalShaderSize;
    s32     totalEmitterSize;
};
static_assert(sizeof(HeaderData) == 0x48, "nw::eft::HeaderData size mismatch");

struct EmitterSetData
{
    u32     userData;
    u32     lastUpdateDate;
    s32     namePos;
    char*   name;
    s32     numEmitter;
    s32     emitterTblPos;
    u32*    emitterTbl;
};
static_assert(sizeof(EmitterSetData) == 0x1C, "nw::eft::EmitterSetData size mismatch");

struct EmitterTblData
{
    s32                         emitterPos;
    CommonEmitterData*          emitterResource;
    EmitterStaticUniformBlock*  staticUniformBlock;
    EmitterStaticUniformBlock*  childStaticUniformBlock;
};
static_assert(sizeof(EmitterTblData) == 0x10, "nw::eft::EmitterTblData size mismatch");

struct AnimKeyTable
{
    void*               animKeyTable;
    u32                 animPos;
    u32                 dataSize;
};
static_assert(sizeof(AnimKeyTable) == 0xC, "nw::eft::AnimKeyTable size mismatch");

struct CustomShaderParam
{
    u32                 paramNum;
    u32                 paramPos;
    f32*                param;
};
static_assert(sizeof(CustomShaderParam) == 0xC, "nw::eft::CustomShaderParam size mismatch");
typedef CustomShaderParam UserShaderParam;

struct AnimKeyFrameKey
{
    f32     x;
    union
    {
        f32     y;
        s32     yFixed;
    };
};
static_assert(sizeof(AnimKeyFrameKey) == 8, "nw::eft::AnimKeyFrameKey size mismatch");

struct KeyFrameAnimArray
{
    u8                                  magic[4];
    u32                                 numAnims;
};
static_assert(sizeof(KeyFrameAnimArray) == 8, "nw::eft::KeyFrameAnimArray size mismatch");

struct KeyFrameAnim
{
    u32                                 keyNum;
    AnimKeyFrameInterpolationType       interpolation;
    AnimKeyFrameApplyType               target;
    u32                                 isLoop;
    u32                                 loopFrame;
    u32                                 isStartRandom;
    u32                                 offset;
};
static_assert(sizeof(KeyFrameAnim) == 0x1C, "nw::eft::KeyFrameAnim size mismatch");

struct TextureRes
{
    u16                 width;
    u16                 height;
    GX2TileMode         tileMode;
    u32                 swizzle;
    u8                  wrapMode;
    u8                  filterMode;
    u8                  depth;
    u8                  dummy;
    u32                 mipLevel;
    u32                 compSel;
    f32                 enableMipLevel;
    f32                 mipMapBias;
    TextureFormat       originalDataFormat;
    s32                 originalDataPos;
    s32                 originalDataSize;
    TextureFormat       nativeDataFormat;
    s32                 nativeDataSize;
    s32                 nativeDataPos;
    u32                 handle;
    GX2Texture          gx2Texture;
};
static_assert(sizeof(TextureRes) == 0xD8, "nw::eft::TextureRes size mismatch");

struct PrimitiveFigure
{
    void*               primitiveTableInfo;
    u32                 dataSize;
    u32                 index;
};
static_assert(sizeof(PrimitiveFigure) == 0xC, "nw::eft::PrimitiveFigure size mismatch");

struct CommonEmitterData
{
    EmitterType         type;
    u32                 flg;
    u32                 randomSeed;
    u32                 userData;
    u32                 userData2;
    f32                 userDataF[EFT_CUSTOM_ACTION_PARAM_MAX];
    s32                 userCallbackID;
    s32                 namePos;
    char*               name;
    TextureRes          texRes[EFT_TEXTURE_SLOT_BIN_MAX];
    AnimKeyTable        animKeyTable;
    PrimitiveFigure     primitiveFigure;
    PrimitiveFigure     emitVolumeFigure;
    CustomShaderParam   customShaderParam;
};
static_assert(sizeof(CommonEmitterData) == 0x2F8, "nw::eft::CommonEmitterData size mismatch");

struct TextureEmitterData
{
    u8                      isTexPatAnim;
    u8                      isTexPatAnimRand;
    u8                      isTexPatAnimClamp;
    u8                      numTexDivX;

    u8                      numTexDivY;
    u8                      numTexPat;
    u8                      isTexURandomInv;
    u8                      isTexVRandomInv;

    u8                      isCrossFade;
    u8                      isSphereMap;
    u8                      dummy[2];

    u8                      texPatTbl[EFT_TEXTURE_PATTERN_NUM];

    TexturePatternAnimMode  texPtnAnimMode;

    s16                     texPatFreq;
    s16                     texPatTblUse;
    f32                     texUScale;
    f32                     texVScale;
    UvShiftAnimMode         uvShiftAnimMode;
    nw::math::VEC2          uvScroll;
    nw::math::VEC2          uvScrollInit;
    nw::math::VEC2          uvScrollInitRand;
    nw::math::VEC2          uvScale;
    nw::math::VEC2          uvScaleInit;
    nw::math::VEC2          uvScaleInitRand;
    f32                     uvRot;
    f32                     uvRotInit;
    f32                     uvRotInitRand;
};
static_assert(sizeof(TextureEmitterData) == 0x7C, "nw::eft::TextureEmitterData size mismatch");

struct anim3v4Key
{
    f32                 init;
    f32                 diff21;
    f32                 diff32;
    f32                 section1;
    f32                 section2;
};
static_assert(sizeof(anim3v4Key) == 0x14, "nw::eft::anim3v4Key size mismatch");

struct SimpleEmitterData : public CommonEmitterData
{
    //------------------------------------------------------------------------------
    //  Flags
    //------------------------------------------------------------------------------
    u8                  isWorldGravity;
    u8                  isStopEmitInFade;
    u8                  isDisplayParent;
    u8                  isEmitDistEnabled;

    u8                  isRotDirRand[3];
    u8                  isOnetime;

    u8                  isVolumeLatitudeEnabled;
    u8                  isUpdateEmitterMatrixByEmit;
    u8                  volumeTblIndex;
    u8                  volumeSweepStartRandom;

    u8                  primitiveScaleY2Z;
    u8                  useFixedUVRandomAnimation;
    u8                  dummy[2];
    u32                 behaviorFlag;

    //------------------------------------------------------------------------------
    //  Various Settings
    //------------------------------------------------------------------------------
    PtclRotType         ptclRotType;
    PtclFollowType      ptclFollowType;
    CombinerType        colorCombinerType;
    AlphaCombinerType   alphaCombinerType;
    s32                 drawPath;
    DisplaySideType     displaySide;
    BlendType           blendType;
    ZBufATestType       zBufATestType;
    AnimType            ptclAnimTypeScale;
    AnimType            ptclAnimTypeColor0;
    AnimType            ptclAnimTypeColor1;
    AnimType            ptclAnimTypeAlpha0;
    AnimType            ptclAnimTypeAlpha1;

    //------------------------------------------------------------------------------
    //  Emitter Volume
    //------------------------------------------------------------------------------
    VolumeType          volumeType;
    VolumePrimEmitType  volumePrimEmitType;
    nw::math::VEC3      volumeRadius;
    f32                 volumeSweepStart;
    f32                 volumeSweepParam;
    f32                 volumeSurfacePosRand;
    f32                 volumeCaliber;
    f32                 volumeLatitude;
    nw::math::VEC3      volumeLatitudeDir;
    f32                 volumeLineCenter;
    nw::math::VEC3      volumeFormScale;

    //------------------------------------------------------------------------------
    //  Emitter
    //------------------------------------------------------------------------------
    nw::ut::Color4f     emitterColor0;
    nw::ut::Color4f     emitterColor1;
    f32                 emitterAlpha;
    nw::math::MTX34     emitterTransformSRT;
    nw::math::MTX34     emitterTransformRT;
    nw::math::VEC3      emitterScale;
    nw::math::VEC3      emitterRotate;
    nw::math::VEC3      emitterTrans;
    nw::math::VEC3      emitterRotateRnd;
    nw::math::VEC3      emitterTransRnd;

    //------------------------------------------------------------------------------
    //  Emission
    //------------------------------------------------------------------------------
    f32                 emitRate;
    s32                 emitRateRandom;
    s32                 emitStartFrame;
    s32                 emitEndFrame;
    s32                 emitStep;
    s32                 emitStepRnd;
    f32                 emitFigureVel;
    f32                 emitInitVelRnd;
    f32                 emitVel;
    nw::math::VEC3      emitVelDir;
    f32                 emitVelDirAngle;
    nw::math::VEC3      emitSpreadVec;
    f32                 emitAirRegist;
    f32                 emitXzDiffusionVel;
    f32                 emitInitPosRand;
    f32                 emitEmitterVelInherit;
    f32                 emitDistUnit;
    f32                 emitDistMax;
    f32                 emitDistMin;
    f32                 emitDistMargin;

    nw::math::VEC3      gravity;

    //------------------------------------------------------------------------------
    //  Particle
    //------------------------------------------------------------------------------
    s32                 ptclLife;
    s32                 ptclLifeRnd;
    MeshType            meshType;
    BillboardType       billboardType;
    f32                 dynamicsRandom;
    nw::math::VEC2      rotBasis;
    OffsetType          offsetType;
    f32                 offsetParam;

    //------------------------------------------------------------------------------
    //  Texture
    //------------------------------------------------------------------------------
    TextureEmitterData  textureData[EFT_TEXTURE_SLOT_BIN_MAX];

    //------------------------------------------------------------------------------
    //  Particle Color
    //------------------------------------------------------------------------------
    ColorCalcType       colorCalcType[EFT_COLOR_KIND_MAX];
    nw::ut::Color4f     color[EFT_COLOR_KIND_MAX][8];
    s32                 colorSection1[EFT_COLOR_KIND_MAX];
    s32                 colorSection2[EFT_COLOR_KIND_MAX];
    s32                 colorSection3[EFT_COLOR_KIND_MAX];
    s32                 colorNumRepeat[EFT_COLOR_KIND_MAX];
    s32                 colorRepeatStartRand[EFT_COLOR_KIND_MAX];
    f32                 colorScale;

    //------------------------------------------------------------------------------
    //  Particle Alpha
    //------------------------------------------------------------------------------
    AlphaCalcType       alphaCalcType[EFT_ALPHA_KIND_MAX];
    anim3v4Key          alpha3v4kAnim[EFT_ALPHA_KIND_MAX];

    //------------------------------------------------------------------------------
    //  Combiner
    //------------------------------------------------------------------------------
    ColorBlendType      texture1ColorBlend;
    ColorBlendType      texture2ColorBlend;
    ColorBlendType      primitiveColorBlend;
    ColorBlendType      texture1AlphaBlend;
    ColorBlendType      texture2AlphaBlend;
    ColorBlendType      primitiveAlphaBlend;

    //------------------------------------------------------------------------------
    //  Particle Scale
    //------------------------------------------------------------------------------
    s32                 scaleSection1;
    s32                 scaleSection2;
    nw::math::VEC2      scaleRand;
    nw::math::VEC2      baseScale;
    nw::math::VEC2      initScale;
    nw::math::VEC2      diffScale21;
    nw::math::VEC2      diffScale32;
    f32                 scaleVelAddY;

    //------------------------------------------------------------------------------
    //  Particle Rotation
    //------------------------------------------------------------------------------
    nw::math::VEC3      initRot;
    nw::math::VEC3      initRotRand;
    nw::math::VEC3      rotVel;
    nw::math::VEC3      rotVelRand;
    f32                 rotRegist;

    //------------------------------------------------------------------------------
    //  Fade Out
    //------------------------------------------------------------------------------
    f32                 alphaAddInFade;

    //------------------------------------------------------------------------------
    //  Shader
    //------------------------------------------------------------------------------
    u8                  shaderType;
    u8                  userShaderSetting;
    u8                  shaderUseSoftEdge;
    u8                  shaderUseFresnelAlpha;
    u8                  shaderUseNearAlpha;
    u8                  shaderUseFarAlpha;
    u8                  shaderUseDecal;
    u8                  shaderApplyAlphaToRefract;
    u8                  shaderUseParamAnim;
    u8                  dummy1[3];
    f32                 shaderParam0;
    f32                 shaderParam1;
    u32                 shaderIndex;
    u32                 userShaderIndex1;
    u32                 userShaderIndex2;
    SoftProcessingMethod softProcessingMethod;
    f32                 softFadeDistance;
    f32                 softAlphaOffset;
    f32                 softVolumeParam;
    f32                 fresnelAlphaMin;
    f32                 fresnelAlphaMax;
    f32                 decalVolume;
    f32                 nearAlphaMin;
    f32                 nearAlphaMax;
    f32                 farAlphaMin;
    f32                 farAlphaMax;
    u8                  userShaderDefine1[16];
    u8                  userShaderDefine2[16];
    u32                 userShaderFlag;
    u32                 userShaderSwitchFlag;
    u32                 genShederNumber;
    anim3v4Key          shaderParam3v4kAnim;
};
static_assert(sizeof(SimpleEmitterData) == 0x8BC, "nw::eft::SimpleEmitterData size mismatch");

struct ComplexEmitterData : public SimpleEmitterData
{
    u32                 childFlg;
    u16                 fieldFlg;
    u16                 fluctuationFlg;
    u16                 stripeFlg;
    u16                 dummy;

    u16                 childDataOffset;
    u16                 fieldDataOffset;
    u16                 fluctuationDataOffset;
    u16                 stripeDataOffset;

    s32                 emitterDataSize;
};
static_assert(sizeof(ComplexEmitterData) == 0x8D4, "nw::eft::ComplexEmitterData size mismatch");

struct ChildData
{
    s32                 childEmitRate;
    s32                 childEmitTiming;
    s32                 childLife;
    s32                 childEmitStep;

    u32                 behaviorFlag;

    f32                 childVelInheritRate;
    f32                 childFigureVel;
    nw::math::VEC3      childRandVel;
    f32                 childInitPosRand;
    PrimitiveFigure     childPrimitiveFigure;
    f32                 childDynamicsRandom;

    BlendType           childBlendType;
    MeshType            childMeshType;
    BillboardType       childBillboardType;
    ZBufATestType       childZBufATestType;
    TextureRes          childTex;
    DisplaySideType     childDisplaySide;

    nw::math::VEC3      childColor0;
    nw::math::VEC3      childColor1;
    f32                 childColorScale;

    ColorBlendType      primitiveColorBlend;
    ColorBlendType      primitiveAlphaBlend;

    AlphaCalcType       childAlphaCalcType[EFT_ALPHA_KIND_MAX];
    anim3v4Key          childAlpha3v4kAnim[EFT_ALPHA_KIND_MAX];
    f32                 childScaleInheritRate;
    nw::math::VEC2      childScale;
    f32                 childScaleRand;

    PtclRotType         childRotType;
    nw::math::VEC3      childInitRot;
    nw::math::VEC3      childInitRotRand;
    nw::math::VEC3      childRotVel;
    nw::math::VEC3      childRotVelRand;
    f32                 childRotRegist;
    u8                  isRotDirRand[3];
    u8                  primitiveScaleY2Z;

    nw::math::VEC2      childRotBasis;

    nw::math::VEC3      childGravity;
    s32                 childScaleStartFrame;
    nw::math::VEC2      childScaleTarget;

    TextureEmitterData  textureData;

    CombinerType        childCombinerType;
    AlphaCombinerType   childAlphaCombinerType;
    f32                 childAirRegist;

    //------------------------------------------------------------------------------
    //  Shader
    //------------------------------------------------------------------------------
    u8                  childShaderType;
    u8                  childUserShaderSetting;
    u8                  childShaderUseSoftEdge;
    u8                  childShaderUseFresnelAlpha;
    u8                  childShaderUseNearAlpha;
    u8                  childShaderUseFarAlpha;
    u8                  childShaderApplyAlphaToRefract;
    u8                  childShaderUseDecal;
    u8                  shaderUseParamAnim;
    u8                  dummy[3];
    f32                 childShaderParam0;
    f32                 childShaderParam1;
    u32                 childShaderIndex;
    u32                 childUserShaderIndex1;
    u32                 childUserShaderIndex2;
    SoftProcessingMethod childSoftProcessingMethod;
    f32                 childSoftFadeDistance;
    f32                 childSoftAlphaOffset;
    f32                 childSoftVolumeParam;
    f32                 childFresnelAlphaMin;
    f32                 childFresnelAlphaMax;
    f32                 childNearAlphaMin;
    f32                 childNearAlphaMax;
    f32                 childFarAlphaMin;
    f32                 childFarAlphaMax;
    f32                 childDecalVolume;
    u8                  childUserShaderDefine1[16];
    u8                  childUserShaderDefine2[16];
    u32                 childUserShaderFlag;
    u32                 childUserShaderSwitchFlag;
    CustomShaderParam   childCustomShaderParam;
    u32                 childGenShederNumber;
    anim3v4Key          shaderParam3v4kAnim;
};
static_assert(sizeof(ChildData) == 0x308, "nw::eft::ChildData size mismatch");

struct FieldRandomData
{
    s32                 fieldRandomBlank;
    nw::math::VEC3      fieldRandomVelAdd;
};
static_assert(sizeof(FieldRandomData) == 0x10, "nw::eft::FieldRandomData size mismatch");

struct FieldMagnetData
{
    f32                 fieldMagnetPower;
    nw::math::VEC3      fieldMagnetPos;
    u32                 fieldMagnetFlg;
    u8                  isFollowEmitter;
    u8                  dummy[3];
};
static_assert(sizeof(FieldMagnetData) == 0x18, "nw::eft::FieldMagnetData size mismatch");

struct FieldSpinData
{
    u32                 fieldSpinRotate;
    s32                 fieldSpinAxis;
    f32                 fieldSpinOuter;
};
static_assert(sizeof(FieldSpinData) == 0xC, "nw::eft::FieldSpinData size mismatch");

struct FieldCollisionData
{
    u8                  fieldCollisionType;
    u8                  fieldCollisionIsWorld;
    u8                  fieldCollisionIsCommonCoord;
    u8                  dummy;
    f32                 fieldCollisionCoord;
    f32                 fieldCollisionCoef;
    s32                 fieldCollisionCnt;
    f32                 fieldCollisionRegist;
};
static_assert(sizeof(FieldCollisionData) == 0x14, "nw::eft::FieldCollisionData size mismatch");

struct FieldConvergenceData
{
    FieldConvergenceType    fieldConvergenceType;
    nw::math::VEC3          fieldConvergencePos;
    f32                     fieldConvergenceRatio;
};
static_assert(sizeof(FieldConvergenceData) == 0x14, "nw::eft::FieldConvergenceData size mismatch");

struct FieldPosAddData
{
    nw::math::VEC3      fieldPosAdd;
    u8                  isFieldPosAddGlobal;
    u8                  dummy[3];
};
static_assert(sizeof(FieldPosAddData) == 0x10, "nw::eft::FieldPosAddData size mismatch");

struct FieldCurlNoiseData
{
    u8                  isFieldCurlNoiseInterpolation;
    u8                  isFieldCurlNoiseBaseRandom;
    u8                  dummy[2];
    f32                 fieldCurlNoiseScale;
    nw::math::VEC3      fieldCurlNoiseInfluence;
    nw::math::VEC3      fieldCurlNoiseSpeed;
    f32                 fieldCurlNoiseBase;
};
static_assert(sizeof(FieldCurlNoiseData) == 0x24, "nw::eft::FieldCurlNoiseData size mismatch");

struct FluctuationData
{
    f32                 fluctuationScaleX;
    f32                 fluctuationFreqX;
    u32                 fluctuationPhaseRndX;
    f32                 fluctuationPhaseInitX;

    f32                 fluctuationScaleY;
    f32                 fluctuationFreqY;
    u32                 fluctuationPhaseRndY;
    f32                 fluctuationPhaseInitY;
};
static_assert(sizeof(FluctuationData) == 0x20, "nw::eft::FluctuationData size mismatch");

struct StripeData
{
    StripeType              stripeType;
    StripeOption            stripeOption;
    StripeConnectOption     stripeConnectOpt;
    StripeTexCoordOption    stripeTexCoordOpt;
    s32                     stripeNumHistory;
    s32                     stripeDivideNum;
    f32                     stripeStartAlpha;
    f32                     stripeEndAlpha;
    nw::math::VEC2          stripeUVScroll;
    s32                     stripeHistoryStep;
    f32                     stripeHistoryInterpolate;
    f32                     stripeDirInterpolate;
};
static_assert(sizeof(StripeData) == 0x34, "nw::eft::StripeData size mismatch");

} } // namespace nw::eft

#endif // EFT_DATA_H_
