#pragma once

#include <system/platform.h>

#include <graphics/wrapper/wrapper.h>

#include <system/array.h>

#ifndef SHIP_OPTIMIZED
#define VALIDATE_SHADER_INPUT_PROVIDER_BINDING
#endif // #ifndef SHIP_OPTIMIZED

namespace Shipyard
{
    class ShaderInputProvider;
    class ShaderInputProviderDeclaration;

    enum class ShaderInputType : shipUint8;
    enum class ShaderInputProviderUsage : shipUint8;

    class SHIPYARD_GRAPHICS_API ShaderResourceBinder
    {
    public:
        void AddShaderResourceBinderEntryForProviderToDescriptor(
                const ShaderInputProviderDeclaration* shaderInputProviderDeclaration,
                shipUint16 rootIndexToBindTo,
                ShaderVisibility shaderStageToBindTo);

        void AddShaderResourceBinderEntryForProviderToDescriptorTable(
                const ShaderInputProviderDeclaration* shaderInputProviderDeclaration,
                shipUint16 rootIndexToBindTo,
                shipUint16 descriptorRangeIndexToBind,
                shipUint16 descriptorRangeEntryIndexToBind,
                ShaderVisibility shaderStageToBindTo);

        void AddShaderResourceBinderEntryForDescriptorToDescriptor(
                const ShaderInputProviderDeclaration* shaderInputProviderDeclaration,
                shipUint16 rootIndexToBindTo,
                shipInt32 offsetInProvider,
                ShaderInputType shaderInputType,
                ShaderVisibility shaderStageToBindTo);

        void AddShaderResourceBinderEntryForDescriptorToDescriptorTable(
                const ShaderInputProviderDeclaration* shaderInputProviderDeclaration,
                shipUint16 rootIndexToBindTo,
                shipUint16 descriptorRangeIndexToBind,
                shipUint16 descriptorRangeEntryIndexToBind,
                shipInt32 offsetInProvider,
                ShaderInputType shaderInputType,
                ShaderVisibility shaderStageToBindTo);

        void AddShaderResourceBinderEntryForSamplerToDescriptorTable(
                const SamplerState& samplerState,
                shipUint16 rootIndexToBindTo,
                shipUint16 descriptorRangeIndexToBind,
                shipUint16 descriptorRangeEntryIndexToBind,
                ShaderVisibility shaderStageToBindTo);

        void AddShaderResourceBinderEntryForGlobalBufferToDescriptor(
                ShaderInputProviderUsage shaderInputProviderUsage,
                shipUint16 rootIndexToBindTo,
                ShaderVisibility shaderStageToBindTo);

        void AddShaderResourceBinderEntryForGlobalBufferToDescriptorTable(
                ShaderInputProviderUsage shaderInputProviderUsage,
                shipUint16 rootIndexToBindTo,
                shipUint16 descriptorRangeIndexToBind,
                shipUint16 descriptorRangeEntryIndexToBind,
                ShaderVisibility shaderStageToBindTo);

        void BindShaderInputProvders(
                GFXRenderDevice& gfxRenderDevice,
                GFXDirectRenderCommandList& gfxDirectRenderCommandList,
                const Array<const ShaderInputProvider*>& shaderInputProviders,
                GFXDescriptorSetHandle gfxDescriptorSetHandle) const;

        void BindSamplerStates(
                GFXRenderDevice& gfxRenderDevice,
                const Array<GFXSamplerHandle>& gfxSamplerHandles,
                GFXDescriptorSetHandle gfxDescriptorSetHandle) const;

    public:
        struct ShaderResourceBinderEntry
        {
            const ShaderInputProviderDeclaration* Declaration = nullptr;
            shipUint16 RootIndexToBindTo = 0 ;
            shipUint16 DescriptorRangeIndexToBindTo = 0;
            shipUint16 DescriptorRangeEntryIndexToBind = 0;
            shipInt32 DataOffsetInProvider = 0;

            ShaderInputType DescriptorType;
            ShaderInputProviderUsage GlobalBufferUsage;
            ShaderVisibility ShaderStageToBindTo = ShaderVisibility::ShaderVisibility_None;

            SamplerState Sampler;

            shipBool BindDescriptorTable = false;
            shipBool BindConstantBuffer = false;
            shipBool BindDescriptor = false;
            shipBool BindGlobalBuffer = false;
            shipBool BindSamplerState = false;
        };

    public:
        Array<ShaderResourceBinderEntry>& GetShaderResourceBinderEntries() { return m_ShaderResourceBinderEntries; }
        const Array<ShaderResourceBinderEntry>& GetShaderResourceBinderEntries() const { return m_ShaderResourceBinderEntries; }

    private:
        Array<ShaderResourceBinderEntry> m_ShaderResourceBinderEntries;

        void BindShaderInputProviderDescriptor(
                const ShaderResourceBinderEntry& shaderResourceBinderEntry,
                const ShaderInputProvider* shaderInputProvider,
                GFXRenderDevice& gfxRenderDevice,
                GFXDescriptorSet& gfxDescriptorSet) const;

        void BindShaderInputProviderConstantBuffer(
                const ShaderResourceBinderEntry& shaderResourceBinderEntry,
                const ShaderInputProvider* shaderInputProvider,
                GFXRenderDevice& gfxRenderDevice,
                GFXDescriptorSet& gfxDescriptorSet) const;

        void BindShaderGlobalBuffer(
                const ShaderResourceBinderEntry& shaderResourceBinderEntry,
                GFXRenderDevice& gfxRenderDevice,
                GFXDirectRenderCommandList& gfxDirectRenderCommandList,
                GFXDescriptorSet& gfxDescriptorSet) const;
    };
}