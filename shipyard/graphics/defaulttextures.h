#pragma once

#include <graphics/graphicstypes.h>

#include <graphics/wrapper/wrapper_common.h>

namespace Shipyard
{
    namespace DefaultTextures
    {
        extern GFXTexture2DHandle WhiteTexture;
        extern GFXTexture2DHandle BlackTexture;

        void CreateDefaultTextures(GFXRenderDevice& gfxRenderDevice);
        void DestroyDefaultTextures(GFXRenderDevice& gfxRenderDevice);
    }
}