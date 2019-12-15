#include <graphics/graphicsprecomp.h>

#include <graphics/wrapper/dx11/dx11texture.h>

#include <graphics/wrapper/dx11/dx11_common.h>

#include <system/logger.h>

#pragma warning( disable : 4005 )

#include <d3d11.h>

#pragma warning( default : 4005 )


namespace Shipyard
{;

DX11BaseTexture::DX11BaseTexture()
    : m_ShaderResourceView(nullptr)
    , m_UnorderedAccessView(nullptr)
{

}

D3D11_BIND_FLAG GetTextureBindFlagFromTextureUsage(TextureUsage textureUsage)
{
    DWORD bindFlags = 0;

    if ((textureUsage & TextureUsage::TextureUsage_ShaderResourceView) > 0)
    {
        bindFlags |= D3D11_BIND_SHADER_RESOURCE;
    }

    if ((textureUsage & TextureUsage::TextureUsage_UnorderedAccessView) > 0)
    {
        bindFlags |= D3D11_BIND_UNORDERED_ACCESS;
    }

    if ((textureUsage & TextureUsage::TextureUsage_RenderTarget) > 0)
    {
        bindFlags |= D3D11_BIND_RENDER_TARGET;
    }

    if ((textureUsage & TextureUsage::TextureUsage_DepthStencil) > 0)
    {
        bindFlags |= D3D11_BIND_DEPTH_STENCIL;
    }

    return D3D11_BIND_FLAG(bindFlags);
}

ID3D11Texture2D* CreateTexture2D(
        ID3D11Device& device,
        shipUint32 width,
        shipUint32 height,
        shipUint32 arraySize,
        GfxFormat pixelFormat,
        shipBool dynamic,
        shipBool generateMips,
        TextureUsage textureUsage,
        void* initialData)
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
    textureDesc.BindFlags = GetTextureBindFlagFromTextureUsage(textureUsage);

    if ((textureUsage & TextureUsage::TextureUsage_DepthStencil) > 0)
    {
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

    ID3D11Texture2D* texture;

    HRESULT hr;
    if (initialData != nullptr)
    {
        shipUint32 rowPitch = GetRowPitch(width, pixelFormat);

        D3D11_SUBRESOURCE_DATA data;
        data.pSysMem = initialData;
        data.SysMemPitch = rowPitch;
        data.SysMemSlicePitch = rowPitch * height;

        hr = device.CreateTexture2D(&textureDesc, &data, &texture);
    }
    else
    {
        hr = device.CreateTexture2D(&textureDesc, nullptr, &texture);
    }

    if (FAILED(hr))
    {
        SHIP_LOG_ERROR("CreateTexture2D() --> Couldn't create Texture2D.");
        return nullptr;
    }

    return texture;
}

ID3D11ShaderResourceView* CreateShaderResourceView(
        ID3D11Device& device,
        ID3D11Resource& resource,
        GfxFormat pixelFormat,
        TextureViewType textureViewType,
        TextureUsage textureUsage,
        shipUint32 numMipMaps,
        shipUint32 mostDetailedMip,
        shipUint32 numSlices,
        shipUint32 firstSlice)
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
        shaderResourceViewDesc.Texture2D.MipLevels = numMipMaps;
        shaderResourceViewDesc.Texture2D.MostDetailedMip = mostDetailedMip;
        break;

    case TextureViewType::Tex2DArray:
        shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
        shaderResourceViewDesc.Texture2DArray.ArraySize = numSlices;
        shaderResourceViewDesc.Texture2DArray.FirstArraySlice = firstSlice;
        shaderResourceViewDesc.Texture2DArray.MipLevels = numMipMaps;
        shaderResourceViewDesc.Texture2DArray.MostDetailedMip = mostDetailedMip;
        break;

    case TextureViewType::Tex3D:
        shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
        shaderResourceViewDesc.Texture3D.MipLevels = numMipMaps;
        shaderResourceViewDesc.Texture3D.MostDetailedMip = mostDetailedMip;
        break;

    case TextureViewType::TexCube:
        shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
        shaderResourceViewDesc.TextureCube.MipLevels = numMipMaps;
        shaderResourceViewDesc.TextureCube.MostDetailedMip = mostDetailedMip;
        break;

    default:
        SHIP_ASSERT(false);
    }

    ID3D11ShaderResourceView* shaderResourceView = nullptr;
    HRESULT hr = device.CreateShaderResourceView(&resource, &shaderResourceViewDesc, &shaderResourceView);
    if (FAILED(hr))
    {
        SHIP_LOG_ERROR("CreateShaderResourceView() --> Couldn't create shader resource view.");
        return nullptr;
    }

    return shaderResourceView;
}

