#include <graphics/wrapper/descriptorset.h>

#include <graphics/wrapper/rootsignature.h>

#include <system/systemcommon.h>

namespace Shipyard
{;

DescriptorSet::DescriptorSet(DescriptorSetType descriptorSetType, const RootSignature& rootSignature)
    : m_RootSignature(&rootSignature)
    , m_DescriptorSetType(descriptorSetType)
{
}

void DescriptorSet::SetDescriptorForRootIndex(uint32_t rootIndex, GfxResource& descriptorResource)
{
    RootSignatureParameterType paramType = m_RootSignature->GetRootSignatureParameters()[rootIndex].parameterType;

    SHIP_ASSERT(paramType == RootSignatureParameterType::ConstantBufferView ||
           paramType == RootSignatureParameterType::ShaderResourceView ||
           paramType == RootSignatureParameterType::UnorderedAccessView);
    
    DescriptorSetEntry& newDescriptorSetEntry = m_Resources.Grow();
    newDescriptorSetEntry.rootIndex = uint16_t(rootIndex);
    newDescriptorSetEntry.descriptorResources.Add(&descriptorResource);
}

void DescriptorSet::SetDescriptorTableForRootIndex(uint32_t rootIndex, const Array<GfxResource*>& descriptorTableResources)
{
    SHIP_ASSERT(descriptorTableResources.Size() > 0);
    SHIP_ASSERT(m_RootSignature->GetRootSignatureParameters()[rootIndex].parameterType == RootSignatureParameterType::DescriptorTable);

    DescriptorSetEntry& newDescriptorSetEntry = m_Resources.Grow();
    newDescriptorSetEntry.rootIndex = uint16_t(rootIndex);

    for (GfxResource* descriptorResource : descriptorTableResources)
    {
        newDescriptorSetEntry.descriptorResources.Add(descriptorResource);
    }

    newDescriptorSetEntry.isDescriptorTable = true;
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