#pragma once

#include <graphics/rendergraph.h>

#include <graphics/wrapper/wrapper_common.h>

namespace Shipyard
{
    class RenderContext;

    class SHIPYARD_GRAPHICS_API Renderer
    {
    public:
        using RenderGraphSchedulerPtrFunction = void (*)(RenderGraph& renderGraph);

    public:
        void Initialize(GFXRenderDevice& gfxRenderDevice);

        void PrepareRenderGraph();
        void ExecuteRenderGraph(RenderContext& renderContext);

        void SetCurrentRenderGraph(RenderGraph* pRenderGraph, RenderGraphSchedulerPtrFunction renderGraphSchedulerPtrFunction);
        void ResetToDefaultRenderGraph();

    private:
        RenderGraph m_DefaultRenderGraph;
        RenderGraph* m_CurrentRenderGraph;
        RenderGraphSchedulerPtrFunction m_CurrentRenderGraphSchedulerPtrFunction;
    };
}