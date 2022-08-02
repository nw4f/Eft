#ifndef EFT_ENUM_H_
#define EFT_ENUM_H_

#include <nw/typeDef.h>

namespace nw { namespace eft {

enum SystemConstants
{
    EFT_BINARY_VERSION                  = 0x00000041,
    EFT_EMITTER_INSET_NUM               = 16,
    EFT_TEXTURE_PATTERN_NUM             = 32 ,
    EFT_INFINIT_LIFE                    = 0x7fffffff,
    EFT_GROUP_MAX                       = 64,
    EFT_CALLBACK_MAX                    = 8,
    EFT_PTCL_BINARY_ALIGNMENT           = (16 * 1024),
    EFT_DEFAULT_DRAW_PRIORITY           = 128,
    EFT_CUSTOM_SHADER_PARAM_NUM         = 32,
    EFT_EMITTER_RELEASE_DELAY_COUNTER   = 3,

    EFT_SYSTEM_CONSTANTS_FORCE_4BYTE    = 0x80000000
};

enum CpuCore
{
    EFT_CPU_CORE_0 = 0,
    EFT_CPU_CORE_1 = 1,
    EFT_CPU_CORE_2 = 2,
    EFT_CPU_CORE_MAX = 3,
    EFT_CPU_CORE_FORCE_4BYTE = 0x80000000
};
static_assert(sizeof(CpuCore) == 4, "nw::eft::CpuCore size mismatch");

enum EmitterType
{
    EFT_EMITTER_TYPE_SIMPLE     = 0 ,
    EFT_EMITTER_TYPE_COMPLEX,
    EFT_EMITTER_TYPE_SIMPLE_GPU,
    EFT_EMITTER_TYPE_MAX,
    EFT_EMITTER_TYPE_FORCE_4BYTE = 0x80000000
};
static_assert(sizeof(EmitterType) == 4, "nw::eft::EmitterType size mismatch");

enum PtclType
{
    EFT_PTCL_TYPE_SIMPLE     = 0 ,
    EFT_PTCL_TYPE_COMPLEX,
    EFT_PTCL_TYPE_CHILD,
    EFT_PTCL_TYPE_COMPACT,
    EFT_PTCL_TYPE_MAX,
    EFT_PTCL_TYPE_FORCE_4BYTE = 0x80000000
};
static_assert(sizeof(PtclType) == 4, "nw::eft::PtclType size mismatch");

enum PtclFollowType
{
    EFT_FOLLOW_TYPE_ALL = 0,
    EFT_FOLLOW_TYPE_NONE,
    EFT_FOLLOW_TYPE_POS_ONLY,
    EFT_FOLLOW_TYPE_MAX,
    EFT_FOLLOW_TYPE_FORCE_4BYTE = 0x80000000
};
static_assert(sizeof(PtclFollowType) == 4, "nw::eft::PtclFollowType size mismatch");

enum CustomActionCallBackID
{
    EFT_CUSTOM_ACTION_CALLBACK_ID_NONE  = 0,
    EFT_CUSTOM_ACTION_CALLBACK_ID_0     = 1,
    EFT_CUSTOM_ACTION_CALLBACK_ID_1     = 2,
    EFT_CUSTOM_ACTION_CALLBACK_ID_2     = 3,
    EFT_CUSTOM_ACTION_CALLBACK_ID_3     = 4,
    EFT_CUSTOM_ACTION_CALLBACK_ID_4     = 5,
    EFT_CUSTOM_ACTION_CALLBACK_ID_5     = 6,
    EFT_CUSTOM_ACTION_CALLBACK_ID_6     = 7,
    EFT_CUSTOM_ACTION_CALLBACK_ID_7     = 8,
    EFT_CUSTOM_ACTION_CALLBACK_ID_MAX   = 9,

    EFT_USER_CALLBACK_ID_FORCE_4BYTE = 0x80000000
};
static_assert(sizeof(CustomActionCallBackID) == 4, "nw::eft::CustomActionCallBackID size mismatch");

enum CustomActionFlag
{
    EFT_CUSTOM_ACTION_FLAG_0  = 0x01 << 16,
    EFT_CUSTOM_ACTION_FLAG_1  = 0x01 << 17,
    EFT_CUSTOM_ACTION_FLAG_2  = 0x01 << 18,
    EFT_CUSTOM_ACTION_FLAG_3  = 0x01 << 19,
    EFT_CUSTOM_ACTION_FLAG_4  = 0x01 << 20,
    EFT_CUSTOM_ACTION_FLAG_5  = 0x01 << 21,
    EFT_CUSTOM_ACTION_FLAG_6  = 0x01 << 22,
    EFT_CUSTOM_ACTION_FLAG_7  = 0x01 << 23,
    EFT_CUSTOM_ACTION_FLAG_8  = 0x01 << 24,
    EFT_CUSTOM_ACTION_FLAG_9  = 0x01 << 25,
    EFT_CUSTOM_ACTION_FLAG_10 = 0x01 << 26,
    EFT_CUSTOM_ACTION_FLAG_11 = 0x01 << 27,
    EFT_CUSTOM_ACTION_FLAG_12 = 0x01 << 28,
    EFT_CUSTOM_ACTION_FLAG_13 = 0x01 << 29,
    EFT_CUSTOM_ACTION_FLAG_14 = 0x01 << 30,
    EFT_CUSTOM_ACTION_FLAG_15 = 0x01 << 31,
};

enum CustomActionParamIdx
{
    EFT_CUSTOM_ACTION_PARAM_0   = 0,
    EFT_CUSTOM_ACTION_PARAM_1   = 1,
    EFT_CUSTOM_ACTION_PARAM_2   = 2,
    EFT_CUSTOM_ACTION_PARAM_3   = 3 ,
    EFT_CUSTOM_ACTION_PARAM_4   = 4 ,
    EFT_CUSTOM_ACTION_PARAM_5   = 5 ,
    EFT_CUSTOM_ACTION_PARAM_6   = 6 ,
    EFT_CUSTOM_ACTION_PARAM_7   = 7 ,
    EFT_CUSTOM_ACTION_PARAM_MAX = 8,
};

typedef u64 GroupFlag;
#define EFT_GROUP_FLAG(groupID) (u64)((u64)0x1 << (u64)groupID)

enum DrawPathFlag
{
    EFT_DRAW_PATH_FLAG_0  = 0x01 << 0,
    EFT_DRAW_PATH_FLAG_1  = 0x01 << 1,
    EFT_DRAW_PATH_FLAG_2  = 0x01 << 2,
    EFT_DRAW_PATH_FLAG_3  = 0x01 << 3,
    EFT_DRAW_PATH_FLAG_4  = 0x01 << 4,
    EFT_DRAW_PATH_FLAG_5  = 0x01 << 5,
    EFT_DRAW_PATH_FLAG_6  = 0x01 << 6,
    EFT_DRAW_PATH_FLAG_7  = 0x01 << 7,
    EFT_DRAW_PATH_FLAG_8  = 0x01 << 8,
    EFT_DRAW_PATH_FLAG_9  = 0x01 << 9,
    EFT_DRAW_PATH_FLAG_10  = 0x01 << 10,
    EFT_DRAW_PATH_FLAG_11  = 0x01 << 11,
    EFT_DRAW_PATH_FLAG_12  = 0x01 << 12,
    EFT_DRAW_PATH_FLAG_13  = 0x01 << 13,
    EFT_DRAW_PATH_FLAG_14  = 0x01 << 14,
    EFT_DRAW_PATH_FLAG_15  = 0x01 << 15,
    EFT_DRAW_PATH_FLAG_16  = 0x01 << 16,
    EFT_DRAW_PATH_FLAG_17  = 0x01 << 17,
    EFT_DRAW_PATH_FLAG_18  = 0x01 << 18,
    EFT_DRAW_PATH_FLAG_19  = 0x01 << 19,
    EFT_DRAW_PATH_FLAG_20  = 0x01 << 20,
    EFT_DRAW_PATH_FLAG_21  = 0x01 << 21,
    EFT_DRAW_PATH_FLAG_22  = 0x01 << 22,
    EFT_DRAW_PATH_FLAG_23  = 0x01 << 23,
    EFT_DRAW_PATH_FLAG_24  = 0x01 << 24,
    EFT_DRAW_PATH_FLAG_25  = 0x01 << 25,
    EFT_DRAW_PATH_FLAG_26  = 0x01 << 26,
    EFT_DRAW_PATH_FLAG_27  = 0x01 << 27,
    EFT_DRAW_PATH_FLAG_28  = 0x01 << 28,
    EFT_DRAW_PATH_FLAG_29  = 0x01 << 29,
    EFT_DRAW_PATH_FLAG_30  = 0x01 << 30,
    EFT_DRAW_PATH_FLAG_31  = 0x01 << 31,

