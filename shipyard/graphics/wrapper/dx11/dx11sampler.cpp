#include <graphics/graphicsprecomp.h>

#include <graphics/wrapper/dx11/dx11sampler.h>

#include <graphics/wrapper/dx11/dx11_common.h>

#include <system/logger.h>

#pragma warning( disable : 4005 )

#include <d3d11.h>

#pragma warning( default : 4005 )

namespace Shipyard
{;

DX11Sampler::DX11Sampler()
    : m_NativeSamplerState(nullptr)
{
}

shipBool DX11Sampler::Create(ID3D11Device& device, const SamplerState& samplerState)
{
    SHIP_ASSERT(samplerState.MaxAnisotropy > 0 && samplerState.MaxAnisotropy <= 16);

    D3D11_SAMPLER_DESC samplerDesc;
    samplerDesc.Filter = ConvertShipyardSamplingFilterToDX11(
            samplerState.MinificationFiltering,
            samplerState.MagnificationFiltering,
            samplerState.MipmapFiltering,
            samplerState.UseAnisotropicFiltering,
            (samplerState.ComparisonFunction != ComparisonFunc::Never));
    samplerDesc.AddressU = ConvertShipyardTextureAddressModeToDX11(samplerState.AddressModeU);
    samplerDesc.AddressV = ConvertShipyardTextureAddressModeToDX11(samplerState.AddressModeV);
    samplerDesc.AddressW = ConvertShipyardTextureAddressModeToDX11(samplerState.AddressModeW);
    samplerDesc.ComparisonFunc = ConvertShipyardComparisonFuncToDX11(samplerState.ComparisonFunction);
    samplerDesc.MaxAnisotropy = samplerState.MaxAnisotropy;
    samplerDesc.MipLODBias = samplerState.MipLodBias;
    samplerDesc.MinLOD = samplerState.MinLod;
    samplerDesc.MaxLOD = samplerState.MaxLod;
    samplerDesc.BorderColor[0] = samplerState.BorderRGBA[0];
    samplerDesc.BorderColor[1] = samplerState.BorderRGBA[1];
    samplerDesc.BorderColor[2] = samplerState.BorderRGBA[2];
    samplerDesc.BorderColor[3] = samplerState.BorderRGBA[3];

    HRESULT hr = device.CreateSamplerState(&samplerDesc, &m_NativeSamplerState);
    if (FAILED(hr))
    {
        SHIP_LOG_ERROR("DX11Sampler::Create() --> Couldn't create texture sampler.");
        return false;
    }

    m_SamplerState = samplerState;

    return true;
}

void DX11Sampler::Destroy()
{
    SHIP_ASSERT_MSG(m_NativeSamplerState != nullptr, "Can't call Destroy on invalid texture sampler 0x%p", this);

    m_NativeSamplerState->Release();
    m_NativeSamplerState = nullptr;
}

ID3D11SamplerState* DX11Sampler::GetNativeSamplerState() const
{
    return m_NativeSamplerState;
}

}