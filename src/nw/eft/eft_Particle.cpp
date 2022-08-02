#include <nw/eft/eft_Particle.h>
#include <nw/eft/eft_Emitter.h>
#include <nw/eft/eft_EmitterSet.h>

namespace nw { namespace eft {

void PtclInstance::GetDrawColor0(nw::ut::Color4f* dstColor) const
{
    nw::ut::Color4f setColor = emitter->emitterSet->GetColor();
    setColor.r *= emitter->res->colorScale;
    setColor.g *= emitter->res->colorScale;
    setColor.b *= emitter->res->colorScale;

    dstColor->r = color[EFT_COLOR_KIND_0].r * setColor.r  * emitter->emitterAnimValue[EFT_ANIM_COLOR0_R];
    dstColor->g = color[EFT_COLOR_KIND_0].g * setColor.g  * emitter->emitterAnimValue[EFT_ANIM_COLOR0_G];
    dstColor->b = color[EFT_COLOR_KIND_0].b * setColor.b  * emitter->emitterAnimValue[EFT_ANIM_COLOR0_B];
    dstColor->a = alpha0 * setColor.a * emitter->fadeAlpha * emitter->emitterAnimValue[EFT_ANIM_ALPHA] * fluctuationAlpha;
}

} } // namespace nw::eft
