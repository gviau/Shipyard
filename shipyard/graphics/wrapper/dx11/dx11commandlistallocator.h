#pragma once

#include <graphics/wrapper/commandlistallocator.h>

namespace Shipyard
{
    class SHIPYARD_API DX11CommandListAllocator : public CommandListAllocator
    {
    public:
        DX11CommandListAllocator(GFXRenderDevice& gfxRenderDevice);

        bool Create();
        void Destroy();
    };
}