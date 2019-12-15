#include <graphics/graphicsprecomp.h>

#include <graphics/renderer.h>

#include <graphics/renderpass/imguirenderpass.h>

#include <graphics/rendercontext.h>

namespace Shipyard
{;

static ImGuiRenderPassHandle imguiRenderPassHandle;

void DefaultRenderGraphScheduler(RenderGraph& renderGraph)
{
    renderGraph.AddRenderPassHandleForScheduling(imguiRenderPassHandle);
}

void Renderer::Initialize(GFXRenderDevice& gfxRenderDevice)
{
    ResetToDefaultRenderGraph();

    RenderGraph::RenderGraphRenderPassRegisterScope renderPassRegisterScope(m_DefaultRenderGraph, gfxRenderDevice);
    renderPassRegisterScope.RenderPassHandlesToRegister.Add(&imguiRenderPassHandle);
}

void Renderer::PrepareRenderGraph()
{
    m_CurrentRenderGraph->ClearRenderGraph();

    m_CurrentRenderGraphSchedulerPtrFunction(*m_CurrentRenderGraph);
}

void Renderer::ExecuteRenderGraph(RenderContext& renderContext)
{
    if (m_CurrentRenderGraph->BuildRenderGraph())
    {
        m_CurrentRenderGraph->ExecuteRenderGraph(renderContext);
    }
}

void Renderer::SetCurrentRenderGraph(RenderGraph* pRenderGraph, RenderGraphSchedulerPtrFunction renderGraphSchedulerPtrFunction)
{
    if (pRenderGraph == nullptr)
    {
        if (m_CurrentRenderGraph != &m_DefaultRenderGraph)
        {
            ResetToDefaultRenderGraph();
        }
    }
    else
    {
        m_CurrentRenderGraph = pRenderGraph;
        m_CurrentRenderGraphSchedulerPtrFunction = renderGraphSchedulerPtrFunction;
    }
}

void Renderer::ResetToDefaultRenderGraph()
{
    m_CurrentRenderGraph = &m_DefaultRenderGraph;
    m_CurrentRenderGraphSchedulerPtrFunction = &DefaultRenderGraphScheduler;
}

}