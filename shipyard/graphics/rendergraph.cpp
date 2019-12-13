#include <graphics/graphicsprecomp.h>

#include <graphics/rendergraph.h>

#include <graphics/renderpass/renderpass.h>

#include <graphics/rendercontext.h>

#include <system/systemdebug.h>

namespace Shipyard
{;

RenderGraph::RenderGraphRenderPassRegisterScope::RenderGraphRenderPassRegisterScope(RenderGraph& renderGraph, GFXRenderDevice& gfxRenderDevice)
    : m_RenderGraph(renderGraph)
    , m_GfxRenderDevice(gfxRenderDevice)
{

}

RenderGraph::RenderGraphRenderPassRegisterScope::~RenderGraphRenderPassRegisterScope()
{
    m_RenderGraph.m_RegisteredRenderPassHandles = RenderPassHandlesToRegister;

    for (RenderPassHandle* renderPassHandle : m_RenderGraph.m_RegisteredRenderPassHandles)
    {
        SHIP_ASSERT_MSG(!renderPassHandle->IsRegistered, "RenderPassHandles are meant to be registered only once across RenderGraphs.");

        renderPassHandle->CreateRenderPassInstance(m_GfxRenderDevice);

#ifdef SHIP_ENABLE_ASSERTS
        renderPassHandle->IsRegistered = true;
#endif // #ifdef SHIP_ENABLE_ASSERTS
    }

    m_RenderGraph.m_OrderedRenderPassesToExecute.Reserve(m_RenderGraph.m_RegisteredRenderPassHandles.Size());
}

void RenderGraph::UnregisterAllRenderPassHandles()
{
    for (RenderPassHandle* renderPassHandle : m_RegisteredRenderPassHandles)
    {
        renderPassHandle->DestroyRenderPassInstance();
    }
}

void RenderGraph::ClearRenderGraph()
{
    m_OrderedRenderPassesToExecute.Resize(0);
}

void RenderGraph::AddRenderPassHandle(RenderPassHandle& renderPassHandle)
{
    renderPassHandle.AddRenderPassToRenderGraph(*this);
}

void RenderGraph::AddRenderPass(RenderPass* renderPass)
{
#ifdef SHIP_ENABLE_ASSERTS
    for (shipUint32 i = 0; i < m_OrderedRenderPassesToExecute.Size(); i++)
    {
        SHIP_ASSERT_MSG(m_OrderedRenderPassesToExecute[i] != renderPass, "RenderPass already added to RenderGraph! A separate RenderPassHandle, even if it is of the same type, should be used to add a second RenderPass of the same type.");
    }
#endif // #ifdef SHIP_ENABLE_ASSERTS

    m_OrderedRenderPassesToExecute.Add(renderPass);
}

shipBool RenderGraph::BuildRenderGraph()
{
    return true;
}

void RenderGraph::ExecuteRenderGraph(RenderContext& renderContext)
{
    for (shipUint32 i = 0; i < m_OrderedRenderPassesToExecute.Size(); i++)
    {
        m_OrderedRenderPassesToExecute[i]->Execute(renderContext);
    }
}

}