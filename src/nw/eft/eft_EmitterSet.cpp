#include <nw/eft/eft_Emitter.h>
#include <nw/eft/eft_EmitterSet.h>
#include <nw/eft/eft_System.h>

namespace nw { namespace eft {

void EmitterController::SetEmissionRatio(s32 ratio)
{
    mEmissionRatio = static_cast<f32>(ratio) / EFT_EMT_CTRLR_SCALE_ONE;

    if (mEmitter->res->flg & EFT_EMITTER_FLAG_USE_BEHAVIOR_CALC_GPU)
    {
        if (mEmissionRatio > 1.0f)
            mEmissionRatio = 1.0f;
    }

    if (mEmitter->cnt != 0.0f)
        mEmissionRatioChanged = true;
}

void EmitterController::SetEmissionRatio(f32 ratio)
{
    mEmissionRatio = ratio;

    if (mEmitter->res->flg & EFT_EMITTER_FLAG_USE_BEHAVIOR_CALC_GPU)
    {
        if (mEmissionRatio > 1.0f)
            mEmissionRatio = 1.0f;
    }

    if (mEmitter->cnt != 0.0f)
        mEmissionRatioChanged = true;
}

void EmitterController::SetEmitterColor0(const nw::ut::Color4f& color)
{
    mEmitter->emitterAnimValue[EFT_ANIM_COLOR0_R] = color.r;
    mEmitter->emitterAnimValue[EFT_ANIM_COLOR0_G] = color.g;
    mEmitter->emitterAnimValue[EFT_ANIM_COLOR0_B] = color.b;
}

void EmitterController::SetEmitterColor1(const nw::ut::Color4f& color)
{
    mEmitter->emitterAnimValue[EFT_ANIM_COLOR1_R] = color.r;
    mEmitter->emitterAnimValue[EFT_ANIM_COLOR1_G] = color.g;
    mEmitter->emitterAnimValue[EFT_ANIM_COLOR1_B] = color.b;
}

const EmitterInstance* EmitterSet::GetAliveEmitter(u32 idx)
{
    u32 cnt = 0;

    for (s32 i = 0; i < mNumEmitterFirst; i++)
        if (mInstance[i]->emitterSetCreateID == GetCreateID() && mInstance[i]->calc)
        {
            if (cnt == idx)
                return mInstance[i];

            cnt++;
        }

    return NULL;
}

void EmitterSet::SetMtx(const nw::math::MTX34& mtx)
{
    mSRT = mtx;

    nw::math::VEC3 basisX(mtx.m[0][0], mtx.m[1][0], mtx.m[2][0]);
    nw::math::VEC3 basisY(mtx.m[0][1], mtx.m[1][1], mtx.m[2][1]);
    nw::math::VEC3 basisZ(mtx.m[0][2], mtx.m[1][2], mtx.m[2][2]);

    mAutoCalcScale.x = basisX.Length();
    mAutoCalcScale.y = basisY.Length();
    mAutoCalcScale.z = basisZ.Length();

    if (mAutoCalcScale.x > 0.0f)
    {
        f32 inv = 1.0f / mAutoCalcScale.x;
        mRT.m[0][0] = mtx.m[0][0] * inv;
        mRT.m[1][0] = mtx.m[1][0] * inv;
        mRT.m[2][0] = mtx.m[2][0] * inv;
    }
    else
    {
        mRT.m[0][0] = mRT.m[1][0] = mRT.m[2][0] = 0.0f;
    }

    if (mAutoCalcScale.y > 0.0f)
    {
        f32 inv = 1.0f / mAutoCalcScale.y;
        mRT.m[0][1] = mtx.m[0][1] * inv;
        mRT.m[1][1] = mtx.m[1][1] * inv;
        mRT.m[2][1] = mtx.m[2][1] * inv;
    }
    else
    {
        mRT.m[0][1] = mRT.m[1][1] = mRT.m[2][1] = 0.0f;
    }

    if (mAutoCalcScale.z > 0.0f)
    {
        f32 inv = 1.0f / mAutoCalcScale.z;
        mRT.m[0][2] = mtx.m[0][2] * inv;
        mRT.m[1][2] = mtx.m[1][2] * inv;
        mRT.m[2][2] = mtx.m[2][2] * inv;
    }
    else
    {
        mRT.m[0][2] = mRT.m[1][2] = mRT.m[2][2] = 0.0f;
    }

    mRT.m[0][3] = mtx.m[0][3];
    mRT.m[1][3] = mtx.m[1][3];
    mRT.m[2][3] = mtx.m[2][3];

    updateParticleScale_();
}

void EmitterSet::Kill()
{
    mSystem->KillEmitterSet(this);
}

void EmitterSet::Fade()
{
    mFadeRequest = true;

    for (s32 i = 0; i < mNumEmitterFirst; i++)
        if (mInstance[i] && mInstance[i]->calc)
            mInstance[i]->fadeRequestFrame = mInstance[i]->cnt;
}

void EmitterSet::ForceCalc(s32 numLoop)
{
    for (s32 loop = 0; loop < numLoop; loop++)
    {
        for (s32 i = 0; i < mNumEmitterFirst; i++)
        {
            EmitterInstance* e = mInstance[i];

            if (e->emitterSetCreateID == GetCreateID() && e->calc)
            {
                e->calc->CalcEmitter(e);
                if (!e->calc)
                    continue;

                e->calc->CalcParticle(e, EFT_CPU_CORE_1, false, true);

                if (e->res->type == EFT_EMITTER_TYPE_COMPLEX)
                {
                    const ComplexEmitterData* res  = static_cast<const ComplexEmitterData*>(e->res);

                    if (res->childFlg & EFT_CHILD_FLAG_ENABLE)
                    {
                        mSystem->EmitChildParticle();
                        e->calc->CalcChildParticle(e, EFT_CPU_CORE_1, false, true);
                    }
                }
            }
        }

        mSystem->RemovePtcl();
    }
}

void EmitterSet::SetStopCalc(bool isStopCalc)
{
    mIsStopCalc = isStopCalc;
}

void EmitterSet::SetStopDraw(bool isStopDraw)
{
    mIsStopDraw = isStopDraw;
}

} } // namespace nw::eft
