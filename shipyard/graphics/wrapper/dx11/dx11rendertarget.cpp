#include <graphics/graphicsprecomp.h>

#include <graphics/wrapper/dx11/dx11rendertarget.h>

#include <graphics/wrapper/dx11/dx11_common.h>

#include <graphics/wrapper/dx11/dx11texture.h>

#include <system/logger.h>

#pragma warning( disable : 4005 )

#include <d3d11.h>

#pragma warning( default : 4005 )

namespace Shipyard
{;

ID3D11RenderTargetView* CreateRenderTargetView(ID3D11Device& device,ID3D11Texture2D* d3dTexture, DXGI_FORMAT format);
ID3D11DepthStencilView* CreateDepthStencilView(ID3D11Device& device, ID3D11Texture2D* d3dTexture, DXGI_FORMAT format);

DX11RenderTarget::DX11RenderTarget()
    : m_NumRenderTargetsAttached(0)
{
    memset(m_RenderTargetViews, 0, sizeof(m_RenderTargetViews[0]) * GfxConstants::GfxConstants_MaxRenderTargetsBound);
    memset(m_RenderTargetShaderResourceViews, 0, sizeof(m_RenderTargetShaderResourceViews[0]) * GfxConstants::GfxConstants_MaxRenderTargetsBound);
    memset(m_Formats, shipUint16(GfxFormat::Unknown), sizeof(m_Formats[0]) * GfxConstants::GfxConstants_MaxRenderTargetsBound);
}

shipBool DX11RenderTarget::Create(ID3D11Device& device, DX11Texture2D** texturesToAttach, shipUint32 numTexturesToAttach)
{
    m_NumRenderTargetsAttached = numTexturesToAttach;

    shipBool isFirstRenderTarget = true;

    for (shipUint32 i = 0; i < m_NumRenderTargetsAttached; i++)
    {
        DX11Texture2D* gfxTexture = reinterpret_cast<DX11Texture2D*>(texturesToAttach[i]);
        if (gfxTexture == nullptr)
        {
            continue;
        }

        if ((gfxTexture->GetTextureUsage() & TextureUsage::TextureUsage_RenderTarget) == 0)
        {
            SHIP_LOG_ERROR("DX11RenderTarget::DX11RenderTarget() --> Couldn't attach texture to render target as it is either nullptr or not marked to be used as a RenderTarget.");
            return false;
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
                SHIP_LOG_ERROR("DX11RenderTarget::DX11RenderTarget() --> Rendre targets do not all have the same dimension.");
                return false;
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

    m_IsValid = true;

    return true;
}

void DX11RenderTarget::Destroy()
{
    for (shipUint32 i = 0; i < m_NumRenderTargetsAttached; i++)
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

    memset(m_RenderTargetViews, 0, sizeof(m_RenderTargetViews[0]) * GfxConstants::GfxConstants_MaxRenderTargetsBound);
    memset(m_RenderTargetShaderResourceViews, 0, sizeof(m_RenderTargetShaderResourceViews[0]) * GfxConstants::GfxConstants_MaxRenderTargetsBound);
    memset(m_Formats, shipUint16(GfxFormat::Unknown), sizeof(m_Formats[0]) * GfxConstants::GfxConstants_MaxRenderTargetsBound);

    m_NumRenderTargetsAttached = 0;
}

DX11DepthStencilRenderTarget::DX11DepthStencilRenderTarget()
    : m_DepthStencilView(nullptr)
    , m_DepthStencilShaderResourceView(nullptr)
    , m_Format(GfxFormat::Unknown)
{

}

shipBool DX11DepthStencilRenderTarget::Create(ID3D11Device& device, GFXTexture2D& depthStencilTexture)
{
    if ((depthStencilTexture.GetTextureUsage() & TextureUsage::TextureUsage_DepthStencil) == 0)
    {
        SHIP_LOG_ERROR("DX11DepthStencilRenderTarget::DX11DepthStencilRenderTarget() --> Couldn't attach texture to render target as it is either nullptr or not marked to be used as a RenderTarget.");
        return false;
    }

    m_Width = depthStencilTexture.GetWidth();
    m_Height = depthStencilTexture.GetHeight();
    m_Format = depthStencilTexture.GetPixelFormat();

    m_DepthStencilView = CreateDepthStencilView(device, depthStencilTexture.GetTexture(), ConvertShipyardFormatToDX11(depthStencilTexture.GetPixelFormat()));

    m_DepthStencilShaderResourceView = depthStencilTexture.GetShaderResourceView();
    m_DepthStencilShaderResourceView->AddRef();

    return true;
}

void DX11DepthStencilRenderTarget::Destroy()
{
    if (m_DepthStencilView != nullptr)
    {
        m_DepthStencilView->Release();

        m_DepthStencilView = nullptr;
    }

    if (m_DepthStencilShaderResourceView != nullptr)
    {
        m_DepthStencilShaderResourceView->Release();
        m_DepthStencilShaderResourceView = nullptr;
    }
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
        SHIP_LOG_ERROR("CreateRenderTargetView() --> Couldn't create render target view.");
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
        SHIP_LOG_ERROR("CreateDepthStencilView() --> Couldn't create depth stencil view.");
        return nullptr;
    }

    return depthStencilView;
}

}