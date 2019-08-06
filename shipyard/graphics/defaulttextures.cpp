#include <graphics/defaulttextures.h>

#include <graphics/wrapper/wrapper.h>

namespace Shipyard
{;

namespace DefaultTextures
{;

GFXTexture2DHandle WhiteTexture;
GFXTexture2DHandle BlackTexture;

void CreateDefaultTextures(GFXRenderDevice& gfxRenderDevice)
{
    SHIP_ASSERT(!WhiteTexture.IsValid());
    SHIP_ASSERT(!BlackTexture.IsValid());

    shipUint8 whiteTextureData[] =
    {
        255, 255, 255, 255
    };

    WhiteTexture = gfxRenderDevice.CreateTexture2D(1, 1, GfxFormat::R8G8B8A8_UNORM, false, whiteTextureData, false);

    shipUint8 blackTextureData[] =
    {
        0, 0, 0, 0
    };

    BlackTexture = gfxRenderDevice.CreateTexture2D(1, 1, GfxFormat::R8G8B8A8_UNORM, false, blackTextureData, false);
}

void DestroyDefaultTextures(GFXRenderDevice& gfxRenderDevice)
{
    SHIP_ASSERT(WhiteTexture.IsValid());
    SHIP_ASSERT(BlackTexture.IsValid());

    gfxRenderDevice.DestroyTexture2D(WhiteTexture);
    gfxRenderDevice.DestroyTexture2D(BlackTexture);
}

}

}