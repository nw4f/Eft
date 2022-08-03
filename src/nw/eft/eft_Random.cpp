#include <nw/eft/eft_Heap.h>
#include <nw/eft/eft_Random.h>

namespace nw { namespace eft {

math::VEC3* PtclRandom::mVec3Tbl;
math::VEC3* PtclRandom::mNormalizedVec3Tbl;

Random PtclRandom::gRandom;

Random* PtclRandom::GetGlobalRandom()
{
    return &PtclRandom::gRandom;
}

void PtclRandom::CreateRandomTbl(Heap* heap)
{
    Random rnd(12345679);

    mVec3Tbl           = static_cast<math::VEC3*>(heap->Alloc(sizeof(math::VEC3) * cNumVec3Tbl));
    mNormalizedVec3Tbl = static_cast<math::VEC3*>(heap->Alloc(sizeof(math::VEC3) * cNumVec3Tbl));

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

void PtclRandom::DestroyRandomTbl(Heap* heap)
{
    heap->Free(mVec3Tbl);
    heap->Free(mNormalizedVec3Tbl);
}

} } // namespace nw::eft
