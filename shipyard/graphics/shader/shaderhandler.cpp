#include <graphics/shader/shaderhandler.h>

#include <graphics/material/materialunifiedconstantbuffer.h>

#include <graphics/shader/shaderinputprovider.h>

#include <graphics/wrapper/wrapper.h>

namespace Shipyard
{;

ShaderHandler::ShaderHandler(ShaderKey shaderKey)
    : m_ShaderKey(shaderKey)
{
}

ShaderHandler::~ShaderHandler()
{
}

void ShaderHandler::ApplyShaderInputProviders(
        GFXRenderDevice& gfxRenderDevice,
        GFXDirectRenderCommandList& gfxDirectRenderCommandList,
        const Array<const ShaderInputProvider*>& shaderInputProviders)
{
    if (shaderInputProviders.Empty())
    {
        return;
    }

    GFXMaterialUnifiedConstantBuffer& gfxMaterialUnifiedConstantBuffer = GetGFXMaterialUnifiedConstantBuffer();

    for (const ShaderInputProvider* shaderInputProvider : shaderInputProviders)
    {
        SHIP_ASSERT(shaderInputProvider != nullptr);

        if (shaderInputProvider->GetRequiredSizeForProvider() == 0)
        {
            continue;
        }

        ShaderInputProviderDeclaration* shaderInputProviderDeclaration = shaderInputProvider->GetShaderInputProviderDeclaration();

        if (ShaderInputProviderUtils::IsUsingConstantBuffer(shaderInputProviderDeclaration->GetShaderInputProviderUsage()))
        {
            constexpr size_t bufferOffset = 0;
            void* pMappedBuffer = gfxDirectRenderCommandList.MapConstantBuffer(shaderInputProvider->m_GfxConstantBufferHandle, MapFlag::Write_Discard, bufferOffset);

            GetShaderInputProviderManager().CopyShaderInputsToBuffer(*shaderInputProvider, pMappedBuffer);
        }
        else
        {
            gfxMaterialUnifiedConstantBuffer.ApplyShaderInputProvider(*shaderInputProvider);
        }
    }

    m_ShaderResourceBinder.BindShaderInputProvders(
            gfxRenderDevice,
            gfxDirectRenderCommandList,
            shaderInputProviders,
            m_ShaderRenderElements.GfxDescriptorSetHandle);

    m_ShaderResourceBinder.BindSamplerStates(
            gfxRenderDevice,
            m_SamplerStateHandles,
            m_ShaderRenderElements.GfxDescriptorSetHandle);
}

ShaderHandler::ShaderRenderElements ShaderHandler::GetShaderRenderElements(GFXRenderDevice& gfxRenderDevice, const RenderState& renderState)
{
    ShaderRenderElements shaderRenderElements = m_ShaderRenderElements;
    SHIP_ASSERT(shaderRenderElements.GfxPipelineStateObjectHandle.handle != InvalidGfxHandle);
    SHIP_ASSERT(shaderRenderElements.GfxRootSignatureHandle.handle != InvalidGfxHandle);
    SHIP_ASSERT(shaderRenderElements.GfxDescriptorSetHandle.handle != InvalidGfxHandle);

    const GFXPipelineStateObject& gfxPipelineStateObject = gfxRenderDevice.GetPipelineStateObject(shaderRenderElements.GfxPipelineStateObjectHandle);

    PipelineStateObjectCreationParameters pipelineStateObjectCreationParameters = gfxPipelineStateObject.GetCreationParameters();

    pipelineStateObjectCreationParameters.PrimitiveTopologyToUse = renderState.PrimitiveTopologyToRender;
    pipelineStateObjectCreationParameters.VertexFormatTypeToUse = renderState.VertexFormatTypeToRender;

    if (renderState.GfxRenderTargetHandle.handle == InvalidGfxHandle)
    {
        pipelineStateObjectCreationParameters.NumRenderTargets = 0;
    }
    else
    {
        const GFXRenderTarget& gfxRenderTarget = gfxRenderDevice.GetRenderTarget(renderState.GfxRenderTargetHandle);

        pipelineStateObjectCreationParameters.NumRenderTargets = gfxRenderTarget.GetNumRenderTargetsAttached();

        GfxFormat const * renderTargetFormats = gfxRenderTarget.GetRenderTargetFormats();

        memcpy(&pipelineStateObjectCreationParameters.RenderTargetsFormat[0], &renderTargetFormats[0], pipelineStateObjectCreationParameters.NumRenderTargets);
    }

    if (renderState.GfxDepthStencilRenderTargetHandle.handle != InvalidGfxHandle)
    {
        const GFXDepthStencilRenderTarget& gfxDepthStencilRenderTarget = gfxRenderDevice.GetDepthStencilRenderTarget(renderState.GfxDepthStencilRenderTargetHandle);

        pipelineStateObjectCreationParameters.DepthStencilFormat = gfxDepthStencilRenderTarget.GetDepthStencilFormat();
    }

    // Apply override, if any.
    if (renderState.OptionalRenderStateBlockStateOverride != nullptr)
    {
        renderState.OptionalRenderStateBlockStateOverride->ApplyOverridenValues(pipelineStateObjectCreationParameters.RenderStateBlockToUse);
    }

    shipBool needToCreateEffectivePso = (m_GfxEffectivePipelineStateObjectHandle.handle == InvalidGfxHandle);
    if (!needToCreateEffectivePso)
    {
        const GFXPipelineStateObject& gfxEffectivePipelineStateObject = gfxRenderDevice.GetPipelineStateObject(m_GfxEffectivePipelineStateObjectHandle);
        needToCreateEffectivePso = (gfxEffectivePipelineStateObject.GetCreationParameters() != pipelineStateObjectCreationParameters);

        if (needToCreateEffectivePso)
        {
            gfxRenderDevice.DestroyPipelineStateObject(m_GfxEffectivePipelineStateObjectHandle);
        }
    }

    if (needToCreateEffectivePso)
    {
        m_GfxEffectivePipelineStateObjectHandle = gfxRenderDevice.CreatePipelineStateObject(pipelineStateObjectCreationParameters);
    }

    shaderRenderElements.GfxPipelineStateObjectHandle = m_GfxEffectivePipelineStateObjectHandle;

    return shaderRenderElements;
}

}