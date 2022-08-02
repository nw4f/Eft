#ifndef EFT_CALLBACK_H_
#define EFT_CALLBACK_H_

#include <nw/eft/eft_Enum.h>

namespace nw { namespace eft {

struct EmitterInstance;
class  EmitterSet;
struct PtclInstance;
class  Renderer;
class  System;

struct EmitterMatrixSetArg
{
    EmitterInstance*    emitter;
};
static_assert(sizeof(EmitterMatrixSetArg) == 4, "nw::eft::EmitterMatrixSetArg size mismatch");

struct EmitterPreCalcArg
{
    EmitterInstance*    emitter;
};
static_assert(sizeof(EmitterPreCalcArg) == 4, "nw::eft::EmitterPreCalcArg size mismatch");

struct EmitterPostCalcArg
{
    EmitterInstance* emitter;
};
static_assert(sizeof(EmitterPostCalcArg) == 4, "nw::eft::EmitterPostCalcArg size mismatch");

struct EmitterDrawOverrideArg
{
    const EmitterInstance*  emitter;
    Renderer*               renderer;
    bool                    cacheFlush;
    void*                   userParam;
};
static_assert(sizeof(EmitterDrawOverrideArg) == 0x10, "nw::eft::EmitterDrawOverrideArg size mismatch");

struct ParticleEmitArg
{
    PtclInstance*   particle;
};
static_assert(sizeof(ParticleEmitArg) == 4, "nw::eft::ParticleEmitArg size mismatch");

struct ParticleRemoveArg
{
    PtclInstance*   particle;
};
static_assert(sizeof(ParticleRemoveArg) == 4, "nw::eft::ParticleRemoveArg size mismatch");

struct ParticleCalcArg
{
    EmitterInstance*    emitter;
    PtclInstance*       ptcl;
    CpuCore             core;
    bool                skipBehavior;
};
static_assert(sizeof(ParticleCalcArg) == 0x10, "nw::eft::ParticleCalcArg size mismatch");

struct ParticleMakeAttrArg
{
    EmitterInstance*    emitter;
    const PtclInstance* ptcl;
    CpuCore             core;
    bool                skipBehavior;
};
static_assert(sizeof(ParticleMakeAttrArg) == 0x10, "nw::eft::ParticleMakeAttrArg size mismatch");

struct ShaderEmitterInitializeArg
{
    EmitterInstance*    emitter;
};
static_assert(sizeof(ShaderEmitterInitializeArg) == 4, "nw::eft::ShaderEmitterInitializeArg size mismatch");

struct ShaderEmitterFinalizeArg
{
    EmitterInstance*    emitter;
};
static_assert(sizeof(ShaderEmitterFinalizeArg) == 4, "nw::eft::ShaderEmitterFinalizeArg size mismatch");

struct ShaderEmitterPreCalcArg
{
    EmitterInstance*    emitter;
};
static_assert(sizeof(ShaderEmitterPreCalcArg) == 4, "nw::eft::ShaderEmitterPreCalcArg size mismatch");

struct ShaderEmitterPostCalcArg
{
    EmitterInstance* emitter;
    bool             skipBehavior;
    bool             isChild;
};
static_assert(sizeof(ShaderEmitterPostCalcArg) == 8, "nw::eft::ShaderEmitterPostCalcArg size mismatch");

struct ShaderDrawOverrideArg
{
    const EmitterInstance*  emitter;
    Renderer*               renderer;
    bool                    cacheFlush;
    void*                   userParam;
};
static_assert(sizeof(ShaderDrawOverrideArg) == 0x10, "nw::eft::ShaderDrawOverrideArg size mismatch");

struct RenderStateSetArg
{
    const EmitterInstance*  emitter;
    Renderer*               renderer;
    bool                    cacheFlush;
    void*                   userParam;
};
static_assert(sizeof(RenderStateSetArg) == 0x10, "nw::eft::RenderStateSetArg size mismatch");

struct RenderEmitterProfilerArg
{
    System*                 system;
    const EmitterSet*       emitterSet;
    const EmitterInstance*  emitter;
    s32                     beforeRenderEmitter;
    bool                    cacheFlush;
    void*                   userParam;
};
static_assert(sizeof(RenderEmitterProfilerArg) == 0x18, "nw::eft::RenderEmitterProfilerArg size mismatch");

typedef void (*CustomActionEmitterMatrixSetCallback)(EmitterMatrixSetArg& arg);
typedef void (*CustomActionEmitterPreCalcCallback)(EmitterPreCalcArg& arg);
typedef void (*CustomActionEmitterPostCalcCallback)(EmitterPostCalcArg& arg);
typedef void (*CustomActionEmitterDrawOverrideCallback)(EmitterDrawOverrideArg& arg);
typedef bool (*CustomActionParticleEmitCallback)(ParticleEmitArg& arg);
typedef bool (*CustomActionParticleRemoveCallback)(ParticleRemoveArg& arg);
typedef void (*CustomActionParticleCalcCallback)(ParticleCalcArg& arg);
typedef void (*CustomActionParticleMakeAttributeCallback)(const ParticleMakeAttrArg& arg);
typedef void (*CustomShaderEmitterInitializeCallback)(ShaderEmitterInitializeArg& arg);
typedef void (*CustomShaderEmitterFinalizeCallback)(ShaderEmitterFinalizeArg& arg);
typedef void (*CustomShaderEmitterPreCalcCallback)(ShaderEmitterPreCalcArg& arg);
typedef void (*CustomShaderEmitterPostCalcCallback)(ShaderEmitterPostCalcArg& arg);
typedef void (*CustomShaderDrawOverrideCallback)(ShaderDrawOverrideArg& arg);
typedef void (*CustomShaderRenderStateSetCallback)(RenderStateSetArg& arg);
typedef void (*RenderEmitterProfilerCallback)(RenderEmitterProfilerArg& arg);
typedef void (*DrawPathRenderStateSetCallback)(RenderStateSetArg& arg);

} } // namespace nw::eft

#endif // EFT_CALLBACK_H_
