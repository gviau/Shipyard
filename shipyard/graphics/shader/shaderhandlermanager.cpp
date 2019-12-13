#include <graphics/graphicsprecomp.h>

#include <graphics/shader/shaderhandlermanager.h>

#include <graphics/shader/shaderdatabase.h>
#include <graphics/shader/shaderfamilies.h>
#include <graphics/shader/shaderhandler.h>
#include <graphics/shader/shadervariationset.h>
#include <graphics/shader/shadervariationsetmanager.h>

#include <graphics/shadercompiler/shadercompiler.h>
#include <graphics/shadercompiler/shaderwatcher.h>

#include <graphics/wrapper/wrapper.h>

namespace Shipyard
{;

template SHIPYARD_GRAPHICS_API class GraphicsSingleton<ShaderHandlerManager>;

SHIP_DECLARE_SHADER_VARIATION_SET_BEGIN(Error)
{
    shaderVariationSetManager.SetShaderOptionValueRange(ShaderOption::ShaderOption_VERTEX_FORMAT_TYPE, 0, shipUint32(VertexFormatType::VertexFormatType_Count));
}
SHIP_DECLARE_SHADER_VARIATION_SET_END(Error)

SHIP_DECLARE_SHADER_VARIATION_SET_BEGIN(Generic)
{
    shaderVariationSetManager.SetShaderOptionValueRange(ShaderOption::ShaderOption_VERTEX_FORMAT_TYPE, 0, shipUint32(VertexFormatType::Pos_UV_Normal));

    {
        ShaderKey invalidShaderKey;
        invalidShaderKey.SetShaderFamily(ShaderFamily::Generic);
        SET_SHADER_OPTION(invalidShaderKey, Test1Bit, 1);
        SET_SHADER_OPTION(invalidShaderKey, Test2Bits, 1);

        shaderVariationSetManager.SetInvalidPartialShaderKey(invalidShaderKey);
    }
}
SHIP_DECLARE_SHADER_VARIATION_SET_END(Generic)

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

    // Mandatory shader families for which we have to wait for to go on.
    InplaceArray<ShaderFamily, 2> mandatoryShaderFamilies;
    mandatoryShaderFamilies.Add(ShaderFamily::Error);
    mandatoryShaderFamilies.Add(ShaderFamily::ImGui);

    BigArray<ShaderKey> mandatoryShaderKeys;

    for (ShaderFamily shaderFamily : mandatoryShaderFamilies)
    {
        ShaderKey::GetEveryValidShaderKeyForShaderFamily(shaderFamily, mandatoryShaderKeys);
    }

    for (ShaderKey shaderKey : mandatoryShaderKeys)
    {
        ShaderDatabase::ShaderEntrySet compiledShaderEntrySet;
        if (m_ShaderDatabase->RetrieveShadersForShaderKey(shaderKey, compiledShaderEntrySet))
        {
            continue;
        }

        ShaderCompiler::GetInstance().AddCompilationRequestForShaderKey(shaderKey);

        shipBool isShaderCompiled = false;

        do
        {
            shipBool dummy = false;
            isShaderCompiled = ShaderCompiler::GetInstance().GetRawShadersForShaderKey(shaderKey, compiledShaderEntrySet, dummy);
        } while (!isShaderCompiled);

        m_ShaderDatabase->AppendShadersForShaderKey(shaderKey, compiledShaderEntrySet);
    }

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

        if (shaderHandler->m_GfxVertexShaderHandle.handle != InvalidGfxHandle)
        {
            m_RenderDevice->DestroyVertexShader(shaderHandler->m_GfxVertexShaderHandle);
        }

        if (shaderHandler->m_GfxPixelShaderHandle.handle != InvalidGfxHandle)
        {
            m_RenderDevice->DestroyPixelShader(shaderHandler->m_GfxPixelShaderHandle);
        }

        if (shaderHandler->m_ShaderRenderElements.GfxPipelineStateObjectHandle.handle != InvalidGfxHandle)
        {
            m_RenderDevice->DestroyPipelineStateObject(shaderHandler->m_ShaderRenderElements.GfxPipelineStateObjectHandle);
        }

        if (shaderHandler->m_ShaderRenderElements.GfxRootSignatureHandle.handle != InvalidGfxHandle)
        {
            m_RenderDevice->DestroyRootSignature(shaderHandler->m_ShaderRenderElements.GfxRootSignatureHandle);
        }

