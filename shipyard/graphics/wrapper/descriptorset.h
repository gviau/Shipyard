#pragma once

#include <graphics/wrapper/wrapper_common.h>

namespace Shipyard
{
    class GfxResource;
    class RootSignature;

    class SHIPYARD_API DescriptorSet
    {
    public:
        struct DescriptorSetEntry
        {
            static const uint16_t InvalidDescriptorRangeIndex = uint16_t(-1);

            Array<GfxResource*> descriptorResources;
            uint16_t rootIndex = 0;
            uint16_t descriptorRangeIndex = InvalidDescriptorRangeIndex;
        };

    public:
        void SetDescriptorForRootIndex(uint32_t rootIndex, GfxResource& descriptorResource);
        void SetDescriptorTableForRootIndex(uint32_t rootIndex, uint32_t descriptorRangeIndex, const Array<GfxResource*>& descriptorTableResources);

        const Array<DescriptorSetEntry>& GetDescriptorSetEntries() const;
        
        DescriptorSetType GetDescriptorSetType() const;

    protected:
        const RootSignature* m_RootSignature;
        Array<DescriptorSetEntry> m_DescriptorSetEntries;
        DescriptorSetType m_DescriptorSetType;
    };
}