#include <graphics/wrapper/dx11/dx11descriptorset.h>

namespace Shipyard
{;

bool DX11DescriptorSet::Create(DescriptorSetType descriptorSetType, RootSignature* rootSignature, const Array<DescriptorSetEntryDeclaration>& descriptorSetEntryDeclarations)
{
    m_DescriptorSetType = descriptorSetType;
    m_RootSignature = rootSignature;

    uint32_t numResourcesToReserve = 0;
    for (const DescriptorSetEntryDeclaration& descriptorSetEntryDeclaration : descriptorSetEntryDeclarations)
    {
        numResourcesToReserve = max(numResourcesToReserve, uint32_t(descriptorSetEntryDeclaration.rootIndex + 1));
    }

    m_Resources.Resize(numResourcesToReserve);

    for (const DescriptorSetEntryDeclaration& descriptorSetEntryDeclaration : descriptorSetEntryDeclarations)
    {
        DescriptorSetEntry& newDescriptorSetEntry = m_Resources[descriptorSetEntryDeclaration.rootIndex];
        newDescriptorSetEntry.descriptorResources.Resize(descriptorSetEntryDeclaration.numResources);
    }

    return true;
}

void DX11DescriptorSet::Destroy()
{

}

}