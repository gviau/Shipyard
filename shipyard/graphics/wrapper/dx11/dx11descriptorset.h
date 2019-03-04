#pragma once

#include <graphics/wrapper/descriptorset.h>

namespace Shipyard
{
    class SHIPYARD_API DX11DescriptorSet : public DescriptorSet
    {
    public:
        DX11DescriptorSet(DescriptorSetType descriptorSetType, const RootSignature& rootSignature);
    };
}