    EFT_DRAW_PATH_FLAG_FORCE_4BYTE = 0x80000000
};
static_assert(sizeof(DrawPathFlag) == 8, "nw::eft::DrawPathFlag size mismatch");

enum
{
    EFT_INVALID_EMITTER_SET_ID  = 0xFFFFFFFF,
    EFT_INVALID_EMITTER_ID      = 0xFFFFFFFF,
    EFT_INVALID_ATTRIBUTE       = 0xFFFFFFFF,
    EFT_INVALID_LOCATION        = 0xFFFFFFFF,
    EFT_INVALID_SAMPLER         = 0xFFFFFFFF,
};

enum CustomShaderCallBackID
{
    EFT_CUSTOM_SHADER_CALLBACK_NONE  = 0,
    EFT_CUSTOM_SHADER_CALLBACK_1     = 1,
    EFT_CUSTOM_SHADER_CALLBACK_2     = 2,
    EFT_CUSTOM_SHADER_CALLBACK_3     = 3,
    EFT_CUSTOM_SHADER_CALLBACK_4     = 4,
    EFT_CUSTOM_SHADER_CALLBACK_5     = 5,
    EFT_CUSTOM_SHADER_CALLBACK_6     = 6,
    EFT_CUSTOM_SHADER_CALLBACK_7     = 7,
    EFT_CUSTOM_SHADER_CALLBACK_8     = 8,
    EFT_CUSTOM_SHADER_CALLBACK_MAX   = 9,

    EFT_CUSTOM_SHADER_CALLBACK_FORCE_4BYTE = 0x80000000
};
static_assert(sizeof(CustomShaderCallBackID) == 4, "nw::eft::CustomShaderCallBackID size mismatch");

enum DrawPathCallBackID
{
    EFT_DRAW_PATH_CALLBACK_0     = 0,
    EFT_DRAW_PATH_CALLBACK_1     = 1,
    EFT_DRAW_PATH_CALLBACK_2     = 2,
    EFT_DRAW_PATH_CALLBACK_3     = 3,
    EFT_DRAW_PATH_CALLBACK_4     = 4,
    EFT_DRAW_PATH_CALLBACK_5     = 5,
    EFT_DRAW_PATH_CALLBACK_6     = 6,
    EFT_DRAW_PATH_CALLBACK_7     = 7,
    EFT_DRAW_PATH_CALLBACK_MAX   = 8,

    EFT_DRAW_PATH_CALLBACK_FORCE_4BYTE = 0x80000000
};
static_assert(sizeof(DrawPathCallBackID) == 4, "nw::eft::DrawPathCallBackID size mismatch");

enum TextureSlot
{
    EFT_TEXTURE_SLOT_0                  = 0,
    EFT_TEXTURE_SLOT_1                  = 1,
    EFT_TEXTURE_SLOT_2                  = 2,
    EFT_TEXTURE_SLOT_BIN_MAX            = 3,
    EFT_TEXTURE_SLOT_FRAME_BUFFER       = EFT_TEXTURE_SLOT_BIN_MAX,
    EFT_TEXTURE_SLOT_DEPTH_BUFFER       = 4,
    EFT_TEXTURE_SLOT_CURL_NOISE         = 5,
    EFT_TEXTURE_SLOT_CUBE_MAP           = 6,
    EFT_USER_TEXTURE_SLOT_0             = 7,
    EFT_USER_TEXTURE_SLOT_1             = 8,
    EFT_USER_TEXTURE_SLOT_2             = 9,
    EFT_USER_TEXTURE_SLOT_3             = 10,
    EFT_USER_TEXTURE_SLOT_4             = 11,
    EFT_USER_TEXTURE_SLOT_5             = 12,
    EFT_USER_TEXTURE_SLOT_6             = 13,
    EFT_TEXTURE_SLOT_MAX                = 14,
    EFT_USER_TEXTURE_SLOT_FORCE_4BYTE   = 0x80000000
};
static_assert(sizeof(TextureSlot) == 4, "nw::eft::TextureSlot size mismatch");

enum TextureWrapMode
{
    EFT_TEXTURE_WRAP_TYPE_MIRROR,
    EFT_TEXTURE_WRAP_TYPE_REPEAT,
    EFT_TEXTURE_WRAP_TYPE_CLAMP,
    EFT_TEXTURE_WRAP_TYPE_MIROOR_ONCE,
    EFT_TEXTURE_WRAP_TYPE_MAX,

