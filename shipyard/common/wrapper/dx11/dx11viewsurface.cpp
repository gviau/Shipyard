#include <common/wrapper/dx11/dx11viewsurface.h>

#include <common/wrapper/dx11/dx11_common.h>

#include <common/wrapper/dx11/dx11renderdevice.h>
#include <common/wrapper/dx11/dx11renderdevicecontext.h>
#include <common/wrapper/dx11/dx11rendertarget.h>
#include <common/wrapper/dx11/dx11texture.h>

#include <system/logger.h>

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
    , m_WindowHandle(windowHandle)
    , m_SwapChain(nullptr)
    , m_BackBufferRenderTarget(nullptr)
    , m_BackBufferTexture(nullptr)
{
    m_SwapChain = renderDevice.CreateSwapchain(m_Width, m_Height, m_ViewSurfaceFormat, m_WindowHandle);

    ID3D11Texture2D* backBufferTexture = nullptr;
    HRESULT hr = m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBufferTexture);
    if (FAILED(hr))
    {
        SHIP_LOG_ERROR("DX11ViewSurface::DX11ViewSurface() --> Couldn't create view surface.");
        return;
    }

    ID3D11Device* device = m_RenderDevice.GetDevice();

    m_BackBufferTexture = MemAlloc(DX11Texture2D)(*device, *backBufferTexture, m_ViewSurfaceFormat);
    
    m_BackBufferRenderTarget = m_RenderDevice.CreateRenderTarget((GFXTexture2D**)&m_BackBufferTexture, 1);

    backBufferTexture->Release();
}

DX11ViewSurface::~DX11ViewSurface()
{
    if (m_SwapChain != nullptr)
    {
        m_SwapChain->Release();
    }

    MemFree(m_BackBufferTexture);
    MemFree(m_BackBufferRenderTarget);
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