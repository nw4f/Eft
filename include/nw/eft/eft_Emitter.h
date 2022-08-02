#ifndef EFT_EMITTER_H_
#define EFT_EMITTER_H_

#include <nw/eft/eft_Animation.h>
#include <nw/eft/eft_Data.h>
#include <nw/eft/eft_Particle.h>
#include <nw/eft/eft_Random.h>
#include <nw/eft/eft_StreamOutBuffer.h>

namespace nw { namespace eft {

class  EmitterCalc;
class  EmitterController;
struct EmitterDynamicUniformBlock;
class  EmitterSet;
struct EmitterStaticUniformBlock;
class  ParticleShader;
class  Primitive;
struct PtclAttributeBuffer;
struct PtclAttributeBufferGpu;
struct StripeUniformBlock;
class  StripeVertexBuffer;

f32 _initialize3v4kAnim(AlphaAnim* anim, const anim3v4Key* key, s32 maxTime);

struct EmitterInstance
{
    u64                                                 gpuCounters[2];
    s32                                                 ptclNum;
    s32                                                 childPtclNum;
    s32                                                 groupID;
    s32                                                 emissionInterval;
    u32                                                 emitterBehaviorFlag;
    u32                                                 particleBehaviorFlag;
    u32                                                 childBehaviorFlag;
    f32                                                 emitDistVessel;
    f32                                                 cnt;
    f32                                                 fadeCnt;
    f32                                                 preCnt;
    f32                                                 emitCnt;
    f32                                                 preEmitCnt;
    f32                                                 emitSaving;
    f32                                                 filedCollisionY;
    u32                                                 primEmitCounter;
    f32                                                 fadeRequestFrame;
    f32                                                 emitterInitialRandom;
    f32                                                 fadeAlpha;
    f32                                                 frameRate;
    f32                                                 emitVessel;
    u32                                                 entryNum;
    u32                                                 gpuParticleBufferFillNum;
    u32                                                 gpuParticleBufferNum;
    u32                                                 childEntryNum;
    u32                                                 stripeVertexNum;
    u32                                                 emitterSetCreateID;
    EmitterSet*                                         emitterSet;
    EmitterController*                                  controller;
    nw::math::MTX34                                     emitterRT;
    nw::math::MTX34                                     emitterSRT;
    PtclRandom                                          rnd;
    nw::math::VEC3                                      emitDistPrevPos;
    nw::math::VEC3                                      scaleRnd;
    nw::math::VEC3                                      rotatRnd;
    nw::math::VEC3                                      transRnd;
    PtclFollowType                                      followType;
    EmitterInstance*                                    prev;
    EmitterInstance*                                    next;
    EmitterInstance*                                    nextStreamOutList;
    EmitterCalc*                                        calc;
    const SimpleEmitterData*                            res;
    PtclInstance*                                       ptclHead;
    PtclInstance*                                       childHead;
    PtclInstance*                                       ptclTail;
    PtclInstance*                                       childTail;
    ParticleShader*                                     shader[EFT_SHADER_TYPE_MAX];
    ParticleShader*                                     childShader[EFT_SHADER_TYPE_MAX];
    Primitive*                                          primitive;
    Primitive*                                          childPrimitive;
    Primitive*                                          emitVolumePrimitive;
    KeyFrameAnimArray*                                  animArrayEmitter;
    u32                                                 emitterAnimNum;
    KeyFrameAnim*                                       emitterAnimKey[EFT_ANIM_EMITTER_MAX];
    f32                                                 emitterAnimValue[EFT_ANIM_EMITTER_MAX];
    KeyFrameAnim*                                       particleAnimKey[EFT_ANIM_8KEY_MAX];
    nw::math::MTX34                                     animValEmitterRT;
    nw::math::MTX34                                     animValEmitterSRT;
    PtclAttributeBuffer*                                ptclAttributeBuffer;
    PtclAttributeBuffer*                                childPtclAttributeBuffer;
    PtclAttributeBufferGpu*                             ptclAttributeBufferGpu;
    StripeVertexBuffer*                                 stripeBuffer;
    EmitterStaticUniformBlock*                          emitterStaticUniformBlock;
    EmitterStaticUniformBlock*                          childEmitterStaticUniformBlock;
    EmitterDynamicUniformBlock*                         emitterDynamicUniformBlock;
    EmitterDynamicUniformBlock*                         childEmitterDynamicUniformBlock;
    StripeUniformBlock*                                 connectionStripeUniformBlock;
    StripeUniformBlock*                                 connectionStripeUniformBlockForCross;
    StreamOutAttributeBuffer                            positionStreamOutBuffer;
    StreamOutAttributeBuffer                            vectorStreamOutBuffer;
    mutable bool                                        streamOutFlip;
    mutable u32                                         streamOutCounter;
    void*                                               userCustomPtr;
    u8                                                  _3dc[4];