    EFT_TEXTURE_WRAP_MODE_FORCE_4BYTE = 0x80000000
};
static_assert(sizeof(TextureWrapMode) == 4, "nw::eft::TextureWrapMode size mismatch");

enum TextureFilterMode
{
    EFT_TEXTURE_FILTER_TYPE_LINEAR,
    EFT_TEXTURE_FILTER_TYPE_NEAR,

    EFT_TEXTURE_FILTER_TYPE_FORCE_4BYTE = 0x80000000
};
static_assert(sizeof(TextureFilterMode) == 4, "nw::eft::TextureFilterMode size mismatch");

enum DrawViewFlag
{
    EFT_DRAW_VIEW_FLAG_0          = ( 0x01 << 0 ),
    EFT_DRAW_VIEW_FLAG_1          = ( 0x01 << 1 ),
    EFT_DRAW_VIEW_FLAG_2          = ( 0x01 << 2 ),
    EFT_DRAW_VIEW_FLAG_3          = ( 0x01 << 3 ),
    EFT_DRAW_VIEW_FLAG_4          = ( 0x01 << 4 ),
    EFT_DRAW_VIEW_FLAG_5          = ( 0x01 << 5 ),
    EFT_DRAW_VIEW_FLAG_ALL        = EFT_DRAW_VIEW_FLAG_0 |
                                    EFT_DRAW_VIEW_FLAG_1 |
                                    EFT_DRAW_VIEW_FLAG_2 |
                                    EFT_DRAW_VIEW_FLAG_3 |
                                    EFT_DRAW_VIEW_FLAG_4 |
                                    EFT_DRAW_VIEW_FLAG_5,
    EFT_DRAW_VIEW_FLAG_NONE       = 0,

    EFT_DRAW_VIEW_FLAG_FORCE_4BYTE = 0x80000000
};
static_assert(sizeof(DrawViewFlag) == 4, "nw::eft::DrawViewFlag size mismatch");

enum ShaderType
{
    EFT_SHADER_TYPE_NORMAL      = 0,
    EFT_SHADER_TYPE_USER_DEF1   = 1,
    EFT_SHADER_TYPE_USER_DEF2   = 2,
    EFT_SHADER_TYPE_MAX         = 3,

    EFT_SHADER_TYPE_FORCE_4BYTE = 0x80000000
};
static_assert(sizeof(ShaderType) == 4, "nw::eft::ShaderType size mismatch");

enum BlendType
{
    EFT_BLEND_TYPE_NORMAL = 0,
    EFT_BLEND_TYPE_ADD,
    EFT_BLEND_TYPE_SUB,
    EFT_BLEND_TYPE_SCREEN,
    EFT_BLEND_TYPE_MULT,
    EFT_BLEND_TYPE_NONE,
    EFT_BLEND_TYPE_MAX,
    EFT_BLEND_TYPE_FORCE_4BYTE = 0x80000000
};
static_assert(sizeof(BlendType) == 4, "nw::eft::BlendType size mismatch");

enum ZBufATestType
{
    EFT_ZBUFF_ATEST_TYPE_NORMAL = 0,
    EFT_ZBUFF_ATEST_TYPE_ZIGNORE,
    EFT_ZBUFF_ATEST_TYPE_ENTITY,
    EFT_ZBUFF_ATEST_TYPE_OPAQUE,
    EFT_ZBUFF_ATEST_TYPE_MAX,
    EFT_ZBUFF_ATEST_TYPE_FORCE_4BYTE = 0x80000000
};
static_assert(sizeof(ZBufATestType) == 4, "nw::eft::ZBufATestType size mismatch");

enum ColorKind
{
    EFT_COLOR_KIND_0    = 0,
    EFT_COLOR_KIND_1    = 1,
    EFT_COLOR_KIND_MAX  = 2,
    EFT_COLOR_KIND_FORCE_4BYTE = 0x80000000
};
static_assert(sizeof(ColorKind) == 4, "nw::eft::ColorKind size mismatch");

enum AlphaKind
{
    EFT_ALPHA_KIND_0                    = 0,
    EFT_ALPHA_KIND_1                    = 1,
    EFT_ALPHA_KIND_MAX                  = 2,
    EFT_ALPHA_KIND_FORCE_4BYTE          = 0x80000000
};
static_assert(sizeof(AlphaKind) == 4, "nw::eft::AlphaKind size mismatch");

enum PtclRotType
{
    EFT_ROT_TYPE_NO_WORK   = 0 ,
    EFT_ROT_TYPE_ROTX,
    EFT_ROT_TYPE_ROTY,
    EFT_ROT_TYPE_ROTZ,
    EFT_ROT_TYPE_ROTYZX,
    EFT_ROT_TYPE_ROTXYZ,
    EFT_ROT_TYPE_ROTZXY,
    EFT_ROT_TYPE_MAX,
    EFT_ROT_TYPE_FORCE_4BYTE = 0x80000000
};
static_assert(sizeof(PtclRotType) == 4, "nw::eft::PtclRotType size mismatch");

enum MeshType
{
    EFT_MESH_TYPE_PARTICLE = 0,
    EFT_MESH_TYPE_PRIMITIVE,
    EFT_MESH_TYPE_STRIPE,
    EFT_MESH_TYPE_FORCE_4BYTE = 0x80000000
};
static_assert(sizeof(MeshType) == 4, "nw::eft::MeshType size mismatch");

enum BillboardType
{
    EFT_BILLBOARD_TYPE_BILLBOARD = 0,
    EFT_BILLBOARD_TYPE_POLYGON_XY,
    EFT_BILLBOARD_TYPE_POLYGON_XZ,
    EFT_BILLBOARD_TYPE_VEL_LOOK,
    EFT_BILLBOARD_TYPE_VEL_LOOK_POLYGON,
    EFT_BILLBOARD_TYPE_STRIPE,
    EFT_BILLBOARD_TYPE_COMPLEX_STRIPE,
    EFT_BILLBOARD_TYPE_PRIMITIVE,
    EFT_BILLBOARD_TYPE_Y_BILLBOARD,
    EFT_BILLBOARD_TYPE_COMPLEX_BILLBOARD,

