#include <graphics/graphicsprecomp.h>

#include <graphics/shader/shaderhandler.h>

#include <graphics/material/gfxmaterialunifiedconstantbuffer.h>

#include <graphics/shader/shaderfamilies.h>
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

void ShaderHandler::ApplyShaderInputProvidersForGraphics(
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
            m_ShaderRenderElementsForGraphics.GfxDescriptorSetHandle);

    m_ShaderResourceBinder.BindSamplerStates(
            gfxRenderDevice,
            m_SamplerStateHandles,
            m_ShaderRenderElementsForGraphics.GfxDescriptorSetHandle);
}

void ShaderHandler::ApplyShaderInputProvidersForCompute(
        GFXRenderDevice& gfxRenderDevice,
        GFXDirectRenderCommandList& gfxDirectRenderCommandList,
        const Array<const ShaderInputProvider*>& shaderInputProviders)
{
    if (shaderInputProviders.Empty() || m_ShaderKey.GetShaderFamily() == ShaderFamily::Error)
    {
        return;
    }

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
    }

    m_ShaderResourceBinder.BindShaderInputProvders(
            gfxRenderDevice,
            gfxDirectRenderCommandList,
            shaderInputProviders,
            m_ShaderRenderElementsForCompute.GfxDescriptorSetHandle);

    m_ShaderResourceBinder.BindSamplerStates(
            gfxRenderDevice,
            m_SamplerStateHandles,
            m_ShaderRenderElementsForCompute.GfxDescriptorSetHandle);
}

ShaderHandler::ShaderRenderElementsForGraphics ShaderHandler::GetShaderRenderElementsForGraphicsCommands(GFXRenderDevice& gfxRenderDevice, const RenderState& renderState){
    ShaderRenderElementsForGraphics shaderRenderElements = m_ShaderRenderElementsForGraphics;
    SHIP_ASSERT(shaderRenderElements.GfxGraphicsPipelineStateObjectHandle.IsValid());
    SHIP_ASSERT(shaderRenderElements.GfxRootSignatureHandle.IsValid());
    SHIP_ASSERT(shaderRenderElements.GfxDescriptorSetHandle.IsValid());

    const GFXGraphicsPipelineStateObject& gfxPipelineStateObject = gfxRenderDevice.GetGraphicsPipelineStateObject(shaderRenderElements.GfxGraphicsPipelineStateObjectHandle);

    GraphicsPipelineStateObjectCreationParameters pipelineStateObjectCreationParameters = gfxPipelineStateObject.GetCreationParameters();

    pipelineStateObjectCreationParameters.PrimitiveTopologyToUse = renderState.PrimitiveTopologyToRender;
    pipelineStateObjectCreationParameters.VertexFormatTypeToUse = renderState.VertexFormatTypeToRender;

    if (!renderState.GfxRenderTargetHandle.IsValid())
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

    if (renderState.GfxDepthStencilRenderTargetHandle.IsValid())
    {
        const GFXDepthStencilRenderTarget& gfxDepthStencilRenderTarget = gfxRenderDevice.GetDepthStencilRenderTarget(renderState.GfxDepthStencilRenderTargetHandle);

        pipelineStateObjectCreationParameters.DepthStencilFormat = gfxDepthStencilRenderTarget.GetDepthStencilFormat();
    }

    // Apply override, if any.
    if (renderState.OptionalRenderStateBlockStateOverride != nullptr)
    {
        renderState.OptionalRenderStateBlockStateOverride->ApplyOverridenValues(pipelineStateObjectCreationParameters.RenderStateBlockToUse);
    }

    shipBool needToCreateEffectivePso = (!m_GfxEffectivePipelineStateObjectHandle.IsValid());
    if (!needToCreateEffectivePso)
    {
        const GFXGraphicsPipelineStateObject& gfxEffectivePipelineStateObject = gfxRenderDevice.GetGraphicsPipelineStateObject(m_GfxEffectivePipelineStateObjectHandle);
        needToCreateEffectivePso = (gfxEffectivePipelineStateObject.GetCreationParameters() != pipelineStateObjectCreationParameters);

        if (needToCreateEffectivePso)
        {
            gfxRenderDevice.DestroyGraphicsPipelineStateObject(m_GfxEffectivePipelineStateObjectHandle);
        }
    }

    if (needToCreateEffectivePso)
    {
        m_GfxEffectivePipelineStateObjectHandle = gfxRenderDevice.CreateGraphicsPipelineStateObject(pipelineStateObjectCreationParameters);
    }

    shaderRenderElements.GfxGraphicsPipelineStateObjectHandle = m_GfxEffectivePipelineStateObjectHandle;

    return shaderRenderElements;
}

ShaderHandler::ShaderRenderElementsForCompute ShaderHandler::GetShaderRenderElementsForCompute(GFXRenderDevice& gfxRenderDevice)
{
    ShaderRenderElementsForCompute shaderRenderElements = m_ShaderRenderElementsForCompute;

    if (m_ShaderKey.GetShaderFamily() == ShaderFamily::Error)
    {
        shaderRenderElements.GfxComputePipelineStateObjectHandle = { InvalidGfxHandle };
    }

    return shaderRenderElements;
}

}