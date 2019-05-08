#include <system/singletonstorer.h>

#include <graphics/shaderhandlermanager.h>
#include <graphics/shadercompiler/shadercompiler.h>
#include <graphics/shadercompiler/shaderwatcher.h>

#include <system/logger.h>

namespace Shipyard
{;

SingletonStorer::SingletonStorer()
{
    ShaderHandlerManager::CreateInstance();
    ShaderCompiler::CreateInstance();
    ShaderWatcher::CreateInstance();
}

SingletonStorer::~SingletonStorer()
{
    ShaderWatcher::DestroyInstance();
    ShaderCompiler::DestroyInstance();
    ShaderHandlerManager::DestroyInstance();
}

}