        if (shaderHandler->m_ShaderRenderElements.GfxDescriptorSetHandle.handle != InvalidGfxHandle)
        {
            m_RenderDevice->DestroyDescriptorSet(shaderHandler->m_ShaderRenderElements.GfxDescriptorSetHandle);
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

    ShaderVariationSetManager& shaderVariationSetManager = GetShaderVariationSetManager();
    if (!shaderVariationSetManager.ValidateShaderKey(shaderKey, ShaderVariationSetManager::ShaderKeyValidationOption::AssertOnError))
    {
        return nullptr;
    }

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
        if (shaderHandler->m_GfxVertexShaderHandle.handle != InvalidGfxHandle)
        {
            m_RenderDevice->DestroyVertexShader(shaderHandler->m_GfxVertexShaderHandle);
            shaderHandler->m_GfxVertexShaderHandle.handle = InvalidGfxHandle;
        }

        if (shaderHandler->m_GfxPixelShaderHandle.handle != InvalidGfxHandle)
        {
            m_RenderDevice->DestroyPixelShader(shaderHandler->m_GfxPixelShaderHandle);
            shaderHandler->m_GfxPixelShaderHandle.handle = InvalidGfxHandle;
        }

        if (compiledShaderEntrySet.rawVertexShaderSize > 0)
        {
            shaderHandler->m_GfxVertexShaderHandle = m_RenderDevice->CreateVertexShader(compiledShaderEntrySet.rawVertexShader, compiledShaderEntrySet.rawVertexShaderSize);
        }

        if (compiledShaderEntrySet.rawPixelShaderSize > 0)
        {
            shaderHandler->m_GfxPixelShaderHandle = m_RenderDevice->CreatePixelShader(compiledShaderEntrySet.rawPixelShader, compiledShaderEntrySet.rawPixelShaderSize);
        }

        if (shaderHandler->m_ShaderRenderElements.GfxRootSignatureHandle.handle != InvalidGfxHandle)
        {
            m_RenderDevice->DestroyRootSignature(shaderHandler->m_ShaderRenderElements.GfxRootSignatureHandle);
            shaderHandler->m_ShaderRenderElements.GfxRootSignatureHandle.handle = InvalidGfxHandle;
        }

        shaderHandler->m_ShaderRenderElements.GfxRootSignatureHandle = m_RenderDevice->CreateRootSignature(compiledShaderEntrySet.rootSignatureParameters);

        if (shaderHandler->m_ShaderRenderElements.GfxPipelineStateObjectHandle.handle != InvalidGfxHandle)
        {
            m_RenderDevice->DestroyPipelineStateObject(shaderHandler->m_ShaderRenderElements.GfxPipelineStateObjectHandle);
            shaderHandler->m_ShaderRenderElements.GfxPipelineStateObjectHandle.handle = InvalidGfxHandle;
        }

        PipelineStateObjectCreationParameters pipelineStateObjectCreationParameters;
        pipelineStateObjectCreationParameters.GfxRootSignatureHandle = shaderHandler->m_ShaderRenderElements.GfxRootSignatureHandle;
        pipelineStateObjectCreationParameters.RenderStateBlockToUse = compiledShaderEntrySet.renderStateBlock;
        pipelineStateObjectCreationParameters.GfxVertexShaderHandle = shaderHandler->m_GfxVertexShaderHandle;
        pipelineStateObjectCreationParameters.GfxPixelShaderHandle = shaderHandler->m_GfxPixelShaderHandle;

        shaderHandler->m_ShaderRenderElements.GfxPipelineStateObjectHandle = m_RenderDevice->CreatePipelineStateObject(pipelineStateObjectCreationParameters);

        if (shaderHandler->m_ShaderRenderElements.GfxDescriptorSetHandle.handle != InvalidGfxHandle)
        {
            m_RenderDevice->DestroyDescriptorSet(shaderHandler->m_ShaderRenderElements.GfxDescriptorSetHandle);
            shaderHandler->m_ShaderRenderElements.GfxDescriptorSetHandle.handle = InvalidGfxHandle;
        }

        shaderHandler->m_ShaderRenderElements.GfxDescriptorSetHandle = m_RenderDevice->CreateDescriptorSet(DescriptorSetType::ConstantBuffer_ShaderResource_UnorderedAccess_Views, compiledShaderEntrySet.descriptorSetEntryDeclarations);

        shaderHandler->m_ShaderResourceBinder = compiledShaderEntrySet.shaderResourceBinder;

        for (GFXSamplerHandle gfxSamplerStateHandle : shaderHandler->m_SamplerStateHandles)
        {
            if (gfxSamplerStateHandle.handle != InvalidGfxHandle)
            {
                m_RenderDevice->DestroySampler(gfxSamplerStateHandle);
            }
        }
        shaderHandler->m_SamplerStateHandles.Clear();

        for (const SamplerState& samplerState : compiledShaderEntrySet.samplerStates)
        {
            GFXSamplerHandle gfxSamplerHandle = m_RenderDevice->CreateSampler(samplerState);
            shaderHandler->m_SamplerStateHandles.Add(gfxSamplerHandle);
        }
    }

    return shaderHandler;
}

SHIPYARD_GRAPHICS_API ShaderHandlerManager& GetShaderHandlerManager()
{
    return ShaderHandlerManager::GetInstance();
}

}