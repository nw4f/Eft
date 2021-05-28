#ifndef EFT_ENUM_H_
#define EFT_ENUM_H_

namespace nw { namespace eft {

enum BlendType
{
    BlendType_Type0 = 0,
    BlendType_Type1 = 1,
    BlendType_Type2 = 2,
    BlendType_Type3 = 3,
    BlendType_Type4 = 4,
    BlendType_Max = 5,
};

enum CpuCore
{
    CpuCore_0 = 0,
    CpuCore_1 = 1,
    CpuCore_2 = 2,
    CpuCore_Max = 3,
};

enum CustomActionCallBackID
{
    CustomActionCallBackID_Invalid = 0xFFFFFFFF,
    CustomActionCallBackID_Max = 8,
};

enum CustomShaderCallBackID
{
    CustomShaderCallBackID_Max = 9,
};

enum ZBufATestType
{
    ZBufATestType_Type0 = 0,
    ZBufATestType_Type1 = 1,
    ZBufATestType_Type2 = 2,
    ZBufATestType_Max = 3,
};

} } // namespace nw::eft

#endif // EFT_ENUM_H_
