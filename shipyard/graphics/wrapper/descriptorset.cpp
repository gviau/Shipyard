#include <graphics/wrapper/descriptorset.h>

#include <graphics/wrapper/rootsignature.h>

#include <system/systemcommon.h>

namespace Shipyard
{;

void DescriptorSet::SetDescriptorForRootIndex(shipUint32 rootIndex, GfxResource& descriptorResource)
{
    DescriptorSetEntry& descriptorSetEntry = m_DescriptorSetEntries[rootIndex];
    SHIP_ASSERT_MSG(descriptorSetEntry.descriptorRangeIndex == descriptorSetEntry.InvalidDescriptorRangeIndex, "DescriptorSetEntry for root index %d was created for a descriptor table. Use DescriptorSet::SetDescriptorTableForRootIndex instead!", rootIndex);

    descriptorSetEntry.descriptorResources[0] = &descriptorResource;
}

void DescriptorSet::SetDescriptorTableForRootIndex(shipUint32 rootIndex, shipUint32 descriptorRangeIndex, const Array<GfxResource*>& descriptorTableResources)
{
    SHIP_ASSERT(descriptorTableResources.Size() > 0);
    
    DescriptorSetEntry* pDescriptorSetEntry = nullptr;

    for (shipUint32 i = 0; i < m_DescriptorSetEntries.Size(); i++)
    {
        if (m_DescriptorSetEntries[i].rootIndex == rootIndex && m_DescriptorSetEntries[i].descriptorRangeIndex == descriptorRangeIndex)
        {
            pDescriptorSetEntry = &m_DescriptorSetEntries[i];
            break;
        }
    }

    SHIP_ASSERT_MSG(pDescriptorSetEntry != nullptr, "DescriptorSetEntry for root index %d and for descriptor range index %d wasn't found. Was it created for a descriptor table?", rootIndex, descriptorRangeIndex);

    for (shipUint32 i = 0; i < descriptorTableResources.Size(); i++)
    {
        GfxResource* descriptorResource = descriptorTableResources[i];
        pDescriptorSetEntry->descriptorResources[i] = descriptorResource;
    }
}

void DescriptorSet::SetDescriptorTableEntryForRootIndex(shipUint32 rootIndex, shipUint32 descriptorRangeIndex, shipUint32 descriptorRangeEntryIndex, GfxResource& descriptorResource)
{
    DescriptorSetEntry* pDescriptorSetEntry = nullptr;

    for (shipUint32 i = 0; i < m_DescriptorSetEntries.Size(); i++)
    {
        if (m_DescriptorSetEntries[i].rootIndex == rootIndex && m_DescriptorSetEntries[i].descriptorRangeIndex == descriptorRangeIndex)
        {
            pDescriptorSetEntry = &m_DescriptorSetEntries[i];
            break;
        }
    }

    SHIP_ASSERT_MSG(pDescriptorSetEntry != nullptr, "DescriptorSetEntry for root index %d and for descriptor range index %d wasn't found. Was it created for a descriptor table?", rootIndex, descriptorRangeIndex);

    pDescriptorSetEntry->descriptorResources[descriptorRangeEntryIndex] = &descriptorResource;
}

const Array<DescriptorSet::DescriptorSetEntry>& DescriptorSet::GetDescriptorSetEntries() const
{
    return m_DescriptorSetEntries;
}

DescriptorSetType DescriptorSet::GetDescriptorSetType() const
{
    return m_DescriptorSetType;
}

}