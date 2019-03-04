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
    : m_ShaderDatabase(nullptr)
{
    ShaderKey::InitializeShaderKeyGroups();
}

ShaderHandlerManager::~ShaderHandlerManager()
{
    for (const std::pair<ShaderKey, ShaderHandler*>& keyValue: m_ShaderHandlers)
    {
        ShaderHandler* shaderHandler = keyValue.second;

        delete shaderHandler;
    }

    m_ShaderHandlers.clear();
}

void ShaderHandlerManager::SetShaderDatabase(ShaderDatabase& shaderDatabase)
{
    m_ShaderDatabase = &shaderDatabase;

    ShaderKey errorShaderKey;
    errorShaderKey.SetShaderFamily(ShaderFamily::Error);

    ShaderDatabase::ShaderEntrySet compiledShaderEntrySet;
    uint64_t lastModifiedTimestamp = 0;

    if (m_ShaderDatabase->RetrieveShadersForShaderKey(errorShaderKey, lastModifiedTimestamp, compiledShaderEntrySet))
    {
        return;
    }

    bool dummy = false;
    ShaderCompiler::GetInstance().GetRawShadersForShaderKey(errorShaderKey, compiledShaderEntrySet, dummy);

    m_ShaderDatabase->AppendShadersForShaderKey(errorShaderKey, compiledShaderEntrySet);
}

ShaderHandler* ShaderHandlerManager::GetShaderHandlerForShaderKey(ShaderKey shaderKey, GFXRenderDevice& gfxRenderDevice)
{
    assert(m_ShaderDatabase != nullptr);

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

            assert(foundValidShaderInDatabase);
        }

        createShaders = true;
    }
    else
    {
        shaderHandler = it->second;
    }

    if (createShaders)
    {
        if (compiledShaderEntrySet.rawVertexShaderSize > 0)
        {
            shaderHandler->m_VertexShader.reset(
                    gfxRenderDevice.CreateVertexShader(compiledShaderEntrySet.rawVertexShader, compiledShaderEntrySet.rawVertexShaderSize));
        }
        else
        {
            shaderHandler->m_VertexShader.reset();
        }

        if (compiledShaderEntrySet.rawPixelShaderSize > 0)
        {
            shaderHandler->m_PixelShader.reset(
                    gfxRenderDevice.CreatePixelShader(compiledShaderEntrySet.rawPixelShader, compiledShaderEntrySet.rawPixelShaderSize));
        }
        else
        {
            shaderHandler->m_PixelShader.reset();
        }
    }

    shaderHandler->m_RenderStateBlock = compiledShaderEntrySet.renderStateBlock;

    return shaderHandler;
}

}