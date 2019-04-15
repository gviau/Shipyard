#include <graphics/wrapper/commandlist.h>

namespace Shipyard
{;

CommandQueueType BaseRenderCommandListType::GetCommandListType() const
{
    return m_CommandListType;
}

RenderCommandList::RenderCommandList(GFXRenderDevice& gfxRenderDevice)
    : m_RenderDevice(gfxRenderDevice)
{

}

CopyRenderCommandList::CopyRenderCommandList(GFXRenderDevice& gfxRenderDevice)
    : RenderCommandList(gfxRenderDevice)
{
    m_CommandListType = CommandQueueType::Copy;
}

ComputeRenderCommandList::ComputeRenderCommandList(GFXRenderDevice& gfxRenderDevice)
    : RenderCommandList(gfxRenderDevice)
{
    m_CommandListType = CommandQueueType::Compute;
}

DirectRenderCommandList::DirectRenderCommandList(GFXRenderDevice& gfxRenderDevice)
    : RenderCommandList(gfxRenderDevice)
{
    m_CommandListType = CommandQueueType::Direct;
}

}