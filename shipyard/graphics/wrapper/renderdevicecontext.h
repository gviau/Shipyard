#pragma once

#include <graphics/wrapper/wrapper_common.h>

namespace Shipyard
{
    class SHIPYARD_API RenderDeviceContext
    {
    public:
        RenderDeviceContext(GFXRenderDevice& renderDevice);
        virtual ~RenderDeviceContext();

#ifdef DEBUG_WRAPPER_INTERFACE_COMPILATION
        virtual void ClearFullRenderTarget(GFXRenderTargetHandle gfxRenderTargetHandle, float red, float green, float blue, float alpha) = 0;
        virtual void ClearSingleRenderTarget(GFXRenderTargetHandle gfxRenderTargetHandle, uint32_t renderTargetIndex, float red, float green, float blue, float alpha) = 0;
        virtual void ClearDepthStencilRenderTarget(GFXDepthStencilRenderTargetHandle gfxDepthStencilRenderTargetHandle, DepthStencilClearFlag depthStencilClearFlag, float depthValue, uint8_t stencilValue) = 0;

        virtual void Draw(
                const DrawItem& drawItem,
                GFXVertexBufferHandle* gfxVertexBufferHandles,
                uint32_t startSlot,
                uint32_t numVertexBuffers,
                uint32_t startVertexLocation,
                uint32_t* vertexBufferOffsets) = 0;
        virtual void DrawIndexed(
                const DrawItem& drawItem,
                GFXVertexBufferHandle* gfxVertexBufferHandles,
                uint32_t startSlot,
                uint32_t numVertexBuffers,
                uint32_t* vertexBufferOffsets,
                GFXIndexBufferHandle gfxIndexBufferHandle,
                uint32_t startVertexLocation,
                uint32_t startIndexLocation,
                uint32_t indexBufferOffset) = 0;
#endif // #ifdef DEBUG_WRAPPER_INTERFACE_COMPILATION

    protected:
        GFXRenderDevice& m_RenderDevice;
    };
}