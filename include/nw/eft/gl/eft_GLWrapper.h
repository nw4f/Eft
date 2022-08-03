#ifndef EFT_GL_WRAPPER_H_
#define EFT_GL_WRAPPER_H_

#include <nw/eft/eft_Data.h>
#include <nw/eft/eft_Heap.h>

#if EFT_IS_WIN

namespace nw { namespace eft {

class Draw
{
public:
    typedef u32 PrimitiveType;
    enum
    {
        PRIM_TYPE_TRIANGLES                 = GL_TRIANGLES,
        PRIM_TYPE_TRIANGLE_STRIP            = GL_TRIANGLE_STRIP,
        PRIM_TYPE_QUADS                     = GL_QUADS,
    };

    static void DrawPrimitive(PrimitiveType primType, u32 vertexNum)
    {
        glDrawArrays(primType, 0, vertexNum);
    }

    static void DrawInstanceIndexedPrimitive(PrimitiveType primType, u32 vertexNum, const void* indexPtr, u32 primCount)
    {
        glDrawElementsInstanced(primType, vertexNum, GL_UNSIGNED_INT, indexPtr, primCount);
    }

    static void DrawPrimitive(PrimitiveType primType, u32 startNum, u32 vertexNum)
    {
        glDrawArrays(primType, startNum, vertexNum);
    }

    static void DrawInstancePrimitive(PrimitiveType primType, u32 vertexNum, u32 primCount)
    {
        glDrawArraysInstanced(primType, 0, vertexNum, primCount);
    }
};

class TextureSampler
{
public:
    TextureSampler();
    ~TextureSampler();

    bool SetupLOD(f32 maxMip, f32 bais);
    bool Setup(TextureFilterMode textureFilter, TextureWrapMode wrapModeU, TextureWrapMode wrapModeV);

    GLuint GetSampler() const { return mTextureSampler; }

private:
    GLuint  mTextureSampler;
};

typedef GLuint Texture;
typedef GLuint Sampler;

class Rendercontext
{
public:
    Rendercontext();
    ~Rendercontext();

    void SetupCommonState();
    void SetupBlendType(BlendType blendType);
    void SetupZBufATest(ZBufATestType zBufATestType);
    void SetupDisplaySideType(DisplaySideType displaySideType) const;
    void SetupTexture(const TextureRes* texture, TextureSlot slot, FragmentTextureLocation location);
    void SetupTexture(const Texture texture, TextureSlot slot, FragmentTextureLocation location);

    const TextureSampler* GetDefautSampler() const { return &mDefaultTextureSampler; }

private:
    TextureSampler  mTextureSampler[EFT_TEXTURE_SLOT_MAX];
    TextureSampler  mDefaultTextureSampler;
    GLuint          mVertexArrayId;
};

class VertexBuffer
{
public:
    VertexBuffer();

    void* AllocateVertexBuffer(Heap* heap, u32 size, u32 element);
    void Finalize(Heap* heap);
    void* GetVertexBuffer() { return mVertexBuffer; }
    u32 GetVertexBufferSize() { return mVertexBufferSize; }
    void Invalidate();
    void BindBuffer(u32 index, u32 size, u32 stride);

    static void BindExtBuffer(u32 index, u32 element, u32 stride, u32 offset);

private:
    Heap*   mHeap;
    u32     mVertexElement;
    u32     mVertexBufferSize;
    void*   mVertexBuffer;
    GLuint  mVertexBufferId;
};

class Shader
{
public:
    typedef u32 VertexFormat;
    enum
    {
        FORMAT_32_UINT,
        FORMAT_32_32_32_FLOAT,
        FORMAT_32_32_32_32_FLOAT,
    };

public:
    Shader();

    bool IsInitialized() const
    {
        return mProgram != GL_NONE;
    }

    void Finalize(Heap* heap);
    void BindShader();
    bool CreateShader(Heap* heap, const void* gshBuffer, u32 gshBufferSize);
    u32 GetFragmentSamplerLocation(const char* name);
    u32 GetAttributeLocation(const char* name) { return EFT_INVALID_ATTRIBUTE; }
    u32 GetAttribute(const char* name, u32 index, VertexFormat fmt, u32 offset = 0, bool instancingAttr = false);
    void SetupShader(Heap* heap) { }

    const GX2VertexShader* GetVertexShader()
    {
        return mpVertexShader;
    }

    const GX2PixelShader* GetPixelShader()
    {
        return mpPixelShader;
    }

  //const GX2GeometryShader* GetGeometryShader()
  //{
  //    return mpGeometryShader;
  //}

    GLuint GetProgramID() const
    {
        return mProgram;
    }

public:
    static Shader*      sBoundShader;

    static GLenum       sAlphaTest;
    static f32          sAlphaTestRef;

private:
    GX2VertexShader*    mpVertexShader;
    GX2PixelShader*     mpPixelShader;
  //GX2GeometryShader*  mpGeometryShader;
    GLuint              mProgram;
    GFDFile*            mGFDFile;
    GLenum              mCurrentAlphaTest;
    f32                 mCurrentAlphaTestRef;
};

class UniformBlock
{
public:
    UniformBlock()
    {
        mIsInitialized      = false;
        mIsFailed           = false;
        mBufferBindPoint    = -1;
        mBufferSize         = 0;
        mBufferId           = GL_NONE;
        mBufferIndex        = 0;
    }

    bool InitializeVertexUniformBlock(Shader* shader, const char* name, u32 bindPoint);
    bool InitializePixelUniformBlock(Shader* shader, const char* name, u32 bindPoint);
    void BindUniformBlock(const void* param);

    bool IsInitialized() const { return mIsInitialized; }
    bool IsFailed() const { return mIsFailed; }

private:
    bool    mIsInitialized;
    bool    mIsFailed;
    GLint	mBufferBindPoint;
    GLint	mBufferSize;
    GLuint	mBufferId;
    GLuint	mBufferIndex;
};

} } // namespace nw::eft

#endif // EFT_IS_WIN

#endif // EFT_GL_WRAPPER_H_