    EFT_BILLBOARD_TYPE_MAX,
    EFT_BILLBOARD_TYPE_FORCE_4BYTE = 0x80000000
};
static_assert(sizeof(BillboardType) == 4, "nw::eft::BillboardType size mismatch");

enum OffsetType
{
    EFT_OFFSET_TYPE_CAMERA_DIR = 0,
    EFT_OFFSET_TYPE_DEPTH,
    EFT_OFFSET_TYPE_FORCE_4BYTE = 0x80000000
};
static_assert(sizeof(OffsetType) == 4, "nw::eft::OffsetType size mismatch");

enum DisplaySideType
{
    EFT_DISPLAYSIDETYPE_BOTH = 0,
    EFT_DISPLAYSIDETYPE_FRONT,
    EFT_DISPLAYSIDETYPE_BACK,

    EFT_DISPLAYSIDETYPE_FORCE_4BYTE = 0x80000000
};
static_assert(sizeof(DisplaySideType) == 4, "nw::eft::DisplaySideType size mismatch");

enum EmitterFlg
{
    EFT_EMITTER_FLAG_COLOR_RANDOM               = ( 1 << 0 ) ,
    EFT_EMITTER_FLAG_DISP_EMITTER_MATRIX        = ( 1 << 1 ) ,
    EFT_EMITTER_FLAG_DISP_VOLUME                = ( 1 << 2 ) ,
    EFT_EMITTER_FLAG_DISP_FIELD_MAGNET          = ( 1 << 3 ) ,
    EFT_EMITTER_FLAG_DISP_FIELD_SPIN            = ( 1 << 4 ) ,
    EFT_EMITTER_FLAG_DISP_FIELD_COLLISION       = ( 1 << 5 ) ,
    EFT_EMITTER_FLAG_DISP_FIELD_CONVERGENCE     = ( 1 << 6 ) ,
    EFT_EMITTER_FLAG_ENABLE_FOG                 = ( 1 << 7 ) ,
    EFT_EMITTER_FLAG_ENABLE_SOFTEDGE            = ( 1 << 8 ) ,
    EFT_EMITTER_FLAG_ENABLE_SORTPARTICLE        = ( 1 << 9 ) ,
    EFT_EMITTER_FLAG_REVERSE_ORDER_PARTICLE     = ( 1 << 10 ) ,

    EFT_EMITTER_FLAG_TEXTURE0_COLOR_ONE         = ( 1 << 11 ) ,
    EFT_EMITTER_FLAG_TEXTURE1_COLOR_ONE         = ( 1 << 12 ) ,
    EFT_EMITTER_FLAG_PRIMITIVE_COLOR_ONE        = ( 1 << 13 ) ,
    EFT_EMITTER_FLAG_TEXTURE0_ALPHA_ONE         = ( 1 << 14 ) ,
    EFT_EMITTER_FLAG_TEXTURE1_ALPHA_ONE         = ( 1 << 15 ) ,
    EFT_EMITTER_FLAG_PRIMITIVE_ALPHA_ONE        = ( 1 << 16 ) ,
    EFT_EMITTER_FLAG_TEXTURE2_COLOR_ONE         = ( 1 << 17 ) ,
    EFT_EMITTER_FLAG_TEXTURE2_ALPHA_ONE         = ( 1 << 18 ) ,
    EFT_EMITTER_FLAG_TEXTURE0_ALPHA_RED         = ( 1 << 19 ) ,
    EFT_EMITTER_FLAG_TEXTURE1_ALPHA_RED         = ( 1 << 20 ) ,
    EFT_EMITTER_FLAG_TEXTURE2_ALPHA_RED         = ( 1 << 21 ) ,

    EFT_EMITTER_FLAG_USE_COMPACT_EMITTER        = ( 1 << 22 ) ,
    EFT_EMITTER_FLAG_USE_BEHAVIOR_CALC_GPU      = ( 1 << 23 ) ,

    EFT_EMITTER_FLAG_DISP_FIELD_MASK = EFT_EMITTER_FLAG_DISP_FIELD_MAGNET |
    EFT_EMITTER_FLAG_DISP_FIELD_SPIN |
    EFT_EMITTER_FLAG_DISP_FIELD_COLLISION |
    EFT_EMITTER_FLAG_DISP_FIELD_CONVERGENCE,

    EFT_EMITTER_FLAG_ALL                        = 0xffffffff
};
static_assert(sizeof(EmitterFlg) == 4, "nw::eft::EmitterFlg size mismatch");

enum ParticleBehaviorFlag
{
    EFT_PARTICLE_BEHAVIOR_FLAG_AIR_REGIST                = ( 1 << 0 ) ,
    EFT_PARTICLE_BEHAVIOR_FLAG_GRAVITY                   = ( 1 << 1 ) ,
    EFT_PARTICLE_BEHAVIOR_FLAG_ROTATE                    = ( 1 << 2 ) ,
    EFT_PARTICLE_BEHAVIOR_FLAG_ROT_REGIST                = ( 1 << 3 ) ,
    EFT_PARTICLE_BEHAVIOR_FLAG_WLD_POSDIF                = ( 1 << 4 ) ,
    EFT_PARTICLE_BEHAVIOR_FLAG_LCL_POSDIF                = ( 1 << 5 ) ,
    EFT_PARTICLE_BEHAVIOR_FLAG_SCALE_ANIM                = ( 1 << 6 ) ,
    EFT_PARTICLE_BEHAVIOR_FLAG_TEXTURE0_UV_ANIM          = ( 1 << 11 ) ,
    EFT_PARTICLE_BEHAVIOR_FLAG_TEXTURE1_UV_ANIM          = ( 1 << 12 ) ,
    EFT_PARTICLE_BEHAVIOR_FLAG_TEXTURE2_UV_ANIM          = ( 1 << 13 ) ,
    EFT_PARTICLE_BEHAVIOR_FLAG_TEXTURE0_PTN_ANIM         = ( 1 << 14 ) ,
    EFT_PARTICLE_BEHAVIOR_FLAG_TEXTURE1_PTN_ANIM         = ( 1 << 15 ) ,
    EFT_PARTICLE_BEHAVIOR_FLAG_TEXTURE2_PTN_ANIM         = ( 1 << 16 ) ,
    EFT_PARTICLE_BEHAVIOR_FLAG_TEXTURE0_SURFACE_ANIM     = ( 1 << 17 ) ,
    EFT_PARTICLE_BEHAVIOR_FLAG_TEXTURE1_SURFACE_ANIM     = ( 1 << 18 ) ,
    EFT_PARTICLE_BEHAVIOR_FLAG_VELOCITY_APPLY_SCALE_Y    = ( 1 << 19 ) ,

