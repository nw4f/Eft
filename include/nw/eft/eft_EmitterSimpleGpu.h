#ifndef EFT_EMITTER_SIMPLE_GPU_H_
#define EFT_EMITTER_SIMPLE_GPU_H_

#include <nw/eft/eft_EmitterSimple.h>

namespace nw { namespace eft {

class EmitterSimpleGpuCalc : public EmitterSimpleCalc
{
public:
    explicit EmitterSimpleGpuCalc(System* sys)
        : EmitterSimpleCalc(sys)
    {
    }

    virtual void CalcEmitter(EmitterInstance* emitter);
    virtual u32 CalcParticle(EmitterInstance* emitter, CpuCore core, bool skipBehavior, bool skipMakeAttribute);

    virtual PtclType GetPtclType() const
    {
        return EFT_PTCL_TYPE_SIMPLE;
    }
};
static_assert(sizeof(EmitterSimpleGpuCalc) == 4, "nw::eft::EmitterSimpleGpuCalc size mismatch");

} } // namespace nw::eft

#endif // EFT_EMITTER_SIMPLE_GPU_H_
