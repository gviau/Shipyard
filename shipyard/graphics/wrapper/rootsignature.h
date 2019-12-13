#pragma once

#include <graphics/wrapper/wrapper_common.h>

#include <system/array.h>

namespace Shipyard
{
    class SHIPYARD_GRAPHICS_API RootSignature
    {
    public:
        const Array<RootSignatureParameterEntry>& GetRootSignatureParameters() const;

    protected:
        Array<RootSignatureParameterEntry> m_RootSignatureParameters;
    };
}