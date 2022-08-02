#ifndef EFT_RENDER_CONTEXT_H_
#define EFT_RENDER_CONTEXT_H_

#include <nw/eft/cafe/eft_CafeWrapper.h>
#include <nw/eft/eft_Data.h>

namespace nw { namespace eft {

typedef const GX2Texture* Texture;
typedef const GX2Sampler* Sampler;

class RenderContext
{
public:
    RenderContext();

    void SetupCommonState();
    void SetupBlendType(BlendType blendType) const;
    void SetupZBufATest(ZBufATestType zBufATestType) const;
    void SetupDisplaySideType(DisplaySideType displaySideType) const;

    void SetupTexture           (const TextureRes* texture, TextureSlot slot, FragmentTextureLocation location);
    void SetupTexture           (const Texture     texture, TextureSlot slot, FragmentTextureLocation location);
    void SetupCubeMapTexture    (const Texture     texture, TextureSlot slot, FragmentTextureLocation location);
    void SetupDepthTexture      (const Texture     texture, TextureSlot slot, FragmentTextureLocation location);
    void SetupFrameBufferTexture(const Texture     texture, TextureSlot slot, FragmentTextureLocation location);

    void SetupUserFragmentTexture(const Texture texture, TextureSlot slot, FragmentTextureLocation location) const;
    void SetupUserFragmentTexture(const Texture texture, const TextureSampler* sampler, TextureSlot slot, FragmentTextureLocation location) const;
    void SetupUserFragmentTexture(const Texture texture, const Sampler sampler, TextureSlot slot, FragmentTextureLocation location) const;

    void SetupUserFragment2DArrayTexture(const Texture texture, TextureSlot slot, FragmentTextureLocation location) const;
    void SetupUserFragment2DArrayTexture(const Texture texture, const TextureSampler* sampler, TextureSlot slot, FragmentTextureLocation location) const;
    void SetupUserFragment2DArrayTexture(const Texture texture, const Sampler sampler, TextureSlot slot, FragmentTextureLocation location) const;

    void SetupUserFragmentCubeMapTexture(const Texture texture, TextureSlot slot, FragmentTextureLocation location) const;
    void SetupUserFragmentCubeMapTexture(const Texture texture, const TextureSampler* sampler, TextureSlot slot, FragmentTextureLocation location) const;
    void SetupUserFragmentCubeMapTexture(const Texture texture, const Sampler sampler, TextureSlot slot, FragmentTextureLocation location) const;

    void SetupUserFragmentCubeMapArrayTexture(const Texture texture, TextureSlot slot, FragmentTextureLocation location) const;
    void SetupUserFragmentCubeMapArrayTexture(const Texture texture, const TextureSampler* sampler, TextureSlot slot, FragmentTextureLocation location) const;
    void SetupUserFragmentCubeMapArrayTexture(const Texture texture, const Sampler sampler, TextureSlot slot, FragmentTextureLocation location) const;

    void SetupUserVertexTexture(const Texture texture, TextureSlot slot, VertexTextureLocation location) const;
    void SetupUserVertexTexture(const Texture texture, const TextureSampler* sampler, TextureSlot slot, VertexTextureLocation location) const;
    void SetupUserVertexTexture(const Texture texture, const Sampler sampler, TextureSlot slot, VertexTextureLocation location) const;

    void SetupUserVertexArrayTexture(const Texture texture, const Sampler sampler, TextureSlot slot, VertexTextureLocation location) const;

    void SetupVertexArrayTexture(const Texture texture, TextureSlot slot, VertexTextureLocation location) const;

    void SetupUserVertexCubeMapTexture(const Texture texture, TextureSlot slot, VertexTextureLocation location) const;
    void SetupUserVertexCubeMapTexture(const Texture texture, const TextureSampler* sampler, TextureSlot slot, VertexTextureLocation location) const;
    void SetupUserVertexCubeMapTexture(const Texture texture, const Sampler sampler, TextureSlot slot, VertexTextureLocation location) const;

    void SetupUserVertexCubeMapArrayTexture(const Texture texture, TextureSlot slot, VertexTextureLocation location) const;
    void SetupUserVertexCubeMapArrayTexture(const Texture texture, const TextureSampler* sampler, TextureSlot slot, VertexTextureLocation location) const;
    void SetupUserVertexCubeMapArrayTexture(const Texture texture, const Sampler sampler, TextureSlot slot, VertexTextureLocation location) const;

    const TextureSampler* GetDefautSampler() const { return &mDefaultTextureSampler; }

private:
    void _SetupFragmentTexture              (const Texture texture, const Sampler sampler, TextureSlot slot, FragmentTextureLocation location) const;
    void _SetupFragment2DArrayTexture       (const Texture texture, const Sampler sampler, TextureSlot slot, FragmentTextureLocation location) const;
    void _SetupFragmentCubeMapTexture       (const Texture texture, const Sampler sampler, TextureSlot slot, FragmentTextureLocation location) const;
    void _SetupFragmentCubeMapArrayTexture  (const Texture texture, const Sampler sampler, TextureSlot slot, FragmentTextureLocation location) const;
    void _SetupVertexTexture                (const Texture texture, const Sampler sampler, TextureSlot slot, VertexTextureLocation   location) const;
    void _SetupVertex2DArrayTexture         (const Texture texture, const Sampler sampler, TextureSlot slot, VertexTextureLocation   location) const;
    void _SetupVertexCubeMapTexture         (const Texture texture, const Sampler sampler, TextureSlot slot, VertexTextureLocation   location) const;
    void _SetupVertexCubeMapArrayTexture    (const Texture texture, const Sampler sampler, TextureSlot slot, VertexTextureLocation   location) const;

private:
    TextureSampler  mTextureSampler[EFT_TEXTURE_SLOT_MAX];
    TextureSampler  mDefaultTextureSampler;
    TextureSampler  m2DArrayTextureSampler;
};
static_assert(sizeof(RenderContext) == 0xC0, "nw::eft::RenderContext size mismatch");
typedef RenderContext Rendercontext;

} } // namespace nw::eft

#endif // EFT_RENDER_CONTEXT_H_
