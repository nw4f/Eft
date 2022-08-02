#include <nw/eft/eft_Config.h>
#include <nw/eft/eft_Renderer.h>

namespace nw { namespace eft {

void Renderer::SwapDoubleBuffer()
{
    mTemporaryBuffer.Swap();

    mStripeVertexCalcNum    = 0;
    mStripeVertexDrawNum    = 0;
    mRenderingParticleNum   = 0;
    mRenderingEmitterNum    = 0;
}

void* Renderer::AllocFromDoubleBuffer(u32 size)
{
    if (size == 0)
        return NULL;

    void* retPtr = mTemporaryBuffer.Alloc(size);
    return retPtr;
}

void Renderer::FlushCache()
{
    mTemporaryBuffer.FlushCache();
}

} } // namespace nw::eft
