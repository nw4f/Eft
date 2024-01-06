#include <nw/eft/gl/eft_GLWrapper.h>
#include <nw/eft/eft_Heap.h>

#if EFT_IS_WIN

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <sstream>

#include <file.hpp>
#include <globals.hpp>
#include <md5.hpp>

static GLuint CompileShader(const char* source, GLenum type)
{
    GLuint id = glCreateShader(type);
    glShaderSource(id, 1, &source, NULL);
    glCompileShader(id);

    GLint result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE)
    {
        GLint length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);

        char* message = new char[length];
        glGetShaderInfoLog(id, length, &length, message);
        printf("Failed to compile %s shader\n", type == GL_VERTEX_SHADER ? "vertex" : "fragment");
        printf("%s\n", message);
        delete[] message;

        glDeleteShader(id);
        return GL_NONE;
    }

    return id;
}

static GLuint CompileProgram(const char* vertexShader,
                             const char* fragmentShader)
{
    GLuint program = glCreateProgram();
    GLuint vs = CompileShader(vertexShader, GL_VERTEX_SHADER);
    GLuint fs = CompileShader(fragmentShader, GL_FRAGMENT_SHADER);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    GLint result;
    glGetProgramiv(program, GL_LINK_STATUS, &result);
    if (result == GL_FALSE)
    {
        GLint length;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);

        char* message = new char[length];
        glGetProgramInfoLog(program, length, &length, message);
        printf("Failed to link program\n");
        printf("%s\n", message);
        delete[] message;

        glDeleteProgram(program);

        glDeleteShader(vs);
        glDeleteShader(fs);

        return GL_NONE;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

static size_t SaveGX2VertexShader(nw::eft::Heap* heap,
                                  GX2VertexShader* shader,
                                  void** pShaderBuf)
{
    size_t shaderBufSize = sizeof(GX2VertexShader);

    const size_t uniformBlocksOffs = shaderBufSize;
    shaderBufSize += shader->numUniformBlocks * sizeof(GX2UniformBlock);

    const size_t uniformVarsOffs = shaderBufSize;
    shaderBufSize += shader->numUniforms * sizeof(GX2UniformVar);

    const size_t initialValuesOffs = shaderBufSize;
    shaderBufSize += shader->numInitialValues * sizeof(GX2UniformInitialValue);

    const size_t loopVarsOffs = shaderBufSize;
    shaderBufSize += shader->_numLoops * sizeof(u32) * 2;

    const size_t samplerVarsOffs = shaderBufSize;
    shaderBufSize += shader->numSamplers * sizeof(GX2SamplerVar);

    const size_t attribVarsOffs = shaderBufSize;
    shaderBufSize += shader->numAttribs * sizeof(GX2AttribVar);

    const size_t strBaseOffs = shaderBufSize;

    for (u32 i = 0; i < shader->numUniformBlocks; i++)
    {
        const char* const name = shader->uniformBlocks[i].name;
        if (name != NULL)
            shaderBufSize += std::strlen(name) + 1;
    }

    for (u32 i = 0; i < shader->numUniforms; i++)
    {
        const char* const name = shader->uniformVars[i].name;
        if (name != NULL)
            shaderBufSize += std::strlen(name) + 1;
    }

    for (u32 i = 0; i < shader->numSamplers; i++)
    {
        const char* const name = shader->samplerVars[i].name;
        if (name != NULL)
            shaderBufSize += std::strlen(name) + 1;
    }

    for (u32 i = 0; i < shader->numAttribs; i++)
    {
        const char* const name = shader->attribVars[i].name;
        if (name != NULL)
            shaderBufSize += std::strlen(name) + 1;
    }

    const size_t shaderDataOffs = shaderBufSize;
    shaderBufSize += shader->shaderSize;

    u8* shaderBuf = (u8*)heap->Alloc(shaderBufSize);

    std::memcpy(shaderBuf, shader, sizeof(GX2VertexShader));
    std::memcpy(shaderBuf + shaderDataOffs, shader->shaderPtr, shader->shaderSize);

    *(u32*)(shaderBuf + offsetof(GX2VertexShader, shaderPtr)) = shaderDataOffs;

    if (shader->numUniformBlocks != 0)
    {
        std::memcpy(shaderBuf + uniformBlocksOffs,
                    shader->uniformBlocks,
                    shader->numUniformBlocks * sizeof(GX2UniformBlock));

        *(u32*)(shaderBuf + offsetof(GX2VertexShader, uniformBlocks)) = uniformBlocksOffs;
    }

    if (shader->numUniforms != 0)
    {
        std::memcpy(shaderBuf + uniformVarsOffs,
                    shader->uniformVars,
                    shader->numUniforms * sizeof(GX2UniformVar));

        *(u32*)(shaderBuf + offsetof(GX2VertexShader, uniformVars)) = uniformVarsOffs;
    }

    if (shader->numInitialValues != 0)
    {
        std::memcpy(shaderBuf + initialValuesOffs,
                    shader->initialValues,
                    shader->numInitialValues * sizeof(GX2UniformInitialValue));

        *(u32*)(shaderBuf + offsetof(GX2VertexShader, initialValues)) = initialValuesOffs;
    }

    if (shader->_numLoops != 0)
    {
        std::memcpy(shaderBuf + loopVarsOffs,
                    shader->_loopVars,
                    shader->_numLoops * sizeof(u32) * 2);

        *(u32*)(shaderBuf + offsetof(GX2VertexShader, _loopVars)) = loopVarsOffs;
    }

    if (shader->numSamplers != 0)
    {
        std::memcpy(shaderBuf + samplerVarsOffs,
                    shader->samplerVars,
                    shader->numSamplers * sizeof(GX2SamplerVar));

        *(u32*)(shaderBuf + offsetof(GX2VertexShader, samplerVars)) = samplerVarsOffs;
    }

    if (shader->numAttribs != 0)
    {
        std::memcpy(shaderBuf + attribVarsOffs,
                    shader->attribVars,
                    shader->numAttribs * sizeof(GX2AttribVar));

        *(u32*)(shaderBuf + offsetof(GX2VertexShader, attribVars)) = attribVarsOffs;
    }

    size_t strOffs = strBaseOffs;

    for (u32 i = 0; i < shader->numUniformBlocks; i++)
    {
        const char* const name = shader->uniformBlocks[i].name;
        if (name != NULL)
        {
            const size_t nameLen = std::strlen(name) + 1;
            std::memcpy(shaderBuf + strOffs, name, nameLen);

            *(u32*)(shaderBuf +
                    uniformBlocksOffs
                    + i * sizeof(GX2UniformBlock)
                    + offsetof(GX2UniformBlock, name)) = strOffs;

            strOffs += nameLen;
        }
    }

    for (u32 i = 0; i < shader->numUniforms; i++)
    {
        const char* const name = shader->uniformVars[i].name;
        if (name != NULL)
        {
            const size_t nameLen = std::strlen(name) + 1;
            std::memcpy(shaderBuf + strOffs, name, nameLen);

            *(u32*)(shaderBuf +
                    uniformVarsOffs
                    + i * sizeof(GX2UniformVar)
                    + offsetof(GX2UniformVar, name)) = strOffs;

            strOffs += nameLen;
        }
    }

    for (u32 i = 0; i < shader->numSamplers; i++)
    {
        const char* const name = shader->samplerVars[i].name;
        if (name != NULL)
        {
            const size_t nameLen = std::strlen(name) + 1;
            std::memcpy(shaderBuf + strOffs, name, nameLen);

            *(u32*)(shaderBuf +
                    samplerVarsOffs
                    + i * sizeof(GX2SamplerVar)
                    + offsetof(GX2SamplerVar, name)) = strOffs;

            strOffs += nameLen;
        }
    }

    for (u32 i = 0; i < shader->numAttribs; i++)
    {
        const char* const name = shader->attribVars[i].name;
        if (name != NULL)
        {
            const size_t nameLen = std::strlen(name) + 1;
            std::memcpy(shaderBuf + strOffs, name, nameLen);

            *(u32*)(shaderBuf +
                    attribVarsOffs
                    + i * sizeof(GX2AttribVar)
                    + offsetof(GX2AttribVar, name)) = strOffs;

            strOffs += nameLen;
        }
    }

    *pShaderBuf = shaderBuf;
    return shaderBufSize;
}

static size_t SaveGX2PixelShader(nw::eft::Heap* heap,
                                 GX2PixelShader* shader,
                                 void** pShaderBuf)
{
    size_t shaderBufSize = sizeof(GX2PixelShader);

    const size_t uniformBlocksOffs = shaderBufSize;
    shaderBufSize += shader->numUniformBlocks * sizeof(GX2UniformBlock);

    const size_t uniformVarsOffs = shaderBufSize;
    shaderBufSize += shader->numUniforms * sizeof(GX2UniformVar);

    const size_t initialValuesOffs = shaderBufSize;
    shaderBufSize += shader->numInitialValues * sizeof(GX2UniformInitialValue);

    const size_t loopVarsOffs = shaderBufSize;
    shaderBufSize += shader->_numLoops * sizeof(u32) * 2;

    const size_t samplerVarsOffs = shaderBufSize;
    shaderBufSize += shader->numSamplers * sizeof(GX2SamplerVar);

    const size_t strBaseOffs = shaderBufSize;

    for (u32 i = 0; i < shader->numUniformBlocks; i++)
    {
        const char* const name = shader->uniformBlocks[i].name;
        if (name != NULL)
            shaderBufSize += std::strlen(name) + 1;
    }

    for (u32 i = 0; i < shader->numUniforms; i++)
    {
        const char* const name = shader->uniformVars[i].name;
        if (name != NULL)
            shaderBufSize += std::strlen(name) + 1;
    }

    for (u32 i = 0; i < shader->numSamplers; i++)
    {
        const char* const name = shader->samplerVars[i].name;
        if (name != NULL)
            shaderBufSize += std::strlen(name) + 1;
    }

    const size_t shaderDataOffs = shaderBufSize;
    shaderBufSize += shader->shaderSize;

    u8* shaderBuf = (u8*)heap->Alloc(shaderBufSize);

    std::memcpy(shaderBuf, shader, sizeof(GX2PixelShader));
    std::memcpy(shaderBuf + shaderDataOffs, shader->shaderPtr, shader->shaderSize);

    *(u32*)(shaderBuf + offsetof(GX2PixelShader, shaderPtr)) = shaderDataOffs;

    if (shader->numUniformBlocks != 0)
    {
        std::memcpy(shaderBuf + uniformBlocksOffs,
                    shader->uniformBlocks,
                    shader->numUniformBlocks * sizeof(GX2UniformBlock));

        *(u32*)(shaderBuf + offsetof(GX2PixelShader, uniformBlocks)) = uniformBlocksOffs;
    }

    if (shader->numUniforms != 0)
    {
        std::memcpy(shaderBuf + uniformVarsOffs,
                    shader->uniformVars,
                    shader->numUniforms * sizeof(GX2UniformVar));

        *(u32*)(shaderBuf + offsetof(GX2PixelShader, uniformVars)) = uniformVarsOffs;
    }

    if (shader->numInitialValues != 0)
    {
        std::memcpy(shaderBuf + initialValuesOffs,
                    shader->initialValues,
                    shader->numInitialValues * sizeof(GX2UniformInitialValue));

        *(u32*)(shaderBuf + offsetof(GX2PixelShader, initialValues)) = initialValuesOffs;
    }

    if (shader->_numLoops != 0)
    {
        std::memcpy(shaderBuf + loopVarsOffs,
                    shader->_loopVars,
                    shader->_numLoops * sizeof(u32) * 2);

        *(u32*)(shaderBuf + offsetof(GX2PixelShader, _loopVars)) = loopVarsOffs;
    }

    if (shader->numSamplers != 0)
    {
        std::memcpy(shaderBuf + samplerVarsOffs,
                    shader->samplerVars,
                    shader->numSamplers * sizeof(GX2SamplerVar));

        *(u32*)(shaderBuf + offsetof(GX2PixelShader, samplerVars)) = samplerVarsOffs;
    }

    size_t strOffs = strBaseOffs;

    for (u32 i = 0; i < shader->numUniformBlocks; i++)
    {
        const char* const name = shader->uniformBlocks[i].name;
        if (name != NULL)
        {
            const size_t nameLen = std::strlen(name) + 1;
            std::memcpy(shaderBuf + strOffs, name, nameLen);

            *(u32*)(shaderBuf +
                    uniformBlocksOffs
                    + i * sizeof(GX2UniformBlock)
                    + offsetof(GX2UniformBlock, name)) = strOffs;

            strOffs += nameLen;
        }
    }

    for (u32 i = 0; i < shader->numUniforms; i++)
    {
        const char* const name = shader->uniformVars[i].name;
        if (name != NULL)
        {
            const size_t nameLen = std::strlen(name) + 1;
            std::memcpy(shaderBuf + strOffs, name, nameLen);

            *(u32*)(shaderBuf +
                    uniformVarsOffs
                    + i * sizeof(GX2UniformVar)
                    + offsetof(GX2UniformVar, name)) = strOffs;

            strOffs += nameLen;
        }
    }

    for (u32 i = 0; i < shader->numSamplers; i++)
    {
        const char* const name = shader->samplerVars[i].name;
        if (name != NULL)
        {
            const size_t nameLen = std::strlen(name) + 1;
            std::memcpy(shaderBuf + strOffs, name, nameLen);

            *(u32*)(shaderBuf +
                    samplerVarsOffs
                    + i * sizeof(GX2SamplerVar)
                    + offsetof(GX2SamplerVar, name)) = strOffs;

            strOffs += nameLen;
        }
    }

    *pShaderBuf = shaderBuf;
    return shaderBufSize;
}

static bool GX2UniformBlockComp(const GX2UniformBlock& a, const GX2UniformBlock& b)
{
    return a.location > b.location;
}

static void ReplaceString(std::string& str, const std::string& a, const std::string& b)
{
    size_t pos = 0;
    while ((pos = str.find(a, pos)) != std::string::npos)
    {
         str.replace(pos, a.length(), b);
         pos += b.length();
    }
}

static void DecompileProgram(nw::eft::Heap* heap,
                             GX2VertexShader* vertexShader,
                             GX2PixelShader* pixelShader,
                             std::string* vertexShaderSrc,
                             std::string* fragmentShaderSrc)
{
    void* vertexShaderBuf;
    size_t vertexShaderBufSize = SaveGX2VertexShader(heap, vertexShader, &vertexShaderBuf);

    void* pixelShaderBuf;
    size_t pixelShaderBufSize = SaveGX2PixelShader(heap, pixelShader, &pixelShaderBuf);

    u8* shaderBuf = (u8*)heap->Alloc(vertexShaderBufSize + pixelShaderBufSize);
    std::memcpy(shaderBuf, vertexShaderBuf, vertexShaderBufSize);
    std::memcpy(shaderBuf + vertexShaderBufSize, pixelShaderBuf, pixelShaderBufSize);

    std::string key = md5(shaderBuf, vertexShaderBufSize + pixelShaderBufSize);

    std::string glVertexShader;
    std::string glFragmentShader;

    ShaderCacheMap::const_iterator it = g_ShaderCache.find(key);
    if (it != g_ShaderCache.end())
    {
        const ShaderCache& shaderCache = it->second;
        glVertexShader = shaderCache.vertexShader;
        glFragmentShader = shaderCache.fragmentShader;
    }
    else
    {
        const std::string basePath = g_CafeCachePath + '/' + key;

        const std::string vertexShaderPath = basePath + "VS";
        const std::string fragmentShaderPath = basePath + "FS";

        const std::string vertexShaderSrcPath = vertexShaderPath + ".vert";
        const std::string fragmentShaderSrcPath = fragmentShaderPath + ".frag";

        if (FileExists(vertexShaderSrcPath.c_str()) && FileExists(fragmentShaderSrcPath.c_str()))
        {
            assert(ReadFile(vertexShaderSrcPath, &glVertexShader));
            assert(ReadFile(fragmentShaderSrcPath, &glFragmentShader));
        }
        else
        {
            const std::string vertexShaderSpirvPath = vertexShaderSrcPath + ".spv";
            const std::string fragmentShaderSpirvPath = fragmentShaderSrcPath + ".spv";

            printf("\n");
            //printf("%s\n", vertexShaderSpirvPath.c_str());
            //printf("%s\n", fragmentShaderSpirvPath.c_str());

            WriteFile(vertexShaderPath.c_str(), (u8*)vertexShaderBuf, vertexShaderBufSize);
            WriteFile(fragmentShaderPath.c_str(), (u8*)pixelShaderBuf, pixelShaderBufSize);

            std::string cmd;

            {
                std::ostringstream cmdStrm;
                cmdStrm << "\"" << g_CafePath << "/gx2shader-decompiler.exe\" -v \"" << vertexShaderPath << "\" -p \"" << fragmentShaderPath << "\"";
                cmd = cmdStrm.str();
            }

            printf("%s\n", cmd.c_str());
            RunCommand(cmd.c_str());

            assert(FileExists(vertexShaderSpirvPath.c_str()));
            assert(FileExists(fragmentShaderSpirvPath.c_str()));

            DeleteFile(vertexShaderPath.c_str());
            DeleteFile(fragmentShaderPath.c_str());

            {
                std::ostringstream cmdStrm;
                cmdStrm << "\"" << g_CafePath << "/spirv-cross.exe\" \"" << vertexShaderSpirvPath << "\" --no-es "  \
                    "--no-420pack-extension --no-support-nonzero-baseinstance " \
                    "--version 410 --output \"" << vertexShaderSrcPath << "\"";
                cmd = cmdStrm.str();
            }

            printf("%s\n", cmd.c_str());
            RunCommand(cmd.c_str());

            assert(FileExists(vertexShaderSrcPath.c_str()));

            {
                std::ostringstream cmdStrm;
                cmdStrm << "\"" << g_CafePath << "/spirv-cross.exe\" \"" << fragmentShaderSpirvPath << "\" --no-es "  \
                    "--no-420pack-extension --no-support-nonzero-baseinstance " \
                    "--version 410 --output \"" << fragmentShaderSrcPath << "\"";
                cmd = cmdStrm.str();
            }

            printf("%s\n", cmd.c_str());
            RunCommand(cmd.c_str());

            assert(FileExists(fragmentShaderSrcPath.c_str()));

            DeleteFile(vertexShaderSpirvPath.c_str());
            DeleteFile(fragmentShaderSpirvPath.c_str());

            assert(ReadFile(vertexShaderSrcPath, &glVertexShader));
            assert(ReadFile(fragmentShaderSrcPath, &glFragmentShader));

            ReplaceString(glVertexShader, "\r\n", "\n");
            ReplaceString(glFragmentShader, "\r\n", "\n");

            std::vector<GX2UniformBlock> vertexUBOs = std::vector<GX2UniformBlock>(vertexShader->uniformBlocks,
                                                                                   vertexShader->uniformBlocks + vertexShader->numUniformBlocks);

            std::sort(vertexUBOs.begin(), vertexUBOs.end(), GX2UniformBlockComp);

            for (u32 i = 0; i < vertexShader->numUniformBlocks; i++)
            {
                std::ostringstream formatOldStrm;
                formatOldStrm << "layout(std430) readonly buffer CBUFFER_DATA_" << vertexUBOs[i].location << std::endl
                              << "{" << std::endl
                              << "    vec4 values[];" << std::endl
                              << "}";

                std::ostringstream formatNewStrm;
                formatNewStrm << "layout(std140) uniform " << vertexUBOs[i].name << std::endl
                              << "{" << std::endl
                              << "    vec4 values[" << ((vertexUBOs[i].size + 15) / 16) << "];" << std::endl
                              << "}";

                ReplaceString(glVertexShader, formatOldStrm.str(), formatNewStrm.str());
            }

            std::vector<GX2UniformBlock> pixelUBOs = std::vector<GX2UniformBlock>(pixelShader->uniformBlocks,
                                                                                  pixelShader->uniformBlocks + pixelShader->numUniformBlocks);

            std::sort(pixelUBOs.begin(), pixelUBOs.end(), GX2UniformBlockComp);

            for (u32 i = 0; i < pixelShader->numUniformBlocks; i++)
            {
                std::ostringstream formatOldStrm;
                formatOldStrm << "layout(std430) readonly buffer CBUFFER_DATA_" << pixelUBOs[i].location << std::endl
                              << "{" << std::endl
                              << "    vec4 values[];" << std::endl
                              << "}";

                std::ostringstream formatNewStrm;
                formatNewStrm << "layout(std140) uniform " << pixelUBOs[i].name << std::endl
                              << "{" << std::endl
                              << "    vec4 values[" << ((pixelUBOs[i].size + 15) / 16) << "];" << std::endl
                              << "}";

                ReplaceString(glFragmentShader, formatOldStrm.str(), formatNewStrm.str());
            }

            WriteFile(vertexShaderSrcPath, glVertexShader);
            WriteFile(fragmentShaderSrcPath, glFragmentShader);
        }

        g_ShaderCache.insert(ShaderCacheMap::value_type(key, ShaderCache(glVertexShader, glFragmentShader)));
    }

    heap->Free(vertexShaderBuf);
    heap->Free(pixelShaderBuf);
    heap->Free(shaderBuf);

    *vertexShaderSrc = glVertexShader;
    *fragmentShaderSrc = glFragmentShader;
}

namespace nw { namespace eft {

TextureSampler::TextureSampler()
{
    glGenSamplers(1, &mTextureSampler);

    glSamplerParameteri(mTextureSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(mTextureSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glSamplerParameteri(mTextureSampler, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glSamplerParameteri(mTextureSampler, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glSamplerParameteri(mTextureSampler, GL_TEXTURE_WRAP_R, GL_REPEAT);
}

TextureSampler::~TextureSampler()
{
    if (mTextureSampler != GL_NONE)
    {
        glDeleteSamplers(1, &mTextureSampler);
        mTextureSampler = GL_NONE;
    }
}

bool TextureSampler::Setup(TextureFilterMode textureFilter, TextureWrapMode wrapModeU, TextureWrapMode wrapModeV)
{
    if (textureFilter == EFT_TEXTURE_FILTER_TYPE_LINEAR)
    {
        glSamplerParameteri(mTextureSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glSamplerParameteri(mTextureSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    else
    {
        glSamplerParameteri(mTextureSampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glSamplerParameteri(mTextureSampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }

    GLenum clampX = GL_MIRRORED_REPEAT;
    GLenum clampY = GL_MIRRORED_REPEAT;
    GLenum clampZ = GL_REPEAT;

    switch (wrapModeU)
    {
    case EFT_TEXTURE_WRAP_TYPE_MIRROR:              clampX = GL_MIRRORED_REPEAT;      break;
    case EFT_TEXTURE_WRAP_TYPE_REPEAT:              clampX = GL_REPEAT;               break;
    case EFT_TEXTURE_WRAP_TYPE_CLAMP:               clampX = GL_CLAMP_TO_EDGE;        break;
    case EFT_TEXTURE_WRAP_TYPE_MIROOR_ONCE:         clampX = GL_MIRROR_CLAMP_TO_EDGE; break;
    }

    switch (wrapModeV)
    {
    case EFT_TEXTURE_WRAP_TYPE_MIRROR:              clampY = GL_MIRRORED_REPEAT;      break;
    case EFT_TEXTURE_WRAP_TYPE_REPEAT:              clampY = GL_REPEAT;               break;
    case EFT_TEXTURE_WRAP_TYPE_CLAMP:               clampY = GL_CLAMP_TO_EDGE;        break;
    case EFT_TEXTURE_WRAP_TYPE_MIROOR_ONCE:         clampY = GL_MIRROR_CLAMP_TO_EDGE; break;
    }

    glSamplerParameteri(mTextureSampler, GL_TEXTURE_WRAP_S, clampX);
    glSamplerParameteri(mTextureSampler, GL_TEXTURE_WRAP_T, clampY);
    glSamplerParameteri(mTextureSampler, GL_TEXTURE_WRAP_R, clampZ);

    return true;
}

bool TextureSampler::SetupLOD(f32 maxMip, f32 bais)
{
    glSamplerParameterf(mTextureSampler, GL_TEXTURE_MIN_LOD, 0.0f);
    glSamplerParameterf(mTextureSampler, GL_TEXTURE_MAX_LOD, maxMip);
    glSamplerParameterf(mTextureSampler, GL_TEXTURE_LOD_BIAS, bais);

    return true;
}

Rendercontext::Rendercontext()
{
    for (u32 i = 0; i < EFT_TEXTURE_SLOT_MAX; i++)
        mTextureSampler[i].Setup(EFT_TEXTURE_FILTER_TYPE_LINEAR, EFT_TEXTURE_WRAP_TYPE_REPEAT, EFT_TEXTURE_WRAP_TYPE_REPEAT);

    mDefaultTextureSampler.Setup(EFT_TEXTURE_FILTER_TYPE_LINEAR, EFT_TEXTURE_WRAP_TYPE_MIRROR, EFT_TEXTURE_WRAP_TYPE_MIRROR);

    glGenVertexArrays(1, &mVertexArrayId);
}

Rendercontext::~Rendercontext()
{
    if (mVertexArrayId != GL_NONE)
    {
        glDeleteVertexArrays(1, &mVertexArrayId);
        mVertexArrayId = GL_NONE;
    }
}

void Rendercontext::SetupCommonState()
{
    // glEnable(GL_TEXTURE_2D);

    glFrontFace(GL_CCW);
    glDisable(GL_CULL_FACE);

    glEnable(GL_BLEND);

    glBindVertexArray(mVertexArrayId);
    glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_NONE);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
    glDisableVertexAttribArray(4);
    glDisableVertexAttribArray(5);
    glDisableVertexAttribArray(6);
    glDisableVertexAttribArray(7);
    glDisableVertexAttribArray(8);
    glDisableVertexAttribArray(9);
    glDisableVertexAttribArray(10);
    glDisableVertexAttribArray(11);
    glDisableVertexAttribArray(12);
    glDisableVertexAttribArray(13);
    glDisableVertexAttribArray(14);
    glDisableVertexAttribArray(15);
}

void Rendercontext::SetupBlendType(BlendType blendType)
{
    switch (blendType)
    {
    case EFT_BLEND_TYPE_NORMAL:
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquation(GL_FUNC_ADD);
        break;
    case EFT_BLEND_TYPE_ADD:
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glBlendEquation(GL_FUNC_ADD);
        break;
    case EFT_BLEND_TYPE_SUB:
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
        break;
    case EFT_BLEND_TYPE_SCREEN:
        glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ONE);
        glBlendEquation(GL_FUNC_ADD);
        break;
    case EFT_BLEND_TYPE_MULT:
        glBlendFunc(GL_ZERO, GL_SRC_COLOR);
        glBlendEquation(GL_FUNC_ADD);
        break;
    }
}

void Rendercontext::SetupZBufATest(ZBufATestType zBufATestType)
{
    switch (zBufATestType)
    {
    case EFT_ZBUFF_ATEST_TYPE_NORMAL:
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glDepthFunc(GL_LEQUAL);
        Shader::sAlphaTest = GL_GREATER;
        Shader::sAlphaTestRef = 0.0f;
        glEnable(GL_BLEND);
        break;
    case EFT_ZBUFF_ATEST_TYPE_ZIGNORE:
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glDepthFunc(GL_LEQUAL);
        Shader::sAlphaTest = GL_GREATER;
        Shader::sAlphaTestRef = 0.0f;
        glEnable(GL_BLEND);
        break;
    case EFT_ZBUFF_ATEST_TYPE_ENTITY:
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LEQUAL);
        Shader::sAlphaTest = GL_GREATER;
        Shader::sAlphaTestRef = 0.5f;
        glDisable(GL_BLEND);
        break;
    }

    // Apply Alpha Test
    Shader* shader = Shader::sBoundShader;
    if (shader)
        shader->BindShader();
}

void Rendercontext::SetupDisplaySideType(DisplaySideType displaySideType) const
{
    switch (displaySideType)
    {
    case EFT_DISPLAYSIDETYPE_BOTH:
        glDisable(GL_CULL_FACE);
        break;
    case EFT_DISPLAYSIDETYPE_FRONT:
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        break;
    case EFT_DISPLAYSIDETYPE_BACK:
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        break;
    }
}

void Rendercontext::SetupTexture(const TextureRes* texture, TextureSlot slot, FragmentTextureLocation location)
{
    if (texture == NULL || texture->handle == GL_NONE || location.loc == EFT_INVALID_LOCATION)
        return;

    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, texture->handle);

    mTextureSampler[slot].Setup(static_cast<TextureFilterMode>(texture->filterMode),
                                static_cast<TextureWrapMode>( texture->wrapMode       & 0xF),
                                static_cast<TextureWrapMode>((texture->wrapMode >> 4) & 0xF));
    mTextureSampler[slot].SetupLOD(texture->enableMipLevel, texture->mipMapBias);

    glBindSampler(slot, mTextureSampler[slot].GetSampler());
    glUniform1i(location.loc, slot);
}

void Rendercontext::SetupTexture(const GLuint texture, TextureSlot slot, FragmentTextureLocation location)
{
    if (texture == GL_NONE || location.loc == EFT_INVALID_LOCATION)
        return;

    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, texture);

    glBindSampler(slot, mDefaultTextureSampler.GetSampler());
    glUniform1i(location.loc, slot);
}

VertexBuffer::VertexBuffer()
{
    mVertexBufferSize   = 0;
    mVertexBuffer       = NULL;
}

void* VertexBuffer::AllocateVertexBuffer(Heap* heap, u32 size, u32 element)
{
    glGenBuffers(1, &mVertexBufferId);

    mVertexBufferSize = size;
    mVertexBuffer = heap->Alloc(mVertexBufferSize);
    mVertexElement = element;
    return mVertexBuffer;
}

void VertexBuffer::Finalize(Heap* heap)
{
    if (mVertexBuffer != NULL)
    {
        heap->Free(mVertexBuffer);
        mVertexBuffer = NULL;
    }

    if (mVertexBufferId != GL_NONE)
    {
        glDeleteBuffers(1, &mVertexBufferId);
        mVertexBufferId = GL_NONE;
    }
}

void VertexBuffer::Invalidate()
{
    if (mVertexBuffer == NULL || mVertexBufferSize < sizeof(u32))
        return;

    u32* buf_32 = (u32*)mVertexBuffer;
    for (u32 i = 0; i < mVertexBufferSize; i += sizeof(u32))
    {
        *buf_32 = __builtin_bswap32(*buf_32);
        buf_32++;
    }
}

void VertexBuffer::BindBuffer(u32 index, u32 size, u32 stride)
{
    if (index == EFT_INVALID_ATTRIBUTE)
        return;

    // assert(stride == 4 * mVertexElement);

    glBindBuffer(GL_ARRAY_BUFFER, mVertexBufferId);
    glBufferData(GL_ARRAY_BUFFER, mVertexBufferSize, mVertexBuffer, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(index);
    glVertexAttribPointer(index, mVertexElement, GL_FLOAT, GL_FALSE, stride, (void*)0);
}

void VertexBuffer::BindExtBuffer(u32 index, u32 element, u32 stride, u32 offset)
{
    if (index == EFT_INVALID_ATTRIBUTE)
        return;

    glEnableVertexAttribArray(index);
    glVertexAttribPointer(index, element, GL_FLOAT, GL_FALSE, stride, (void*)offset);
}

Shader* Shader::sBoundShader    = NULL;

GLenum  Shader::sAlphaTest      = GL_ALWAYS;
f32     Shader::sAlphaTestRef   = 0.5f;

Shader::Shader()
{
    mCurrentAlphaTest           = GL_ALWAYS;
    mCurrentAlphaTestRef        = 0.5f;

    mpVertexShader              = NULL;
    mpPixelShader               = NULL;
  //mpGeometryShader            = NULL;

    mGFDFile                    = NULL;
    mProgram                    = GL_NONE;
}

void Shader::Finalize(Heap* heap)
{
    if (mGFDFile)
    {
        mGFDFile->~GFDFile();
        heap->Free(mGFDFile);

        mpVertexShader      = NULL;
        mpPixelShader       = NULL;
      //mpGeometryShader    = NULL;
    }

    if (mProgram != GL_NONE)
    {
        glDeleteProgram(mProgram);
        mProgram = GL_NONE;
        sBoundShader = NULL;
    }
}

void Shader::BindShader()
{
    if (mProgram != GL_NONE)
    {
        glUseProgram(mProgram);
        sBoundShader = this;

        if (sAlphaTest != mCurrentAlphaTest)
        {
            switch (sAlphaTest)
            {
            case GL_NEVER:
                glUniform1ui(glGetUniformLocation(mProgram, "PS_PUSH.alphaFunc"), 0); break;
            case GL_LESS:
                glUniform1ui(glGetUniformLocation(mProgram, "PS_PUSH.alphaFunc"), 1); break;
            case GL_EQUAL:
                glUniform1ui(glGetUniformLocation(mProgram, "PS_PUSH.alphaFunc"), 2); break;
            case GL_LEQUAL:
                glUniform1ui(glGetUniformLocation(mProgram, "PS_PUSH.alphaFunc"), 3); break;
            case GL_GREATER:
                glUniform1ui(glGetUniformLocation(mProgram, "PS_PUSH.alphaFunc"), 4); break;
            case GL_NOTEQUAL:
                glUniform1ui(glGetUniformLocation(mProgram, "PS_PUSH.alphaFunc"), 5); break;
            case GL_GEQUAL:
                glUniform1ui(glGetUniformLocation(mProgram, "PS_PUSH.alphaFunc"), 6); break;
            case GL_ALWAYS:
            default:
                glUniform1ui(glGetUniformLocation(mProgram, "PS_PUSH.alphaFunc"), 7); break;
            }
            mCurrentAlphaTest = sAlphaTest;
        }

        if (sAlphaTestRef != mCurrentAlphaTestRef)
        {
            glUniform1f (glGetUniformLocation(mProgram, "PS_PUSH.alphaRef"), sAlphaTestRef);
            mCurrentAlphaTestRef = sAlphaTestRef;
        }
    }
}

bool Shader::CreateShader(Heap* heap, const void* binary, u32 binarySize)
{
    mGFDFile = new (heap->Alloc(sizeof(GFDFile))) GFDFile;
    assert(mGFDFile->load(binary) == binarySize);
    //printf("Shader, binary size: %d\n", binarySize);

    mpVertexShader      = &mGFDFile->mVertexShaders[0];
    mpPixelShader       = &mGFDFile->mPixelShaders[0];
  //mpGeometryShader    = mGFDFile->mGeometryShaders.size() > 0 ? &mGFDFile->mGeometryShaders[0] : NULL;

    assert(mpVertexShader != NULL);
    assert(mpPixelShader != NULL);
    assert(mGFDFile->mGeometryShaders.size() == 0);

    std::string vertexShaderSrc;
    std::string fragmentShaderSrc;
    DecompileProgram(heap,
                     mpVertexShader,
                     mpPixelShader,
                     &vertexShaderSrc,
                     &fragmentShaderSrc);

    mProgram = CompileProgram(vertexShaderSrc.c_str(), fragmentShaderSrc.c_str());
    if (mProgram == GL_NONE)
        return false;

    glUseProgram(mProgram);

    glUniform4f(glGetUniformLocation(mProgram, "VS_PUSH.posMulAdd"), 1.0f, -1.0f, 0.0f, 0.0f);
    glUniform4f(glGetUniformLocation(mProgram, "VS_PUSH.zSpaceMul"), 0.0f,  1.0f, 1.0f, 1.0f);
    glUniform1f(glGetUniformLocation(mProgram, "VS_PUSH.pointSize"), 1.0f);

    glUniform1ui(glGetUniformLocation(mProgram, "PS_PUSH.alphaFunc"),        4);
    glUniform1f (glGetUniformLocation(mProgram, "PS_PUSH.alphaRef"),         0.0f);
    glUniform1ui(glGetUniformLocation(mProgram, "PS_PUSH.needsPremultiply"), 0);

    mCurrentAlphaTest = GL_GREATER;
    mCurrentAlphaTestRef = 0.0f;

    BindShader();

    return true;
}

u32 Shader::GetFragmentSamplerLocation(const char* name)
{
    if (!IsInitialized())
        return EFT_INVALID_SAMPLER;

    for (u32 i = 0; i < mpPixelShader->numSamplers; i++)
    {
        if (std::strcmp(mpPixelShader->samplerVars[i].name, name) == 0)
        {
            assert(mpPixelShader->samplerVars[i].type == 1);
            u32 location = mpPixelShader->samplerVars[i].location;
            std::ostringstream samplerNameStrm;
            samplerNameStrm << "SPIRV_Cross_CombinedTEXTURE_" << location << "SAMPLER_" << location;
            return glGetUniformLocation(mProgram, samplerNameStrm.str().c_str());
        }
    }

    return EFT_INVALID_SAMPLER;
}

u32 Shader::GetAttribute(const char* name, u32 index, VertexFormat fmt, u32 offset, bool instancingAttr)
{
    if (!IsInitialized())
        return EFT_INVALID_ATTRIBUTE;

    for (u32 i = 0; i < mpVertexShader->numAttribs; i++)
    {
        if (std::strcmp(mpVertexShader->attribVars[i].name, name) == 0)
        {
            std::ostringstream attribNameStrm;
            attribNameStrm << name << "_0_0";
            return glGetAttribLocation(mProgram, attribNameStrm.str().c_str());
        }
    }

    return EFT_INVALID_ATTRIBUTE;
}

bool UniformBlock::InitializeVertexUniformBlock(Shader* shader, const char* name, u32 bindPoint)
{
    if (!shader->IsInitialized())
    {
failure:
        mIsFailed = true;
        return false;
    }

    mBufferBindPoint = bindPoint;

    mBufferIndex = glGetUniformBlockIndex(shader->GetProgramID(), name);
    if (mBufferIndex == GL_INVALID_INDEX)
        goto failure;

    for (u32 i = 0; i < shader->GetVertexShader()->numUniformBlocks; i++)
    {
        if (std::strcmp(shader->GetVertexShader()->uniformBlocks[i].name, name) == 0)
        {
            mBufferSize = shader->GetVertexShader()->uniformBlocks[i].size;
            goto continue_1;
        }
    }

    goto failure;

continue_1:
    glGenBuffers(1, &mBufferId);
    glBindBuffer(GL_UNIFORM_BUFFER, mBufferId);
    glBufferData(GL_UNIFORM_BUFFER, mBufferSize, NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, mBufferBindPoint, mBufferId);
    glUniformBlockBinding(shader->GetProgramID(), mBufferIndex, mBufferBindPoint);

    mIsInitialized = true;

    return true;
}

bool UniformBlock::InitializePixelUniformBlock(Shader* shader, const char* name, u32 bindPoint)
{
    if (!shader->IsInitialized())
    {
failure:
        mIsFailed = true;
        return false;
    }

    mBufferBindPoint = bindPoint;

    mBufferIndex = glGetUniformBlockIndex(shader->GetProgramID(), name);
    if (mBufferIndex == GL_INVALID_INDEX)
        goto failure;

    for (u32 i = 0; i < shader->GetPixelShader()->numUniformBlocks; i++)
    {
        if (std::strcmp(shader->GetPixelShader()->uniformBlocks[i].name, name) == 0)
        {
            mBufferSize = shader->GetPixelShader()->uniformBlocks[i].size;
            goto continue_1;
        }
    }

    goto failure;

continue_1:
    glGenBuffers(1, &mBufferId);
    glBindBuffer(GL_UNIFORM_BUFFER, mBufferId);
    glBufferData(GL_UNIFORM_BUFFER, mBufferSize, NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, mBufferBindPoint, mBufferId);
    glUniformBlockBinding(shader->GetProgramID(), mBufferIndex, mBufferBindPoint);

    mIsInitialized = true;

    return true;
}

void UniformBlock::BindUniformBlock(const void* buffer)
{
    if (!mIsInitialized || mBufferSize == 0)
        return;

    glBindBuffer(GL_UNIFORM_BUFFER, mBufferId);
    glBindBufferBase(GL_UNIFORM_BUFFER, mBufferBindPoint, mBufferId);
    glBufferData(GL_UNIFORM_BUFFER, mBufferSize, buffer, GL_DYNAMIC_DRAW);
}

} } // namespace nw::eft

#endif // EFT_WIN
