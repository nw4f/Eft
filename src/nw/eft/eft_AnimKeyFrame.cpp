#include <nw/eft/eft_AnimKeyFrame.h>

namespace nw { namespace eft {

f32 CalcAnimKeyFrame(KeyFrameAnim* info, f32 x)
{
    AnimKeyFrameKey* keys = reinterpret_cast<AnimKeyFrameKey*>(info + 1);
    if (info->keyNum == 1)
        return keys[0].y;

    f32 xS = keys[0].x;
    f32 yS = keys[0].y;

    f32 xE = keys[info->keyNum - 1].x;
    f32 yE = keys[info->keyNum - 1].y;

    if (info->isLoop)
    {
        x = nw::math::FMod(x, xE);
        if (x <= xS)
            return yS;
    }
    else
    {
        if (x <= xS)
            return yS;

        else if (xE <= x)
            return yE;
    }

    s32 index = -1;
    for (u32 i = 0; i < info->keyNum; i++)
    {
        if (x < keys[i].x) break;
        index++;
    }

    f32 x0 = keys[index].x;
    f32 y0 = keys[index].y;

    f32 x1 = keys[index].x;
    f32 y1 = keys[index].y;

    if (u32(index + 1) != info->keyNum)
    {
        x1 = keys[index + 1].x;
        y1 = keys[index + 1].y;
    }

    f32 y;
    f32 w = x1 - x0;

    f32 t;
    if (w == 0.0f)
        return y0;
    else
        t = (x - x0) / w;

    if (info->interpolation == EFT_ANIM_KEY_FRAME_SMOOTH)
        t = t * t * (3.0f - 2.0f * t);

    y = y0 * (1.0f - t) + y1 * t;
    return y;
}

} } // namespace nw::eft
