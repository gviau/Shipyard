#include <tools/textureimporter.h>

#include <system/array.h>
#include <system/systemdebug.h>

#include <FreeImage/FreeImage.h>

namespace Shipyard
{;

namespace TextureImporter
{;

GfxFormat ConvertFreeImageImageFormatToGfxFormat(FREE_IMAGE_TYPE imageType, unsigned int bytesPerPixel, unsigned int bpp)
{
    switch (bytesPerPixel)
    {
    case 1: return GfxFormat::R8_UNORM;
    case 3: return GfxFormat::R8G8B8A8_UNORM; // Data is padded with an alpha value of 255.
    case 4: return GfxFormat::R8G8B8A8_UNORM;

    default:
        SHIP_ASSERT(!"Unsupported FreeImage format");
        break;
    }

    return GfxFormat::Unknown;
}

ErrorCode LoadTextureFromFile(const shipChar* filename, ImportedTexture* importedTexture)
{
    SHIP_ASSERT(importedTexture != nullptr);

    FREE_IMAGE_FORMAT format = FIF_UNKNOWN;

    format = FreeImage_GetFileType(filename);
    if (format == FIF_UNKNOWN)
    {
        format = FreeImage_GetFIFFromFilename(filename);
    }

    if ((format == FIF_UNKNOWN) || !FreeImage_FIFSupportsReading(format))
    {
        return ErrorCode::UnsupportedImageFormat;
    }

    FIBITMAP* dib = FreeImage_Load(format, filename);
    if (dib == nullptr)
    {
        return ErrorCode::FileNotFound;
    }

    unsigned int width = FreeImage_GetWidth(dib);
    unsigned int height = FreeImage_GetHeight(dib);
    unsigned int bpp = FreeImage_GetBPP(dib);
    unsigned int pitch = FreeImage_GetPitch(dib);

    FREE_IMAGE_TYPE imageType = FreeImage_GetImageType(dib);

    importedTexture->Width = static_cast<shipUint32>(width);
    importedTexture->Height = static_cast<shipUint32>(height);
    importedTexture->Depth = 1;

    unsigned int bytesPerPixel = bpp / 8;

    unsigned int size = height * pitch;

    BigArray<BYTE> textureDataFromFileArray;
    textureDataFromFileArray.Resize(shipUint32(size));

    BYTE* textureDataFromFile = &textureDataFromFileArray[0];

    unsigned int pad = (bytesPerPixel == 3) ? 1 : 0;
    size += (pad * width * height);

    unsigned int realPad = pitch - (width * bytesPerPixel);

    importedTexture->TextureData.Resize(shipUint32(size));
    shipUint8* textureData = &importedTexture->TextureData[0];

    constexpr BOOL topDown = FALSE;
    FreeImage_ConvertToRawBits(textureDataFromFile, dib, pitch, bpp, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, topDown);

    // We store the texture data in rgba, where r is LSB and a is MSB
    size_t idx = 0;
    size_t textureIdx = 0;

    for (size_t y = 0; y < height; y++)
    {
        for (size_t x = 0; x < width; x++)
        {
            if (bytesPerPixel == 1)
            {
                textureData[idx] = textureDataFromFile[textureIdx];
            }
            else if (bytesPerPixel == 4)
            {
                textureData[idx] = textureDataFromFile[textureIdx + 2];
                textureData[idx + 1] = textureDataFromFile[textureIdx + 1];
                textureData[idx + 2] = textureDataFromFile[textureIdx];
                textureData[idx + 3] = textureDataFromFile[textureIdx + 3];
            }
            else if (bytesPerPixel == 3)
            {
                textureData[idx] = textureDataFromFile[textureIdx + 2];
                textureData[idx + 1] = textureDataFromFile[textureIdx + 1];
                textureData[idx + 2] = textureDataFromFile[textureIdx];
                textureData[idx + 3] = 255;
            }

            idx += bytesPerPixel + pad;
            textureIdx += bytesPerPixel;
        }

        textureIdx += realPad;
    }

    FreeImage_Unload(dib);

    importedTexture->PixelFormat = ConvertFreeImageImageFormatToGfxFormat(imageType, bytesPerPixel, bpp);

    return ErrorCode::None;
}

ErrorCode CreateGfxTextureFromFile(
        const shipChar* filename,
        GFXRenderDevice& gfxRenderDevice,
        GfxTextureCreationFlags gfxTextureCreationFlags,
        GFXTexture2DHandle* gfxTexture2DHandle)
{
    SHIP_ASSERT(gfxTexture2DHandle != nullptr);

    ImportedTexture importedTexture;
    ErrorCode errorCode = LoadTextureFromFile(filename, &importedTexture);

    if (errorCode != ErrorCode::None)
    {
        return errorCode;
    }

    constexpr shipBool dynamic = false;
    *gfxTexture2DHandle = gfxRenderDevice.CreateTexture2D(
            importedTexture.Width,
            importedTexture.Height,
            importedTexture.PixelFormat,
            dynamic,
            &importedTexture.TextureData[0],
            (gfxTextureCreationFlags == GfxTextureCreationFlags::GenerateMips));

    return ErrorCode::None;
}

}

}