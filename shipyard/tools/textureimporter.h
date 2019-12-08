#pragma once

#include <graphics/graphicscommon.h>

namespace Shipyard
{
    namespace TextureImporter
    {
        struct ImportedTexture
        {
            shipUint32 Width;
            shipUint32 Height;
            shipUint32 Depth;

            GfxFormat PixelFormat;

            // To be interpreted according to the dimension and pixel format. The memory is laid out, row by rows, for each slice of the texture.
            BigArray<shipUint8> TextureData;
        };

        enum class ErrorCode : shipUint8
        {
            None,
            FileNotFound,
            UnsupportedImageFormat
        };

        SHIPYARD_API ErrorCode LoadTextureFromFile(const shipChar* filename, ImportedTexture* importedTexture);
    }
}