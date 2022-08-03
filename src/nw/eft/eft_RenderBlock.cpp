#include <nw/eft/eft_Renderer.h>
#include <nw/eft/eft_Shader.h>

namespace nw { namespace eft {

void Renderer::BindParticleAttributeBlock(PtclAttributeBuffer* ptclAttributeBuffer, ParticleShader* shader, u32 entryNum)
{
#if EFT_IS_WIN
    VertexBuffer::BindExtBuffer(shader->mWldPosAttr,     4, sizeof(PtclAttributeBuffer),  0 * 4);
    VertexBuffer::BindExtBuffer(shader->mSclAttr,        4, sizeof(PtclAttributeBuffer),  4 * 4);
    VertexBuffer::BindExtBuffer(shader->mColor0Attr,     4, sizeof(PtclAttributeBuffer),  8 * 4);
    VertexBuffer::BindExtBuffer(shader->mColor1Attr,     4, sizeof(PtclAttributeBuffer), 12 * 4);
    VertexBuffer::BindExtBuffer(shader->mTexAnimAttr,    4, sizeof(PtclAttributeBuffer), 16 * 4);
    VertexBuffer::BindExtBuffer(shader->mWldPosDfAttr,   4, sizeof(PtclAttributeBuffer), 20 * 4);
    VertexBuffer::BindExtBuffer(shader->vRotAttr,        4, sizeof(PtclAttributeBuffer), 24 * 4);
    VertexBuffer::BindExtBuffer(shader->mSubTexAnimAttr, 4, sizeof(PtclAttributeBuffer), 28 * 4);
    VertexBuffer::BindExtBuffer(shader->mEmtMatAttr0,    4, sizeof(PtclAttributeBuffer), 32 * 4);
    VertexBuffer::BindExtBuffer(shader->mEmtMatAttr1,    4, sizeof(PtclAttributeBuffer), 36 * 4);
    VertexBuffer::BindExtBuffer(shader->mEmtMatAttr2,    4, sizeof(PtclAttributeBuffer), 40 * 4);
#endif // EFT_IS_WIN
#if EFT_IS_CAFE
    VertexBuffer::BindExtBuffer(5, sizeof(PtclAttributeBuffer) * entryNum, 0, sizeof(PtclAttributeBuffer), ptclAttributeBuffer);
#endif // EFT_IS_CAFE
}

} } // namespace nw::eft
