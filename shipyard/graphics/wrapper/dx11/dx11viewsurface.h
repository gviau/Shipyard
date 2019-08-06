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

        shipBool Create(
                DX11RenderDevice& renderDevice,
                shipUint32 width,
                shipUint32 height,
                GfxFormat viewSurfaceFormat,
                HWND windowHandle);
        void Destroy();

        void Flip();

        GFXRenderTargetHandle GetBackBufferRenderTargetHandle() const { return m_BackBufferRenderTargetHandle; }

    private:
        GFXRenderDevice* m_RenderDevice;
        HWND m_WindowHandle;
        IDXGISwapChain* m_SwapChain;

        GFXRenderTargetHandle m_BackBufferRenderTargetHandle;
        GFXTexture2DHandle m_BackBufferTextureHandle;
    };
}