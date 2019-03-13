#include <graphics/wrapper/dx11/dx11rootsignature.h>

namespace Shipyard
{;

bool DX11RootSignature::Create(const Array<RootSignatureParameterEntry>& rootSignatureParameters)
{
    m_RootSignatureParameters = rootSignatureParameters;

    return true;
}

void DX11RootSignature::Destroy()
{

}

}