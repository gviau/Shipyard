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
            DescriptorSetEntry()
                : rootIndex(0)
                , isDescriptorTable(false)
            {
            }

            Array<GfxResource*> descriptorResources;
            uint16_t rootIndex;
            bool isDescriptorTable;
        };

    public:
        void SetDescriptorForRootIndex(uint32_t rootIndex, GfxResource& descriptorResource);
        void SetDescriptorTableForRootIndex(uint32_t rootIndex, const Array<GfxResource*>& descriptorTableResources);

        const Array<DescriptorSetEntry>& GetDescriptorSetEntries() const;
        
        DescriptorSetType GetDescriptorSetType() const;

    protected:
        const RootSignature* m_RootSignature;
        Array<DescriptorSetEntry> m_Resources;
        DescriptorSetType m_DescriptorSetType;
    };
}