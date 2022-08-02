#include <nw/math/math_Triangular.h>
#include <nw/eft/eft_Animation.h>
#include <nw/eft/eft_Emitter.h>
#include <nw/eft/eft_EmitterSet.h>
#include <nw/eft/eft_System.h>

namespace nw { namespace eft {

static const f32 EPSILON = 0.0001f;

const void* EmitterCalc::_ptclField_Random(EmitterInstance* __restrict e, PtclInstance* __restrict ptcl, const void* fieldData, f32 frameRate)
{
    const FieldRandomData* dat = static_cast<const FieldRandomData*>(fieldData);
    nw::math::VEC3 fieldRandomVelAdd;

    if (ptcl->cntS % dat->fieldRandomBlank == 0)
    {
        const nw::math::VEC3& rndVec3 = e->rnd.GetVec3();

        if (_isExistParticleAnim8Key(e, EFT_ANIM_8KEY_FIELD_RND_X))
        {
            KeyFrameAnim* animX = e->particleAnimKey[EFT_ANIM_8KEY_OFFSET(EFT_ANIM_8KEY_FIELD_RND_X)];
            KeyFrameAnim* animY = e->particleAnimKey[EFT_ANIM_8KEY_OFFSET(EFT_ANIM_8KEY_FIELD_RND_Y)];
            KeyFrameAnim* animZ = e->particleAnimKey[EFT_ANIM_8KEY_OFFSET(EFT_ANIM_8KEY_FIELD_RND_Z)];

            f32 time = _calcParticleAnimTime(e, ptcl, EFT_ANIM_8KEY_FIELD_RND_X);
            u32 index = CalcAnimKeyFrameIndex(animX, time);

            fieldRandomVelAdd.x = CalcAnimKeyFrameSimple(animX, time, index);
            fieldRandomVelAdd.y = CalcAnimKeyFrameSimple(animY, time, index);
            fieldRandomVelAdd.z = CalcAnimKeyFrameSimple(animZ, time, index);
        }
        else
        {
            fieldRandomVelAdd.x = dat->fieldRandomVelAdd.x;
            fieldRandomVelAdd.y = dat->fieldRandomVelAdd.y;
            fieldRandomVelAdd.z = dat->fieldRandomVelAdd.z;
        }

        ptcl->vel.x += rndVec3.x * fieldRandomVelAdd.x;
        ptcl->vel.y += rndVec3.y * fieldRandomVelAdd.y;
        ptcl->vel.z += rndVec3.z * fieldRandomVelAdd.z;
    }

    return dat + 1;
}

const void* EmitterCalc::_ptclField_Magnet(EmitterInstance* __restrict e, PtclInstance* __restrict ptcl, const void* fieldData, f32 frameRate)
{
    const FieldMagnetData* dat = static_cast<const FieldMagnetData*>(fieldData);
    f32 fieldMagnetPower;

    if (_isExistParticleAnim8Key(e, EFT_ANIM_8KEY_FIELD_MAGNET))
    {
        KeyFrameAnim* anim = e->particleAnimKey[EFT_ANIM_8KEY_OFFSET(EFT_ANIM_8KEY_FIELD_MAGNET)];

        f32 time = _calcParticleAnimTime(anim, ptcl, EFT_ANIM_8KEY_FIELD_MAGNET);
        u32 index = CalcAnimKeyFrameIndex(anim, time);

        fieldMagnetPower = CalcAnimKeyFrameSimple(anim, time, index);
    }
    else
    {
        fieldMagnetPower = dat->fieldMagnetPower;
    }

    if (dat->isFollowEmitter)
    {
        nw::math::VEC3 local;
        nw::math::VEC3 src;
        src.x = e->emitterRT._03;
        src.y = e->emitterRT._13;
        src.z = e->emitterRT._23;
        e->TransformLocalVec(&local, &src, ptcl);

        if (dat->fieldMagnetFlg & EFT_MAGNET_FLAG_X) ptcl->vel.x += ( local.x + dat->fieldMagnetPos.x - ptcl->pos.x - ptcl->vel.x) * fieldMagnetPower;
        if (dat->fieldMagnetFlg & EFT_MAGNET_FLAG_Y) ptcl->vel.y += ( local.y + dat->fieldMagnetPos.y - ptcl->pos.y - ptcl->vel.y) * fieldMagnetPower;
        if (dat->fieldMagnetFlg & EFT_MAGNET_FLAG_Z) ptcl->vel.z += ( local.z + dat->fieldMagnetPos.z - ptcl->pos.z - ptcl->vel.z) * fieldMagnetPower;
    }
    else
    {
        if (dat->fieldMagnetFlg & EFT_MAGNET_FLAG_X) ptcl->vel.x += ( dat->fieldMagnetPos.x - ptcl->pos.x - ptcl->vel.x) * fieldMagnetPower;
        if (dat->fieldMagnetFlg & EFT_MAGNET_FLAG_Y) ptcl->vel.y += ( dat->fieldMagnetPos.y - ptcl->pos.y - ptcl->vel.y) * fieldMagnetPower;
        if (dat->fieldMagnetFlg & EFT_MAGNET_FLAG_Z) ptcl->vel.z += ( dat->fieldMagnetPos.z - ptcl->pos.z - ptcl->vel.z) * fieldMagnetPower;
    }

    return dat + 1;
}

const void* EmitterCalc::_ptclField_Spin(EmitterInstance* __restrict e, PtclInstance* __restrict ptcl, const void* fieldData, f32 frameRate)
{
    const FieldSpinData* dat = static_cast<const FieldSpinData*>(fieldData);

    register float sinV, cosV;

    u32 fieldSpinRotate;
    f32 fieldSpinOuter;

    if (_isExistParticleAnim8Key(e, EFT_ANIM_8KEY_FIELD_SPIN_ROT))
    {
        KeyFrameAnim* anim = e->particleAnimKey[EFT_ANIM_8KEY_OFFSET(EFT_ANIM_8KEY_FIELD_SPIN_ROT)];

        f32 time = _calcParticleAnimTime(anim, ptcl, EFT_ANIM_8KEY_FIELD_SPIN_ROT);
        u32 index = CalcAnimKeyFrameIndex(anim, time);

        fieldSpinRotate = CalcAnimKeyFrameSimpleS32(anim, time, index);
    }
    else
    {
        fieldSpinRotate = dat->fieldSpinRotate;
    }

    if (_isExistParticleAnim8Key(e, EFT_ANIM_8KEY_FIELD_SPIN_OUTER))
    {
        KeyFrameAnim* anim = e->particleAnimKey[EFT_ANIM_8KEY_OFFSET(EFT_ANIM_8KEY_FIELD_SPIN_OUTER)];

        f32 time = _calcParticleAnimTime(anim, ptcl, EFT_ANIM_8KEY_FIELD_SPIN_OUTER);
        u32 index = CalcAnimKeyFrameIndex(anim, time);

        fieldSpinOuter = CalcAnimKeyFrameSimple(anim, time, index);
    }
    else
    {
        fieldSpinOuter = dat->fieldSpinOuter;
    }

    {
        const u64 INDEX_MAX = 0x100000000;
        f32 degree = (fieldSpinRotate / (f32)INDEX_MAX) * 360.0f;
        if (degree > 180.0f)
            degree -= 360.0f;

        degree *= ptcl->dynamicsRnd * frameRate;
        if (degree < 0)
            degree += 360.0f;

        u32 idx = (u32)((degree / 360.0f) * INDEX_MAX);
        nw::math::SinCosIdx(&sinV, &cosV, idx);
    }

    switch (dat->fieldSpinAxis)
    {
    case 0:
    {
        f32 v0 = ptcl->pos.y *  cosV  + ptcl->pos.z * sinV;
        f32 v1 = ptcl->pos.y * -sinV  + ptcl->pos.z * cosV;
        ptcl->pos.y = v0;
        ptcl->pos.z = v1;

        if (fieldSpinOuter == 0.0f)
            break;

        f32 length2 = v0 * v0 + v1 * v1;
        if (length2 <= 0.0f)
            break;

        f32 r = 1.0f / nw::math::FSqrt(length2) * fieldSpinOuter * ptcl->dynamicsRnd * frameRate;
        ptcl->pos.y += v0 * r;
        ptcl->pos.z += v1 * r;
    }
    break;

    case 1:
    {
        f32 v0 = ptcl->pos.z *  cosV  + ptcl->pos.x * sinV;
        f32 v1 = ptcl->pos.z * -sinV  + ptcl->pos.x * cosV;
        ptcl->pos.z = v0;
        ptcl->pos.x = v1;

        if (fieldSpinOuter == 0.0f)
            break;

        f32 length2 = v0 * v0 + v1 * v1;
        if (length2 <= 0.0f)
            break;

        f32 r = 1.0f / nw::math::FSqrt(length2) * fieldSpinOuter * ptcl->dynamicsRnd * frameRate;
        ptcl->pos.z += v0 * r;
        ptcl->pos.x += v1 * r;
    }
    break;

    case 2:
    {
        f32 v0 = ptcl->pos.x *  cosV  + ptcl->pos.y * sinV;
        f32 v1 = ptcl->pos.x * -sinV  + ptcl->pos.y * cosV;
        ptcl->pos.x = v0;
        ptcl->pos.y = v1;

        if (fieldSpinOuter == 0.0f)
            break;

        f32 length2 = v0 * v0 + v1 * v1;
        if (length2 <= 0.0f)
            break;

        f32 r = 1.0f / nw::math::FSqrt(length2) * fieldSpinOuter * ptcl->dynamicsRnd * frameRate;
        ptcl->pos.x += v0 * r;
        ptcl->pos.y += v1 * r;
    }
    break;
    }

    return dat + 1;
}

const void* EmitterCalc::_ptclField_Collision(EmitterInstance* __restrict e, PtclInstance* __restrict ptcl, const void* fieldData, f32 frameRate)
{
    const FieldCollisionData* dat = static_cast<const FieldCollisionData*>(fieldData);

    if (dat->fieldCollisionCnt != -1 && dat->fieldCollisionCnt <= ptcl->fieldCollisionCnt)
        return dat + 1;

    f32 y = e->filedCollisionY;

    if (dat->fieldCollisionIsCommonCoord)
    {
        if (!e->emitterSet->GetSystem()->IsFieldCollisionCommonPlaneEnable())
            return dat + 1;

        f32 minX, maxX;
        f32 minZ, maxZ;

        e->emitterSet->GetSystem()->GetFieldCollisionCommonPlane(&minX, &maxX, &y, &minZ, &maxZ);

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
                if (worldPos.y < y && minX < worldPos.x && worldPos.x < maxX &&
                                      minZ < worldPos.z && worldPos.z < maxZ )
                {
                    worldPos.y = y;
                    ptcl->cnt = (f32)ptcl->life;
                    ptcl->cntS = ptcl->life;
                }
                break;

            case EFT_FIELD_COLLISION_REACTION_REFLECTION:
                if (worldPos.y < y && minX < worldPos.x && worldPos.x < maxX &&
                                      minZ < worldPos.z && worldPos.z < maxZ)
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
                    matEmitterInv.SetInverse(*matEmitter);

                    ptcl->pos.SetTransform(matEmitterInv, worldPos);
                    {
                        f32 tx = worldVel.x;
                        f32 ty = worldVel.y;
                        f32 tz = worldVel.z;
                        nw::math::MTX34& m = matEmitterInv;
                        ptcl->vel.x = m.m[0][0] * tx + m.m[0][1] * ty + m.m[0][2] * tz;
                        ptcl->vel.y = m.m[1][0] * tx + m.m[1][1] * ty + m.m[1][2] * tz;
                        ptcl->vel.z = m.m[2][0] * tx + m.m[2][1] * ty + m.m[2][2] * tz;

                        ptcl->vel.x *= dat->fieldCollisionRegist;
                        ptcl->vel.y *= dat->fieldCollisionRegist;
                        ptcl->vel.z *= dat->fieldCollisionRegist;
                    }

                    ptcl->fieldCollisionCnt++;
                }
                break;
            }
        }
        else
        {
            switch (dat->fieldCollisionType)
            {
            case EFT_FIELD_COLLISION_REACTION_CESSER:
                if (ptcl->pos.y < y && minX < ptcl->pos.x && ptcl->pos.x < maxX &&
                                       minZ < ptcl->pos.z && ptcl->pos.z < maxZ)
                {
                    ptcl->pos.y = y;
                    ptcl->cnt = (f32)ptcl->life;
                    ptcl->cntS = ptcl->life;
                }
                break;

            case EFT_FIELD_COLLISION_REACTION_REFLECTION:
                if (ptcl->pos.y < y && minX < ptcl->pos.x && ptcl->pos.x < maxX &&
                                       minZ < ptcl->pos.z && ptcl->pos.z < maxZ)
                {
                    ptcl->pos.y = y;
                    ptcl->vel.y *= -dat->fieldCollisionCoef;

                    ptcl->vel.x *= dat->fieldCollisionRegist;
                    ptcl->vel.y *= dat->fieldCollisionRegist;
                    ptcl->vel.z *= dat->fieldCollisionRegist;

                    ptcl->fieldCollisionCnt++;
                }
                break;
            }
        }
    }
    else
    {
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
                    ptcl->cnt = (f32)ptcl->life;
                    ptcl->cntS = ptcl->life;
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
                    matEmitterInv.SetInverse(*matEmitter);

                    ptcl->pos.SetTransform(matEmitterInv, worldPos);
                    {
                        f32 tx = worldVel.x;
                        f32 ty = worldVel.y;
                        f32 tz = worldVel.z;
                        nw::math::MTX34& m = matEmitterInv;
                        ptcl->vel.x = m.m[0][0] * tx + m.m[0][1] * ty + m.m[0][2] * tz;
                        ptcl->vel.y = m.m[1][0] * tx + m.m[1][1] * ty + m.m[1][2] * tz;
                        ptcl->vel.z = m.m[2][0] * tx + m.m[2][1] * ty + m.m[2][2] * tz;

                        ptcl->vel.x *= dat->fieldCollisionRegist;
                        ptcl->vel.y *= dat->fieldCollisionRegist;
                        ptcl->vel.z *= dat->fieldCollisionRegist;
                    }

                    ptcl->fieldCollisionCnt++;
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
                    ptcl->cnt = (f32)ptcl->life;
                    ptcl->cntS = ptcl->life;
                }
                break;

            case EFT_FIELD_COLLISION_REACTION_REFLECTION:
                if (ptcl->pos.y < y)
                {
                    ptcl->pos.y = y;
                    ptcl->vel.y *= -dat->fieldCollisionCoef;

                    ptcl->vel.x *= dat->fieldCollisionRegist;
                    ptcl->vel.y *= dat->fieldCollisionRegist;
                    ptcl->vel.z *= dat->fieldCollisionRegist;

                    ptcl->fieldCollisionCnt++;
                }
                break;
            }
        }
    }

    return dat + 1;
}

