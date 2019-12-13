#pragma once

#include <graphics/graphicstypes.h>

#include <graphics/wrapper/wrapper_common.h>

namespace Shipyard
{
    namespace DefaultTextures
    {
        extern SHIPYARD_GRAPHICS_API GFXTexture2DHandle WhiteTexture;
        extern SHIPYARD_GRAPHICS_API GFXTexture2DHandle BlackTexture;

        void CreateDefaultTextures(GFXRenderDevice& gfxRenderDevice);
        void DestroyDefaultTextures(GFXRenderDevice& gfxRenderDevice);
    }
}