    EFT_PARTICLE_BEHAVIOR_FLAG_FORCE_4BYTE = 0x80000000
};
static_assert(sizeof(ParticleBehaviorFlag) == 4, "nw::eft::ParticleBehaviorFlag size mismatch");

enum EmitterBehaviorFlag
{
    EFT_EMITTER_BEHAVIOR_FLAG_MATRIX_TRANS_ANIM         = ( 1 << 0 ) ,
    EFT_EMITTER_BEHAVIOR_FLAG_MATRIX_ROTATE_ANIM        = ( 1 << 1 ) ,
    EFT_EMITTER_BEHAVIOR_FLAG_MATRIX_ANIM               = ( 1 << 2 ) ,
    EFT_EMITTER_BEHAVIOR_FLAG_IS_EMIT                   = ( 1 << 3 ) ,
    EFT_EMITTER_BEHAVIOR_FLAG_IS_CALCULATED             = ( 1 << 4 ) ,
    EFT_EMITTER_BEHAVIOR_FLAG_IS_SET_PREV_POS           = ( 1 << 5 ) ,
};
static_assert(sizeof(EmitterBehaviorFlag) == 4, "nw::eft::EmitterBehaviorFlag size mismatch");

enum ShaderEnableAttrFlg
{
    EFT_SHADER_ATTRIBUTE_HAS_SCALE          = ( 1 << 0 ) ,
    EFT_SHADER_ATTRIBUTE_HAS_WORLD_POS      = ( 1 << 1 ) ,
    EFT_SHADER_ATTRIBUTE_HAS_WORLD_POS_DIFF = ( 1 << 2 ) ,
    EFT_SHADER_ATTRIBUTE_HAS_COLOR0         = ( 1 << 3 ) ,
    EFT_SHADER_ATTRIBUTE_HAS_COLOR1         = ( 1 << 4 ) ,
    EFT_SHADER_ATTRIBUTE_HAS_ROTATE         = ( 1 << 5 ) ,
    EFT_SHADER_ATTRIBUTE_HAS_EMITTER_MATRIX = ( 1 << 6 ) ,
    EFT_SHADER_ATTRIBUTE_HAS_RANDOM         = ( 1 << 7 ) ,
    EFT_SHADER_ATTRIBUTE_HAS_VECTOR         = ( 1 << 8 ) ,

    EFT_SHADER_ATTRIBUTE_FORCE_4BYTE = 0x80000000
};
static_assert(sizeof(ShaderEnableAttrFlg) == 4, "nw::eft::ShaderEnableAttrFlg size mismatch");

enum VolumeType
{
    EFT_VOLUME_TYPE_POINT = 0,
    EFT_VOLUME_TYPE_CIRCLE,
    EFT_VOLUME_TYPE_CIRCLE_SAME_DIVIDE,
    EFT_VOLUME_TYPE_CIRCLE_FILL,
    EFT_VOLUME_TYPE_SPHERE,
    EFT_VOLUME_TYPE_SPHERE_SAME_DIVIDE,
    EFT_VOLUME_TYPE_SPHERE_SAME_DIVIDE64,
    EFT_VOLUME_TYPE_SPHERE_FILL,
    EFT_VOLUME_TYPE_CYLINDER,
    EFT_VOLUME_TYPE_CYLINDER_FILL,
    EFT_VOLUME_TYPE_BOX,
    EFT_VOLUME_TYPE_BOX_FILL,
    EFT_VOLUME_TYPE_LINE,
    EFT_VOLUME_TYPE_LINE_SAME_DIVIDE,
    EFT_VOLUME_TYPE_RECTANGLE,
    EFT_VOLUME_TYPE_PRIMITIVE,

    EFT_VOLUME_TYPE_MAX,
    EFT_VOLUME_TYPE_FORCE_4BYTE = 0x80000000
};
static_assert(sizeof(VolumeType) == 4, "nw::eft::VolumeType size mismatch");

enum VolumePrimEmitType
{
    EFT_VOLUME_PRIM_EMIT_TYPE_NORMAL = 0,
    EFT_VOLUME_PRIM_EMIT_TYPE_RANDOM = 1,
    EFT_VOLUME_PRIM_EMIT_TYPE_INDEX  = 2,
    EFT_VOLUME_PRIM_EMIT_TYPE_MAX    = 3,

    EFT_VOLUME_PRIM_EMIT_TYPE_FORCE_4BYTE = 0x80000000
};
static_assert(sizeof(VolumePrimEmitType) == 4, "nw::eft::VolumePrimEmitType size mismatch");

enum SoftProcessingMethod
{
    EFT_SOFT_EDGE_METHOD_FRAGMENT = 0,
    EFT_SOFT_EDGE_METHOD_VERTEX,
    EFT_SOFT_EDGE_METHOD_LENSFLARE,
    EFT_SOFT_EDGE_METHOD_TYPE_FORCE_4BYTE = 0x80000000
};
static_assert(sizeof(SoftProcessingMethod) == 4, "nw::eft::SoftProcessingMethod size mismatch");

enum CombinerType
{
    EFT_COMBINER_TYPE_COLOR                 = 0,
    EFT_COMBINER_TYPE_TEXTURE               = 1,
    EFT_COMBINER_TYPE_TEXTURE_INTERPOLATE   = 2,
    EFT_COMBINER_TYPE_TEXTURE_ADD           = 3,
    EFT_COMBINER_TYPE,
    EFT_COMBINER_TYPE_FORCE4BYTE            = 0x80000000
};
static_assert(sizeof(CombinerType) == 4, "nw::eft::CombinerType size mismatch");

enum AlphaCombinerType
{
    EFT_ALPHA_COMBINER_TYPE_MOD                     = 0,
    EFT_ALPHA_COMBINER_TYPE_SUB                     = 1,
    EFT_ALPHA_COMBINER_TYPE_MOD_ALPHA0_ALPHA1       = 4,
    EFT_ALPHA_COMBINER_TYPE_SUB_ALPHA0_MOD_ALPHA1   = 5,