const void* EmitterCalc::_ptclField_Convergence(EmitterInstance* __restrict e, PtclInstance* __restrict ptcl, const void* fieldData, f32 frameRate)
{
    const FieldConvergenceData* dat = static_cast<const FieldConvergenceData*>(fieldData);
    f32 fieldConvergenceRatio;

    if (_isExistParticleAnim8Key(e, EFT_ANIM_8KEY_FIELD_CONVERGENCE))
    {
        KeyFrameAnim* anim = e->particleAnimKey[EFT_ANIM_8KEY_OFFSET(EFT_ANIM_8KEY_FIELD_CONVERGENCE)];

        f32 time = _calcParticleAnimTime(anim, ptcl, EFT_ANIM_8KEY_FIELD_CONVERGENCE);
        u32 index = CalcAnimKeyFrameIndex(anim, time);

        fieldConvergenceRatio = CalcAnimKeyFrameSimple(anim, time, index);
    }
    else
    {
        fieldConvergenceRatio = dat->fieldConvergenceRatio;
    }

    if (dat->fieldConvergenceType == EFT_FIELD_CONVERGENCE_TYPE_ASSIGNED_POSITION)
    {
        ptcl->pos.x += (dat->fieldConvergencePos.x - ptcl->pos.x) * fieldConvergenceRatio * ptcl->dynamicsRnd * frameRate;
        ptcl->pos.y += (dat->fieldConvergencePos.y - ptcl->pos.y) * fieldConvergenceRatio * ptcl->dynamicsRnd * frameRate;
        ptcl->pos.z += (dat->fieldConvergencePos.z - ptcl->pos.z) * fieldConvergenceRatio * ptcl->dynamicsRnd * frameRate;
    }
    else
    {
        nw::math::VEC3 local;
        nw::math::VEC3 src;
        src.x = e->emitterRT._03;
        src.y = e->emitterRT._13;
        src.z = e->emitterRT._23;
        e->TransformLocalVec(&local, &src, ptcl);

        ptcl->pos.x += (local.x + dat->fieldConvergencePos.x - ptcl->pos.x) * fieldConvergenceRatio * ptcl->dynamicsRnd * frameRate;
        ptcl->pos.y += (local.y + dat->fieldConvergencePos.y - ptcl->pos.y) * fieldConvergenceRatio * ptcl->dynamicsRnd * frameRate;
        ptcl->pos.z += (local.z + dat->fieldConvergencePos.z - ptcl->pos.z) * fieldConvergenceRatio * ptcl->dynamicsRnd * frameRate;
    }

    return dat + 1;
}

