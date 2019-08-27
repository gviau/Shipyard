#include <graphics/rendercontext.h>

namespace Shipyard
{;

void RenderContext::SetRenderDevice(GFXRenderDevice* gfxRenderDevice)
{
    m_RenderDevice = gfxRenderDevice;
}

GFXRenderDevice* RenderContext::GetRenderDevice()
{
    return m_RenderDevice;
}

void RenderContext::SetRenderCommandList(GFXDirectRenderCommandList* gfxDirectRenderCommandList)
{
    m_DirectRenderCommandList = gfxDirectRenderCommandList;
}

GFXDirectRenderCommandList* RenderContext::GetRenderCommandList()
{
    return m_DirectRenderCommandList;
}

void RenderContext::SetViewSurface(GFXViewSurface* gfxViewSurface)
{
    m_ViewSurface = gfxViewSurface;
}

GFXViewSurface* RenderContext::GetViewSurface() const
{
    return m_ViewSurface;
}

}