#include <common/wrapper/dx11/dx11texture.h>

#include <cassert>

#include <common/wrapper/dx11/dx11_common.h>

#pragma warning( disable : 4005 )

#include <d3d11.h>

#pragma warning( default : 4005 )


namespace Shipyard
{;

DX11BaseTexture::DX11BaseTexture(
        ID3D11Device& device,
        uint32_t width,
        uint32_t height,
        GfxFormat pixelFormat,
        bool dynamic,
        void* initialData,
        bool generateMips,
        TextureUsage textureUsage)
    : BaseTexture(width, height, pixelFormat, textureUsage)
    , m_ShaderResourceView(nullptr)
{

}

DX11BaseTexture::~DX11BaseTexture()
{
    if (m_ShaderResourceView != nullptr)
    {
        m_ShaderResourceView->Release();
    }
}

ID3D11ShaderResourceView* DX11BaseTexture::CreateShaderResourceView(ID3D11Device& device, ID3D11Resource& resource, GfxFormat pixelFormat, TextureViewType textureViewType, TextureUsage textureUsage)
{
    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;

    // A depth stencil view with a depth format cannot be created as a shader resource view. Instead we, need to use
    // an equivalent non-depth format and create the depth stencil view with the proper depth format.
    if ((textureUsage & TextureUsage::TextureUsage_DepthStencil) > 0)
    {
        switch (pixelFormat)
        {
        case GfxFormat::D16_UNORM:
            shaderResourceViewDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R16_UNORM;
            break;

        case GfxFormat::D24_UNORM_S8_UINT:
            shaderResourceViewDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
            break;

        case GfxFormat::D32_FLOAT:
            shaderResourceViewDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT;
            break;

        case GfxFormat::D32_FLOAT_S8X24_UINT:
            shaderResourceViewDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
            break;

        default:
            shaderResourceViewDesc.Format = ConvertShipyardFormatToDX11(pixelFormat);
            break;
        }
    }
    else
    {
        shaderResourceViewDesc.Format = ConvertShipyardFormatToDX11(pixelFormat);
    }

    switch (textureViewType)
    {
    case TextureViewType::Tex2D:
        shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        shaderResourceViewDesc.Texture2D.MipLevels = 1;
        shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
        break;

    default:
        assert(false);
    }

    ID3D11ShaderResourceView* shaderResourceView = nullptr;
    HRESULT hr = device.CreateShaderResourceView(&resource, &shaderResourceViewDesc, &shaderResourceView);
    if (FAILED(hr))
    {
        MessageBox(NULL, "CreateShaderResourceView", "DX11 error", MB_OK);
        return nullptr;
    }

    return shaderResourceView;
}

DX11Texture2D::DX11Texture2D(
        ID3D11Device& device,
        uint32_t width,
        uint32_t height,
        GfxFormat pixelFormat,
        bool dynamic,
        void* initialData,
        bool generateMips,
        TextureUsage textureUsage)
    : DX11BaseTexture(device, width, height, pixelFormat, dynamic, initialData, generateMips, textureUsage)
    , m_Texture(nullptr)
{
    D3D11_USAGE usage = (dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT);

    D3D11_TEXTURE2D_DESC textureDesc;
    textureDesc.Width = width;
    textureDesc.Height = height;
    textureDesc.Format = ConvertShipyardFormatToDX11(pixelFormat);
    textureDesc.MipLevels = (generateMips ? 0 : 1);
    textureDesc.ArraySize = 1;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Usage = usage;
    textureDesc.CPUAccessFlags = (dynamic ? D3D11_CPU_ACCESS_WRITE : 0);
    textureDesc.MiscFlags = 0;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    if ((textureUsage & TextureUsage::TextureUsage_RenderTarget) > 0)
    {
        textureDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
    }

    if ((textureUsage & TextureUsage::TextureUsage_DepthStencil) > 0)
    {
        textureDesc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;

        // A depth stencil view with a depth format cannot be created as a shader resource view. Instead we, need to use
        // an equivalent non-depth format and create the depth stencil view with the proper depth format.
        switch (pixelFormat)
        {
        case GfxFormat::D16_UNORM:
            textureDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R16_TYPELESS;
            break;

        case GfxFormat::D24_UNORM_S8_UINT:
            textureDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R24G8_TYPELESS;
            break;

        case GfxFormat::D32_FLOAT:
            textureDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R32_TYPELESS;
            break;

        case GfxFormat::D32_FLOAT_S8X24_UINT:
            textureDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R32G8X24_TYPELESS;
            break;

        default:
            break;
        }
    }

    HRESULT hr;
    if (initialData != nullptr)
    {
        uint32_t rowPitch = GetRowPitch(width, pixelFormat);

        D3D11_SUBRESOURCE_DATA data;
        data.pSysMem = initialData;
        data.SysMemPitch = rowPitch;
        data.SysMemSlicePitch = rowPitch * height;

        hr = device.CreateTexture2D(&textureDesc, &data, &m_Texture);
    }
    else
    {
        hr = device.CreateTexture2D(&textureDesc, nullptr, &m_Texture);
    }

    if (FAILED(hr))
    {
        MessageBox(NULL, "CreateTexture2D failed", "DX11 error", MB_OK);
        return;
    }

    m_ShaderResourceView = DX11BaseTexture::CreateShaderResourceView(device, *m_Texture, m_PixelFormat, TextureViewType::Tex2D, textureUsage);
}

DX11Texture2D::DX11Texture2D(ID3D11Device& device, ID3D11Texture2D& texture, GfxFormat format)
    : DX11BaseTexture(device, 0, 0, format, false, nullptr, false, TextureUsage::TextureUsage_RenderTarget)
    , m_Texture(nullptr)
{
    m_Texture = &texture;

    m_Texture->AddRef();

    D3D11_TEXTURE2D_DESC textureDesc;
    m_Texture->GetDesc(&textureDesc);

    m_Width = textureDesc.Width;
    m_Height = textureDesc.Height;
}

DX11Texture2D::~DX11Texture2D()
{
    if (m_Texture != nullptr)
    {
        m_Texture->Release();
    }
}

}