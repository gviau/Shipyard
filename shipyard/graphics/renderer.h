#pragma once

#include <graphics/rendergraph.h>

#include <graphics/wrapper/wrapper_common.h>

namespace Shipyard
{
    class RenderContext;

    class SHIPYARD_GRAPHICS_API Renderer
    {
    public:
        void Initialize(GFXRenderDevice& gfxRenderDevice);

        void PrepareRenderGraph();
        void ExecuteRenderGraph(RenderContext& renderContext);

    private:
        RenderGraph m_RenderGraph;
    };
}