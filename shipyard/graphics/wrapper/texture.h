#pragma once

#include <graphics/wrapper/gfxresource.h>

namespace Shipyard
{
    uint32_t GetBytesPerPixel(GfxFormat pixelFormat);
    uint32_t GetRowPitch(uint32_t width, GfxFormat pixelFormat);
    uint32_t Get2DSlicePitch(uint32_t width, uint32_t height, GfxFormat pixelFormat);

    class SHIPYARD_API BaseTexture : public GfxResource
    {
    public:
        BaseTexture(uint32_t width, uint32_t height, GfxFormat pixelFormat, TextureUsage textureUsage = TextureUsage::TextureUsage_Default);
        virtual ~BaseTexture();

        uint32_t GetWidth() const { return m_Width; }
        uint32_t GetHeight() const { return m_Height; }
        GfxFormat GetPixelFormat() const { return m_PixelFormat; }
        TextureUsage GetTextureUsage() const { return m_TextureUsage; }

    protected:
        uint32_t m_Width;
        uint32_t m_Height;
        GfxFormat m_PixelFormat;
        TextureUsage m_TextureUsage;
    };

    class SHIPYARD_API Texture2D
    {
    public:
#ifdef DEBUG_WRAPPER_INTERFACE_COMPILATION
#endif // #ifdef DEBUG_WRAPPER_INTERFACE_COMPILATION
    };
}