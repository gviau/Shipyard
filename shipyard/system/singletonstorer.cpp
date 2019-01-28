#include <system/singletonstorer.h>

#include <common/shaderhandlermanager.h>
#include <common/shadercompiler/shadercompiler.h>
#include <common/shadercompiler/shaderwatcher.h>

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