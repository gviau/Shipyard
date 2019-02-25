#include <common/wrapper/dx11/dx11rendertarget.h>

#include <common/wrapper/dx11/dx11_common.h>

#include <common/wrapper/dx11/dx11texture.h>

#pragma warning( disable : 4005 )

#include <d3d11.h>

#pragma warning( default : 4005 )

#include <cassert>

namespace Shipyard
{;

ID3D11RenderTargetView* CreateRenderTargetView(ID3D11Device& device,ID3D11Texture2D* d3dTexture, DXGI_FORMAT format);
ID3D11DepthStencilView* CreateDepthStencilView(ID3D11Device& device, ID3D11Texture2D* d3dTexture, DXGI_FORMAT format);

DX11RenderTarget::DX11RenderTarget(ID3D11Device& device, DX11Texture2D** texturesToAttach, uint32_t numTexturesToAttach)
    : m_NumRenderTargetsAttached(0)
{
    memset(m_RenderTargetViews, 0, sizeof(m_RenderTargetViews[0]) * GfxConstants::GfxConstants_MaxRenderTargetsBound);
    memset(m_RenderTargetShaderResourceViews, 0, sizeof(m_RenderTargetShaderResourceViews[0]) * GfxConstants::GfxConstants_MaxRenderTargetsBound);
    memset(m_Formats, uint16_t(GfxFormat::Uknown), sizeof(m_Formats[0]) * GfxConstants::GfxConstants_MaxRenderTargetsBound);

    m_NumRenderTargetsAttached = numTexturesToAttach;

    m_IsValid = true;
    bool isFirstRenderTarget = true;

    for (uint32_t i = 0; i < m_NumRenderTargetsAttached; i++)
    {
        DX11Texture2D* gfxTexture = reinterpret_cast<DX11Texture2D*>(texturesToAttach[i]);
        if (gfxTexture == nullptr)
        {
            continue;
        }

        if ((gfxTexture->GetTextureUsage() & TextureUsage::TextureUsage_RenderTarget) == 0)
        {
            m_IsValid = false;

            MessageBox(NULL, "Couldn't attach texture to render target as it is either nullptr or not marked to be used a RenderTarget.", "DX11 Error", MB_OK);
            return;
        }

        if (isFirstRenderTarget)
        {
            m_Width = gfxTexture->GetWidth();
            m_Height = gfxTexture->GetHeight();

            isFirstRenderTarget = false;
        }
        else
        {
            if (m_Width != gfxTexture->GetWidth() || m_Height != gfxTexture->GetHeight())
            {
                m_IsValid = false;

                MessageBox(NULL, "Render target does not all have the same dimension.", "DX11 Error", MB_OK);
                return;
            }
        }

        m_Formats[i] = gfxTexture->GetPixelFormat();

        m_RenderTargetViews[i] = CreateRenderTargetView(device, gfxTexture->GetTexture(), ConvertShipyardFormatToDX11(gfxTexture->GetPixelFormat()));

        m_RenderTargetShaderResourceViews[i] = gfxTexture->GetShaderResourceView();
        if (m_RenderTargetShaderResourceViews[i] != nullptr)
        {
            m_RenderTargetShaderResourceViews[i]->AddRef();
        }
    }
}

DX11RenderTarget::~DX11RenderTarget()
{
    for (uint32_t i = 0; i < m_NumRenderTargetsAttached; i++)
    {
        if (m_RenderTargetViews[i] != nullptr)
        {
            m_RenderTargetViews[i]->Release();
        }
        
        if (m_RenderTargetShaderResourceViews[i] != nullptr)
        {
            m_RenderTargetShaderResourceViews[i]->Release();
        }
    }
}

DX11DepthStencilRenderTarget::DX11DepthStencilRenderTarget(ID3D11Device& device, DX11Texture2D& depthStencilTexture)
    : m_DepthStencilView(nullptr)
    , m_DepthStencilShaderResourceView(nullptr)
{
    if ((depthStencilTexture.GetTextureUsage() & TextureUsage::TextureUsage_DepthStencil) == 0)
    {
        MessageBox(NULL, "Couldn't attach texture to render target as it is either nullptr or not marked to be used a RenderTarget.", "DX11 Error", MB_OK);
        return;
    }

    m_Width = depthStencilTexture.GetWidth();
    m_Height = depthStencilTexture.GetHeight();
    m_Format = depthStencilTexture.GetPixelFormat();

    m_DepthStencilView = CreateDepthStencilView(device, depthStencilTexture.GetTexture(), ConvertShipyardFormatToDX11(depthStencilTexture.GetPixelFormat()));

    m_DepthStencilShaderResourceView = depthStencilTexture.GetShaderResourceView();
    m_DepthStencilShaderResourceView->AddRef();
}

DX11DepthStencilRenderTarget::~DX11DepthStencilRenderTarget()
{
    m_DepthStencilView->Release();
    m_DepthStencilShaderResourceView->Release();
}

ID3D11RenderTargetView* CreateRenderTargetView(ID3D11Device& device, ID3D11Texture2D* d3dTexture, DXGI_FORMAT format)
{
    D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
    renderTargetViewDesc.Format = format;
    renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    renderTargetViewDesc.Texture2D.MipSlice = 0;

    ID3D11RenderTargetView* renderTargetView = nullptr;
    HRESULT hr = device.CreateRenderTargetView(d3dTexture, &renderTargetViewDesc, &renderTargetView);
    if (FAILED(hr))
    {
        MessageBox(NULL, "CreateRenderTargetView failed for RenderTarget", "DX11 error", MB_OK);
        return nullptr;
    }

    return renderTargetView;
}

ID3D11DepthStencilView* CreateDepthStencilView(ID3D11Device& device, ID3D11Texture2D* d3dTexture, DXGI_FORMAT format)
{
    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
    depthStencilViewDesc.Format = format;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDesc.Texture2D.MipSlice = 0;
    depthStencilViewDesc.Flags = 0;

    ID3D11DepthStencilView* depthStencilView = nullptr;
    HRESULT hr = device.CreateDepthStencilView(d3dTexture, &depthStencilViewDesc, &depthStencilView);
    if (FAILED(hr))
    {
        MessageBox(NULL, "CreateDepthStencilView failed for RenderTarget", "DX11 error", MB_OK);
        return nullptr;
    }

    return depthStencilView;
}

}