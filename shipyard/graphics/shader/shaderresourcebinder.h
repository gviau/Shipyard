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
        ShaderResourceBinder(GFXDescriptorSetHandle* pGfxDescriptorSetHandle);

        template <typename ShaderInputProviderType>
        void AddShaderResourceBinderEntry(uint16_t rootIndexToBindTo)
        {
            ShaderResourceBinderEntry& shaderResourceBinderEntry = m_ShaderResourceBinderEntries.Grow();
            shaderResourceBinderEntry.Declaration = ShaderInputProviderType::ms_ShaderInputProviderDeclaration;
            shaderResourceBinderEntry.RootIndexToBindTo = rootIndexToBindTo;
        }

        void BindShaderInputProvders(
                GFXRenderDevice& gfxRenderDevice,
                GFXDirectRenderCommandList& gfxDirectRenderCommandList,
                const Array<ShaderInputProvider*>& shaderInputProviders) const;

    private:
        struct ShaderResourceBinderEntry
        {
            ShaderInputProviderDeclaration* Declaration;
            uint16_t RootIndexToBindTo;
        };

    private:
        GFXDescriptorSetHandle* m_pGfxDescriptorSetHandle;
        Array<ShaderResourceBinderEntry> m_ShaderResourceBinderEntries;
    };
}