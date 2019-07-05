#pragma once

#include <graphics/wrapper/wrapper_common.h>

namespace Shipyard
{
    class SHIPYARD_API CommandListAllocator
    {
    public:
        CommandListAllocator(GFXRenderDevice& gfxRenderDevice);

#ifdef DEBUG_WRAPPER_INTERFACE_COMPILATION
        virtual shipBool Create() = 0;
        virtual void Destroy() = 0;
#endif // #ifdef DEBUG_WRAPPER_INTERFACE_COMPILATION
    };
}