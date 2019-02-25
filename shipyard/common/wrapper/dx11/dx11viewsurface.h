#pragma once

#include <common/wrapper/viewsurface.h>

#include <windows.h>

struct IDXGISwapChain;
struct ID3D11DepthStencilView;
struct ID3D11RenderTargetView;

namespace Shipyard
{
    class DX11RenderDevice;
    class DX11RenderDeviceContext;
    class DX11RenderTarget;
    class DX11Texture2D;

    class SHIPYARD_API DX11ViewSurface : public ViewSurface
    {
    public:
        DX11ViewSurface(
                DX11RenderDevice& renderDevice,
                DX11RenderDeviceContext& immediateRenderContext,
                uint32_t width,
                uint32_t height,
                GfxFormat viewSurfaceFormat,
                HWND windowHandle);
        ~DX11ViewSurface();

        void Flip();

        DX11RenderTarget* GetBackBufferRenderTarget() const { return m_BackBufferRenderTarget; }

    private:
        DX11RenderDevice& m_RenderDevice;
        DX11RenderDeviceContext& m_ImmediateRenderContext;
        HWND m_WindowHandle;
        IDXGISwapChain* m_SwapChain;

        DX11RenderTarget* m_BackBufferRenderTarget;
        DX11Texture2D* m_BackBufferTexture;
    };
}