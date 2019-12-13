#pragma once

#include <graphics/wrapper/rootsignature.h>

namespace Shipyard
{
    class SHIPYARD_GRAPHICS_API DX11RootSignature : public RootSignature
    {
    public:
        shipBool Create(const Array<RootSignatureParameterEntry>& rootSignatureParameters);
        void Destroy();
    };
}