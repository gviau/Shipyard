#include <graphics/shaderhandlermanager.h>

#include <graphics/shaderdatabase.h>
#include <graphics/shaderfamilies.h>
#include <graphics/shaderhandler.h>

#include <graphics/shadercompiler/shadercompiler.h>
#include <graphics/shadercompiler/shaderwatcher.h>

#include <graphics/wrapper/wrapper.h>

namespace Shipyard
{;

ShaderHandlerManager::ShaderHandlerManager()
    : m_RenderDevice(nullptr)
    , m_ShaderDatabase(nullptr)
{
    ShaderKey::InitializeShaderKeyGroups();
}

ShaderHandlerManager::~ShaderHandlerManager()
{
    Destroy();
}

bool ShaderHandlerManager::Initialize(GFXRenderDevice& gfxRenderDevice, ShaderDatabase& shaderDatabase)
{
    m_RenderDevice = &gfxRenderDevice;

    m_ShaderDatabase = &shaderDatabase;

    ShaderKey errorShaderKey;
    errorShaderKey.SetShaderFamily(ShaderFamily::Error);

    ShaderDatabase::ShaderEntrySet compiledShaderEntrySet;
    uint64_t lastModifiedTimestamp = 0;

    if (m_ShaderDatabase->RetrieveShadersForShaderKey(errorShaderKey, lastModifiedTimestamp, compiledShaderEntrySet))
    {
        return false;
    }

    bool dummy = false;
    ShaderCompiler::GetInstance().GetRawShadersForShaderKey(errorShaderKey, compiledShaderEntrySet, dummy);

    m_ShaderDatabase->AppendShadersForShaderKey(errorShaderKey, compiledShaderEntrySet);

    return true;
}

void ShaderHandlerManager::Destroy()
{
    if (m_RenderDevice == nullptr)
    {
        return;
    }

    for (const std::pair<ShaderKey, ShaderHandler*>& keyValue : m_ShaderHandlers)
    {
        ShaderHandler* shaderHandler = keyValue.second;

        if (shaderHandler->m_VertexShaderHandle.handle != InvalidGfxHandle)
        {
            m_RenderDevice->DestroyVertexShader(shaderHandler->m_VertexShaderHandle);
        }

        if (shaderHandler->m_PixelShaderHandle.handle != InvalidGfxHandle)
        {
            m_RenderDevice->DestroyPixelShader(shaderHandler->m_PixelShaderHandle);
        }

        delete shaderHandler;
    }

    m_ShaderHandlers.clear();

    m_RenderDevice = nullptr;
}

ShaderHandler* ShaderHandlerManager::GetShaderHandlerForShaderKey(ShaderKey shaderKey)
{
    SHIP_ASSERT(m_RenderDevice != nullptr);
    SHIP_ASSERT(m_ShaderDatabase != nullptr);

    ShaderWatcher& shaderWatcher = ShaderWatcher::GetInstance();
    ShaderCompiler& shaderCompiler = ShaderCompiler::GetInstance();

    uint64_t lastModifiedTimestamp = shaderWatcher.GetTimestampForShaderKey(shaderKey);

    ShaderDatabase::ShaderEntrySet compiledShaderEntrySet;
    bool foundValidShaderInDatabase = m_ShaderDatabase->RetrieveShadersForShaderKey(shaderKey, lastModifiedTimestamp, compiledShaderEntrySet);

    bool gotRecompiledSinceLastAccess = false;
    bool isShaderKeyCompiled = true;

    if (foundValidShaderInDatabase)
    {
        if (lastModifiedTimestamp > compiledShaderEntrySet.lastModifiedTimestamp)
        {
            m_ShaderDatabase->RemoveShadersForShaderKey(shaderKey);

            shaderCompiler.AddCompilationRequestForShaderKey(shaderKey);

            isShaderKeyCompiled = false;
        }
    }
    else
    {
        isShaderKeyCompiled = shaderCompiler.GetRawShadersForShaderKey(shaderKey, compiledShaderEntrySet, gotRecompiledSinceLastAccess);

        if (isShaderKeyCompiled)
        {
            compiledShaderEntrySet.lastModifiedTimestamp = lastModifiedTimestamp;
            m_ShaderDatabase->AppendShadersForShaderKey(shaderKey, compiledShaderEntrySet);
        }
    }

    if (!isShaderKeyCompiled)
    {
        shaderCompiler.AddCompilationRequestForShaderKey(shaderKey);

        ShaderKey errorShaderKey;
        errorShaderKey.SetShaderFamily(ShaderFamily::Error);

        shaderKey = errorShaderKey;
    }

    ShaderHandler* shaderHandler = nullptr;

    bool createShaders = gotRecompiledSinceLastAccess;

    auto it = m_ShaderHandlers.find(shaderKey);
    if (it == m_ShaderHandlers.end())
    {
        shaderHandler = new ShaderHandler(shaderKey);

        m_ShaderHandlers[shaderKey] = shaderHandler;

        if (shaderKey.GetShaderFamily() == ShaderFamily::Error)
        {
            foundValidShaderInDatabase = m_ShaderDatabase->RetrieveShadersForShaderKey(shaderKey, lastModifiedTimestamp, compiledShaderEntrySet);

            SHIP_ASSERT(foundValidShaderInDatabase);
        }

        createShaders = true;
    }
    else
    {
        shaderHandler = it->second;
    }

    if (createShaders)
    {
        if (shaderHandler->m_VertexShaderHandle.handle != InvalidGfxHandle)
        {
            m_RenderDevice->DestroyVertexShader(shaderHandler->m_VertexShaderHandle);
            shaderHandler->m_VertexShaderHandle.handle = InvalidGfxHandle;
        }

        if (shaderHandler->m_PixelShaderHandle.handle != InvalidGfxHandle)
        {
            m_RenderDevice->DestroyPixelShader(shaderHandler->m_PixelShaderHandle);
            shaderHandler->m_PixelShaderHandle.handle = InvalidGfxHandle;
        }

        if (compiledShaderEntrySet.rawVertexShaderSize > 0)
        {
            shaderHandler->m_VertexShaderHandle = m_RenderDevice->CreateVertexShader(compiledShaderEntrySet.rawVertexShader, compiledShaderEntrySet.rawVertexShaderSize);
        }

        if (compiledShaderEntrySet.rawPixelShaderSize > 0)
        {
            shaderHandler->m_PixelShaderHandle = m_RenderDevice->CreatePixelShader(compiledShaderEntrySet.rawPixelShader, compiledShaderEntrySet.rawPixelShaderSize);
        }
    }

    shaderHandler->m_RenderStateBlock = compiledShaderEntrySet.renderStateBlock;

    return shaderHandler;
}

}