    EFT_ALPHA_COMBINER_TYPE,
    EFT_ALPHA_COMBINER_TYPE_FORCE4BYTE = 0x80000000
};
static_assert(sizeof(AlphaCombinerType) == 4, "nw::eft::AlphaCombinerType size mismatch");

enum ColorBlendType
{
    EFT_COLOR_BLEND_TYPE_MOD = 0,
    EFT_COLOR_BLEND_TYPE_ADD,
    EFT_COLOR_BLEND_TYPE_SUB,
    EFT_COLOR_BLEND_TYPE_FORCE4BYTE = 0x80000000
};
static_assert(sizeof(ColorBlendType) == 4, "nw::eft::ColorBlendType size mismatch");

enum ColorCalcType
{
    EFT_COLOR_CALC_TYPE_FIXED           = 0,
    EFT_COLOR_CALC_TYPE_RANDOM          = 1,
    EFT_COLOR_CALC_TYPE_3V4KEY          = 2,
    EFT_COLOR_CALC_TYPE_8KEY            = 3,
    EFT_COLOR_CALC_TYPE_MAX             = 4,
    EFT_COLOR_CALC_TYPE_FORCE4BYTE      = 0x80000000
};
static_assert(sizeof(ColorCalcType) == 4, "nw::eft::ColorCalcType size mismatch");

enum AlphaCalcType
{
    EFT_ALPHA_CALC_TYPE_FIXED           = 0,
    EFT_ALPHA_CALC_TYPE_3V4KEY          = 1,
    EFT_ALPHA_CALC_TYPE_8KEY            = 2,
    EFT_ALPHA_CALC_TYPE_MAX             = 3,
    EFT_ALPHA_CALC_TYPE_FORCE4BYTE      = 0x80000000
};
static_assert(sizeof(AlphaCalcType) == 4, "nw::eft::AlphaCalcType size mismatch");

enum TextureFormat
{
    EFT_TEXTURE_FORMAT_NONE = 0,
    EFT_TEXTURE_FORMAT_24BIT_COLOR,
    EFT_TEXTURE_FORMAT_32BIT_COLOR,
    EFT_TEXTURE_FORMAT_UNORM_BC1,
    EFT_TEXTURE_FORMAT_SRGB_BC1,
    EFT_TEXTURE_FORMAT_UNORM_BC2,
    EFT_TEXTURE_FORMAT_SRGB_BC2,
    EFT_TEXTURE_FORMAT_UNORM_BC3,
    EFT_TEXTURE_FORMAT_SRGB_BC3,
    EFT_TEXTURE_FORMAT_UNORM_BC4,
    EFT_TEXTURE_FORMAT_SNORM_BC4,
    EFT_TEXTURE_FORMAT_UNORM_BC5,
    EFT_TEXTURE_FORMAT_SNORM_BC5,
    EFT_TEXTURE_FORMAT_UNORM_8,
    EFT_TEXTURE_FORMAT_UNORM_8_8,
    EFT_TEXTURE_FORMAT_SRGB_8_8_8_8,
    EFT_TEXTURE_FORMAT_SNORM_8,
    EFT_TEXTURE_FORMAT_UNORM_4_4,
    EFT_TEXTURE_FORMAT_FLOAT_11_11_10,
    EFT_TEXTURE_FORMAT_FLOAT_16,
    EFT_TEXTURE_FORMAT_FLOAT_16_16,
    EFT_TEXTURE_FORMAT_FLOAT_16_16_16_16,
    EFT_TEXTURE_FORMAT_FLOAT_32,
    EFT_TEXTURE_FORMAT_FLOAT_32_32,
    EFT_TEXTURE_FORMAT_FLOAT_32_32_32_32,
    EFT_TEXTURE_FORMAT_UNORM_5_6_5,
    EFT_TEXTURE_FORMAT_UNORM_5_5_5_1,

    EFT_TEXTURE_FORMAT_FORCE4BYTE = 0x80000000
};
static_assert(sizeof(TextureFormat) == 4, "nw::eft::TextureFormat size mismatch");

enum TexturePatternAnimMode
{
    EFT_TEX_PTN_ANIM_NONE       = 0,
    EFT_TEX_PTN_ANIM_LIFEFIT    = 1,
    EFT_TEX_PTN_ANIM_LOOP       = 2,
    EFT_TEX_PTN_ANIM_RANDOM     = 3,
    EFT_TEX_PTN_ANIM_CLAMP      = 4,
    EFT_TEX_PTN_ANIM_SURFACE    = 5,

    EFT_TEX_PTN_ANIM_FORCE4BYTE = 0x80000000
};
static_assert(sizeof(TexturePatternAnimMode) == 4, "nw::eft::TexturePatternAnimMode size mismatch");

enum UvShiftAnimMode
{
    EFT_UV_SHIFT_ANIM_NONE      = 0,
    EFT_UV_SHIFT_ANIM_SCROLL    = ( 1 << 0 ),
    EFT_UV_SHIFT_ANIM_SCALE     = ( 1 << 1 ),
    EFT_UV_SHIFT_ANIM_ROT       = ( 1 << 2 ),

    EFT_TEXTURE_ADD_ANIM_FORCE4BYTE = 0x80000000
};
static_assert(sizeof(UvShiftAnimMode) == 4, "nw::eft::UvShiftAnimMode size mismatch");

enum ChildFlg
{
    EFT_CHILD_FLAG_ENABLE                       = ( 1 << 0 ) ,
    EFT_CHILD_FLAG_COLOR0_INHERIT               = ( 1 << 1 ) ,
  //EFT_CHILD_FLAG_ALPHA_INHERIT                = ( 1 << 2 ) ,
    EFT_CHILD_FLAG_SCALE_INHERIT                = ( 1 << 3 ) ,
    EFT_CHILD_FLAG_ROTATE_INHERIT               = ( 1 << 4 ) ,
    EFT_CHILD_FLAG_VEL_INHERIT                  = ( 1 << 5 ) ,
    EFT_CHILD_FLAG_EMITTER_FOLLOW               = ( 1 << 6 ) ,
    EFT_CHILD_FLAG_DISP_PARENT                  = ( 1 << 7 ) ,
    EFT_CHILD_FLAG_WORLD_FIELD                  = ( 1 << 8 ) ,
    EFT_CHILD_FLAG_IS_POLYGON                   = ( 1 << 9 ) ,
    EFT_CHILD_FLAG_IS_EMITTER_BILLBOARD_MTX     = ( 1 << 10 ),
    EFT_CHILD_FLAG_PARENT_FIELD                 = ( 1 << 11 ),
    EFT_CHILD_FLAG_PRE_CHILD_DRAW               = ( 1 << 12 ),
    EFT_CHILD_FLAG_IS_TEXTURE_PAT_ANIM          = ( 1 << 13 ),
    EFT_CHILD_FLAG_IS_TEXTURE_PAT_ANIM_RAND     = ( 1 << 14 ),
    EFT_CHILD_FLAG_COLOR1_INHERIT               = ( 1 << 15 ),
    EFT_CHILD_FLAG_COLOR_SCALE_INHERIT          = ( 1 << 16 ),

