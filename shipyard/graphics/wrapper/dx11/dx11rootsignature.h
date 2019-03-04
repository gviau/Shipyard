#pragma once

#include <graphics/wrapper/rootsignature.h>

namespace Shipyard
{
    class SHIPYARD_API DX11RootSignature : public RootSignature
    {
    public:
        DX11RootSignature(const Array<RootSignatureParameterEntry>& rootSignatureParameters);
    };
}