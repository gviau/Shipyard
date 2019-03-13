#pragma once

#include <graphics/wrapper/rootsignature.h>

namespace Shipyard
{
    class SHIPYARD_API DX11RootSignature : public RootSignature
    {
    public:
        bool Create(const Array<RootSignatureParameterEntry>& rootSignatureParameters);
        void Destroy();
    };
}