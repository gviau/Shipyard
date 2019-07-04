#include <graphics/wrapper/dx11/dx11descriptorset.h>

namespace Shipyard
{;

bool DX11DescriptorSet::Create(DescriptorSetType descriptorSetType, RootSignature* rootSignature, const Array<DescriptorSetEntryDeclaration>& descriptorSetEntryDeclarations)
{
    m_DescriptorSetType = descriptorSetType;
    m_RootSignature = rootSignature;

    for (const DescriptorSetEntryDeclaration& descriptorSetEntryDeclaration : descriptorSetEntryDeclarations)
    {
        DescriptorSetEntry& newDescriptorSetEntry = m_DescriptorSetEntries.Grow();
        newDescriptorSetEntry.descriptorResources.Resize(descriptorSetEntryDeclaration.numResources);
        newDescriptorSetEntry.rootIndex = descriptorSetEntryDeclaration.rootIndex;
        newDescriptorSetEntry.descriptorRangeIndex = descriptorSetEntryDeclaration.descriptorRangeIndex;
    }

    return true;
}

void DX11DescriptorSet::Destroy()
{

}

}