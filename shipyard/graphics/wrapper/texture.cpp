#include <graphics/graphicsprecomp.h>

#include <graphics/wrapper/texture.h>

namespace Shipyard
{;
BaseTexture::BaseTexture()
    : GfxResource(GfxResourceType::Texture)
    , m_Width(0)
    , m_Height(0)
    , m_PixelFormat(GfxFormat::Unknown)
    , m_TextureUsage(TextureUsage::TextureUsage_Default)
{

}

shipUint32 GetBytesPerPixel(GfxFormat pixelFormat)
{
    shipUint32 bytesPerPixel = 0;

    switch (pixelFormat)
    {
    case GfxFormat::R8_SINT:
    case GfxFormat::R8_UINT:
    case GfxFormat::R8_UNORM:
    case GfxFormat::R8_SNORM:
        bytesPerPixel = 1;
        break;

    case GfxFormat::R8G8_SINT:
    case GfxFormat::R8G8_UINT:
    case GfxFormat::R8G8_UNORM:
    case GfxFormat::R8G8_SNORM:
        bytesPerPixel = 2;
        break;

    case GfxFormat::R8G8B8A8_SINT:
    case GfxFormat::R8G8B8A8_UINT:
    case GfxFormat::R8G8B8A8_SNORM:
    case GfxFormat::R8G8B8A8_UNORM:
    case GfxFormat::R8G8B8A8_UNORM_SRGB:
        bytesPerPixel = 4;
        break;

    case GfxFormat::R32_FLOAT:
    case GfxFormat::R32_SINT:
    case GfxFormat::R32_UINT:
    case GfxFormat::R16G16_FLOAT:
    case GfxFormat::R16G16_SINT:
    case GfxFormat::R16G16_UINT:
    case GfxFormat::R16G16_SNORM:
    case GfxFormat::R16G16_UNORM:
        bytesPerPixel = 4;
        break;

    case GfxFormat::R32G32_FLOAT:
    case GfxFormat::R32G32_UINT:
    case GfxFormat::R32G32_SINT:
    case GfxFormat::R16G16B16A16_FLOAT:
    case GfxFormat::R16G16B16A16_SINT:
    case GfxFormat::R16G16B16A16_UINT:
    case GfxFormat::R16G16B16A16_SNORM:
    case GfxFormat::R16G16B16A16_UNORM:
        bytesPerPixel = 8;
        break;

    case GfxFormat::R32G32B32A32_FLOAT:
    case GfxFormat::R32G32B32A32_SINT:
    case GfxFormat::R32G32B32A32_UINT:
        bytesPerPixel = 16;
        break;

    case GfxFormat::D16_UNORM:
        bytesPerPixel = 2;
        break;

    case GfxFormat::D24_UNORM_S8_UINT:
        bytesPerPixel = 4;
        break;

    case GfxFormat::D32_FLOAT:
        bytesPerPixel = 4;
        break;

    case GfxFormat::D32_FLOAT_S8X24_UINT:
        bytesPerPixel = 8;
        break;

    case GfxFormat::Unknown:
        bytesPerPixel = 0;
        break;
    default:
        SHIP_ASSERT(false);
    }

    return bytesPerPixel;
}

shipUint32 GetRowPitch(shipUint32 width, GfxFormat pixelFormat)
{
    shipUint32 bytesPerPixel = GetBytesPerPixel(pixelFormat);

    return bytesPerPixel * width;
}

shipUint32 Get2DSlicePitch(shipUint32 width, shipUint32 height, GfxFormat pixelFormat)
{
    shipUint32 rowPitch = GetRowPitch(width, pixelFormat);
    return height * rowPitch;
}

}