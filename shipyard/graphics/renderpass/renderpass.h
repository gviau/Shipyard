#pragma once

#include <graphics/rendergraph.h>

namespace Shipyard
{
    class RenderContext;

    class SHIPYARD_API RenderPass
    {
    public:
        virtual void Execute(RenderContext& renderContext) = 0;
    };

#define DEFINE_RENDER_PASS_HANDLE_TYPE(RenderPassHandleType) RenderPassHandleType

#define DEFINE_RENDER_PASS_HANDLE(RenderPassType) \
    struct SHIPYARD_API DEFINE_RENDER_PASS_HANDLE_TYPE(##RenderPassType) ##Handle : public RenderPassHandle \
    { \
        void CreateRenderPassInstance(GFXRenderDevice& gfxRenderDevice) override \
        { \
            RenderPassInstance = SHIP_NEW(RenderPassType, 1); \
        } \
        void DestroyRenderPassInstance() override \
        { \
            SHIP_DELETE(RenderPassInstance); \
        } \
        void AddRenderPassToRenderGraph(RenderGraph& renderGraph) override \
        { \
            SHIP_ASSERT_MSG(RenderPassInstance != nullptr, "RenderPassHandle of type %s was never registered to a RenderGraph! It can't be added to a RenderGraph before being registered.", #RenderPassType); \
            renderGraph.AddRenderPass(RenderPassInstance); \
        } \
        RenderPassType* RenderPassInstance = nullptr; \
    }
}