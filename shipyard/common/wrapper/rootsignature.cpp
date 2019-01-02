#include <common/wrapper/rootsignature.h>

namespace Shipyard
{;

RootSignature::RootSignature(const Array<RootSignatureParameterEntry>& rootSignatureParameters)
    : m_RootSignatureParameters(rootSignatureParameters)
{

}

const Array<RootSignatureParameterEntry>& RootSignature::GetRootSignatureParameters() const
{
    return m_RootSignatureParameters;
}

}