    EFT_CHILD_FLAG_TEXTURE_COLOR_ONE            = ( 1 << 17 ) ,
    EFT_CHILD_FLAG_PRIMITIVE_COLOR_ONE          = ( 1 << 18 ) ,
    EFT_CHILD_FLAG_TEXTURE_ALPHA_ONE            = ( 1 << 19 ) ,
    EFT_CHILD_FLAG_PRIMITIVE_ALPHA_ONE          = ( 1 << 20 ) ,
    EFT_CHILD_FLAG_TEXTURE_ALPHA_RED            = ( 1 << 21 ) ,

    EFT_CHILD_FLAG_ALPHA0_INHERIT               = ( 1 << 22 ) ,
    EFT_CHILD_FLAG_ALPHA1_INHERIT               = ( 1 << 23 ) ,


    EFT_CHILD_FLAG_FORCE_4BYTE = 0x80000000
};
static_assert(sizeof(ChildFlg) == 4, "nw::eft::ChildFlg size mismatch");

enum FieldType
{
    EFT_FIELD_TYPE_RANDOM,
    EFT_FIELD_TYPE_MAGNET,
    EFT_FIELD_TYPE_SPIN,
    EFT_FIELD_TYPE_COLLISION,
    EFT_FIELD_TYPE_CONVERGENCE,
    EFT_FIELD_TYPE_POSADD,
    EFT_FIELD_TYPE_CURL_NOISE,
    EFT_FIELD_TYPE_MAX,
    EFT_FIELD_TYPE_FORCE_4BYTE = 0x80000000
};
static_assert(sizeof(FieldType) == 4, "nw::eft::FieldType size mismatch");

enum FieldMask
{
    EFT_FIELD_MASK_RANDOM       = ( 1 << EFT_FIELD_TYPE_RANDOM      ),
    EFT_FIELD_MASK_MAGNET       = ( 1 << EFT_FIELD_TYPE_MAGNET      ),
    EFT_FIELD_MASK_SPIN         = ( 1 << EFT_FIELD_TYPE_SPIN        ),
    EFT_FIELD_MASK_COLLISION    = ( 1 << EFT_FIELD_TYPE_COLLISION   ),
    EFT_FIELD_MASK_CONVERGENCE  = ( 1 << EFT_FIELD_TYPE_CONVERGENCE ),
    EFT_FIELD_MASK_POSADD       = ( 1 << EFT_FIELD_TYPE_POSADD      ),
    EFT_FIELD_MASK_CURL_NOISE   = ( 1 << EFT_FIELD_TYPE_CURL_NOISE  ),
    EFT_FIELD_MASK_FORCE_4BYTE = 0x80000000
};
static_assert(sizeof(FieldMask) == 4, "nw::eft::FieldMask size mismatch");

enum FluctuationFlg
{
    EFT_FLUCTUATION_FALG_ENABLE             = ( 1 << 0 ) ,
    EFT_FLUCTUATION_FALG_APPLY_ALPHA        = ( 1 << 1 ) ,
    EFT_FLUCTUATION_FALG_APPLY_SCALE        = ( 1 << 2 ) ,
    EFT_FLUCTUATION_FALG_USE_SIN_WAVE       = ( 1 << 3 ) ,
    EFT_FLUCTUATION_FALG_USE_SAW_TOOTH_WAVE = ( 1 << 4 ) ,
    EFT_FLUCTUATION_FALG_USE_RECT_WAVE      = ( 1 << 5 ) ,
    EFT_FLUCTUATION_FALG_APPLY_SCALEY       = ( 1 << 6 ) ,
    EFT_FLUCTUATION_FALG_FORCE_4BYTE = 0x80000000
};
static_assert(sizeof(FluctuationFlg) == 4, "nw::eft::FluctuationFlg size mismatch");

enum
{
    EFT_FLUCTUATION_TABLE_NUM           = 128 ,
    EFT_FLUCTUATION_TABLE_MASK          = EFT_FLUCTUATION_TABLE_NUM - 1 ,
    EFT_FLUCTUATION_TABLE_FORCE_4BYTE = 0x80000000
};

enum FieldMagnetFlg
{
    EFT_MAGNET_FLAG_X = ( 1 << 0 ) ,
    EFT_MAGNET_FLAG_Y = ( 1 << 1 ) ,
    EFT_MAGNET_FLAG_Z = ( 1 << 2 ) ,
    EFT_MAGNET_FLAG_FORCE_4BYTE = 0x80000000
};
static_assert(sizeof(FieldMagnetFlg) == 4, "nw::eft::FieldMagnetFlg size mismatch");

enum FieldCollisionReaction
{
    EFT_FIELD_COLLISION_REACTION_CESSER = 0 ,
    EFT_FIELD_COLLISION_REACTION_REFLECTION ,
    EFT_FIELD_COLLISION_REACTION_CUSTOM ,
    EFT_FIELD_COLLISION_REACTION_FORCE_4BYTE = 0x80000000
};
static_assert(sizeof(FieldCollisionReaction) == 4, "nw::eft::FieldCollisionReaction size mismatch");

enum FieldConvergenceType
{
    EFT_FIELD_CONVERGENCE_TYPE_ASSIGNED_POSITION = 0 ,
    EFT_FIELD_CONVERGENCE_TYPE_EMITTER_POSITION ,
    EFT_FIELD_CONVERGENCE_TYPE_FORCE_4BYTE = 0x80000000
};
static_assert(sizeof(FieldConvergenceType) == 4, "nw::eft::FieldConvergenceType size mismatch");

enum StripeFlg
{
    EFT_STRIPE_FLAG_EMITTER_COORD       = ( 1 << 0 ) ,
    EFT_STRIPE_FLAG_FORCE_4BYTE = 0x80000000
};
static_assert(sizeof(StripeFlg) == 4, "nw::eft::StripeFlg size mismatch");

enum StripeType
{
    EFT_STRIPE_TYPE_BILLBOARD,
    EFT_STRIPE_TYPE_EMITTER_MATRIX ,
    EFT_STRIPE_TYPE_EMITTER_UP_DOWN,
    EFT_STRIPE_TYPE_MAX,
    EFT_STRIPE_TYPE_FORCE_4BYTE = 0x80000000
};
static_assert(sizeof(StripeType) == 4, "nw::eft::StripeType size mismatch");

enum StripeOption
{
    EFT_STRIPE_OPTION_TYPE_NORMAL,
    EFT_STRIPE_OPTION_TYPE_CROSS,
    EFT_STRIPE_OPTION_TYPE_MAX,
    EFT_STRIPE_OPTION_TYPE_FORCE_4BYTE = 0x80000000
};
static_assert(sizeof(StripeOption) == 4, "nw::eft::StripeOption size mismatch");

enum StripeConnectOption
{
    EFT_STRIPE_CONNECT_OPTION_NORMAL,
    EFT_STRIPE_CONNECT_OPTION_HEAD,
    EFT_STRIPE_CONNECT_OPTION_EMITTER,
    EFT_STRIPE_CONNECT_OPTION_EMITTER_UNBIND,


