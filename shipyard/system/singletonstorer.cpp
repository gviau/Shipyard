#include <system/singletonstorer.h>

#include <graphics/shaderhandlermanager.h>
#include <graphics/shadercompiler/shadercompiler.h>
#include <graphics/shadercompiler/shaderwatcher.h>

#include <system/logger.h>

namespace Shipyard
{;

SingletonStorer::SingletonStorer()
{
    ShaderCompiler::CreateInstance();
    ShaderWatcher::CreateInstance();
    ShaderHandlerManager::CreateInstance();
}

SingletonStorer::~SingletonStorer()
{
    ShaderHandlerManager::DestroyInstance();
    ShaderWatcher::DestroyInstance();
    ShaderCompiler::DestroyInstance();
}

}