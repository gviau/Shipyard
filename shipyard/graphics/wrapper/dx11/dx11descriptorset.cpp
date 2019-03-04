#include <graphics/wrapper/dx11/dx11descriptorset.h>

namespace Shipyard
{;

DX11DescriptorSet::DX11DescriptorSet(DescriptorSetType descriptorSetType, const RootSignature& rootSignature)
    : DescriptorSet(descriptorSetType, rootSignature)
{
}

}