    EFT_CONNECTION_STRIPE_OPTION_FORCE_4BYTE = 0x80000000
};
static_assert(sizeof(StripeConnectOption) == 4, "nw::eft::StripeConnectOption size mismatch");

enum StripeTexCoordOption
{
    EFT_STRIPE_TEXCOORD_OPTION_TYPE_FULL,
    EFT_STRIPE_TEXCOORD_OPTION_TYPE_DIVISION,
    EFT_STRIPE_TEXCOORD_OPTION_TYPE_MAX,
    EFT_STRIPE_TEXCOORD_OPTION_TYPE_FORCE_4BYTE = 0x80000000
};
static_assert(sizeof(StripeTexCoordOption) == 4, "nw::eft::StripeTexCoordOption size mismatch");

enum AnimKeyFrameInterpolationType
{
    EFT_ANIM_KEY_FRAME_LINEAR = 0,
    EFT_ANIM_KEY_FRAME_SMOOTH,
    EFT_ANIM_KEY_FRAME_FORCE_4BYTE = 0x80000000
};
static_assert(sizeof(AnimKeyFrameInterpolationType) == 4, "nw::eft::AnimKeyFrameInterpolationType size mismatch");

enum AnimKeyFrameApplyType
{
    EFT_ANIM_EM_RATE = 0,
    EFT_ANIM_LIFE,
    EFT_ANIM_EM_SX,
    EFT_ANIM_EM_SY,
    EFT_ANIM_EM_SZ,
    EFT_ANIM_EM_RX,
    EFT_ANIM_EM_RY,
    EFT_ANIM_EM_RZ,
    EFT_ANIM_EM_TX,
    EFT_ANIM_EM_TY,
    EFT_ANIM_EM_TZ,
    EFT_ANIM_COLOR0_R,
    EFT_ANIM_COLOR0_G,
    EFT_ANIM_COLOR0_B,
    EFT_ANIM_ALPHA,
    EFT_ANIM_ALL_DIR_VEL,
    EFT_ANIM_DIR_VEL,
    EFT_ANIM_PTCL_SX,
    EFT_ANIM_PTCL_SY,
    EFT_ANIM_COLOR1_R,
    EFT_ANIM_COLOR1_G,
    EFT_ANIM_COLOR1_B,
    EFT_ANIM_EM_FORM_SX,
    EFT_ANIM_EM_FORM_SY,
    EFT_ANIM_EM_FORM_SZ,
    EFT_ANIM_GRAVITY,
    EFT_ANIM_GRAVITY_CHILD,
    EFT_ANIM_EMITTER_MAX,

    EFT_ANIM_8KEY_FIELD_RND_X,
    EFT_ANIM_8KEY_FIELD_RND_Y,
    EFT_ANIM_8KEY_FIELD_RND_Z,
    EFT_ANIM_8KEY_FIELD_MAGNET,
    EFT_ANIM_8KEY_FIELD_SPIN_ROT,
    EFT_ANIM_8KEY_FIELD_SPIN_OUTER,
    EFT_ANIM_8KEY_FIELD_CONVERGENCE,
    EFT_ANIM_8KEY_FIELD_POS_ADD_X,
    EFT_ANIM_8KEY_FIELD_POS_ADD_Y,
    EFT_ANIM_8KEY_FIELD_POS_ADD_Z,
    EFT_ANIM_8KEY_PARTICLE_COLOR0_R,
    EFT_ANIM_8KEY_PARTICLE_COLOR0_G,
    EFT_ANIM_8KEY_PARTICLE_COLOR0_B,
    EFT_ANIM_8KEY_PARTICLE_COLOR1_R,
    EFT_ANIM_8KEY_PARTICLE_COLOR1_G,
    EFT_ANIM_8KEY_PARTICLE_COLOR1_B,
    EFT_ANIM_8KEY_PARTICLE_ALPHA0,
    EFT_ANIM_8KEY_PARTICLE_ALPHA1,
    EFT_ANIM_8KEY_PARTICLE_SCALE_X,
    EFT_ANIM_8KEY_PARTICLE_SCALE_Y,

    EFT_ANIM_MAX,
    EFT_ANIM_FORCE_4BYTE = 0x80000000
};
static_assert(sizeof(AnimKeyFrameApplyType) == 4, "nw::eft::AnimKeyFrameApplyType size mismatch");

#define EFT_ANIM_8KEY_MAX (EFT_ANIM_MAX - EFT_ANIM_EMITTER_MAX - 1)
#define EFT_ANIM_8KEY_OFFSET(id) (id - EFT_ANIM_EMITTER_MAX - 1)

enum AnimType
{
    EFT_ANIM_TYPE_NONE      = 0,
    EFT_ANIM_TYPE_V3_KEY4   = 1,
    EFT_ANIM_TYPE_KEY8      = 2,

    EFT_ANIM_TYPE_FORCE_4BYTE = 0x80000000
};
static_assert(sizeof(AnimType) == 4, "nw::eft::AnimType size mismatch");

enum RuntimeError
{
    EFT_RUNTIME_ERROR_NONE = 0,
    EFT_RUNTIME_ERROR_DIRECTIONAL_VEL_MIN,
    EFT_RUNTIME_ERROR_DIRECTIONAL_VEL_UP_SAME_DIR,
    EFT_RUNTIME_ERROR_FORCE_4BYTE = 0x80000000
};
static_assert(sizeof(RuntimeError) == 4, "nw::eft::RuntimeError size mismatch");

} } // namespace nw::eft

#endif // EFT_ENUM_H_
