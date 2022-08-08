#include <nw/math/math_Triangular.h>
#include <nw/eft/eft_Emitter.h>

namespace nw { namespace eft {

static const f32 EPSILON = 0.0001f;

const void* EmitterCalc::_ptclField_Random(EmitterInstance* __restrict e, PtclInstance* __restrict ptcl, const void* fieldData)
{
    const FieldRandomData* dat = static_cast<const FieldRandomData*>(fieldData);

    if ((s32)ptcl->cnt % dat->fieldRandomBlank == 0)
    {
        const nw::math::VEC3& rndVec3 = e->rnd.GetVec3();
        ptcl->vel.x += rndVec3.x * dat->fieldRandomVelAdd.x;
        ptcl->vel.y += rndVec3.y * dat->fieldRandomVelAdd.y;
        ptcl->vel.z += rndVec3.z * dat->fieldRandomVelAdd.z;
    }

    return dat + 1;
}

const void* EmitterCalc::_ptclField_Magnet(EmitterInstance* __restrict e, PtclInstance* __restrict ptcl, const void* fieldData)
{
    const FieldMagnetData* dat = static_cast<const FieldMagnetData*>(fieldData);

    if (dat->fieldMagnetFlg & EFT_MAGNET_FLAG_X) ptcl->vel.x += (dat->fieldMagnetPos.x - ptcl->pos.x - ptcl->vel.x) * dat->fieldMagnetPower;
    if (dat->fieldMagnetFlg & EFT_MAGNET_FLAG_Y) ptcl->vel.y += (dat->fieldMagnetPos.y - ptcl->pos.y - ptcl->vel.y) * dat->fieldMagnetPower;
    if (dat->fieldMagnetFlg & EFT_MAGNET_FLAG_Z) ptcl->vel.z += (dat->fieldMagnetPos.z - ptcl->pos.z - ptcl->vel.z) * dat->fieldMagnetPower;

    return dat + 1;
}

const void* EmitterCalc::_ptclField_Spin(EmitterInstance* __restrict e, PtclInstance* __restrict ptcl, const void* fieldData)
{
    const FieldSpinData* dat = static_cast<const FieldSpinData*>(fieldData);

    register float sinV, cosV;
    u32 idx = (u32)(dat->fieldSpinRotate * e->frameRate * ptcl->dynamicsRnd);
    nw::math::SinCosIdx(&sinV, &cosV, idx);

    switch (dat->fieldSpinAxis)
    {
    case 0:
        {
            f32 v0 = ptcl->pos.y *  cosV + ptcl->pos.z * sinV;
            f32 v1 = ptcl->pos.y * -sinV + ptcl->pos.z * cosV;
            ptcl->pos.y = v0;
            ptcl->pos.z = v1;

            if (dat->fieldSpinOuter == 0.0f)
                return dat + 1;

            f32 length2 = v0 * v0 + v1 * v1;
            if (length2 <= 0.0f)
                return dat + 1;

            f32 r = 1.0f / nw::math::FSqrt(length2) * dat->fieldSpinOuter * ptcl->dynamicsRnd * e->frameRate; // " * e->frameRate " <-- mistake?
            ptcl->pos.y += v0 * r * e->frameRate;
            ptcl->pos.z += v1 * r * e->frameRate;
            return dat + 1;
        }
    case 1:
        {
            f32 v0 = ptcl->pos.z *  cosV + ptcl->pos.x * sinV;
            f32 v1 = ptcl->pos.z * -sinV + ptcl->pos.x * cosV;
            ptcl->pos.z = v0;
            ptcl->pos.x = v1;

            if (dat->fieldSpinOuter == 0.0f)
                return dat + 1;

            f32 length2 = v0 * v0 + v1 * v1;
            if (length2 <= 0.0f)
                return dat + 1;

            f32 r = 1.0f / nw::math::FSqrt(length2) * dat->fieldSpinOuter * ptcl->dynamicsRnd;
            ptcl->pos.z += v0 * r * e->frameRate;
            ptcl->pos.x += v1 * r * e->frameRate;
            return dat + 1;
        }
    case 2:
        {
            f32 v0 = ptcl->pos.x *  cosV + ptcl->pos.y * sinV;
            f32 v1 = ptcl->pos.x * -sinV + ptcl->pos.y * cosV;
            ptcl->pos.x = v0;
            ptcl->pos.y = v1;

            if (dat->fieldSpinOuter == 0.0f)
                return dat + 1;

            f32 length2 = v0 * v0 + v1 * v1;
            if (length2 <= 0.0f)
                return dat + 1;

            f32 r = 1.0f / nw::math::FSqrt(length2) * dat->fieldSpinOuter * ptcl->dynamicsRnd;
            ptcl->pos.x += v0 * r * e->frameRate;
            ptcl->pos.y += v1 * r * e->frameRate;
            return dat + 1;
        }
    }

    return dat + 1;
}

const void* EmitterCalc::_ptclField_Collision(EmitterInstance* __restrict e, PtclInstance* __restrict ptcl, const void* fieldData)
{
    const FieldCollisionData* dat = static_cast<const FieldCollisionData*>(fieldData);

    f32 y = dat->fieldCollisionCoord;

    if (dat->fieldCollisionIsWorld)
    {
        register nw::math::VEC3 worldPos;

        nw::math::MTX34* matEmitter;

        if (e->followType == EFT_FOLLOW_TYPE_ALL)
            matEmitter = &e->emitterSRT;
        else
            matEmitter = &ptcl->emitterSRT;

        VEC3Transform(&worldPos, matEmitter, &ptcl->pos);

        switch (dat->fieldCollisionType)
        {
        case EFT_FIELD_COLLISION_REACTION_CESSER:
            if (worldPos.y < y)
            {
                worldPos.y = y;
                ptcl->cnt = (f32)ptcl->life - e->frameRate;
            }
            break;
        case EFT_FIELD_COLLISION_REACTION_REFLECTION:
            if (worldPos.y < y)
            {
                worldPos.y = y + EPSILON;

                register nw::math::VEC3 worldVel;
                {
                    f32 tx = ptcl->vel.x;
                    f32 ty = ptcl->vel.y;
                    f32 tz = ptcl->vel.z;
                    nw::math::MTX34& m = *matEmitter;
                    worldVel.x = m.m[0][0] * tx + m.m[0][1] * ty + m.m[0][2] * tz;
                    worldVel.y = m.m[1][0] * tx + m.m[1][1] * ty + m.m[1][2] * tz;
                    worldVel.z = m.m[2][0] * tx + m.m[2][1] * ty + m.m[2][2] * tz;
                }

                worldVel.y *= -dat->fieldCollisionCoef;

                nw::math::MTX34 matEmitterInv;
#if EFT_IS_CAFE && !EFT_IS_CAFE_WUT
                matEmitterInv.SetInverse(*matEmitter);
#else
                // Compiler complains about possibility of matEmitterInv being uninitialized
                if (!nw::math::MTX34Inverse(&matEmitterInv, *matEmitter))
                    matEmitterInv.SetIdentity();
#endif

                ptcl->pos.SetTransform(matEmitterInv, worldPos);
                {
                    f32 tx = worldVel.x;
                    f32 ty = worldVel.y;
                    f32 tz = worldVel.z;
                    nw::math::MTX34 &m = matEmitterInv;
                    ptcl->vel.x = m.m[0][0] * tx + m.m[0][1] * ty + m.m[0][2] * tz;
                    ptcl->vel.y = m.m[1][0] * tx + m.m[1][1] * ty + m.m[1][2] * tz;
                    ptcl->vel.z = m.m[2][0] * tx + m.m[2][1] * ty + m.m[2][2] * tz;
                }
            }
            break;
        }
    }
    else
    {
        switch (dat->fieldCollisionType)
        {
        case EFT_FIELD_COLLISION_REACTION_CESSER:
            if (ptcl->pos.y < y)
            {
                ptcl->pos.y = y;
                ptcl->cnt = (f32)ptcl->life - e->frameRate;
            }
            break;
        case EFT_FIELD_COLLISION_REACTION_REFLECTION:
            if (ptcl->pos.y < y)
            {
                ptcl->pos.y = y;
                ptcl->vel.y *= -dat->fieldCollisionCoef;
            }
            break;
        }
    }

    return dat + 1;
}

const void* EmitterCalc::_ptclField_Convergence(EmitterInstance* __restrict e, PtclInstance* __restrict ptcl, const void* fieldData)
{
    const FieldConvergenceData* dat = static_cast<const FieldConvergenceData*>(fieldData);

    ptcl->pos.x += (dat->fieldConvergencePos.x - ptcl->pos.x) * dat->fieldConvergenceRatio * ptcl->dynamicsRnd * e->frameRate;
    ptcl->pos.y += (dat->fieldConvergencePos.y - ptcl->pos.y) * dat->fieldConvergenceRatio * ptcl->dynamicsRnd * e->frameRate;
    ptcl->pos.z += (dat->fieldConvergencePos.z - ptcl->pos.x) * dat->fieldConvergenceRatio * ptcl->dynamicsRnd * e->frameRate;

    return dat + 1;
}

const void* EmitterCalc::_ptclField_PosAdd(EmitterInstance* __restrict e, PtclInstance* __restrict ptcl, const void* fieldData)
{
    const FieldPosAddData* dat = static_cast<const FieldPosAddData*>(fieldData);

    ptcl->pos.x += dat->fieldPosAdd.x * ptcl->dynamicsRnd * e->frameRate;
    ptcl->pos.y += dat->fieldPosAdd.y * ptcl->dynamicsRnd * e->frameRate;
    ptcl->pos.z += dat->fieldPosAdd.z * ptcl->dynamicsRnd * e->frameRate;

    return dat + 1;
}

} } // namespace nw::eft