const void* EmitterCalc::_ptclField_PosAdd(EmitterInstance* __restrict e, PtclInstance* __restrict ptcl, const void* fieldData, f32 frameRate)
{
    const FieldPosAddData* dat = static_cast<const FieldPosAddData*>(fieldData);
    nw::math::VEC3 fieldPosAdd;

    if (_isExistParticleAnim8Key(e, EFT_ANIM_8KEY_FIELD_POS_ADD_X))
    {
        KeyFrameAnim* animX = e->particleAnimKey[EFT_ANIM_8KEY_OFFSET(EFT_ANIM_8KEY_FIELD_POS_ADD_X)];
        KeyFrameAnim* animY = e->particleAnimKey[EFT_ANIM_8KEY_OFFSET(EFT_ANIM_8KEY_FIELD_POS_ADD_Y)];
        KeyFrameAnim* animZ = e->particleAnimKey[EFT_ANIM_8KEY_OFFSET(EFT_ANIM_8KEY_FIELD_POS_ADD_Z)];

        f32 time = _calcParticleAnimTime(e, ptcl, EFT_ANIM_8KEY_FIELD_POS_ADD_X);
        u32 index = CalcAnimKeyFrameIndex(animX, time);

        fieldPosAdd.x = CalcAnimKeyFrameSimple(animX, time, index);
        fieldPosAdd.y = CalcAnimKeyFrameSimple(animY, time, index);
        fieldPosAdd.z = CalcAnimKeyFrameSimple(animZ, time, index);
    }
    else
    {
        fieldPosAdd.x = dat->fieldPosAdd.x;
        fieldPosAdd.y = dat->fieldPosAdd.y;
        fieldPosAdd.z = dat->fieldPosAdd.z;
    }

    if (!dat->isFieldPosAddGlobal)
    {
        ptcl->pos.x += fieldPosAdd.x * ptcl->dynamicsRnd * frameRate;
        ptcl->pos.y += fieldPosAdd.y * ptcl->dynamicsRnd * frameRate;
        ptcl->pos.z += fieldPosAdd.z * ptcl->dynamicsRnd * frameRate;
    }
    else
    {
        nw::math::MTX34* mrt  = ptcl->coordinateEmitterRT;
        register f32 posAddX = fieldPosAdd.x * ptcl->dynamicsRnd * frameRate;
        register f32 posAddY = fieldPosAdd.y * ptcl->dynamicsRnd * frameRate;
        register f32 posAddZ = fieldPosAdd.z * ptcl->dynamicsRnd * frameRate;

        ptcl->pos.x += posAddX * mrt->m[0][0] + posAddY * mrt->m[1][0] + posAddZ * mrt->m[2][0];
        ptcl->pos.y += posAddX * mrt->m[0][1] + posAddY * mrt->m[1][1] + posAddZ * mrt->m[2][1];
        ptcl->pos.z += posAddX * mrt->m[0][2] + posAddY * mrt->m[1][2] + posAddZ * mrt->m[2][2];
    }

    return dat + 1;
}

} } // namespace nw::eft
