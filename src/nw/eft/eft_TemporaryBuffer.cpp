#include <nw/eft/eft_Misc.h>
#include <nw/eft/eft_TemporaryBuffer.h>

namespace nw { namespace eft {

void TemporaryBuffer::Initialize(u32 size, bool useTriple)
{
    mBufferSide                                     = EFT_TEMPORARY_BUFFER_FIRST;
    mBufferSize                                     = size;
    mAllocedSize   [EFT_TEMPORARY_BUFFER_FIRST]     = 0;
    mFlushedSize   [EFT_TEMPORARY_BUFFER_FIRST]     = 0;
    mPreAllocedSize[EFT_TEMPORARY_BUFFER_FIRST]     = 0;
    mAllocedSize   [EFT_TEMPORARY_BUFFER_SECOND]    = 0;
    mFlushedSize   [EFT_TEMPORARY_BUFFER_SECOND]    = 0;
    mPreAllocedSize[EFT_TEMPORARY_BUFFER_SECOND]    = 0;
    mAllocedSize   [EFT_TEMPORARY_BUFFER_THIRD]     = 0;
    mFlushedSize   [EFT_TEMPORARY_BUFFER_THIRD]     = 0;
    mPreAllocedSize[EFT_TEMPORARY_BUFFER_THIRD]     = 0;

    mBuffer[EFT_TEMPORARY_BUFFER_FIRST]             = AllocFromStaticHeap(mBufferSize, EFT_DOUBLE_BUFFER_ALIGN);
    mBuffer[EFT_TEMPORARY_BUFFER_SECOND]            = AllocFromStaticHeap(mBufferSize, EFT_DOUBLE_BUFFER_ALIGN);
    if (useTriple)
        mBuffer[EFT_TEMPORARY_BUFFER_THIRD]         = AllocFromStaticHeap(mBufferSize, EFT_DOUBLE_BUFFER_ALIGN);
    else
        mBuffer[EFT_TEMPORARY_BUFFER_THIRD]         = NULL;
}

void TemporaryBuffer::Finalize()
{
    FreeFromStaticHeap(mBuffer[EFT_TEMPORARY_BUFFER_FIRST]);
    FreeFromStaticHeap(mBuffer[EFT_TEMPORARY_BUFFER_SECOND]);
    if (mBuffer[EFT_TEMPORARY_BUFFER_THIRD])
        FreeFromStaticHeap(mBuffer[EFT_TEMPORARY_BUFFER_THIRD]);
}

void TemporaryBuffer::Swap()
{
    mAllocedSize[mBufferSide] = 0;
    mFlushedSize[mBufferSide] = 0;
    mPreAllocedSize[mBufferSide] = 0;

    if (mBuffer[EFT_TEMPORARY_BUFFER_THIRD] != NULL)
    {
        if (mBufferSide == EFT_TEMPORARY_BUFFER_THIRD)
            mBufferSide = EFT_TEMPORARY_BUFFER_FIRST;

        else
            mBufferSide++;
    }
    else
    {
        mBufferSide = 1 - mBufferSide;
    }

    mAllocedSize[mBufferSide] = 0;
    mFlushedSize[mBufferSide] = 0;
    mPreAllocedSize[mBufferSide] = 0;

    mAllocedSize[mBufferSide] = mPreAllocedSize[mBufferSide]; // ??
}

void* TemporaryBuffer::Alloc(u32 size)
{
    if (size == 0)
        return NULL;

    //u32 allocSize = nw::ut::RoundUp(size, EFT_DOUBLE_BUFFER_ALIGN);
    u32 allocSize = size + (EFT_DOUBLE_BUFFER_ALIGN - 1) & ~(EFT_DOUBLE_BUFFER_ALIGN - 1);
    if (mAllocedSize[mBufferSide] + allocSize > mBufferSize)
    {
        WARNING("DoubleBuffer is Empty.\n");
        return NULL;
    }

    void* ret = static_cast<u8*>(mBuffer[mBufferSide]) + mAllocedSize[mBufferSide];
    mAllocedSize[mBufferSide] += allocSize;

    DCZeroRange(ret, allocSize);

    return ret;
}

void TemporaryBuffer::FlushCache()
{
    if (mFlushedSize[mBufferSide] != mAllocedSize[mBufferSide])
    {
        void* startAdress = static_cast<u8*>(mBuffer[mBufferSide]) + mFlushedSize[mBufferSide];
        DCFlushRange(startAdress, mAllocedSize[mBufferSide] - mFlushedSize[mBufferSide]);
        mFlushedSize[mBufferSide] = mAllocedSize[mBufferSide];
    }
}

} } // namespace nw::eft
