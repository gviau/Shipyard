#pragma once

#include <graphics/wrapper/wrapper_common.h>

namespace Shipyard
{
    class RenderCommandList;

    class SHIPYARD_API CommandQueue
    {
    public:
        CommandQueue(GFXRenderDevice& gfxRenderDevice, CommandQueueType commandQueueType);

#ifdef DEBUG_WRAPPER_INTERFACE_COMPILATION
        virtual shipBool Create() = 0;
        virtual void Destroy() = 0;

        // This method assumes that the given command lists contain at least one recorded command
        virtual void ExecuteCommandLists(GFXRenderCommandList** ppRenderCommandLists, shipUint32 numRenderCommandLists) = 0;
#endif // #ifdef DEBUG_WRAPPER_INTERFACE_COMPILATION

    protected:
        GFXRenderDevice& m_RenderDevice;
        CommandQueueType m_CommandQueueType;
    };
}