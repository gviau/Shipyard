#include <common/wrapper/dx11/dx11renderdevicecontext.h>

#include <common/wrapper/dx11/dx11renderdevice.h>

#pragma warning( disable : 4005 )

#include <d3d11.h>

#pragma warning( default : 4005 )

namespace Shipyard
{;

DX11RenderDeviceContext::DX11RenderDeviceContext(const GFXRenderDevice& renderDevice)
    : RenderDeviceContext(renderDevice)
{
    m_ImmediateDeviceContext = renderDevice.GetImmediateDeviceContext();
}

}