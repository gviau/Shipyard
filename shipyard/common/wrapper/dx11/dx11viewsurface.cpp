#include <common/wrapper/dx11/dx11viewsurface.h>

#include <common/wrapper/dx11/dx11_common.h>

#include <common/wrapper/dx11/dx11renderdevice.h>
#include <common/wrapper/dx11/dx11renderdevicecontext.h>

#include <dxgi.h>

#pragma warning( disable : 4005 )

#include <d3d11.h>

#pragma warning( default : 4005 )

namespace Shipyard
{;

DX11ViewSurface::DX11ViewSurface(DX11RenderDevice& renderDevice, DX11RenderDeviceContext& immediateRenderContext, uint32_t width, uint32_t height, GfxFormat viewSurfaceFormat, HWND windowHandle)
    : ViewSurface(width, height, viewSurfaceFormat)
    , m_RenderDevice(renderDevice)
    , m_ImmediateRenderContext(immediateRenderContext)
{
    m_SwapChain = renderDevice.CreateSwapchain(m_Width, m_Height, m_ViewSurfaceFormat, m_WindowHandle);
}

DX11ViewSurface::~DX11ViewSurface()
{
    if (m_SwapChain != nullptr)
    {
        m_SwapChain->Release();
    }
}

void DX11ViewSurface::PreRender()
{

}

void DX11ViewSurface::Render()
{

}

void DX11ViewSurface::PostRender()
{

}

void DX11ViewSurface::Flip()
{
    m_SwapChain->Present(0, 0);
}

}