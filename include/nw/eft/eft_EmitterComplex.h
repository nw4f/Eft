#ifndef EFT_EMITTER_COMPLEX_H_
#define EFT_EMITTER_COMPLEX_H_

#include <nw/eft/eft_EmitterSimple.h>

namespace nw { namespace eft {

class EmitterComplexCalc : public EmitterSimpleCalc
{
public:
    EmitterComplexCalc(System* system)
        : EmitterSimpleCalc(system)
    {
    }

    virtual void CalcEmitter(EmitterInstance* emitter);

    virtual u32 CalcParticle(EmitterInstance* emitter, CpuCore core, bool noCalcBehavior, bool noMakePtclAttributeBuffer);
    virtual u32 CalcChildParticle(EmitterInstance* emitter, CpuCore core, bool noCalcBehavior, bool noMakePtclAttributeBuffer);

    static void EmitChildParticle(EmitterInstance* emitter, PtclInstance* ptcl);

    virtual PtclType GetPtclType() const { return PtclType_Complex; }

    static inline void AddChildPtclToList(EmitterInstance* emitter, PtclInstance* childPtcl)
    {
        if (emitter->childParticleHead == NULL)
        {
            emitter->childParticleHead = childPtcl;
            childPtcl->next = NULL;
            childPtcl->prev = NULL;
        }
        else
        {
            emitter->childParticleHead->prev = childPtcl;
            childPtcl->next = emitter->childParticleHead;
            emitter->childParticleHead = childPtcl;
            childPtcl->prev = NULL;
        }

        if (emitter->childParticleTail == NULL)
            emitter->childParticleTail = childPtcl;

        emitter->numChildParticles++;
    }

    static void CalcStripe(EmitterInstance* emitter, PtclInstance* ptcl, const StripeData* stripeData, const ComplexEmitterData* data, CpuCore core, bool noMakePtclAttributeBuffer);
    static void EmitChildParticle(EmitterInstance* emitter, PtclInstance* ptcl, CpuCore core, const ChildData* childData);
};
static_assert(sizeof(EmitterComplexCalc) == 4, "EmitterComplexCalc size mismatch");

} } // namespace nw::eft

#endif // EFT_EMITTER_COMPLEX_H_
