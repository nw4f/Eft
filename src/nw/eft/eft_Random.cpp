#include <nw/eft/eft_Misc.h>
#include <nw/eft/eft_Random.h>

namespace nw { namespace eft {

nw::math::VEC3* PtclRandom::mVec3Tbl           = NULL;
nw::math::VEC3* PtclRandom::mNormalizedVec3Tbl = NULL;

Random PtclRandom::gRandom;

Random* PtclRandom::GetGlobalRandom()
{
    return &PtclRandom::gRandom;
}

void PtclRandom::Initialize()
{
    Random rnd(12345679);

    mVec3Tbl           = static_cast<nw::math::VEC3*>(AllocFromStaticHeap(sizeof(nw::math::VEC3) * cNumVec3Tbl));
    mNormalizedVec3Tbl = static_cast<nw::math::VEC3*>(AllocFromStaticHeap(sizeof(nw::math::VEC3) * cNumVec3Tbl));

    for (s32 i = 0; i < cNumVec3Tbl; i++)
    {
        mVec3Tbl[i].x = /* rnd.GetF32Range(-1.0f, 1.0f) */ rnd.GetF32() * 2.0f - 1.0f;
        mVec3Tbl[i].y = /* rnd.GetF32Range(-1.0f, 1.0f) */ rnd.GetF32() * 2.0f - 1.0f;
        mVec3Tbl[i].z = /* rnd.GetF32Range(-1.0f, 1.0f) */ rnd.GetF32() * 2.0f - 1.0f;

        mNormalizedVec3Tbl[i].x = /* rnd.GetF32Range(-1.0f, 1.0f) */ rnd.GetF32() * 2.0f - 1.0f;
        mNormalizedVec3Tbl[i].y = /* rnd.GetF32Range(-1.0f, 1.0f) */ rnd.GetF32() * 2.0f - 1.0f;
        mNormalizedVec3Tbl[i].z = /* rnd.GetF32Range(-1.0f, 1.0f) */ rnd.GetF32() * 2.0f - 1.0f;
        mNormalizedVec3Tbl[i].Normalize();
    }
}

void PtclRandom::Finalize()
{
    FreeFromStaticHeap(mVec3Tbl);
    FreeFromStaticHeap(mNormalizedVec3Tbl);

    mVec3Tbl = NULL;
    mNormalizedVec3Tbl = NULL;
}

} } // namespace nw::eft