    void Init(const SimpleEmitterData* resource);
    void UpdateResInfo();
    void UpdateEmitterInfoByEmit();

    static void UpdateEmitterStaticUniformBlock(EmitterStaticUniformBlock* emitterStaticUniformBlock,
                                                const SimpleEmitterData*   emitterData,
                                                const ComplexEmitterData*  complexEmitterData);

    static void UpdateChildStaticUniformBlock(EmitterStaticUniformBlock* emitterStaticUniformBlock, const ChildData* childData);

    void TransformWorldVec(nw::math::VEC3* pDst, nw::math::VEC3* pSrc, PtclInstance* pPtcl);
    void TransformWorldVecNormal(nw::math::VEC3* pDst, nw::math::VEC3* pSrc, PtclInstance* pPtcl);
    void TransformLocalVec(nw::math::VEC3* pDst, nw::math::VEC3* pSrc, PtclInstance* pPtcl);
    void TransformLocalVecNormal(nw::math::VEC3* pDst, nw::math::VEC3* pSrc, PtclInstance* pPtcl);

    EmitterType GetEmitterType() const
    {
        return res->type;
    }

    BillboardType GetBillboardType() const
    {
        return res->billboardType;
    }

    f32 GetEmitCounter() const
    {
        return cnt;
    }

    void SetEmitCounter(f32 counter)
    {
        cnt = counter;
    }

    s32 GetEmitStartTime() const
    {
        return res->emitStartFrame;
    }

    s32 GetEmitEndTime() const
    {
        return res->emitEndFrame;
    }

    s32 GetEmitStep() const
    {
        return res->emitStep;
    }

    f32 GetEmitRate() const
    {
        return res->emitRate;
    }

    s32 GetParticleLife() const
    {
        return res->ptclLife;
    }

    s32 GetParticleLifeStep() const
    {
        return res->ptclLife;
    }

    const SimpleEmitterData* GetSimpleEmitterData() const
    {
        return res;
    }

    u32 GetDrawPathFlag() const
    {
        return res->drawPath;
    }

    const char* GetEmitterName() const
    {
        return res->name;
    }

    BlendType GetBlendType() const
    {
        return res->blendType;
    }

    EmitterSet* GetEmitterSet() const
    {
        return emitterSet;
    }

    const ComplexEmitterData* GetComplexEmitterData() const
    {
        if (res->type != EFT_EMITTER_TYPE_COMPLEX)
            return NULL;

        return static_cast<const ComplexEmitterData*>(res);
    }

    const ChildData* GetChildData() const
    {
        if (res->type == EFT_EMITTER_TYPE_SIMPLE)
            return NULL;

        const ComplexEmitterData* cres = static_cast<const ComplexEmitterData*>(res);

        if (cres->childFlg & EFT_CHILD_FLAG_ENABLE)
            return reinterpret_cast<const ChildData*>(cres + 1);
        else
            return NULL;
    }

    const StripeData* GetStripeData() const
    {
        if (res->type == EFT_EMITTER_TYPE_SIMPLE)
            return NULL;

        const ComplexEmitterData* cres = static_cast<const ComplexEmitterData*>(res);

        if (cres->billboardType == EFT_BILLBOARD_TYPE_STRIPE ||
            cres->billboardType == EFT_BILLBOARD_TYPE_COMPLEX_STRIPE)
        {
            return reinterpret_cast<const StripeData*>((u32)cres + cres->stripeDataOffset);
        }

        return NULL;
    }

