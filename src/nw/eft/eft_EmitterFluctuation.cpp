#include <nw/math/math_Triangular.h>
#include <nw/eft/eft_Emitter.h>
#include <nw/eft/eft_Misc.h>

namespace nw { namespace eft {

f32* EmitterCalc::sFluctuationTbl         = NULL;
f32* EmitterCalc::sFluctuationSawToothTbl = NULL;
f32* EmitterCalc::sFluctuationRectTbl     =  NULL;

void EmitterCalc::InitializeFluctuationTable()
{
    sFluctuationTbl         = static_cast<f32*>(AllocFromStaticHeap(sizeof(f32) * EFT_FLUCTUATION_TABLE_NUM));
    sFluctuationSawToothTbl = static_cast<f32*>(AllocFromStaticHeap(sizeof(f32) * EFT_FLUCTUATION_TABLE_NUM));
    sFluctuationRectTbl     = static_cast<f32*>(AllocFromStaticHeap(sizeof(f32) * EFT_FLUCTUATION_TABLE_NUM));

    for (s32 i = 0; i < EFT_FLUCTUATION_TABLE_NUM; i++)
    {
        f32 sinVal = nw::math::CosRad((f32)i / (f32)EFT_FLUCTUATION_TABLE_NUM * 2.0f * 3.14159f) * 0.5f + 0.5f;
        sFluctuationTbl[i] = sinVal;
    }

    for (s32 i = 0; i < EFT_FLUCTUATION_TABLE_NUM; i++)
        sFluctuationSawToothTbl[i] = (f32)i / EFT_FLUCTUATION_TABLE_NUM;

    for (s32 i = 0; i < EFT_FLUCTUATION_TABLE_NUM; i++)
    {
        if (i < EFT_FLUCTUATION_TABLE_NUM / 2)
            sFluctuationRectTbl[i] = 1.0f;
        else
            sFluctuationRectTbl[i] = 0.0f;
    }
}

void EmitterCalc::FinalzieFluctuationTable()
{
    FreeFromStaticHeap(sFluctuationTbl);
    FreeFromStaticHeap(sFluctuationRectTbl);
    FreeFromStaticHeap(sFluctuationSawToothTbl);
}

void EmitterCalc::CalcFluctuation(EmitterInstance* __restrict e, PtclInstance* __restrict ptcl)
{
    const ComplexEmitterData* res  = reinterpret_cast<const ComplexEmitterData*>(e->res);
    const FluctuationData*    fres = reinterpret_cast<const FluctuationData*>((u32)e->res + res->fluctuationDataOffset);

    f32 fluctuationX = 1.0f;
    s32 tblIdxX = ((s32)((ptcl->cntS + fres->fluctuationPhaseInitX + (EFT_FLUCTUATION_TABLE_NUM / fres->fluctuationFreqX) * ptcl->random[0] * fres->fluctuationPhaseRndX) * fres->fluctuationFreqX)) & EFT_FLUCTUATION_TABLE_MASK;

    if (res->fluctuationFlg & EFT_FLUCTUATION_FALG_USE_SIN_WAVE)
        fluctuationX = 1.0f - sFluctuationTbl[tblIdxX] * fres->fluctuationScaleX;

    if (res->fluctuationFlg & EFT_FLUCTUATION_FALG_USE_SAW_TOOTH_WAVE)
        fluctuationX = 1.0f - sFluctuationSawToothTbl[tblIdxX] * fres->fluctuationScaleX;

    if (res->fluctuationFlg & EFT_FLUCTUATION_FALG_USE_RECT_WAVE)
        fluctuationX = 1.0f - sFluctuationRectTbl[tblIdxX] * fres->fluctuationScaleX;

    if (res->fluctuationFlg & EFT_FLUCTUATION_FALG_APPLY_ALPHA) ptcl->fluctuationAlpha  = fluctuationX;
    if (res->fluctuationFlg & EFT_FLUCTUATION_FALG_APPLY_SCALE) ptcl->fluctuationScaleX = fluctuationX;

    if (res->fluctuationFlg & EFT_FLUCTUATION_FALG_APPLY_SCALEY)
    {
        f32 fluctuationY = 1.0f;
        s32 tblIdxY = ((s32)((ptcl->cntS + fres->fluctuationPhaseInitY + (EFT_FLUCTUATION_TABLE_NUM / fres->fluctuationFreqY) * ptcl->random[0] * fres->fluctuationPhaseRndY) * fres->fluctuationFreqY)) & EFT_FLUCTUATION_TABLE_MASK;

        if (res->fluctuationFlg & EFT_FLUCTUATION_FALG_USE_SIN_WAVE)
            fluctuationY = 1.0f - sFluctuationTbl[tblIdxY] * fres->fluctuationScaleY;

        if (res->fluctuationFlg & EFT_FLUCTUATION_FALG_USE_SAW_TOOTH_WAVE)
            fluctuationY = 1.0f - sFluctuationSawToothTbl[tblIdxY] * fres->fluctuationScaleY;

        if (res->fluctuationFlg & EFT_FLUCTUATION_FALG_USE_RECT_WAVE)
            fluctuationY = 1.0f - sFluctuationRectTbl[tblIdxY] * fres->fluctuationScaleY;

        if (res->fluctuationFlg & EFT_FLUCTUATION_FALG_APPLY_SCALE) ptcl->fluctuationScaleY = fluctuationY;
    }
    else
    {
        if (res->fluctuationFlg & EFT_FLUCTUATION_FALG_APPLY_SCALE) ptcl->fluctuationScaleY = fluctuationX;
    }
}

} } // namespace nw::eft
