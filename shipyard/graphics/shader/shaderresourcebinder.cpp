#include <graphics/shader/shaderresourcebinder.h>

#include <graphics/shader/shaderinputprovider.h>

#include <system/systemdebug.h>

namespace Shipyard
{;

void ShaderResourceBinder::BindShaderInputProvders(GFXRenderDevice& gfxRenderDevice, const Array<ShaderInputProvider*>& shaderInputProviders) const
{
    for (ShaderInputProvider* shaderInputProvider : shaderInputProviders)
    {
        for (const ShaderResourceBinderEntry& shaderResourceBinderEntry : m_ShaderResourceBinderEntries)
        {
            if (shaderInputProvider->GetShaderInputProviderDeclaration() == shaderResourceBinderEntry.Declaration)
            {
                GFXConstantBuffer gfxConstantBuffer = gfxRenderDevice.GetConstantBuffer(shaderInputProvider->m_GfxConstantBufferHandle);
                shaderResourceBinderEntry.GfxDescriptorSet->SetDescriptorForRootIndex(shaderResourceBinderEntry.RootIndex, gfxConstantBuffer);

                break;
            }
        }
    }
}

}