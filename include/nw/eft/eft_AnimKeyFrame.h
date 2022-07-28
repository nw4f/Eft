#ifndef EFT_ANIMATION_H_
#define EFT_ANIMATION_H_

#include <nw/eft/eft_typeDef.h>

namespace nw { namespace eft {

struct KeyFrameAnimKey
{
    f32 time;
    f32 value;
};
static_assert(sizeof(KeyFrameAnimKey) == 8, "KeyFrameAnimKey size mismatch");

struct KeyFrameAnim
{
    u32 numKeys;
    u32 interpolation;
    u32 animValIdx;
    u32 loop;
    u32 nextOffs;
    u8 _unusedPad[4];
};
static_assert(sizeof(KeyFrameAnim) == 0x18, "KeyFrameAnim size mismatch");

struct KeyFrameAnimArray
{
    char magic[4];
    u32 numAnim;
};
static_assert(sizeof(KeyFrameAnimArray) == 8, "KeyFrameAnimArray size mismatch");

f32 CalcAnimKeyFrame(KeyFrameAnim* anim, f32 frame);

} } // namespace nw::eft

#endif // EFT_ANIMATION_H_
