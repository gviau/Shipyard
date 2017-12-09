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
    , m_WindowHandle(windowHandle)
    , m_SwapChain(nullptr)
    , m_BackBuffer(nullptr)
{
    m_SwapChain = renderDevice.CreateSwapchain(m_Width, m_Height, m_ViewSurfaceFormat, m_WindowHandle);
   
    CreateBackBuffer();
}

DX11ViewSurface::~DX11ViewSurface()
{
    if (m_SwapChain != nullptr)
    {
        m_SwapChain->Release();
    }

    if (m_BackBuffer != nullptr)
    {
        m_BackBuffer->Release();
    }
}

void DX11ViewSurface::PreRender()
{
    m_ImmediateRenderContext.SetRenderTargetView(0, m_BackBuffer);
    m_ImmediateRenderContext.ClearFirstRenderTarget(0.0f, 0.0f, 0.125f, 1.0f);
}

void DX11ViewSurface::Render()
{

}

void DX11ViewSurface::PostRender()
{

}

void DX11ViewSurface::Flip()
{
    HRESULT hr = m_SwapChain->Present(0, 0);
    if (FAILED(hr))
    {
        MessageBox(NULL, "Present failed", "error", MB_OK);
    }
}

void DX11ViewSurface::CreateBackBuffer()
{
    ID3D11Texture2D* backBufferTexture = nullptr;
    HRESULT hr = m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBufferTexture);
    if (FAILED(hr))
    {
        MessageBox(NULL, "BackBufferTexture creation failed", "DX11 error", MB_OK);
        return;
    }

    ID3D11Device* device = m_RenderDevice.GetDevice();
    hr = device->CreateRenderTargetView(backBufferTexture, nullptr, &m_BackBuffer);
    if (FAILED(hr))
    {
        MessageBox(NULL, "DX11ViewSurface CreateRenderTargetView failed", "DX11 error", MB_OK);
        return;
    }

    backBufferTexture->Release();
}

}