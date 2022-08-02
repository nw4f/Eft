#ifndef EFT_STREAM_OUT_BUFFER_H_
#define EFT_STREAM_OUT_BUFFER_H_

#include <nw/typeDef.h>

namespace nw { namespace eft {

struct StreamOutAttributeBuffer
{
public:
    enum
    {
        EFT_STREAM_OUT_ATTRB_0   = 0,
        EFT_STREAM_OUT_ATTRB_1   = 1,
        EFT_STREAM_OUT_ATTRB_MAX = 2,
    };

    void Initialize(u32 size);
    void Finalize();
    void Invalidate();
    bool Bind(u32 input, u32 index, bool flip, bool output);
    void UnBind(u32 index);

    u32 GetBufferSize() const { return bufferSize; }

private:
    nw::math::VEC4*             pBuffer[EFT_STREAM_OUT_ATTRB_MAX];
    GX2StreamOutBuffer			streamOutBuf[EFT_STREAM_OUT_ATTRB_MAX];
    GX2StreamOutContext			streamOutContext[EFT_STREAM_OUT_ATTRB_MAX];
    u32                         bufferSize;
    bool                        outputBuffer;
};
static_assert(sizeof(StreamOutAttributeBuffer) == 0x58, "nw::eft::StreamOutAttributeBuffer size mismatch");

} } // namespace nw::eft

#endif // EFT_STREAM_OUT_BUFFER_H_
