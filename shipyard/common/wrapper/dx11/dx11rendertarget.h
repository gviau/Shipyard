#pragma once

#include <common/wrapper/rendertarget.h>

struct ID3D11Device;
struct ID3D11DepthStencilView;
struct ID3D11RenderTargetView;
struct ID3D11ShaderResourceView;
struct ID3D11Texture2D;

namespace Shipyard
{
    class DX11Texture2D;

    class SHIPYARD_API DX11RenderTarget : public RenderTarget
    {
    public:
        DX11RenderTarget(ID3D11Device& device, DX11Texture2D** texturesToAttach, uint32_t numTexturesToAttach);
        ~DX11RenderTarget();

        bool IsValid() const { return m_IsValid; }

        uint32_t GetNumRenderTargetsAttached() const { return m_NumRenderTargetsAttached; }

        ID3D11RenderTargetView* const * GetRenderTargetViews() const { return m_RenderTargetViews; }
        ID3D11ShaderResourceView* const * GetRenderTargetShaderResourceView() const { return m_RenderTargetShaderResourceViews; }
        GfxFormat const * GetRenderTargetFormats() const { return m_Formats; }

    private:
        ID3D11RenderTargetView* m_RenderTargetViews[GfxConstants::GfxConstants_MaxRenderTargetsBound];
        ID3D11ShaderResourceView* m_RenderTargetShaderResourceViews[GfxConstants::GfxConstants_MaxRenderTargetsBound];
        GfxFormat m_Formats[GfxConstants::GfxConstants_MaxRenderTargetsBound];

        uint32_t m_NumRenderTargetsAttached;
        bool m_IsValid;
    };

    class SHIPYARD_API DX11DepthStencilRenderTarget : public DepthStencilRenderTarget
    {
    public:
        DX11DepthStencilRenderTarget(ID3D11Device& device, DX11Texture2D& depthStencilTexture);
        ~DX11DepthStencilRenderTarget();

        bool IsValid() const { return (m_DepthStencilView != nullptr); }

        ID3D11DepthStencilView* const GetDepthStencilView() const { return m_DepthStencilView; }
        ID3D11ShaderResourceView* const GetDepthStencilShaderResourceView() const { return m_DepthStencilShaderResourceView; }
        GfxFormat GetDepthStencilFormat() const { return m_Format; }

    private:
        ID3D11DepthStencilView* m_DepthStencilView;
        ID3D11ShaderResourceView* m_DepthStencilShaderResourceView;
        GfxFormat m_Format;
    };
}