#include <nw/eft/eft_Misc.h>
#include <nw/eft/eft_StreamOutBuffer.h>

namespace nw { namespace eft {

void StreamOutAttributeBuffer::Invalidate()
{
    pBuffer[EFT_STREAM_OUT_ATTRB_0] = NULL;
    pBuffer[EFT_STREAM_OUT_ATTRB_1] = NULL;
    memset(&streamOutBuf[EFT_STREAM_OUT_ATTRB_0],     0, sizeof(GX2StreamOutBuffer));
    memset(&streamOutContext[EFT_STREAM_OUT_ATTRB_0], 0, sizeof(GX2StreamOutContext));
    memset(&streamOutBuf[EFT_STREAM_OUT_ATTRB_1],     0, sizeof(GX2StreamOutBuffer));
    memset(&streamOutContext[EFT_STREAM_OUT_ATTRB_1], 0, sizeof(GX2StreamOutContext));
}

void StreamOutAttributeBuffer::Initialize(u32 size)
{
    bufferSize = size;

    u32 memSize = GX2RoundUp(bufferSize + sizeof(GX2StreamOutContext), 32);
    pBuffer[EFT_STREAM_OUT_ATTRB_0] = static_cast<nw::math::VEC4*>(AllocFromDynamicHeap(memSize, GX2_STREAMOUT_BUFFER_ALIGNMENT));
    pBuffer[EFT_STREAM_OUT_ATTRB_1] = static_cast<nw::math::VEC4*>(AllocFromDynamicHeap(memSize, GX2_STREAMOUT_BUFFER_ALIGNMENT));

    if (pBuffer[EFT_STREAM_OUT_ATTRB_0] && pBuffer[EFT_STREAM_OUT_ATTRB_1])
    {
        for (u32 i = 0; i < size / sizeof(nw::math::VEC4); i++)
        {
            pBuffer[EFT_STREAM_OUT_ATTRB_0][i].x = 0.0f;
            pBuffer[EFT_STREAM_OUT_ATTRB_0][i].y = 0.0f;
            pBuffer[EFT_STREAM_OUT_ATTRB_0][i].z = 0.0f;
            pBuffer[EFT_STREAM_OUT_ATTRB_0][i].w = 0.0f;

            pBuffer[EFT_STREAM_OUT_ATTRB_1][i].x = 0.0f;
            pBuffer[EFT_STREAM_OUT_ATTRB_1][i].y = 0.0f;
            pBuffer[EFT_STREAM_OUT_ATTRB_1][i].z = 0.0f;
            pBuffer[EFT_STREAM_OUT_ATTRB_1][i].w = 0.0f;
        }

        memset(&streamOutBuf[EFT_STREAM_OUT_ATTRB_0],     0, sizeof(GX2StreamOutBuffer));
        memset(&streamOutContext[EFT_STREAM_OUT_ATTRB_0], 0, sizeof(GX2StreamOutContext));
        memset(&streamOutBuf[EFT_STREAM_OUT_ATTRB_1],     0, sizeof(GX2StreamOutBuffer));
        memset(&streamOutContext[EFT_STREAM_OUT_ATTRB_1], 0, sizeof(GX2StreamOutContext));

        streamOutBuf[EFT_STREAM_OUT_ATTRB_0].size    = bufferSize;
        streamOutBuf[EFT_STREAM_OUT_ATTRB_0].dataPtr = pBuffer[EFT_STREAM_OUT_ATTRB_0];
        streamOutBuf[EFT_STREAM_OUT_ATTRB_0].ctxPtr  = reinterpret_cast<GX2StreamOutContext*>((u32)streamOutBuf[EFT_STREAM_OUT_ATTRB_0].dataPtr + streamOutBuf[EFT_STREAM_OUT_ATTRB_0].size);

        streamOutBuf[EFT_STREAM_OUT_ATTRB_1].size    = bufferSize;
        streamOutBuf[EFT_STREAM_OUT_ATTRB_1].dataPtr = pBuffer[EFT_STREAM_OUT_ATTRB_1];
        streamOutBuf[EFT_STREAM_OUT_ATTRB_1].ctxPtr  = reinterpret_cast<GX2StreamOutContext*>((u32)streamOutBuf[EFT_STREAM_OUT_ATTRB_1].dataPtr + streamOutBuf[EFT_STREAM_OUT_ATTRB_1].size);


        GX2Invalidate(GX2_INVALIDATE_CPU, streamOutBuf[EFT_STREAM_OUT_ATTRB_0].dataPtr, memSize);
        GX2Invalidate(GX2_INVALIDATE_CPU, streamOutBuf[EFT_STREAM_OUT_ATTRB_1].dataPtr, memSize);

        streamOutBuf[EFT_STREAM_OUT_ATTRB_0].vertexStride = sizeof(nw::math::VEC4);
        streamOutBuf[EFT_STREAM_OUT_ATTRB_1].vertexStride = sizeof(nw::math::VEC4);
    }
    else
    {
        if (pBuffer[EFT_STREAM_OUT_ATTRB_0])
        {
            FreeFromDynamicHeap(pBuffer[EFT_STREAM_OUT_ATTRB_0]);
            pBuffer[EFT_STREAM_OUT_ATTRB_0] = NULL;
        }

        if (pBuffer[EFT_STREAM_OUT_ATTRB_1] != NULL)
        {
            FreeFromDynamicHeap(pBuffer[EFT_STREAM_OUT_ATTRB_1]);
            pBuffer[EFT_STREAM_OUT_ATTRB_1] = NULL;
        }

        Invalidate();
    }
}

void StreamOutAttributeBuffer::Finalize()
{
    if (pBuffer[EFT_STREAM_OUT_ATTRB_0])
    {
        FreeFromDynamicHeap(pBuffer[EFT_STREAM_OUT_ATTRB_0], false);
        pBuffer[EFT_STREAM_OUT_ATTRB_0] = NULL;
    }

    if (pBuffer[EFT_STREAM_OUT_ATTRB_1])
    {
        FreeFromDynamicHeap(pBuffer[EFT_STREAM_OUT_ATTRB_1], false);
        pBuffer[EFT_STREAM_OUT_ATTRB_1] = NULL;
    }
}

bool StreamOutAttributeBuffer::Bind(u32 input, u32 index, bool flip, bool output)
{
    if (pBuffer[EFT_STREAM_OUT_ATTRB_0] && pBuffer[EFT_STREAM_OUT_ATTRB_1])
    {
        GX2SetAttribBuffer(input, streamOutBuf[flip].size, sizeof(nw::math::VEC4), pBuffer[flip]);

        if (flip)
            outputBuffer = false;
        else
            outputBuffer = true;

        if (output)
        {
            GX2SetStreamOutBuffer(index, &streamOutBuf[outputBuffer]);
            GX2SetStreamOutContext(index, &streamOutBuf[outputBuffer], GX2_TRUE);
        }

        return true;
    }
    else
    {
        return false;
    }
}

void StreamOutAttributeBuffer::UnBind(u32 index)
{
    if (pBuffer[EFT_STREAM_OUT_ATTRB_0] && pBuffer[EFT_STREAM_OUT_ATTRB_1])
    {
        GX2SaveStreamOutContext(index, &streamOutBuf[outputBuffer]);

        GX2Invalidate(static_cast<GX2InvalidateType>(GX2_INVALIDATE_ATTRIB_BUFFER |
                                                     GX2_INVALIDATE_STREAMOUT_BUFFER),
                      streamOutBuf[outputBuffer].dataPtr, bufferSize);
    }
}

} } // namespace nw::eft
