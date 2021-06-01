#ifndef EFT_CAFE_WRAPPER_H_
#define EFT_CAFE_WRAPPER_H_

#include <eft_Enum.h>

#define INCLUDE_CAFE
#include <eft_Types.h>

namespace nw { namespace eft {

class Heap;

class Shader
{
public:
    enum VertexFormat
    {
        VertexFormat_U32 = GX2_ATTRIB_FORMAT_32_UINT,
        VertexFormat_VEC3 = GX2_ATTRIB_FORMAT_32_32_32_FLOAT,
        VertexFormat_VEC4 = GX2_ATTRIB_FORMAT_32_32_32_32_FLOAT,
    };

public:
    Shader();

    void Finalize(Heap* heap);
    void BindShader();
    bool CreateShader(Heap* heap, const void* binary, u32 binarySize);
    s32 GetFragmentSamplerLocation(const char* name);
    s32 GetAttributeLocation(const char* name);
    u32 GetAttribute(const char* name, u32 buffer, VertexFormat attribFormat, u32 offset, bool instanceID);
    void SetupShader(Heap* heap);

    GX2VertexShader* vertexShader;
    GX2PixelShader* pixelShader;
    GX2GeometryShader* geometryShader;
    GX2FetchShader fetchShader;
    void* fetchShaderBufPtr;
    u32 numAttribute;
    GX2AttribStream attributes[16];
    u32 attributeBuffer[16];
    bool initialized;
};
static_assert(sizeof(Shader) == 0x278, "Shader size mismatch");

class TextureSampler
{
public:
    TextureSampler();
    ~TextureSampler();

    bool Setup(TextureFilterMode filterMode, TextureWrapMode wrapModeX, TextureWrapMode wrapModeY);

    GX2Sampler sampler;
};
static_assert(sizeof(TextureSampler) == 0xC, "TextureSampler size mismatch");

class UniformBlock
{
public:
    UniformBlock()
    {
        initialized = false;
        blockNotExist = false;
        bufferSize = 0;
        location = 0;
        shaderStage = 3;
    }

    bool InitializeVertexUniformBlock(Shader* shader, const char* name, u32);
    bool InitializePixelUniformBlock(Shader* shader, const char* name, u32);

    bool initialized;
    bool blockNotExist;
    u32 bufferSize;
    u32 location;
    u32 shaderStage; // 0 = Vertex, 1 = Fragment, 2 = Geometry?, 3 = Invalid
};
static_assert(sizeof(UniformBlock) == 0x10, "UniformBlock size mismatch");

class Heap;

class VertexBuffer
{
public:
    VertexBuffer();

    void* AllocateVertexBuffer(Heap* heap, u32 bufSize, u32 size);
    void Finalize(Heap* heap);
    void Invalidate();

    u32 _0;
    u32 size;
    u32 bufferSize;
    void* buffer;
};
static_assert(sizeof(VertexBuffer) == 0x10, "VertexBuffer size mismatch");

} } // namespace nw::eft

BOOL _DEMOGFDReadVertexShader(nw::eft::Heap* heap, GX2VertexShader** shader, u32 index, const void* binary);
BOOL _DEMOGFDReadPixelShader(nw::eft::Heap* heap, GX2PixelShader** shader, u32 index, const void* binary);
BOOL _DEMOGFDReadGeometryShader(nw::eft::Heap* heap, GX2GeometryShader** shader, u32 index, const void* binary);

#endif // EFT_CAFE_WRAPPER_H_
