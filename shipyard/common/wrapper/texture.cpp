#include <common/wrapper/texture.h>

#include <cassert>

namespace Shipyard
{;
BaseTexture::BaseTexture(uint32_t width, uint32_t height, GfxFormat pixelFormat)
    : GfxResource(GfxResourceType::Texture)
    , m_Width(width)
    , m_Height(height)
    , m_PixelFormat(pixelFormat)
{

}

BaseTexture::~BaseTexture()
{
}

uint32_t GetBytesPerPixel(GfxFormat pixelFormat)
{
    uint32_t bytesPerPixel = 0;

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

    default:
        assert(false);
    }

    return bytesPerPixel;
}

uint32_t GetRowPitch(uint32_t width, GfxFormat pixelFormat)
{
    uint32_t bytesPerPixel = GetBytesPerPixel(pixelFormat);

    return bytesPerPixel * width;
}

uint32_t Get2DSlicePitch(uint32_t width, uint32_t height, GfxFormat pixelFormat)
{
    uint32_t rowPitch = GetRowPitch(width, pixelFormat);
    return height * rowPitch;
}

}