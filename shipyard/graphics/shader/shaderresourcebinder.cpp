#include <graphics/shader/shaderresourcebinder.h>

#include <graphics/material/materialunifiedconstantbuffer.h>

#include <graphics/shader/shaderinputprovider.h>

#include <system/systemdebug.h>

namespace Shipyard
{;

ShaderResourceBinder::ShaderResourceBinder(GFXDescriptorSetHandle* pGfxDescriptorSetHandle)
    : m_pGfxDescriptorSetHandle(pGfxDescriptorSetHandle)
{
    SHIP_ASSERT(m_pGfxDescriptorSetHandle != nullptr);
}

void ShaderResourceBinder::BindShaderInputProvders(
        GFXRenderDevice& gfxRenderDevice,
        GFXDirectRenderCommandList& gfxDirectRenderCommandList,
        const Array<ShaderInputProvider*>& shaderInputProviders) const
{
    ShaderInputProviderManager& shaderInputProviderManager = GetShaderInputProviderManager();

    GFXDescriptorSet& gfxDescriptorSet = gfxRenderDevice.GetDescriptorSet(*m_pGfxDescriptorSetHandle);

    InplaceArray<GFXTexture2D*, GfxConstants::GfxConstants_MaxShaderResourceViewsBoundPerShaderStage> texture2Ds;

    for (ShaderInputProvider* shaderInputProvider : shaderInputProviders)
    {
        for (const ShaderResourceBinderEntry& shaderResourceBinderEntry : m_ShaderResourceBinderEntries)
        {
            if (shaderInputProvider->GetShaderInputProviderDeclaration() == shaderResourceBinderEntry.Declaration)
            {
                if (ShaderInputProviderUtils::IsUsingConstantBuffer(shaderResourceBinderEntry.Declaration->GetShaderInputProviderUsage()))
                {
                    if (shaderInputProvider->m_GfxConstantBufferHandle.handle != InvalidGfxHandle)
                    {
                        GFXConstantBuffer& gfxConstantBuffer = gfxRenderDevice.GetConstantBuffer(shaderInputProvider->m_GfxConstantBufferHandle);
                        gfxDescriptorSet.SetDescriptorForRootIndex(shaderResourceBinderEntry.RootIndexToBindTo, gfxConstantBuffer);
                    }
                }

                GFXTexture2DHandle gfxTexture2DHandles[GfxConstants::GfxConstants_MaxShaderResourceViewsBoundPerShaderStage];
                uint32_t numTexture2DHandles = shaderInputProviderManager.GetTexture2DHandlesFromProvider(*shaderInputProvider, gfxTexture2DHandles);
                
                if (numTexture2DHandles > 0)
                {
                    for (uint32_t i = 0; i < numTexture2DHandles; i++)
                    {

                        GFXTexture2DHandle gfxTexture2DHandle = gfxTexture2DHandles[i];
                        if (gfxTexture2DHandle.handle == InvalidGfxHandle)
                        {
                            continue;
                        }

                        GFXTexture2D* gfxTexture2D = gfxRenderDevice.GetTexture2DPtr(gfxTexture2DHandle);
                        texture2Ds.Add(gfxTexture2D);
                    }
                }

                break;
            }
        }
    }

    GFXMaterialUnifiedConstantBuffer& gfxMaterialUnifiedConstantBuffer = GetGFXMaterialUnifiedConstantBuffer();
    GFXByteBufferHandle gfxByteBufferHandle = gfxMaterialUnifiedConstantBuffer.BindMaterialUnfiedConstantBuffer(gfxDirectRenderCommandList);
    
    GFXByteBuffer* pGfxByteBuffer = gfxRenderDevice.GetByteBufferPtr(gfxByteBufferHandle);

    InplaceArray<GFXByteBuffer*, 1> gfxByteBuffers;
    gfxByteBuffers.Add(pGfxByteBuffer);

    gfxDescriptorSet.SetDescriptorTableForRootIndex(0, 0, gfxByteBuffers);
    gfxDescriptorSet.SetDescriptorTableForRootIndex(0, 1, texture2Ds);
}

}