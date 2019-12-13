#include <graphics/graphicsprecomp.h>

#include <graphics/wrapper/rootsignature.h>

namespace Shipyard
{;

const Array<RootSignatureParameterEntry>& RootSignature::GetRootSignatureParameters() const
{
    return m_RootSignatureParameters;
}

}