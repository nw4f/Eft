#ifndef EFT_CALLBACK_H_
#define EFT_CALLBACK_H_

#include <nw/eft/eft_typeDef.h>

namespace nw  { namespace eft {

struct EmitterInstance;
struct PtclInstance;
class Renderer;

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

typedef void (*CustomActionEmitterPreCalcCallback)(EmitterPreCalcArg& arg);
typedef void (*CustomActionEmitterPostCalcCallback)(EmitterPostCalcArg& arg);
typedef bool (*CustomActionParticleEmitCallback)(ParticleEmitArg& arg);
typedef bool (*CustomActionParticleRemoveCallback)(ParticleRemoveArg& arg);
typedef void (*CustomActionParticleCalcCallback)(ParticleCalcArg& arg);
typedef void (*CustomActionParticleMakeAttributeCallback)(const ParticleMakeAttrArg& arg); // const... ?
typedef void (*CustomActionEmitterDrawOverrideCallback)(EmitterDrawOverrideArg& arg);
typedef void (*CustomShaderEmitterPostCalcCallback)(ShaderEmitterPostCalcArg& arg);
typedef void (*CustomShaderDrawOverrideCallback)(ShaderDrawOverrideArg& arg);
typedef void (*CustomShaderRenderStateSetCallback)(RenderStateSetArg& arg);

} } // namespace nw::eft

#endif // EFT_CALLBACK_H_
