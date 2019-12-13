#include <graphics/graphicsprecomp.h>

#include <graphics/renderer.h>

#include <graphics/renderpass/imguirenderpass.h>

#include <graphics/rendercontext.h>

namespace Shipyard
{;

static ImGuiRenderPassHandle imguiRenderPassHandle;

void Renderer::Initialize(GFXRenderDevice& gfxRenderDevice)
{
    RenderGraph::RenderGraphRenderPassRegisterScope renderPassRegisterScope(m_RenderGraph, gfxRenderDevice);
    renderPassRegisterScope.RenderPassHandlesToRegister.Add(&imguiRenderPassHandle);
}

void Renderer::PrepareRenderGraph()
{
    m_RenderGraph.ClearRenderGraph();

    m_RenderGraph.AddRenderPassHandle(imguiRenderPassHandle);
}

void Renderer::ExecuteRenderGraph(RenderContext& renderContext)
{
    if (m_RenderGraph.BuildRenderGraph())
    {
        m_RenderGraph.ExecuteRenderGraph(renderContext);
    }
}

}