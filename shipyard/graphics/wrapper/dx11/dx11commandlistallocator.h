#pragma once

#include <graphics/wrapper/commandlistallocator.h>

namespace Shipyard
{
    class SHIPYARD_GRAPHICS_API DX11CommandListAllocator : public CommandListAllocator
    {
    public:
        DX11CommandListAllocator(GFXRenderDevice& gfxRenderDevice);

        shipBool Create();
        void Destroy();
    };
}