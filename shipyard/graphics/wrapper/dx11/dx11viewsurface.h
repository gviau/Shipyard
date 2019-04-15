#pragma once

#include <graphics/wrapper/viewsurface.h>

#include <graphics/wrapper/dx11/dx11rendertarget.h>
#include <graphics/wrapper/dx11/dx11texture.h>

#include <windows.h>

struct IDXGISwapChain;
struct ID3D11DepthStencilView;
struct ID3D11RenderTargetView;

namespace Shipyard
{
    class DX11RenderDevice;

    class SHIPYARD_API DX11ViewSurface : public ViewSurface
    {
    public:
        DX11ViewSurface();

        bool Create(
                DX11RenderDevice& renderDevice,
                uint32_t width,
                uint32_t height,
                GfxFormat viewSurfaceFormat,
                HWND windowHandle);
        void Destroy();

        void Flip();

        GFXRenderTargetHandle GetBackBufferRenderTargetHandle() { return m_BackBufferRenderTargetHandle; }

    private:
        GFXRenderDevice* m_RenderDevice;
        HWND m_WindowHandle;
        IDXGISwapChain* m_SwapChain;

        GFXRenderTargetHandle m_BackBufferRenderTargetHandle;
        GFXTexture2DHandle m_BackBufferTextureHandle;
    };
}