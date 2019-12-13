#include <graphics/graphicsprecomp.h>

#include <graphics/wrapper/commandqueue.h>

namespace Shipyard
{;
CommandQueue::CommandQueue(GFXRenderDevice& gfxRenderDevice, CommandQueueType commandQueueType)
    : m_RenderDevice(gfxRenderDevice)
    , m_CommandQueueType(commandQueueType)
{

}
}