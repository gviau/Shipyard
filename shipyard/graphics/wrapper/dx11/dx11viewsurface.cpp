#include <graphics/wrapper/dx11/dx11viewsurface.h>

#include <graphics/wrapper/dx11/dx11_common.h>

#include <graphics/wrapper/dx11/dx11renderdevice.h>
#include <graphics/wrapper/dx11/dx11rendertarget.h>
#include <graphics/wrapper/dx11/dx11texture.h>

#include <system/logger.h>

#include <dxgi.h>

#pragma warning( disable : 4005 )

#include <d3d11.h>

#pragma warning( default : 4005 )

namespace Shipyard
{;

DX11ViewSurface::DX11ViewSurface()
    : m_WindowHandle(nullptr)
    , m_SwapChain(nullptr)
{
}

bool DX11ViewSurface::Create(
        DX11RenderDevice& renderDevice,
        uint32_t width,
        uint32_t height,
        GfxFormat viewSurfaceFormat,
        HWND windowHandle)
{
    m_RenderDevice = &renderDevice;
    m_Width = width;
    m_Height = height;
    m_ViewSurfaceFormat = viewSurfaceFormat;
    m_WindowHandle = windowHandle;

    m_SwapChain = renderDevice.CreateSwapchain(m_Width, m_Height, m_ViewSurfaceFormat, m_WindowHandle, m_BackBufferTextureHandle);

    GFXTexture2DHandle* pBackBufferTexture = &m_BackBufferTextureHandle;

    m_BackBufferRenderTargetHandle = renderDevice.CreateRenderTarget(pBackBufferTexture, 1);

    return true;
}

void DX11ViewSurface::Destroy()
{
    SHIP_ASSERT_MSG(m_SwapChain != nullptr, "Can't call Destroy on invalid view surface 0x%p", this);

    m_RenderDevice->DestroyTexture2D(m_BackBufferTextureHandle);

    m_SwapChain->Release();
    m_SwapChain = nullptr;
}

void DX11ViewSurface::Flip()
{
    HRESULT hr = m_SwapChain->Present(0, 0);
    if (FAILED(hr))
    {
        SHIP_LOG_ERROR("DX11ViewSurface::Flip() --> Couldn't flip front buffer.");
    }
}

}