#include <nw/eft/eft_Animation.h>
#include <nw/eft/eft_Data.h>

namespace nw { namespace eft {

f32 CalcAnimKeyFrame(KeyFrameAnim* anim, f32 x)
{
    AnimKeyFrameKey* keys = _getKeyFramAnimKeys(anim);
    if (anim->keyNum == 1)
        return keys[0].y;

    AnimKeyFrameKey* keyS = &keys[0];
    AnimKeyFrameKey* keyE = &keys[anim->keyNum - 1];

    if (anim->isLoop)
    {
        x = nw::math::FMod(x, keyE->x);
        if (x <= keyS->x)
            return keyS->y;
    }
    else
    {
        if (x <= keyS->x)
            return keyS->y;

        if (keyE->x <= x)
            return keyE->y;
    }

    s32 index = -1;
    for (u32 i = 0; i < anim->keyNum; i++)
    {
        if (x < keys[i].x) break;
        index++;
    }

    AnimKeyFrameKey* k0 = &keys[index];
    AnimKeyFrameKey* k1 = &keys[index];
    if (u32(index + 1) != anim->keyNum)
        k1 = &keys[index + 1];

    f32 w = k1->x - k0->x;

    f32 t;
    if (w == 0.0f)
        return k0->y;
    else
        t = (x - k0->x) / w;

    if (anim->interpolation == EFT_ANIM_KEY_FRAME_SMOOTH)
        t = t * t * (3.0f - 2.0f * t);

    return k0->y * (1.0f - t) + k1->y * t;
}

u32 CalcAnimKeyFrameIndex(KeyFrameAnim* anim, f32 x)
{
    AnimKeyFrameKey* keys = _getKeyFramAnimKeys(anim);
    if (anim->keyNum == 1)
        return 0;

    s32 index = -1;
    for (u32 i = 0; i < anim->keyNum; i++)
    {
        if (x < keys[i].x) break;
        index++;
    }

    if (index < 0)
        index = 0;

    return index;
}

f32 CalcAnimKeyFrameSimple(KeyFrameAnim* anim, f32 x, u32 index)
{
    AnimKeyFrameKey* keys = reinterpret_cast<AnimKeyFrameKey*>(anim + 1);
    if (anim->keyNum == 1)
        return keys[0].y;

    AnimKeyFrameKey* k0 = &keys[index];
    AnimKeyFrameKey* k1 = &keys[index];
    if (u32(index + 1) != anim->keyNum)
        k1 = &keys[index + 1];

    f32 w = k1->x - k0->x;

    f32 t;
    if (w == 0.0f)
        return k0->y;
    else
        t = (x - k0->x) / w;

    return k0->y * (1.0f - t) + k1->y * t;
}

s32 CalcAnimKeyFrameSimpleS32(KeyFrameAnim* anim, f32 x, u32 index)
{
    AnimKeyFrameKey* keys = reinterpret_cast<AnimKeyFrameKey*>(anim + 1);
    if (anim->keyNum == 1)
        return keys[0].yFixed;

    AnimKeyFrameKey* k0 = &keys[index];
    AnimKeyFrameKey* k1 = &keys[index];
    if (u32(index + 1) != anim->keyNum)
        k1 = &keys[index + 1];

    f32 w = k1->x - k0->x;

    f32 t;
    if (w == 0.0f)
        return k0->yFixed;
    else
        t = (x - k0->x) / w;

    return (s32)(k0->yFixed * (1.0f - t) + k1->yFixed * t);
}

} } // namespace nw::eft
