#include <graphics/wrapper/dx11/dx11descriptorset.h>

namespace Shipyard
{;

bool DX11DescriptorSet::Create(DescriptorSetType descriptorSetType, const RootSignature& rootSignature)
{
    m_DescriptorSetType = descriptorSetType;
    m_RootSignature = &rootSignature;

    return true;
}

void DX11DescriptorSet::Destroy()
{

}

}