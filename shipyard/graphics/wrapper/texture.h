#pragma once

#include <graphics/wrapper/gfxresource.h>

namespace Shipyard
{
    shipUint32 GetBytesPerPixel(GfxFormat pixelFormat);
    shipUint32 GetRowPitch(shipUint32 width, GfxFormat pixelFormat);
    shipUint32 Get2DSlicePitch(shipUint32 width, shipUint32 height, GfxFormat pixelFormat);

    class SHIPYARD_GRAPHICS_API BaseTexture : public GfxResource
    {
    public:
        BaseTexture();

        shipUint32 GetWidth() const { return m_Width; }
        shipUint32 GetHeight() const { return m_Height; }
        shipUint32 GetDepth() const { return m_Depth; }
        shipUint32 GetNumSlices() const { return m_NumSlices; }
        GfxFormat GetPixelFormat() const { return m_PixelFormat; }
        TextureUsage GetTextureUsage() const { return m_TextureUsage; }

    protected:
        shipUint32 m_Width;
        shipUint32 m_Height;
        shipUint32 m_Depth;
        shipUint32 m_NumSlices;
        GfxFormat m_PixelFormat;
        TextureUsage m_TextureUsage;
    };

    class SHIPYARD_GRAPHICS_API Texture2D
    {

    };

    class SHIPYARD_GRAPHICS_API Texture2DArray
    {

    };

    class SHIPYARD_GRAPHICS_API Texture3D
    {

    };

    class SHIPYARD_GRAPHICS_API TextureCube
    {

    };
}