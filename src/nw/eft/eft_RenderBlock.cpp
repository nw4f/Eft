#include <nw/eft/eft_Renderer.h>
#include <nw/eft/eft_Shader.h>

namespace nw { namespace eft {

void Renderer::BindParticleAttributeBlock(PtclAttributeBuffer* ptclAttributeBuffer, ParticleShader* shader, u32 entryNum)
{
    VertexBuffer::BindExtBuffer(5, sizeof(PtclAttributeBuffer) * entryNum, 0, sizeof(PtclAttributeBuffer), ptclAttributeBuffer);
}

} } // namespace nw::eft
