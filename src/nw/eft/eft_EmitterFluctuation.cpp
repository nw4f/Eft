#include <nw/math/math_Triangular.h>
#include <nw/eft/eft_Emitter.h>
#include <nw/eft/eft_Heap.h>

namespace nw { namespace eft {

f32* EmitterCalc::sFluctuationTbl = NULL;

void EmitterCalc::InitializeFluctuation_(Heap* heap)
{
    sFluctuationTbl = static_cast<f32*>(heap->Alloc(sizeof(f32) * EFT_FLUCTUATION_TABLE_NUM));

    for (s32 i = 0; i < EFT_FLUCTUATION_TABLE_NUM; i++)
        sFluctuationTbl[i] = nw::math::CosRad((f32)i / (f32)EFT_FLUCTUATION_TABLE_NUM * 2.0f * 3.14159f) * 0.5f + 0.5f;
}

void EmitterCalc::CalcFluctuation(EmitterInstance* __restrict e, PtclInstance* __restrict ptcl)
{
    const ComplexEmitterData* res  = static_cast<const ComplexEmitterData*>(e->res);
    const FluctuationData*    fres = reinterpret_cast<const FluctuationData*>((u32)res + res->fluctuationDataOffset);

    s32 tblIx = ((s32)((s32)ptcl->cnt * fres->fluctuationFreq) + ptcl->rnd * fres->fluctuationPhaseRnd) & EFT_FLUCTUATION_TABLE_MASK;
    f32 fluctuation = 1.0f - sFluctuationTbl[tblIx] * fres->fluctuationScale;

    if (res->fluctuationFlg & EFT_FLUCTUATION_FALG_APPLY_ALPHA) ptcl->fluctuationAlpha = fluctuation;
    if (res->fluctuationFlg & EFT_FLUCTUATION_FALG_APPLY_SCLAE) ptcl->fluctuationScale = fluctuation;
}

} } // namespace nw::eft