ID3D11UnorderedAccessView* CreateUnorderedAccessView(
        ID3D11Device& device,
        ID3D11Resource& resource,
        GfxFormat pixelFormat,
        TextureViewType textureViewType,
        shipUint32 mipIndex,
        shipUint32 numSlices,
        shipUint32 firstSlice)
{
    D3D11_UNORDERED_ACCESS_VIEW_DESC unorderedAccessViewDesc;

    unorderedAccessViewDesc.Format = ConvertShipyardFormatToDX11(pixelFormat);

    switch (textureViewType)
    {
    case TextureViewType::Tex2D:
        unorderedAccessViewDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
        unorderedAccessViewDesc.Texture2D.MipSlice = mipIndex;
        break;

    case TextureViewType::Tex2DArray:
        unorderedAccessViewDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
        unorderedAccessViewDesc.Texture2DArray.ArraySize = numSlices;
        unorderedAccessViewDesc.Texture2DArray.FirstArraySlice = firstSlice;
        unorderedAccessViewDesc.Texture2DArray.MipSlice = mipIndex;
        break;

    case TextureViewType::Tex3D:
        unorderedAccessViewDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;
        unorderedAccessViewDesc.Texture3D.FirstWSlice = 0;
        unorderedAccessViewDesc.Texture3D.WSize = 0xFFFFFFFF;
        unorderedAccessViewDesc.Texture3D.MipSlice = mipIndex;
        break;

    case TextureViewType::TexCube:
        unorderedAccessViewDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
        unorderedAccessViewDesc.Texture2DArray.ArraySize = 6;
        unorderedAccessViewDesc.Texture2DArray.FirstArraySlice = 0;
        unorderedAccessViewDesc.Texture2DArray.MipSlice = mipIndex;
        break;

    default:
        SHIP_ASSERT(false);
    }

    ID3D11UnorderedAccessView* unorderedAccessView = nullptr;
    HRESULT hr = device.CreateUnorderedAccessView(&resource, &unorderedAccessViewDesc, &unorderedAccessView);
    if (FAILED(hr))
    {
        SHIP_LOG_ERROR("CreateUnorderedAccessView() --> Couldn't create unordered access view.");
        return nullptr;
    }

    return unorderedAccessView;
}

DX11Texture2D::DX11Texture2D()
    : m_Texture(nullptr)
{

}

DX11Texture2D::DX11Texture2D(ID3D11Device& device, ID3D11Texture2D& texture, GfxFormat format)
    : m_Texture(nullptr)
{
    m_Texture = &texture;

    m_Texture->AddRef();

    D3D11_TEXTURE2D_DESC textureDesc;
    m_Texture->GetDesc(&textureDesc);

    m_Width = textureDesc.Width;
    m_Height = textureDesc.Height;
    m_TextureUsage = TextureUsage::TextureUsage_RenderTarget;

    m_PixelFormat = format;
}

shipBool DX11Texture2D::Create(
        ID3D11Device& device,
        shipUint32 width,
        shipUint32 height,
        GfxFormat pixelFormat,
        shipBool dynamic,
        void* initialData,
        shipBool generateMips,
        TextureUsage textureUsage)
{
    m_Width = width;
    m_Height = height;
    m_Depth = 1;
    m_NumSlices = 1;
    m_PixelFormat = pixelFormat;
    m_TextureUsage = textureUsage;

    m_Texture = CreateTexture2D(
        device,
        width,
        height,
        m_NumSlices,
        pixelFormat,
        dynamic,
        generateMips,
        m_TextureUsage,
        initialData);

    if (m_Texture == nullptr)
    {
        SHIP_LOG_ERROR("DX11Texture2D::Create() --> Couldn't create Texture2D.");
        return false;
    }

    constexpr shipUint32 mostDetailedMip = 0;
    constexpr shipUint32 numSlices = 1;
    constexpr shipUint32 firstSlice = 0;

    if ((m_TextureUsage & TextureUsage::TextureUsage_ShaderResourceView) > 0)
    {
        constexpr shipUint32 numMipMaps = 1;
        m_ShaderResourceView = CreateShaderResourceView(
                device,
                *m_Texture,
                m_PixelFormat,
                TextureViewType::Tex2D,
                textureUsage,
                numMipMaps,
                mostDetailedMip,
                numSlices,
                firstSlice);
    }

    if ((m_TextureUsage & TextureUsage::TextureUsage_UnorderedAccessView) > 0)
    {
        m_UnorderedAccessView = CreateUnorderedAccessView(
                device,
                *m_Texture,
                m_PixelFormat,
                TextureViewType::Tex2D,
                mostDetailedMip,
                numSlices,
                firstSlice);
    }

    return true;
}

