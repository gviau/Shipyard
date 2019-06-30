#include <graphics/shader/shaderresourcebinder.h>

#include <graphics/shader/shaderinputprovider.h>

#include <system/systemdebug.h>

namespace Shipyard
{;

void ShaderResourceBinder::BindShaderInputProvders(GFXRenderDevice& gfxRenderDevice, const Array<ShaderInputProvider*>& shaderInputProviders) const
{
    ShaderInputProviderManager& shaderInputProviderManager = GetShaderInputProviderManager();

    for (ShaderInputProvider* shaderInputProvider : shaderInputProviders)
    {
        for (const ShaderResourceBinderEntry& shaderResourceBinderEntry : m_ShaderResourceBinderEntries)
        {
            if (shaderInputProvider->GetShaderInputProviderDeclaration() == shaderResourceBinderEntry.Declaration)
            {
                GFXDescriptorSet& gfxDescriptorSet = gfxRenderDevice.GetDescriptorSet(*shaderResourceBinderEntry.GfxDescriptorSetHandleToBindTo);

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
                    InplaceArray<GFXTexture2D*, GfxConstants::GfxConstants_MaxShaderResourceViewsBoundPerShaderStage> texture2Ds;
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

                    // Temporary assignation to root index + 1
                    gfxDescriptorSet.SetDescriptorTableForRootIndex(shaderResourceBinderEntry.RootIndexToBindTo + 1, texture2Ds);
                }

                break;
            }
        }
    }
}

}