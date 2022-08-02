#ifndef EFT_EMITTER_SIMPLE_GPU_H_
#define EFT_EMITTER_SIMPLE_GPU_H_

#include <nw/eft/eft_EmitterSimple.h>

namespace nw { namespace eft {

class EmitterSimpleGpuCalc : public EmitterSimpleCalc
{
public:
    EmitterSimpleGpuCalc(System* system)
        : EmitterSimpleCalc(system)
    {
    }

    virtual void CalcEmitter(EmitterInstance* emitter);
    virtual u32 CalcParticle(EmitterInstance* emitter, CpuCore core, bool noCalcBehavior, bool noMakePtclAttributeBuffer);

    virtual PtclType GetPtclType() const { return PtclType_Simple; }
};
static_assert(sizeof(EmitterSimpleGpuCalc) == 4, "EmitterSimpleGpuCalc size mismatch");

} } // namespace nw::eft

#endif // EFT_EMITTER_SIMPLE_GPU_H_
