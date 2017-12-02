#pragma once

#include <common/wrapper/wrapper_common.h>

namespace Shipyard
{
    class RenderDeviceContext
    {
    public:
        RenderDeviceContext(const GFXRenderDevice& renderDevice);

#ifdef DEBUG_WRAPPER_INTERFACE_COMPILATION

#endif // #ifdef DEBUG_WRAPPER_INTERFACE_COMPILATION
    };
}