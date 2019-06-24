#include <graphics/graphicssingletonstorer.h>

#include <graphics/material/materialunifiedconstantbuffer.h>

#include <graphics/shader/shaderhandlermanager.h>

#include <graphics/shadercompiler/shadercompiler.h>
#include <graphics/shadercompiler/shaderwatcher.h>

#include <system/logger.h>

namespace Shipyard
{;

GraphicsSingletonStorer::GraphicsSingletonStorer()
{
    ShaderHandlerManager::CreateInstance();
    ShaderCompiler::CreateInstance();
    ShaderWatcher::CreateInstance();

    GFXMaterialUnifiedConstantBuffer::CreateInstance();
}

GraphicsSingletonStorer::~GraphicsSingletonStorer()
{
    GFXMaterialUnifiedConstantBuffer::DestroyInstance();

    ShaderWatcher::DestroyInstance();
    ShaderCompiler::DestroyInstance();
    ShaderHandlerManager::DestroyInstance();
}

}