void DX11Texture2D::Destroy()
{
    SHIP_ASSERT_MSG(m_Texture != nullptr, "Can't call Destroy on invalid texture2D 0x%p", this);

    if (m_ShaderResourceView != nullptr)
    {
        m_ShaderResourceView->Release();
        m_ShaderResourceView = nullptr;
    }

    if (m_UnorderedAccessView != nullptr)
    {
        m_UnorderedAccessView->Release();
        m_UnorderedAccessView = nullptr;
    }

    m_Texture->Release();
    m_Texture = nullptr;
}

DX11Texture2DArray::DX11Texture2DArray()
    : m_Texture(nullptr)
{

}

shipBool DX11Texture2DArray::Create(
        ID3D11Device& device,
        shipUint32 width,
        shipUint32 height,
        shipUint32 numSlices,
        GfxFormat pixelFormat,
        shipBool dynamic,
        void* initialData,
        shipBool generateMips,
        TextureUsage textureUsage)
{
    m_Width = width;
    m_Height = height;
    m_Depth = 1;
    m_NumSlices = MAX(numSlices, 1);
    m_PixelFormat = pixelFormat;
    m_TextureUsage = textureUsage;

    m_Texture = CreateTexture2D(
            device,
            width,
            height,
            m_NumSlices,
            pixelFormat,
            dynamic,
            generateMips,
            m_TextureUsage,
            initialData);

    if (m_Texture == nullptr)
    {
        SHIP_LOG_ERROR("DX11Texture2DArray::Create() --> Couldn't create Texture2DArray.");
        return false;
    }

    constexpr shipUint32 mostDetailedMip = 0;
    constexpr shipUint32 firstSlice = 0;

    if ((m_TextureUsage & TextureUsage::TextureUsage_ShaderResourceView) > 0)
    {
        constexpr shipUint32 numMipMaps = 1;
        m_ShaderResourceView = CreateShaderResourceView(
                device,
                *m_Texture,
                m_PixelFormat,
                TextureViewType::Tex2DArray,
                textureUsage,
                numMipMaps,
                mostDetailedMip,
                m_NumSlices,
                firstSlice);
    }

    if ((m_TextureUsage & TextureUsage::TextureUsage_UnorderedAccessView) > 0)
    {
        m_UnorderedAccessView = CreateUnorderedAccessView(
                device,
                *m_Texture,
                m_PixelFormat,
                TextureViewType::Tex2DArray,
                mostDetailedMip,
                m_NumSlices,
                firstSlice);
    }

    return true;
}

void DX11Texture2DArray::Destroy()
{
    SHIP_ASSERT_MSG(m_Texture != nullptr, "Can't call Destroy on invalid texture2DArray 0x%p", this);

    if (m_ShaderResourceView != nullptr)
    {
        m_ShaderResourceView->Release();
        m_ShaderResourceView = nullptr;
    }

    if (m_UnorderedAccessView != nullptr)
    {
        m_UnorderedAccessView->Release();
        m_UnorderedAccessView = nullptr;
    }

    m_Texture->Release();
    m_Texture = nullptr;
}

DX11Texture3D::DX11Texture3D()
    : m_Texture(nullptr)
{

}

