#ifndef EFT_EMITTER_FIELD_CURL_NOISE_H_
#define EFT_EMITTER_FIELD_CURL_NOISE_H_

#include <nw/eft/eft_RenderContext.h>

namespace nw { namespace eft {

void InitializeCurlNoise();
void FinalizeCurlNoise();
Texture GetCurlNoiseTexture();

class CurlNoiseTexture
{
public:
    void Initialize();
    void Finalize();
    void Invalidate();

    Texture GetTexture()
    {
        return &gx2Texture;
    }

    GX2Texture  gx2Texture;
    void*       textureImage;
};
static_assert(sizeof(CurlNoiseTexture) == 0xA0, "nw::eft::CurlNoiseTexture size mismatch");

class CurlNoiseTbl
{
public:
    struct Vec3S8
    {
        s8 x;
        s8 y;
        s8 z;
    };

    void Initialize(s32 bufWidth, void* pTbl)
    {
        m_BufWidth = bufWidth;
        m_pCurlNoiseBufS8 = (Vec3S8*)pTbl;
    }

    nw::math::Vector3 GetCurlNoiseS8(s32 x, s32 y, s32 z)
    {
        if (x < 0) x *= -1;
        if (y < 0) y *= -1;
        if (z < 0) z *= -1;
        x %= m_BufWidth;
        y %= m_BufWidth;
        z %= m_BufWidth;

        s32 idx = x * m_BufWidth * m_BufWidth + y * m_BufWidth + z;
        nw::math::Vector3 ret;
        ret.x = m_pCurlNoiseBufS8[idx].x / 127.0f;
        ret.y = m_pCurlNoiseBufS8[idx].y / 127.0f;
        ret.z = m_pCurlNoiseBufS8[idx].z / 127.0f;
        return ret;
    }

    nw::math::Vector3 GetCurlNoiseS8Interpolate(f32 x, f32 y, f32 z)
    {
        s32 ix = (s32)x;
        s32 iy = (s32)y;
        s32 iz = (s32)z;
        f32 dx = x - (f32)ix;
        f32 dy = y - (f32)iy;
        f32 dz = z - (f32)iz;
        f32 dxi = 1.0f - dx;
        f32 dyi = 1.0f - dy;
        f32 dzi = 1.0f - dz;

        nw::math::Vector3 ret000 = GetCurlNoiseS8(ix,     iy,     iz);
        nw::math::Vector3 ret001 = GetCurlNoiseS8(ix + 1, iy,     iz);
        nw::math::Vector3 ret010 = GetCurlNoiseS8(ix,     iy + 1, iz);
        nw::math::Vector3 ret011 = GetCurlNoiseS8(ix + 1, iy + 1, iz);

        nw::math::Vector3 ret100 = GetCurlNoiseS8(ix,     iy,     iz + 1);
        nw::math::Vector3 ret101 = GetCurlNoiseS8(ix + 1, iy,     iz + 1);
        nw::math::Vector3 ret110 = GetCurlNoiseS8(ix,     iy + 1, iz + 1);
        nw::math::Vector3 ret111 = GetCurlNoiseS8(ix + 1, iy + 1, iz + 1);

        nw::math::Vector3 ret00x = ret000 * dxi + ret001 * dx;
        nw::math::Vector3 ret01x = ret010 * dxi + ret011 * dx;
        nw::math::Vector3 ret0y  = ret00x * dyi + ret01x * dy;

        nw::math::Vector3 ret10x = ret100 * dxi + ret101 * dx;
        nw::math::Vector3 ret11x = ret110 * dxi + ret111 * dx;
        nw::math::Vector3 ret1y  = ret10x * dyi + ret11x * dy;

        nw::math::Vector3 ret  = ret0y * dzi + ret1y * dz;
        return ret;
    }

    s32     m_BufWidth;
    Vec3S8* m_pCurlNoiseBufS8;
};
static_assert(sizeof(CurlNoiseTbl) == 8, "nw::eft::CurlNoiseTbl size mismatch");

} } // namespace nw::eft

#endif // EFT_EMITTER_FIELD_CURL_NOISE_H_
