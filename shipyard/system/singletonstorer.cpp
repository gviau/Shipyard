#include <system/singletonstorer.h>

#include <common/shaderhandlermanager.h>
#include <common/shadercompiler/shadercompiler.h>

namespace Shipyard
{;

SingletonStorer::SingletonStorer()
{
    ShaderCompiler::CreateInstance();
    ShaderHandlerManager::CreateInstance();
}

SingletonStorer::~SingletonStorer()
{
    ShaderHandlerManager::DestroyInstance();
    ShaderCompiler::DestroyInstance();
}

}