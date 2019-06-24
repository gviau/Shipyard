#pragma once

#include <system/platform.h>

#include <graphics/wrapper/wrapper.h>

#include <system/array.h>

#include <cinttypes>

namespace Shipyard
{
    class ShaderInputProvider;
    class ShaderInputProviderDeclaration;

    class SHIPYARD_API ShaderResourceBinder
    {
    public:
        // ShaderInputProviderType is assumed to be a BaseShaderInputProvider derived class and setup to be using constant buffers.
        // See ShaderInputProviderUtils::IsUsingConstantBuffer
        template <typename ShaderInputProviderType>
        void AddShaderResourceBinderEntry(GFXDescriptorSet& gfxDescriptorSetToBindTo, uint16_t rootIndexToBindTo)
        {
            SHIP_ASSERT_MSG(
                ShaderInputProviderUtils::IsUsingConstantBuffer(ShaderInputProviderType::ms_ShaderInputProviderDeclaration->GetShaderInputProviderUsage()),
                "ShaderResourceBinder::AddShaderResourceBinderEntry only deals with ShaderInputProviders that are using constant buffers. See ShaderInputProviderUtils::IsUsingConstantBuffer");

            ShaderResourceBinderEntry& shaderResourceBinderEntry = m_ShaderResourceBinderEntries.Grow();
            shaderResourceBinderEntry.Declaration = ShaderInputProviderType::ms_ShaderInputProviderDeclaration;
            shaderResourceBinderEntry.GfxDescriptorSet = &gfxDescriptorSetToBindTo;
            shaderResourceBinderEntry.RootIndex = rootIndexToBindTo;
        }

        void BindShaderInputProvders(GFXRenderDevice& gfxRenderDevice, const Array<ShaderInputProvider*>& shaderInputProviders) const;

    private:
        struct ShaderResourceBinderEntry
        {
            ShaderInputProviderDeclaration* Declaration;
            GFXDescriptorSet* GfxDescriptorSet;
            uint16_t RootIndex;
        };

    private:
        Array<ShaderResourceBinderEntry> m_ShaderResourceBinderEntries;
    };
}