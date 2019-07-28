#pragma once

#include <graphics/wrapper/sampler.h>

struct ID3D11Device;
struct ID3D11SamplerState;

namespace Shipyard
{
    class SHIPYARD_API DX11Sampler : public Sampler
    {
    public:
        DX11Sampler();

        shipBool Create(ID3D11Device& device, const SamplerState& samplerState);
        void Destroy();

        ID3D11SamplerState* GetNativeSamplerState() const;

    private:
        ID3D11SamplerState* m_NativeSamplerState;
    };
}