#include <common/wrapper/dx11/dx11rootsignature.h>

namespace Shipyard
{;

DX11RootSignature::DX11RootSignature(const Array<RootSignatureParameterEntry>& rootSignatureParameters)
    : RootSignature(rootSignatureParameters)
{
}

}