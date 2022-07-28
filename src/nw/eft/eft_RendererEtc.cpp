#include <nw/eft/eft_Config.h>
#include <nw/eft/eft_Renderer.h>

namespace nw { namespace eft {

void Renderer::SwapDoubleBuffer()
{
    mTemporaryBuffer.Swap();
    mStripeVertexCalcNum = 0;
}

void* Renderer::AllocFromDoubleBuffer(u32 size)
{
    if (size == 0)
        return NULL;

    return mTemporaryBuffer.Alloc(size);
}

void Renderer::FlushCache()
{
    mTemporaryBuffer.FlushCache();
}

} } // namespace nw::eft
