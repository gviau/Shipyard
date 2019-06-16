#include <graphics/graphicssingletonstorer.h>

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
}

GraphicsSingletonStorer::~GraphicsSingletonStorer()
{
    ShaderWatcher::DestroyInstance();
    ShaderCompiler::DestroyInstance();
    ShaderHandlerManager::DestroyInstance();
}

}