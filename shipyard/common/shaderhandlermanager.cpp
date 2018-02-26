#include <common/shaderhandlermanager.h>

#include <common/shaderfamilies.h>
#include <common/shaderhandler.h>

#include <common/shadercompiler/shadercompiler.h>

#include <common/wrapper/wrapper.h>

namespace Shipyard
{;

ShaderHandlerManager::~ShaderHandlerManager()
{
    for (const pair<ShaderKey, ShaderHandler*>& keyValue: m_ShaderHandlers)
    {
        ShaderHandler* shaderHandler = keyValue.second;

        delete shaderHandler;
    }

    m_ShaderHandlers.clear();
}

ShaderHandler* ShaderHandlerManager::GetShaderHandlerForShaderKey(ShaderKey shaderKey, GFXRenderDevice& gfxRenderDevice)
{
    ShaderHandler* shaderHandler = nullptr;

    ShaderCompiler& shaderCompiler = ShaderCompiler::GetInstance();

    ID3D10Blob* vertexShaderBlob = nullptr;
    ID3D10Blob* pixelShaderBlob = nullptr;
    ID3D10Blob* computeShaderBlob = nullptr;
    bool gotRecompiledSinceLastAccess = false;

    bool isShaderKeyCompiled = shaderCompiler.GetShaderBlobsForShaderKey(shaderKey, vertexShaderBlob, pixelShaderBlob, computeShaderBlob, gotRecompiledSinceLastAccess);

    if (!isShaderKeyCompiled)
    {
        ShaderKey errorShaderKey;
        errorShaderKey.SetShaderFamily(ShaderFamily::Error);

        shaderKey = errorShaderKey;
    }

    bool createShaders = gotRecompiledSinceLastAccess;

    auto it = m_ShaderHandlers.find(shaderKey);
    if (it == m_ShaderHandlers.end())
    {
        shaderHandler = new ShaderHandler(shaderKey);
        m_ShaderHandlers[shaderKey] = shaderHandler;

        createShaders = true;
    }
    else
    {
        shaderHandler = it->second;
    }

    if (createShaders)
    {
        if (vertexShaderBlob != nullptr)
        {
            shaderHandler->m_VertexShader.reset(gfxRenderDevice.CreateVertexShader(vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize()));
        }
        else
        {
            shaderHandler->m_VertexShader.reset();
        }

        if (pixelShaderBlob != nullptr)
        {
            shaderHandler->m_PixelShader.reset(gfxRenderDevice.CreatePixelShader(pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize()));
        }
        else
        {
            shaderHandler->m_PixelShader.reset();
        }
    }

    return shaderHandler;
}

}