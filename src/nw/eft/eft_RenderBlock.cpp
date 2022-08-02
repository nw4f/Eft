#include <nw/eft/eft_Renderer.h>
#include <nw/eft/eft_Shader.h>

namespace nw { namespace eft {

void Renderer::BindParticleAttributeBlock(PtclAttributeBuffer* ptclAttributeBuffer, ParticleShader* shader, u32 startNum, u32 entryNum)
{
    VertexBuffer::BindExtBuffer(5, sizeof(PtclAttributeBuffer) * entryNum, 0, sizeof(PtclAttributeBuffer), &ptclAttributeBuffer[startNum]);
}

void Renderer::BindGpuParticleAttributeBlock(PtclAttributeBufferGpu* ptclAttributeBuffer, ParticleShader* shader, u32 startNum, u32 entryNum)
{
    VertexBuffer::BindExtBuffer(5, sizeof(PtclAttributeBufferGpu) * entryNum, 0, sizeof(PtclAttributeBufferGpu), &ptclAttributeBuffer[startNum]);
}

} } // namespace nw::eft
