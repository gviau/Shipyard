#pragma once

#include <system/platform.h>

#include <graphics/wrapper/wrapper.h>

#include <system/array.h>

namespace Shipyard
{
    class ShaderInputProvider;
    class ShaderInputProviderDeclaration;

    class SHIPYARD_API ShaderResourceBinder
    {
    public:
        ShaderResourceBinder(GFXDescriptorSetHandle* pGfxDescriptorSetHandle);

        template <typename ShaderInputProviderType>
        void AddShaderResourceBinderEntry(shipUint16 rootIndexToBindTo)
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
            shipUint16 RootIndexToBindTo;
        };

    private:
        GFXDescriptorSetHandle* m_pGfxDescriptorSetHandle;
        Array<ShaderResourceBinderEntry> m_ShaderResourceBinderEntries;
    };
}