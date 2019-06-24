#include <graphics/shader/shaderhandler.h>

#include <graphics/material/materialunifiedconstantbuffer.h>

#include <graphics/shader/shaderinputprovider.h>
#include <graphics/shader/shaderresourcebinder.h>

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

void ShaderHandler::ApplyShader(PipelineStateObjectCreationParameters& pipelineStateObjectCreationParameters) const
{
    pipelineStateObjectCreationParameters.vertexShaderHandle = m_VertexShaderHandle;
    pipelineStateObjectCreationParameters.pixelShaderHandle = m_PixelShaderHandle;

    pipelineStateObjectCreationParameters.renderStateBlock = m_RenderStateBlock;
}

void ShaderHandler::ApplyShaderInputProviders(
        GFXRenderDevice& gfxRenderDevice,
        GFXDirectRenderCommandList& gfxDirectRenderCommandList,
        const ShaderResourceBinder& shaderResourceBinder,
        const Array<ShaderInputProvider*>& shaderInputProviders)
{
    if (shaderInputProviders.Empty())
    {
        return;
    }

    GFXMaterialUnifiedConstantBuffer& gfxMaterialUnifiedConstantBuffer = GetGFXMaterialUnifiedConstantBuffer();

    for (ShaderInputProvider* shaderInputProvider : shaderInputProviders)
    {
        SHIP_ASSERT(shaderInputProvider != nullptr);
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

    shaderResourceBinder.BindShaderInputProvders(gfxRenderDevice, shaderInputProviders);
}

}