shipBool DX11Texture3D::Create(
        ID3D11Device& device,
        shipUint32 width,
        shipUint32 height,
        shipUint32 depth,
        GfxFormat pixelFormat,
        shipBool dynamic,
        void* initialData,
        shipBool generateMips,
        TextureUsage textureUsage)
{
    m_Width = width;
    m_Height = height;
    m_Depth = depth;
    m_NumSlices = 1;
    m_PixelFormat = pixelFormat;
    m_TextureUsage = textureUsage;

    D3D11_USAGE usage = (dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT);

    D3D11_TEXTURE3D_DESC textureDesc;
    textureDesc.Width = width;
    textureDesc.Height = height;
    textureDesc.Depth = depth;
    textureDesc.Format = ConvertShipyardFormatToDX11(pixelFormat);
    textureDesc.MipLevels = (generateMips ? 0 : 1);
    textureDesc.Usage = usage;
    textureDesc.CPUAccessFlags = (dynamic ? D3D11_CPU_ACCESS_WRITE : 0);
    textureDesc.MiscFlags = 0;
    textureDesc.BindFlags = GetTextureBindFlagFromTextureUsage(textureUsage);

    HRESULT hr;
    if (initialData != nullptr)
    {
        shipUint32 rowPitch = GetRowPitch(width, pixelFormat);

        D3D11_SUBRESOURCE_DATA data;
        data.pSysMem = initialData;
        data.SysMemPitch = rowPitch;
        data.SysMemSlicePitch = rowPitch * height;

        hr = device.CreateTexture3D(&textureDesc, &data, &m_Texture);
    }
    else
    {
        hr = device.CreateTexture3D(&textureDesc, nullptr, &m_Texture);
    }

    if (FAILED(hr))
    {
        SHIP_LOG_ERROR("DX11Texture3D::Create() --> Couldn't create Texture3D.");
        return false;
    }

    constexpr shipUint32 mostDetailedMip = 0;
    constexpr shipUint32 numSlices = 1;
    constexpr shipUint32 firstSlice = 0;

    if ((m_TextureUsage & TextureUsage::TextureUsage_ShaderResourceView) > 0)
    {
        constexpr shipUint32 numMipMaps = 1;
        m_ShaderResourceView = CreateShaderResourceView(
                device,
                *m_Texture,
                m_PixelFormat,
                TextureViewType::Tex3D,
                textureUsage,
                numMipMaps,
                mostDetailedMip,
                numSlices,
                firstSlice);
    }

    if ((m_TextureUsage & TextureUsage::TextureUsage_UnorderedAccessView) > 0)
    {
        m_UnorderedAccessView = CreateUnorderedAccessView(
                device,
                *m_Texture,
                m_PixelFormat,
                TextureViewType::Tex3D,
                mostDetailedMip,
                numSlices,
                firstSlice);
    }

    return true;
}

void DX11Texture3D::Destroy()
{
    SHIP_ASSERT_MSG(m_Texture != nullptr, "Can't call Destroy on invalid texture2DArray 0x%p", this);

    if (m_ShaderResourceView != nullptr)
    {
        m_ShaderResourceView->Release();
        m_ShaderResourceView = nullptr;
    }

    if (m_UnorderedAccessView != nullptr)
    {
        m_UnorderedAccessView->Release();
        m_UnorderedAccessView = nullptr;
    }

    m_Texture->Release();
    m_Texture = nullptr;
}

DX11TextureCube::DX11TextureCube()
    : m_Texture(nullptr)
{

}

shipBool DX11TextureCube::Create(
        ID3D11Device& device,
        shipUint32 width,
        shipUint32 height,
        GfxFormat pixelFormat,
        shipBool dynamic,
        void* initialData,
        shipBool generateMips,
        TextureUsage textureUsage)
{
    m_Width = width;
    m_Height = height;
    m_Depth = 1;
    m_NumSlices = 6;
    m_PixelFormat = pixelFormat;
    m_TextureUsage = textureUsage;

    m_Texture = CreateTexture2D(
            device,
            width,
            height,
            m_NumSlices,
            pixelFormat,
            dynamic,
            generateMips,
            m_TextureUsage,
            initialData);

    if (m_Texture == nullptr)
    {
        SHIP_LOG_ERROR("DX11TextureCube::Create() --> Couldn't create TextureCube.");
        return false;
    }

    constexpr shipUint32 mostDetailedMip = 0;
    constexpr shipUint32 firstSlice = 0;

    if ((m_TextureUsage & TextureUsage::TextureUsage_ShaderResourceView) > 0)
    {
        constexpr shipUint32 numMipMaps = 1;
        m_ShaderResourceView = CreateShaderResourceView(
                device,
                *m_Texture,
                m_PixelFormat,
                TextureViewType::TexCube,
                textureUsage,
                numMipMaps,
                mostDetailedMip,
                m_NumSlices,
                firstSlice);
    }

    if ((m_TextureUsage & TextureUsage::TextureUsage_UnorderedAccessView) > 0)
    {
        m_UnorderedAccessView = CreateUnorderedAccessView(
                device,
                *m_Texture,
                m_PixelFormat,
                TextureViewType::Tex2DArray,
                mostDetailedMip,
                m_NumSlices,
                firstSlice);
    }

    return true;
}

void DX11TextureCube::Destroy()
{
    SHIP_ASSERT_MSG(m_Texture != nullptr, "Can't call Destroy on invalid TextureCube 0x%p", this);

    if (m_ShaderResourceView != nullptr)
    {
        m_ShaderResourceView->Release();
        m_ShaderResourceView = nullptr;
    }

    if (m_UnorderedAccessView != nullptr)
    {
        m_UnorderedAccessView->Release();
        m_UnorderedAccessView = nullptr;
    }

    m_Texture->Release();
    m_Texture = nullptr;
}

}