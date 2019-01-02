#pragma once

#include <common/wrapper/wrapper_common.h>

#include <system/array.h>

namespace Shipyard
{
    class SHIPYARD_API RootSignature
    {
    public:
        RootSignature(const Array<RootSignatureParameterEntry>& rootSignatureParameters);

        const Array<RootSignatureParameterEntry>& GetRootSignatureParameters() const;

    protected:
        Array<RootSignatureParameterEntry> m_RootSignatureParameters;
    };
}