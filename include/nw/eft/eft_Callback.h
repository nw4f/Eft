#ifndef EFT_CALLBACK_H_
#define EFT_CALLBACK_H_

#include <nw/eft/eft_Enum.h>

namespace nw { namespace eft {

struct EmitterInstance;
class EmitterSet;
struct PtclInstance;
class Renderer;
class System;

struct EmitterMatrixSetArg
{
    EmitterInstance* emitter;
};
static_assert(sizeof(EmitterMatrixSetArg) == 4, "EmitterMatrixSetArg size mismatch");

struct EmitterPreCalcArg
{
    EmitterInstance* emitter;
};
static_assert(sizeof(EmitterPreCalcArg) == 4, "EmitterPreCalcArg size mismatch");

struct EmitterPostCalcArg
{
    EmitterInstance* emitter;
};
static_assert(sizeof(EmitterPostCalcArg) == 4, "EmitterPostCalcArg size mismatch");

struct ParticleEmitArg
{
    PtclInstance* ptcl;
};
static_assert(sizeof(ParticleEmitArg) == 4, "ParticleEmitArg size mismatch");

struct ParticleRemoveArg
{
    PtclInstance* ptcl;
};
static_assert(sizeof(ParticleRemoveArg) == 4, "ParticleRemoveArg size mismatch");

struct ParticleCalcArg
{
    EmitterInstance* emitter;
    PtclInstance* ptcl;
    CpuCore core;
    bool noCalcBehavior;
};
static_assert(sizeof(ParticleCalcArg) == 0x10, "ParticleCalcArg size mismatch");

struct ParticleMakeAttrArg
{
    EmitterInstance* emitter;
    PtclInstance* ptcl;
    CpuCore core;
    bool noCalcBehavior;
};
static_assert(sizeof(ParticleMakeAttrArg) == 0x10, "ParticleMakeAttrArg size mismatch");

struct EmitterDrawOverrideArg
{
    const EmitterInstance* emitter;
    Renderer* renderer;
    bool flushCache;
    void* argData;
};
static_assert(sizeof(EmitterDrawOverrideArg) == 0x10, "EmitterDrawOverrideArg size mismatch");

struct ShaderEmitterInitializeArg
{
    EmitterInstance* emitter;
};
static_assert(sizeof(ShaderEmitterInitializeArg) == 4, "ShaderEmitterInitializeArg size mismatch");

struct ShaderEmitterFinalizeArg
{
    EmitterInstance* emitter;
};
static_assert(sizeof(ShaderEmitterFinalizeArg) == 4, "ShaderEmitterFinalizeArg size mismatch");

struct ShaderEmitterPreCalcArg
{
    EmitterInstance* emitter;
};
static_assert(sizeof(ShaderEmitterPreCalcArg) == 4, "ShaderEmitterPreCalcArg size mismatch");

struct ShaderEmitterPostCalcArg
{
    EmitterInstance* emitter;
    bool noCalcBehavior;
    bool childParticle;
};
static_assert(sizeof(ShaderEmitterPostCalcArg) == 8, "ShaderEmitterPostCalcArg size mismatch");

struct ShaderDrawOverrideArg
{
    const EmitterInstance* emitter;
    Renderer* renderer;
    bool flushCache;
    void* argData;
};
static_assert(sizeof(ShaderDrawOverrideArg) == 0x10, "ShaderDrawOverrideArg size mismatch");

struct RenderStateSetArg
{
    const EmitterInstance* emitter;
    Renderer* renderer;
    bool flushCache;
    void* argData;
};
static_assert(sizeof(RenderStateSetArg) == 0x10, "RenderStateSetArg size mismatch");

struct RenderEmitterProfilerArg
{
    System* system;
    EmitterSet* emitterSet;
    EmitterInstance* emitter;
    u32 _C;
    bool flushCache;
    void* argData;
};
static_assert(sizeof(RenderEmitterProfilerArg) == 0x18, "RenderEmitterProfilerArg size mismatch");

typedef void (*CustomActionEmitterMatrixSetCallback)(EmitterMatrixSetArg& arg);
typedef void (*CustomActionEmitterPreCalcCallback)(EmitterPreCalcArg& arg);
typedef void (*CustomActionEmitterPostCalcCallback)(EmitterPostCalcArg& arg);
typedef bool (*CustomActionParticleEmitCallback)(ParticleEmitArg& arg);
typedef bool (*CustomActionParticleRemoveCallback)(ParticleRemoveArg& arg);
typedef void (*CustomActionParticleCalcCallback)(ParticleCalcArg& arg);
typedef void (*CustomActionParticleMakeAttributeCallback)(const ParticleMakeAttrArg& arg); // const... ?
typedef void (*CustomActionEmitterDrawOverrideCallback)(EmitterDrawOverrideArg& arg);
typedef void (*CustomShaderEmitterInitializeCallback)(ShaderEmitterInitializeArg& arg);
typedef void (*CustomShaderEmitterFinalizeCallback)(ShaderEmitterFinalizeArg& arg);
typedef void (*CustomShaderEmitterPreCalcCallback)(ShaderEmitterPreCalcArg& arg);
typedef void (*CustomShaderEmitterPostCalcCallback)(ShaderEmitterPostCalcArg& arg);
typedef void (*CustomShaderDrawOverrideCallback)(ShaderDrawOverrideArg& arg);
typedef void (*CustomShaderRenderStateSetCallback)(RenderStateSetArg& arg);
typedef void (*DrawPathRenderStateSetCallback)(RenderStateSetArg& arg);
typedef void (*RenderEmitterProfilerCallback)(RenderEmitterProfilerArg& arg);

} } // namespace nw::eft

#endif // EFT_CALLBACK_H_
