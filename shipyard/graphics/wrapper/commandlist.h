#pragma once

#include <graphics/wrapper/wrapper_common.h>

#include <graphics/graphicstypes.h>

#include <graphics/wrapper/rendercommands.h>

namespace Shipyard
{
    class PipelineStateObject;

    class SHIPYARD_GRAPHICS_API BaseRenderCommandListType
    {
    public:
        CommandQueueType GetCommandListType() const;

    protected:
        CommandQueueType m_CommandListType;
    };

    class SHIPYARD_GRAPHICS_API BaseRenderCommandList
    {
    public:
#ifdef DEBUG_WRAPPER_INTERFACE_COMPILATION
        virtual shipBool Create() = 0;
        virtual void Destroy() = 0;

        virtual void Close() = 0;
        virtual void Reset(GFXCommandListAllocator& gfxCommandListAllocator, PipelineStateObject* pPipelineStateObject) = 0;
#endif // #ifdef DEBUG_WRAPPER_INTERFACE_COMPILATION
    };

    class SHIPYARD_GRAPHICS_API RenderCommandList : public BaseRenderCommandListType
    {
    public:
        RenderCommandList(GFXRenderDevice& gfxRenderDevice);

    protected:
        GFXRenderDevice& m_RenderDevice;
    };

    class SHIPYARD_GRAPHICS_API CopyRenderCommandList : public RenderCommandList
    {
    public:
        CopyRenderCommandList(GFXRenderDevice& gfxRenderDevice);
    };

    class SHIPYARD_GRAPHICS_API ComputeRenderCommandList : public RenderCommandList
    {
    public:
        ComputeRenderCommandList(GFXRenderDevice& gfxRenderDevice);
    };

    class SHIPYARD_GRAPHICS_API DirectRenderCommandList : public RenderCommandList
    {
    public:
        DirectRenderCommandList(GFXRenderDevice& gfxRenderDevice);

#ifdef DEBUG_WRAPPER_INTERFACE_COMPILATION
        virtual ClearFullRenderTargetCommand* ClearFullRenderTarget() = 0;
        virtual ClearSingleRenderTargetCommand* ClearSingleRenderTarget() = 0;
        virtual ClearDepthStencilRenderTargetCommand* ClearDepthStencilRenderTarget() = 0;

        virtual DrawCommand* Draw() = 0;
        virtual DrawSeveralVertexBuffersCommand* DrawSeveralVertexBuffers() = 0;
        virtual DrawIndexedCommand* DrawIndexed() = 0;
        virtual DrawIndexedSeveralVertexBuffersCommand* DrawIndexedSeveralVertexBuffers() = 0;

        virtual DispatchCommand* Dispatch() = 0;

        virtual void* MapVertexBuffer(GFXVertexBufferHandle gfxVertexBufferhandle, MapFlag mapFlag, size_t bufferOffset) = 0;
        virtual void* MapIndexBuffer(GFXIndexBufferHandle gfxIndexBufferHandle, MapFlag mapFlag, size_t bufferOffset) = 0;
        virtual void* MapConstantBuffer(GFXConstantBufferHandle gfxConstantBufferHandle, MapFlag mapFlag, size_t bufferOffset) = 0;
        virtual void* MapByteBuffer(GFXByteBufferHandle gfxByteBufferHandle, MapFlag mapFlag, size_t bufferOffset) = 0;
#endif // #ifdef DEBUG_WRAPPER_INTERFACE_COMPILATION
    };
}