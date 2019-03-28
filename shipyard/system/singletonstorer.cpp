#include <system/singletonstorer.h>

#include <graphics/shaderhandlermanager.h>
#include <graphics/shadercompiler/shadercompiler.h>
#include <graphics/shadercompiler/shaderwatcher.h>

#include <system/logger.h>

namespace Shipyard
{;

SingletonStorer::SingletonStorer()
{
#ifdef SHIP_ENABLE_LOGGING
    Logger::CreateInstance();
#endif // #ifdef SHIP_ENABLE_LOGGING

    ShaderCompiler::CreateInstance();
    ShaderWatcher::CreateInstance();
    ShaderHandlerManager::CreateInstance();
}

SingletonStorer::~SingletonStorer()
{
    ShaderHandlerManager::DestroyInstance();
    ShaderWatcher::DestroyInstance();
    ShaderCompiler::DestroyInstance();

#ifdef SHIP_ENABLE_LOGGING
    Logger::DestroyInstance();
#endif // #ifdef SHIP_ENABLE_LOGGING
}

}