    bool IsHasChildParticle() const
    {
        if (res->type == EFT_EMITTER_TYPE_SIMPLE)
            return false;

        const ComplexEmitterData* cres = static_cast<const ComplexEmitterData*>(res);

        if (cres->childFlg & EFT_CHILD_FLAG_ENABLE)
            return true;
        else
            return false;
    }

    bool IsRequestFrameBufferTexture() const;
    bool IsRequestDepthTexture() const;

    bool EmitChildParticle(PtclInstance* ptcl, CpuCore core);

    bool IsGpuAcceleration() const
    {
        if (shader[EFT_SHADER_TYPE_NORMAL])
            return shader[EFT_SHADER_TYPE_NORMAL]->IsGpuAcceleration();

        return false;
    }

    bool IsUseStreamOut() const
    {
        if (shader[EFT_SHADER_TYPE_NORMAL])
            return shader[EFT_SHADER_TYPE_NORMAL]->IsUseStreamOut();

        return false;
    }

    u32 GetUseTextureNum() const
    {
        if (shader[EFT_SHADER_TYPE_NORMAL])
            return shader[EFT_SHADER_TYPE_NORMAL]->GetTextureVariation() + 1;

        return 0;
    }

    bool IsEnableCustomActionFlag(nw::eft::CustomActionFlag bit) const
    {
        return (res->userData & bit) ? true : false;
    }

    u8 GetCustomActionU8Param0() const
    {
        return static_cast<u8>(res->userData);
    }

    u8 GetCustomActionU8Param1() const
    {
        return static_cast<u8>(res->userData >> 8);
    }

    u8 GetCustomActionU8Param2() const
    {
        return static_cast<u8>(res->userData2);
    }

    u8 GetCustomActionU8Param3() const
    {
        return static_cast<u8>(res->userData2 >> 8);
    }

    u8 GetCustomActionU8Param4() const
    {
        return static_cast<u8>(res->userData2 >> 16);
    }

    u8 GetCustomActionU8Param5() const
    {
        return static_cast<u8>(res->userData2 >> 24);
    }

    f32 GetCustomActionF32Param(nw::eft::CustomActionParamIdx idx) const
    {
        return res->userDataF[idx];
    }
};
static_assert(sizeof(EmitterInstance) == 0x3E0, "nw::eft::EmitterInstance size mismatch");

class System;

class EmitterCalc
{
public:
    explicit EmitterCalc(System* sys)
    {
        mSys = sys;
    }

    virtual ~EmitterCalc() // deleted
    {
    }

    virtual void CalcEmitter(EmitterInstance* emitter) = 0;

    void ApplyAnim(EmitterInstance* e);

    static void Initialize()
    {
        InitializeFluctuationTable();
    }

    static void Finalize()
    {
        mSys = NULL;
        FinalzieFluctuationTable();
    }

    static void EmitParticle(EmitterInstance* __restrict e)
    {
        mEmitFunctions[e->res->volumeType](e);
    }

    static void RemoveParticle(PtclInstance* ptcl, CpuCore core);

    virtual PtclType GetPtclType() const = 0;

    virtual u32 CalcParticle(EmitterInstance* e, CpuCore core, bool skipBehavior, bool skipMakeAttribute)
    {
        return 0;
    }

    virtual u32 CalcChildParticle(EmitterInstance* e, CpuCore core, bool skipBehavior, bool skipMakeAttribute)
    {
        return 0;
    }

    static inline void CalcParticleBehavior(EmitterInstance* __restrict emitter, PtclInstance* __restrict ptcl, f32 frameRate = 1.0f);
    static void CalcSimpleParticleBehavior (EmitterInstance* __restrict emitter, PtclInstance* __restrict ptcl, f32 frameRate = 1.0f);
    static void CalcComplexParticleBehavior(EmitterInstance* __restrict emitter, PtclInstance* __restrict ptcl, f32 frameRate = 1.0f);
    static void CalcChildParticleBehavior  (EmitterInstance* __restrict emitter, PtclInstance* __restrict ptcl, f32 frameRate = 1.0f);
    static void MakeParticleAttributeBuffer(PtclAttributeBuffer* __restrict attrBuffer, PtclInstance* __restrict ptcl, u32 shaderAttrFlag);

protected:
    virtual EmitterDynamicUniformBlock* MakeEmitterUniformBlock(EmitterInstance* emitter, CpuCore core, const ChildData* childDara = NULL, bool skipBehavior = false) = 0;

