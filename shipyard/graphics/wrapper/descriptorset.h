#pragma once

#include <graphics/wrapper/wrapper_common.h>

namespace Shipyard
{
    class GfxResource;
    class RootSignature;

    class SHIPYARD_GRAPHICS_API DescriptorSet
    {
    public:
        struct DescriptorSetEntry
        {
            static const shipUint16 InvalidDescriptorRangeIndex = shipUint16(-1);

            Array<GfxResource*> descriptorResources;
            shipUint16 rootIndex = 0;
            shipUint16 descriptorRangeIndex = InvalidDescriptorRangeIndex;
        };

    public:
        void SetDescriptorForRootIndex(shipUint32 rootIndex, GfxResource& descriptorResource);
        void SetDescriptorTableForRootIndex(shipUint32 rootIndex, shipUint32 descriptorRangeIndex, const Array<GfxResource*>& descriptorTableResources);
        void SetDescriptorTableEntryForRootIndex(shipUint32 rootIndex, shipUint32 descriptorRangeIndex, shipUint32 descriptorRangeEntryIndex, GfxResource& descriptorResource);

        const Array<DescriptorSetEntry>& GetDescriptorSetEntries() const;
        
        DescriptorSetType GetDescriptorSetType() const;

    protected:
        Array<DescriptorSetEntry> m_DescriptorSetEntries;
        DescriptorSetType m_DescriptorSetType;
    };
}