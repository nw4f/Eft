#include <nw/eft/eft_RenderContext.h>
#include <nw/eft/eft_Shader.h>

namespace nw { namespace eft {

RenderContext::RenderContext()
{
    for (u32 i = 0; i < EFT_TEXTURE_SLOT_MAX; i++)
        mTextureSampler[i].Initialize(EFT_TEXTURE_FILTER_TYPE_LINEAR, EFT_TEXTURE_WRAP_TYPE_REPEAT, EFT_TEXTURE_WRAP_TYPE_REPEAT);

    mDefaultTextureSampler.Initialize(EFT_TEXTURE_FILTER_TYPE_LINEAR, EFT_TEXTURE_WRAP_TYPE_MIRROR, EFT_TEXTURE_WRAP_TYPE_MIRROR);
    m2DArrayTextureSampler.Initialize(EFT_TEXTURE_FILTER_TYPE_LINEAR, EFT_TEXTURE_WRAP_TYPE_REPEAT, EFT_TEXTURE_WRAP_TYPE_REPEAT);
}

void RenderContext::SetupCommonState()
{
    GX2SetColorControl(GX2_LOGIC_OP_COPY, GX2_ENABLE, GX2_DISABLE, GX2_ENABLE);
    GX2SetDepthStencilControl(GX2_TRUE, GX2_TRUE, GX2_COMPARE_LESS, GX2_FALSE, GX2_FALSE, GX2_COMPARE_ALWAYS, GX2_STENCIL_REPLACE, GX2_STENCIL_REPLACE, GX2_STENCIL_REPLACE, GX2_COMPARE_ALWAYS, GX2_STENCIL_REPLACE, GX2_STENCIL_REPLACE, GX2_STENCIL_REPLACE);
    GX2SetPolygonControl(GX2_FRONT_FACE_CCW, GX2_DISABLE, GX2_DISABLE, GX2_DISABLE, GX2_POLYGON_MODE_TRIANGLE, GX2_POLYGON_MODE_TRIANGLE, GX2_DISABLE, GX2_DISABLE, GX2_DISABLE);
    GX2SetPolygonOffset(0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    GX2SetPointSize(1.0f, 1.0f);
    GX2SetPointLimits(1.0f, 1.0f);
    GX2SetLineWidth(1.0f);
    GX2SetPrimitiveRestartIndex(0xFFFFFFFF);
    GX2SetAlphaTest(GX2_DISABLE, GX2_COMPARE_LESS, 0.0f);
    GX2SetAlphaToMask(GX2_FALSE, GX2_ALPHA_TO_MASK_0);
    GX2SetBlendConstantColor(0.0f, 0.0f, 0.0f, 0.0f);
    GX2SetStreamOutEnable(GX2_FALSE);
    GX2SetTessellation(GX2_TESSELLATION_MODE_DISCRETE, GX2_PRIMITIVE_TESSELLATE_TRIANGLES, GX2_INDEX_FORMAT_U32);
    GX2SetMaxTessellationLevel(1.0f);
    GX2SetMinTessellationLevel(1.0f);
}

void RenderContext::SetupBlendType(BlendType blendType) const
{
    switch (blendType)
    {
    case EFT_BLEND_TYPE_NORMAL:
        GX2SetBlendControl(GX2_RENDER_TARGET_0,
                           GX2_BLEND_SRC_ALPHA,
                           GX2_BLEND_ONE_MINUS_SRC_ALPHA,
                           GX2_BLEND_COMBINE_ADD,
                           GX2_DISABLE,
                           GX2_BLEND_SRC_ALPHA,
                           GX2_BLEND_ONE_MINUS_SRC_ALPHA,
                           GX2_BLEND_COMBINE_ADD);
        break;
    case EFT_BLEND_TYPE_ADD:
        GX2SetBlendControl(GX2_RENDER_TARGET_0,
                           GX2_BLEND_SRC_ALPHA,
                           GX2_BLEND_ONE,
                           GX2_BLEND_COMBINE_ADD,
                           GX2_DISABLE,
                           GX2_BLEND_SRC_ALPHA,
                           GX2_BLEND_ONE,
                           GX2_BLEND_COMBINE_ADD);
        break;
    case EFT_BLEND_TYPE_SUB:
        GX2SetBlendControl(GX2_RENDER_TARGET_0,
                           GX2_BLEND_SRC_ALPHA,
                           GX2_BLEND_ONE,
                           GX2_BLEND_COMBINE_DST_MINUS_SRC,
                           GX2_DISABLE,
                           GX2_BLEND_SRC_ALPHA,
                           GX2_BLEND_ONE,
                           GX2_BLEND_COMBINE_DST_MINUS_SRC);
        break;
    case EFT_BLEND_TYPE_SCREEN:
        GX2SetBlendControl(GX2_RENDER_TARGET_0,
                           GX2_BLEND_ONE_MINUS_DST_COLOR,
                           GX2_BLEND_ONE,
                           GX2_BLEND_COMBINE_ADD,
                           GX2_DISABLE,
                           GX2_BLEND_ONE_MINUS_DST_COLOR,
                           GX2_BLEND_ONE,
                           GX2_BLEND_COMBINE_ADD);
        break;
    case EFT_BLEND_TYPE_MULT:
        GX2SetBlendControl(GX2_RENDER_TARGET_0,
                           GX2_BLEND_ZERO,
                           GX2_BLEND_SRC_COLOR ,
                           GX2_BLEND_COMBINE_ADD,
                           GX2_DISABLE,
                           GX2_BLEND_ONE_MINUS_DST_COLOR,
                           GX2_BLEND_ONE,
                           GX2_BLEND_COMBINE_ADD);
        break;
    }
}

void RenderContext::SetupZBufATest(ZBufATestType zBufATestType) const
{
    switch (zBufATestType)
    {
    case EFT_ZBUFF_ATEST_TYPE_NORMAL:
        GX2SetDepthOnlyControl(GX2_TRUE, GX2_FALSE, GX2_COMPARE_LEQUAL);
        GX2SetAlphaTest(GX2_TRUE, GX2_COMPARE_GREATER, 0.0f);
        GX2SetColorControl(GX2_LOGIC_OP_COPY, GX2_ENABLE, GX2_DISABLE, GX2_ENABLE);
        break;
    case EFT_ZBUFF_ATEST_TYPE_ZIGNORE:
        GX2SetDepthOnlyControl(GX2_FALSE, GX2_FALSE, GX2_COMPARE_LEQUAL);
        GX2SetAlphaTest(GX2_TRUE, GX2_COMPARE_GREATER, 0.0f);
        GX2SetColorControl(GX2_LOGIC_OP_COPY, GX2_ENABLE, GX2_DISABLE, GX2_ENABLE);
        break;
    case EFT_ZBUFF_ATEST_TYPE_ENTITY:
        GX2SetDepthOnlyControl(GX2_TRUE, GX2_TRUE, GX2_COMPARE_LEQUAL);
        GX2SetAlphaTest(GX2_TRUE, GX2_COMPARE_GREATER, 0.5f);
        GX2SetColorControl(GX2_LOGIC_OP_COPY, GX2_DISABLE, GX2_DISABLE, GX2_ENABLE);
        break;
    case EFT_ZBUFF_ATEST_TYPE_OPAQUE:
        GX2SetDepthOnlyControl(GX2_TRUE, GX2_TRUE, GX2_COMPARE_LEQUAL);
        GX2SetAlphaTest(GX2_FALSE, GX2_COMPARE_GREATER, 0.5f);
        GX2SetColorControl(GX2_LOGIC_OP_COPY, GX2_DISABLE, GX2_DISABLE, GX2_ENABLE);
        break;
    }
}

void RenderContext::SetupDisplaySideType(DisplaySideType displaySideType) const
{
    switch (displaySideType)
    {
    case EFT_DISPLAYSIDETYPE_BOTH:
        GX2SetCullOnlyControl(GX2_FRONT_FACE_CCW, GX2_FALSE, GX2_FALSE);
        break;
    case EFT_DISPLAYSIDETYPE_FRONT:
        GX2SetCullOnlyControl(GX2_FRONT_FACE_CCW, GX2_FALSE, GX2_TRUE);
        break;
    case EFT_DISPLAYSIDETYPE_BACK:
        GX2SetCullOnlyControl(GX2_FRONT_FACE_CCW, GX2_TRUE, GX2_FALSE);
        break;
    }
}

void RenderContext::_SetupFragmentTexture(const Texture texture, const Sampler sampler, TextureSlot slot, FragmentTextureLocation location) const
{
    if (texture == NULL || location.loc == EFT_INVALID_LOCATION)
        return;

    GX2SetPixelTexture(texture, location.loc);
    GX2SetPixelSampler(sampler, location.loc);
}

void RenderContext::_SetupFragment2DArrayTexture(const Texture texture, const Sampler sampler, TextureSlot slot, FragmentTextureLocation location) const
{
    _SetupFragmentTexture(texture, sampler, slot, location);
}

void RenderContext::_SetupFragmentCubeMapTexture(const Texture texture, const Sampler sampler, TextureSlot slot, FragmentTextureLocation location) const
{
    _SetupFragmentTexture(texture, sampler, slot, location);
}

void RenderContext::_SetupFragmentCubeMapArrayTexture(const Texture texture, const Sampler sampler, TextureSlot slot, FragmentTextureLocation location) const
{
    _SetupFragmentTexture(texture, sampler, slot, location);
}

void RenderContext::_SetupVertexTexture(const Texture texture, const Sampler sampler, TextureSlot slot, VertexTextureLocation location) const
{
    //if (texture == NULL || location.loc == EFT_INVALID_LOCATION)
    //    return;

    GX2SetVertexTexture(texture, location.loc);
    GX2SetVertexSampler(sampler, location.loc);
}

void RenderContext::_SetupVertex2DArrayTexture(const Texture texture, const Sampler sampler, TextureSlot slot, VertexTextureLocation location) const
{
    _SetupVertexTexture(texture, sampler, slot, location);
}

void RenderContext::SetupTexture(const TextureRes* texture, TextureSlot slot, FragmentTextureLocation location)
{
    mTextureSampler[slot].Setup(static_cast<TextureFilterMode>(texture->filterMode),
                                static_cast<TextureWrapMode>( texture->wrapMode       & 0xF),
                                static_cast<TextureWrapMode>((texture->wrapMode >> 4) & 0xF));
    mTextureSampler[slot].SetupLOD(texture->enableMipLevel, texture->mipMapBias);

    _SetupFragmentTexture(&texture->gx2Texture, mTextureSampler[slot].GetSampler(), slot, location);
}

void RenderContext::SetupTexture(const Texture texture, TextureSlot slot, FragmentTextureLocation location)
{
    _SetupFragmentTexture(texture, mDefaultTextureSampler.GetSampler(), slot, location);
}

void RenderContext::SetupUserFragmentTexture(const Texture texture, const Sampler sampler, TextureSlot slot, FragmentTextureLocation location) const
{
    _SetupFragmentTexture(texture, sampler, slot, location);
}

void RenderContext::SetupUserFragmentCubeMapTexture(const Texture texture, const Sampler sampler, TextureSlot slot, FragmentTextureLocation location) const
{
    _SetupFragmentCubeMapTexture(texture, sampler, slot, location);
}

void RenderContext::SetupUserFragment2DArrayTexture(const Texture texture, TextureSlot slot, FragmentTextureLocation location) const
{
    _SetupFragment2DArrayTexture(texture, mDefaultTextureSampler.GetSampler(), slot, location);
}

void RenderContext::SetupUserFragment2DArrayTexture(const Texture texture, const Sampler sampler, TextureSlot slot, FragmentTextureLocation location) const
{
    _SetupFragment2DArrayTexture(texture, sampler, slot, location);
}

void RenderContext::SetupUserFragmentCubeMapArrayTexture(const Texture texture, const Sampler sampler, TextureSlot slot, FragmentTextureLocation location) const
{
    _SetupFragmentCubeMapArrayTexture(texture, sampler, slot, location);
}

void RenderContext::SetupUserVertexTexture(const Texture texture, TextureSlot slot, VertexTextureLocation location) const
{
    _SetupVertexTexture(texture, mDefaultTextureSampler.GetSampler(), slot, location);
}

void RenderContext::SetupUserVertexTexture(const Texture texture, const Sampler sampler, TextureSlot slot, VertexTextureLocation location) const
{
    _SetupVertexTexture(texture, sampler, slot, location);
}

void RenderContext::SetupUserVertexArrayTexture(const Texture texture, const Sampler sampler, TextureSlot slot, VertexTextureLocation location) const
{
    _SetupVertex2DArrayTexture(texture, sampler, slot, location);
}

void RenderContext::SetupVertexArrayTexture(const Texture texture, TextureSlot slot, VertexTextureLocation location) const
{
    _SetupVertex2DArrayTexture(texture, m2DArrayTextureSampler.GetSampler(), slot, location);
}

} } // namespace nw::eft