    static System* mSys;

    static void CalcEmitPoint(EmitterInstance* __restrict e);
    static void CalcEmitCircle(EmitterInstance* __restrict e);
    static void CalcEmitCircleSameDivide(EmitterInstance* __restrict e);
    static void CalcEmitFillCircle(EmitterInstance* __restrict e);
    static void CalcEmitSphere(EmitterInstance* __restrict e);
    static void CalcEmitSphereSameDivide(EmitterInstance* __restrict e);
    static void CalcEmitSphereSameDivide64(EmitterInstance* __restrict e);
    static void CalcEmitFillSphere(EmitterInstance* __restrict e);
    static void CalcEmitCylinder(EmitterInstance* __restrict e);
    static void CalcEmitFillCylinder(EmitterInstance* __restrict e);
    static void CalcEmitBox(EmitterInstance* __restrict e);
    static void CalcEmitFillBox(EmitterInstance* __restrict e);
    static void CalcEmitLine(EmitterInstance* __restrict e);
    static void CalcEmitLineSameDivide(EmitterInstance* __restrict e);
    static void CalcEmitRectangle(EmitterInstance* __restrict e);
    static void CalcEmitPrimitive(EmitterInstance* __restrict e);

    typedef void (*EmitFunction)(EmitterInstance* __restrict e);
    static EmitFunction mEmitFunctions[];

    static const void* _ptclField_Random     (EmitterInstance* __restrict e, PtclInstance* __restrict ptcl, const void* fieldData, f32 frameRate);
    static const void* _ptclField_Magnet     (EmitterInstance* __restrict e, PtclInstance* __restrict ptcl, const void* fieldData, f32 frameRate);
    static const void* _ptclField_Spin       (EmitterInstance* __restrict e, PtclInstance* __restrict ptcl, const void* fieldData, f32 frameRate);
    static const void* _ptclField_Collision  (EmitterInstance* __restrict e, PtclInstance* __restrict ptcl, const void* fieldData, f32 frameRate);
    static const void* _ptclField_Convergence(EmitterInstance* __restrict e, PtclInstance* __restrict ptcl, const void* fieldData, f32 frameRate);
    static const void* _ptclField_PosAdd     (EmitterInstance* __restrict e, PtclInstance* __restrict ptcl, const void* fieldData, f32 frameRate);
    static const void* _ptclField_CurlNoise  (EmitterInstance* __restrict e, PtclInstance* __restrict ptcl, const void* fieldData, f32 frameRate);

    static void _calcField(const ComplexEmitterData* __restrict res,  EmitterInstance* __restrict e, PtclInstance* __restrict ptcl, f32 frameRate);

    static f32* sFluctuationTbl;
    static f32* sFluctuationSawToothTbl;
    static f32* sFluctuationRectTbl;

    static void InitializeFluctuationTable();
    static void FinalzieFluctuationTable();
    static void CalcFluctuation(EmitterInstance* __restrict e, PtclInstance* __restrict ptcl);

    typedef void (*ParticleAnimFunction)(EmitterInstance* __restrict e, PtclInstance* __restrict ptcl, f32 frameRate);

    static void CalcPtclAnimation(EmitterInstance* __restrict emitter, PtclInstance* __restrict ptcl, f32 frameRate)
    {
    }

    static void ptclAnim_Scale_8key(EmitterInstance* __restrict e, PtclInstance* __restrict ptcl, f32 frameRate);
    static void ptclAnim_Scale_4k3v(EmitterInstance* __restrict e, PtclInstance* __restrict ptcl, f32 frameRate);

    static ParticleAnimFunction mAnimFunctionsSclae[];

    static void ptclAnim_Color0_8key(EmitterInstance* __restrict e, PtclInstance* __restrict ptcl, f32 frameRate);
    static void ptclAnim_Color1_8key(EmitterInstance* __restrict e, PtclInstance* __restrict ptcl, f32 frameRate);
    static void ptclAnim_Color0_4k3v(EmitterInstance* __restrict e, PtclInstance* __restrict ptcl, f32 frameRate);
    static void ptclAnim_Color1_4k3v(EmitterInstance* __restrict e, PtclInstance* __restrict ptcl, f32 frameRate);

