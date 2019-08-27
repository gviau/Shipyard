#pragma once

#include <graphics/wrapper/wrapper_common.h>

#include <system/platform.h>

namespace Shipyard
{
    shipBool SHIPYARD_API InitializeImGui(void* windowHandle, GFXRenderDevice& gfxRenderDevice);
    void SHIPYARD_API StartNewImGuiFrame();
}