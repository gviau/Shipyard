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
    
    DescriptorSetEntry& newDescriptorSetEntry = m_Resources[rootIndex];
    SHIP_ASSERT_MSG(newDescriptorSetEntry.descriptorResources.Size() <= 1, "DescriptorSetEntry for root index %d was created for a descriptor table. Use DescriptorSet::SetDescriptorTableForRootIndex instead!", rootIndex);

    newDescriptorSetEntry.descriptorResources[0] = &descriptorResource;
}

void DescriptorSet::SetDescriptorTableForRootIndex(uint32_t rootIndex, const Array<GfxResource*>& descriptorTableResources)
{
    SHIP_ASSERT(descriptorTableResources.Size() > 0);
    SHIP_ASSERT(m_RootSignature->GetRootSignatureParameters()[rootIndex].parameterType == RootSignatureParameterType::DescriptorTable);

    DescriptorSetEntry& newDescriptorSetEntry = m_Resources[rootIndex];
    SHIP_ASSERT_MSG(newDescriptorSetEntry.descriptorResources.Size() >= 1, "DescriptorSetEntry for root index %d was not created for a descriptor table. Use DescriptorSet::SetDescriptorForRootIndex instead!", rootIndex);

    for (uint32_t i = 0; i < descriptorTableResources.Size(); i++)
    {
        GfxResource* descriptorResource = descriptorTableResources[i];
        newDescriptorSetEntry.descriptorResources[i] = descriptorResource;
    }
}

const Array<DescriptorSet::DescriptorSetEntry>& DescriptorSet::GetDescriptorSetEntries() const
{
    return m_Resources;
}

DescriptorSetType DescriptorSet::GetDescriptorSetType() const
{
    return m_DescriptorSetType;
}

}