    static ParticleAnimFunction mAnimFunctionsColor0[];
    static ParticleAnimFunction mAnimFunctionsColor1[];

    static void ptclAnim_Alpha0_8key(EmitterInstance* __restrict e, PtclInstance* __restrict ptcl, f32 frameRate);
    static void ptclAnim_Alpha0_4k3v(EmitterInstance* __restrict e, PtclInstance* __restrict ptcl, f32 frameRate);
    static void ptclAnim_Alpha1_8key(EmitterInstance* __restrict e, PtclInstance* __restrict ptcl, f32 frameRate);
    static void ptclAnim_Alpha1_4k3v(EmitterInstance* __restrict e, PtclInstance* __restrict ptcl, f32 frameRate);

    static ParticleAnimFunction mAnimFunctionsAlpha0[];
    static ParticleAnimFunction mAnimFunctionsAlpha1[];

    static void AddPtclToList(EmitterInstance* e, PtclInstance* ptcl);
    static void EmitCommon(EmitterInstance* __restrict e, PtclInstance* __restrict ptcl);
};
static_assert(sizeof(EmitterCalc) == 4, "nw::eft::EmitterCalc size mismatch");

inline void EmitterCalc::_calcField(const ComplexEmitterData* __restrict res,  EmitterInstance* __restrict e, PtclInstance* __restrict ptcl, f32 frameRate)
{
    const void* __restrict fres = reinterpret_cast<const void*>((u32)res + (u32)res->fieldDataOffset);

    if (res->fieldFlg & EFT_FIELD_MASK_RANDOM     ) fres = _ptclField_Random     (e, ptcl, fres, frameRate);
    if (res->fieldFlg & EFT_FIELD_MASK_MAGNET     ) fres = _ptclField_Magnet     (e, ptcl, fres, frameRate);
    if (res->fieldFlg & EFT_FIELD_MASK_SPIN       ) fres = _ptclField_Spin       (e, ptcl, fres, frameRate);
    if (res->fieldFlg & EFT_FIELD_MASK_COLLISION  ) fres = _ptclField_Collision  (e, ptcl, fres, frameRate);
    if (res->fieldFlg & EFT_FIELD_MASK_CONVERGENCE) fres = _ptclField_Convergence(e, ptcl, fres, frameRate);
    if (res->fieldFlg & EFT_FIELD_MASK_POSADD     ) fres = _ptclField_PosAdd     (e, ptcl, fres, frameRate);
    if (res->fieldFlg & EFT_FIELD_MASK_CURL_NOISE ) fres = _ptclField_CurlNoise  (e, ptcl, fres, frameRate);
}

inline void EmitterInstance::TransformLocalVec(nw::math::VEC3* pDst, nw::math::VEC3* pSrc, PtclInstance* pPtcl)
{
    nw::math::MTX34 invMat;
    if (followType == EFT_FOLLOW_TYPE_ALL)
        MTX34Inverse(&invMat, &emitterSRT);
    else
        MTX34Inverse(&invMat, &pPtcl->emitterSRT);

    VEC3Transform(pDst, &invMat, pSrc);
}

f32 _calcParticleAnim8Key(EmitterInstance* __restrict e, u32 id, f32 time, u32 index);
f32 _calcParticleAnimTime(EmitterInstance* __restrict e, PtclInstance* __restrict ptcl, u32 id);
f32 _calcParticleAnimTime(KeyFrameAnim* anim, PtclInstance* __restrict ptcl, u32 id);

inline bool _isExistParticleAnim8Key(EmitterInstance* __restrict e, u32 id)
{
    return e->particleAnimKey[EFT_ANIM_8KEY_OFFSET(id)] != NULL;
}

inline bool _isExistKeyFrameAnim(EmitterInstance* __restrict e, AnimKeyFrameApplyType animType)
{
    return _isExistKeyFrameAnim(e->animArrayEmitter, animType);
}

} } // namespace nw::eft

#endif // EFT_EMITTER_H_
