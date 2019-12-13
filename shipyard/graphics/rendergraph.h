#pragma once

#include <graphics/renderpass/renderpasshandle.h>

#include <system/array.h>

namespace Shipyard
{
    class RenderContext;
    class RenderPass;

    class SHIPYARD_GRAPHICS_API RenderGraph
    {
        friend class RenderGraphRenderPassRegisterScope;

    public:
        class RenderGraphRenderPassRegisterScope
        {
        public:
            RenderGraphRenderPassRegisterScope(RenderGraph& renderGraph, GFXRenderDevice& gfxRenderDevice);
            ~RenderGraphRenderPassRegisterScope();

            Array<RenderPassHandle*> RenderPassHandlesToRegister;

        private:
            RenderGraph& m_RenderGraph;
            GFXRenderDevice& m_GfxRenderDevice;
        };

    public:
        // This method is called once at the end of the application, to free resources held by render passes.
        void UnregisterAllRenderPassHandles();

        void ClearRenderGraph();
        void AddRenderPassHandle(RenderPassHandle& renderPassHandle);
        void AddRenderPass(RenderPass* renderPass);

        // Returns true if the render graph could be properly built and can be executed next.
        shipBool BuildRenderGraph();
        void ExecuteRenderGraph(RenderContext& renderContext);

    private:
        Array<RenderPassHandle*> m_RegisteredRenderPassHandles;
        Array<RenderPass*> m_OrderedRenderPassesToExecute;
    };
}