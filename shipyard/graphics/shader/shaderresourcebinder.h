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
        template <typename ShaderInputProviderType>
        void AddShaderResourceBinderEntry(GFXDescriptorSetHandle* gfxDescriptorSetHandleToBindTo, uint16_t rootIndexToBindTo)
        {
            ShaderResourceBinderEntry& shaderResourceBinderEntry = m_ShaderResourceBinderEntries.Grow();
            shaderResourceBinderEntry.Declaration = ShaderInputProviderType::ms_ShaderInputProviderDeclaration;
            shaderResourceBinderEntry.GfxDescriptorSetHandleToBindTo = gfxDescriptorSetHandleToBindTo;
            shaderResourceBinderEntry.RootIndexToBindTo = rootIndexToBindTo;
        }

        void BindShaderInputProvders(GFXRenderDevice& gfxRenderDevice, const Array<ShaderInputProvider*>& shaderInputProviders) const;

    private:
        struct ShaderResourceBinderEntry
        {
            ShaderInputProviderDeclaration* Declaration;
            GFXDescriptorSetHandle* GfxDescriptorSetHandleToBindTo;
            uint16_t RootIndexToBindTo;
        };

    private:
        Array<ShaderResourceBinderEntry> m_ShaderResourceBinderEntries;
    };
}