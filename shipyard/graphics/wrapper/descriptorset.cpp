#include <graphics/wrapper/descriptorset.h>

#include <graphics/wrapper/rootsignature.h>

#include <system/systemcommon.h>

namespace Shipyard
{;

void DescriptorSet::SetDescriptorForRootIndex(uint32_t rootIndex, GfxResource& descriptorResource)
{
    RootSignatureParameterType paramType = m_RootSignature->GetRootSignatureParameters()[rootIndex].parameterType;

    SHIP_ASSERT(paramType == RootSignatureParameterType::ConstantBufferView ||
           paramType == RootSignatureParameterType::ShaderResourceView ||
           paramType == RootSignatureParameterType::UnorderedAccessView);
    
    DescriptorSetEntry& descriptorSetEntry = m_DescriptorSetEntries[rootIndex];
    SHIP_ASSERT_MSG(descriptorSetEntry.descriptorRangeIndex == descriptorSetEntry.InvalidDescriptorRangeIndex, "DescriptorSetEntry for root index %d was created for a descriptor table. Use DescriptorSet::SetDescriptorTableForRootIndex instead!", rootIndex);

    descriptorSetEntry.descriptorResources[0] = &descriptorResource;
}

void DescriptorSet::SetDescriptorTableForRootIndex(uint32_t rootIndex, uint32_t descriptorRangeIndex, const Array<GfxResource*>& descriptorTableResources)
{
    SHIP_ASSERT(descriptorTableResources.Size() > 0);
    SHIP_ASSERT(m_RootSignature->GetRootSignatureParameters()[rootIndex].parameterType == RootSignatureParameterType::DescriptorTable);

    DescriptorSetEntry* pDescriptorSetEntry = nullptr;

    for (uint32_t i = 0; i < m_DescriptorSetEntries.Size(); i++)
    {
        if (m_DescriptorSetEntries[i].rootIndex == rootIndex && m_DescriptorSetEntries[i].descriptorRangeIndex == descriptorRangeIndex)
        {
            pDescriptorSetEntry = &m_DescriptorSetEntries[i];
            break;
        }
    }

    SHIP_ASSERT_MSG(pDescriptorSetEntry != nullptr, "DescriptorSetEntry for root index %d and for descriptor range index %d wasn't found. Was it created for a descriptor table?", rootIndex, descriptorRangeIndex);

    for (uint32_t i = 0; i < descriptorTableResources.Size(); i++)
    {
        GfxResource* descriptorResource = descriptorTableResources[i];
        pDescriptorSetEntry->descriptorResources[i] = descriptorResource;
    }
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