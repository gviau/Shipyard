#include <graphics/shader/shaderhandlermanager.h>

#include <graphics/shader/shaderdatabase.h>
#include <graphics/shader/shaderfamilies.h>
#include <graphics/shader/shaderhandler.h>

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

shipBool ShaderHandlerManager::Initialize(GFXRenderDevice& gfxRenderDevice, ShaderDatabase& shaderDatabase)
{
    m_RenderDevice = &gfxRenderDevice;

    m_ShaderDatabase = &shaderDatabase;

    ShaderKey errorShaderKey;
    errorShaderKey.SetShaderFamily(ShaderFamily::Error);

    ShaderDatabase::ShaderEntrySet compiledShaderEntrySet;
    if (m_ShaderDatabase->RetrieveShadersForShaderKey(errorShaderKey, compiledShaderEntrySet))
    {
        return false;
    }

    shipBool dummy = false;
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

        SHIP_DELETE(shaderHandler);
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

    shipUint64 lastModifiedTimestamp = shaderWatcher.GetTimestampForShaderKey(shaderKey);

    ShaderDatabase::ShaderEntrySet compiledShaderEntrySet;
    shipBool foundValidShaderInDatabase = m_ShaderDatabase->RetrieveShadersForShaderKey(shaderKey, compiledShaderEntrySet);

    shipBool gotRecompiledSinceLastAccess = false;
    shipBool isShaderKeyCompiled = true;

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

    shipBool createShaders = gotRecompiledSinceLastAccess;

    auto it = m_ShaderHandlers.find(shaderKey);
    if (it == m_ShaderHandlers.end())
    {
        shaderHandler = SHIP_NEW(ShaderHandler, 1)(shaderKey);

        m_ShaderHandlers[shaderKey] = shaderHandler;

        if (shaderKey.GetShaderFamily() == ShaderFamily::Error)
        {
            foundValidShaderInDatabase = m_ShaderDatabase->RetrieveShadersForShaderKey(shaderKey, compiledShaderEntrySet);

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

SHIPYARD_API ShaderHandlerManager& GetShaderHandlerManager()
{
    return ShaderHandlerManager::GetInstance();
}

}