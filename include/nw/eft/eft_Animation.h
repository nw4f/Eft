#ifndef EFT_ANIMATION_H_
#define EFT_ANIMATION_H_

#include <nw/typeDef.h>
#include <nw/eft/eft_Data.h>

namespace nw { namespace eft {

u32 CalcAnimKeyFrameIndex(KeyFrameAnim* anim, f32 x);
f32 CalcAnimKeyFrame(KeyFrameAnim* anim, f32 x);
f32 CalcAnimKeyFrameSimple(KeyFrameAnim* anim, f32 x, u32 index);
s32 CalcAnimKeyFrameSimpleS32(KeyFrameAnim* anim, f32 x, u32 index);

inline AnimKeyFrameKey* _getKeyFramAnimKeys(const KeyFrameAnim* anim)
{
    return (AnimKeyFrameKey*)((u32)anim + sizeof(KeyFrameAnim));
}

inline KeyFrameAnim* _getFirstKeyFrameAnim(KeyFrameAnimArray* keyAnimArray)
{
    return static_cast<KeyFrameAnim*>((void*)((u32)keyAnimArray + sizeof(KeyFrameAnimArray)));
}

inline KeyFrameAnim* _getNextKeyFrameAnim(KeyFrameAnim* anim)
{
    return static_cast<KeyFrameAnim*>((void *)((u32)anim + anim->offset));
}

inline bool _isExistKeyFrameAnim(KeyFrameAnimArray* animArray, AnimKeyFrameApplyType animType)
{
    if (animArray == NULL)
        return false;

    KeyFrameAnim* anim = _getFirstKeyFrameAnim(animArray);
    if (anim == NULL)
        return false;

    for (u32 i = 0; i < animArray->numAnims; ++i)
    {
        if (i != 0)
            anim = _getNextKeyFrameAnim(anim);

        if (anim->target == animType)
            return true;
    }

    return false;
}

} } // namespace nw::eft

#endif // EFT_ANIMATION_H_
