#ifndef EFT_TEMPORARY_BUFFER_H_
#define EFT_TEMPORARY_BUFFER_H_

#include <nw/typeDef.h>

namespace nw { namespace eft {

struct TemporaryBuffer
{
public:
    enum BufferSide
    {
        EFT_TEMPORARY_BUFFER_FIRST      = 0,
        EFT_TEMPORARY_BUFFER_SECOND     = 1,
        EFT_TEMPORARY_BUFFER_THIRD      = 2,
        EFT_TEMPORARY_BUFFER_MAX        = 3,
    };

    enum
    {
       EFT_DOUBLE_BUFFER_ALIGN      = 0x100
    };

    void Initialize(u32 size, bool useTriple = false);
    void Finalize();
    void Swap();
    void MarkingCurrentBufferSize();
    void PreDrawSwap();

    void* Alloc(u32 size);

    u32 GetSize() const
    {
        return mBufferSize;
    }

    u32 GetUsedSize() const
    {
        return mAllocedSize[mBufferSide];
    }

    u32 GetUsedDoubleBufferSize(BufferSide side) const
    {
        return mAllocedSize[side];
    }

    void FlushCache();

private:
    u32                             mBufferSide;
    u32                             mBufferSize;
    u32                             mPreAllocedSize[EFT_TEMPORARY_BUFFER_MAX];
    u32                             mAllocedSize[EFT_TEMPORARY_BUFFER_MAX];
    u32                             mFlushedSize[EFT_TEMPORARY_BUFFER_MAX];
    void*                           mBuffer[EFT_TEMPORARY_BUFFER_MAX];
};
static_assert(sizeof(TemporaryBuffer) == 0x38, "nw::eft::TemporaryBuffer size mismatch");

} } // namespace nw::eft

#endif // EFT_TEMPORARY_BUFFER_H_
