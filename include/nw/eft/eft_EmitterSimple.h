#ifndef EFT_EMITTER_SIMPLE_H_
#define EFT_EMITTER_SIMPLE_H_

#include <nw/eft/eft_Callback.h>
#include <nw/eft/eft_Emitter.h>

namespace nw { namespace eft {

class EmitterSimpleCalc : public EmitterCalc
{
public:
    explicit EmitterSimpleCalc(System* sys)
        : EmitterCalc(sys)
    {
    }

    virtual void CalcEmitter(EmitterInstance* emitter);
    virtual u32 CalcParticle(EmitterInstance* emitter, CpuCore core, bool skipBehavior, bool skipMakeAttribute);

    virtual PtclType GetPtclType() const
    {
        return EFT_PTCL_TYPE_SIMPLE;
    }

protected:
    virtual EmitterDynamicUniformBlock* MakeEmitterUniformBlock(EmitterInstance* emitter, CpuCore core, const ChildData* childDara = NULL, bool skipBehavior = false);

    static inline bool CalcParticleKillCondition(PtclInstance* ptcl, CpuCore core)
    {
        ptcl->cntS = static_cast<s32>(ptcl->cnt);

        if (ptcl->cntS >= ptcl->life || (ptcl->life == 1 && ptcl->cnt > 1.f))
        {
            EmitterCalc::RemoveParticle(ptcl, core);
            return true;
        }

        return false;
    }

    static inline void CalcParticleMakeAttr(EmitterInstance* emitter, PtclInstance* ptcl, PtclAttributeBuffer* attr, u32 shaderAttrFlag)
    {
        ptcl->attributeBuffer = attr;
        EmitterCalc::MakeParticleAttributeBuffer(ptcl->attributeBuffer, ptcl, shaderAttrFlag);
        emitter->entryNum++;
    }

    static inline void CalcParticleCalcCallback(CustomActionParticleCalcCallback callback, EmitterInstance* emitter, PtclInstance* ptcl, CpuCore core, bool skipBehavior)
    {
        ParticleCalcArg arg;
        arg.emitter = emitter;
        arg.ptcl = ptcl;
        arg.core = core;
        arg.skipBehavior = skipBehavior;
        callback(arg);
    }

    static inline void CalcParticleMakeAttrCallback(CustomActionParticleMakeAttributeCallback callback, EmitterInstance* emitter, PtclInstance* ptcl, CpuCore core, bool skipBehavior)
    {
        ParticleMakeAttrArg arg;
        arg.emitter = emitter;
        arg.ptcl = ptcl;
        arg.core = core;
        arg.skipBehavior = skipBehavior;
        callback(arg);
    }

    void EmitSameDistance(const SimpleEmitterData* __restrict res, EmitterInstance* e);
};
static_assert(sizeof(EmitterSimpleCalc) == 4, "nw::eft::EmitterSimpleCalc size mismatch");

} } // namespace nw::eft

#endif // EFT_EMITTER_SIMPLE_H_
