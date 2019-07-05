#pragma once

#include <graphics/wrapper/descriptorset.h>

namespace Shipyard
{
    class SHIPYARD_API DX11DescriptorSet : public DescriptorSet
    {
    public:
        shipBool Create(DescriptorSetType descriptorSetType, RootSignature* rootSignature, const Array<DescriptorSetEntryDeclaration>& descriptorSetEntryDeclarations);
        void Destroy();
    };
}