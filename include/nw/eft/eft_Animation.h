#ifndef EFT_ANIMATION_H_
#define EFT_ANIMATION_H_

#include <nw/typeDef.h>

namespace nw { namespace eft {

struct KeyFrameAnim;

u32 CalcAnimKeyFrameIndex(KeyFrameAnim* anim, f32 frame);
f32 CalcAnimKeyFrame(KeyFrameAnim* anim, f32 frame);
f32 CalcAnimKeyFrameSimple(KeyFrameAnim* anim, f32 frame, u32 index);
s32 CalcAnimKeyFrameSimpleS32(KeyFrameAnim* anim, f32 frame, u32 index);

} } // namespace nw::eft

#endif // EFT_ANIMATION_H_
