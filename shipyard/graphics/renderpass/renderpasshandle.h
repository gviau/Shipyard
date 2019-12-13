#pragma once

#include <system/platform.h>

#include <graphics/wrapper/wrapper_common.h>

#include <system/array.h>

namespace Shipyard
{
    class RenderGraph;

    struct SHIPYARD_GRAPHICS_API RenderPassHandle
    {
        virtual void CreateRenderPassInstance(GFXRenderDevice& gfxRenderDevice) {}
        virtual void DestroyRenderPassInstance() {}
        virtual void AddRenderPassToRenderGraph(RenderGraph& renderGraph) {}

#ifdef SHIP_ENABLE_ASSERTS
        shipBool IsRegistered = false;
#endif // #ifdef SHIP_ENABLE_ASSERTS
    };
}