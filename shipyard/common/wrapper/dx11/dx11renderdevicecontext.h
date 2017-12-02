#pragma once

#include <common/wrapper/renderdevicecontext.h>

struct ID3D11DeviceContext;

namespace Shipyard
{
    class DX11RenderDeviceContext : public RenderDeviceContext
    {
    public:
        DX11RenderDeviceContext(const GFXRenderDevice& renderDevice);

    private:
        ID3D11DeviceContext* m_ImmediateDeviceContext;
    };
}