#pragma once

#include <graphics/wrapper/wrapper_common.h>

#include <graphics/shader/shaderkey.h>

#ifndef SHIP_OPTIMIZED
#define SHIP_RENDER_COMMANDS_DEBUG_INFO
#endif // #ifndef SHIP_OPTIMIZED

namespace Shipyard
{
    enum : shipUint32
    {
        UseIndexBufferSize = 0xffffffff
    };

    enum class RenderCommandType : shipUint8
    {
        ClearFullRenderTarget,
        ClearSingleRenderTarget,
        ClearDepthStencilRenderTarget,
        Draw,
        DrawSeveralVertexBuffers,
        DrawIndexed,
        DrawIndexedSeveralVertexBuffers,
        MapBuffer,
        Dispatch,
    };

    struct BaseRenderCommand
    {
        RenderCommandType renderCommandType;

#ifdef SHIP_RENDER_COMMANDS_DEBUG_INFO
        BaseRenderCommand* pPreviousRenderCommand = nullptr;
        BaseRenderCommand* pNextRenderCommand = nullptr;
#endif // SHIP_RENDER_COMMANDS_DEBUG_INFO
    };

    struct ClearFullRenderTargetCommand : BaseRenderCommand
    {
        GFXRenderTargetHandle gfxRenderTargetHandle = { InvalidGfxHandle };
        shipFloat red = 0.0f;
        shipFloat green = 0.0f;
        shipFloat blue = 0.0f;
        shipFloat alpha = 0.0f;
    };

    struct ClearSingleRenderTargetCommand : BaseRenderCommand
    {
        GFXRenderTargetHandle gfxRenderTargetHandle = { InvalidGfxHandle };
        shipUint32 renderTargetIndex = 0;
        shipFloat red = 0.0f;
        shipFloat green = 0.0f;
        shipFloat blue = 0.0f;
        shipFloat alpha = 0.0f;
    };

    struct ClearDepthStencilRenderTargetCommand : BaseRenderCommand
    {
        GFXDepthStencilRenderTargetHandle gfxDepthStencilRenderTargetHandle = { InvalidGfxHandle };
        DepthStencilClearFlag depthStencilClearFlag = DepthStencilClearFlag::Depth;
        shipFloat depthValue = 1.0f;
        shipUint8 stencilValue = 0;
    };

    struct DrawCommand : BaseRenderCommand
    {
        GfxViewport gfxViewport;
        GfxRect gfxScissorRect;
        
        GFXRenderTargetHandle gfxRenderTargetHandle = { InvalidGfxHandle };
        GFXDepthStencilRenderTargetHandle gfxDepthStencilRenderTargetHandle = { InvalidGfxHandle };

        GFXGraphicsPipelineStateObjectHandle gfxPipelineStateObjectHandle = { InvalidGfxHandle };
        GFXRootSignatureHandle gfxRootSignatureHandle = { InvalidGfxHandle };
        GFXDescriptorSetHandle gfxDescriptorSetHandle = { InvalidGfxHandle };

        GFXVertexBufferHandle gfxVertexBufferHandle = { InvalidGfxHandle };
        shipUint32 startVertexLocation = 0;
        shipUint32 vertexBufferOffset = 0;
    };

    struct DrawSeveralVertexBuffersCommand : BaseRenderCommand
    {
        GfxViewport gfxViewport;
        GfxRect gfxScissorRect;

        GFXRenderTargetHandle gfxRenderTargetHandle = { InvalidGfxHandle };
        GFXDepthStencilRenderTargetHandle gfxDepthStencilRenderTargetHandle = { InvalidGfxHandle };

        GFXGraphicsPipelineStateObjectHandle gfxPipelineStateObjectHandle = { InvalidGfxHandle };
        GFXRootSignatureHandle gfxRootSignatureHandle = { InvalidGfxHandle };
        GFXDescriptorSetHandle gfxDescriptorSetHandle = { InvalidGfxHandle };

        GFXVertexBufferHandle* pGfxVertexBufferHandles = nullptr;
        shipUint32 vertexBufferStartSlot = 0;
        shipUint32 numVertexBuffers = 0;
        shipUint32 startVertexLocation = 0;
        shipUint32* pVertexBufferOffsets = nullptr;
    };

    struct DrawIndexedCommand : BaseRenderCommand
    {
        GfxViewport gfxViewport;
        GfxRect gfxScissorRect;

        GFXRenderTargetHandle gfxRenderTargetHandle = { InvalidGfxHandle };
        GFXDepthStencilRenderTargetHandle gfxDepthStencilRenderTargetHandle = { InvalidGfxHandle };

        GFXGraphicsPipelineStateObjectHandle gfxPipelineStateObjectHandle = { InvalidGfxHandle };
        GFXRootSignatureHandle gfxRootSignatureHandle = { InvalidGfxHandle };
        GFXDescriptorSetHandle gfxDescriptorSetHandle = { InvalidGfxHandle };

        GFXVertexBufferHandle gfxVertexBufferHandle = { InvalidGfxHandle };
        GFXIndexBufferHandle gfxIndexBufferHandle = { InvalidGfxHandle };
        shipUint32 vertexBufferOffset = 0;
        shipUint32 indexCount = UseIndexBufferSize;
        shipUint32 startIndexLocation = 0;
        shipUint32 indexBufferOffset = 0;
        shipInt32 baseVertexLocation = 0;
    };

    struct DrawIndexedSeveralVertexBuffersCommand : BaseRenderCommand
    {
        GfxViewport gfxViewport;
        GfxRect gfxScissorRect;
        
        GFXRenderTargetHandle gfxRenderTargetHandle = { InvalidGfxHandle };
        GFXDepthStencilRenderTargetHandle gfxDepthStencilRenderTargetHandle = { InvalidGfxHandle };

        GFXGraphicsPipelineStateObjectHandle gfxPipelineStateObjectHandle = { InvalidGfxHandle };
        GFXRootSignatureHandle gfxRootSignatureHandle = { InvalidGfxHandle };
        GFXDescriptorSetHandle gfxDescriptorSetHandle = { InvalidGfxHandle };

        GFXVertexBufferHandle pGfxVertexBufferHandles[GfxConstants::GfxConstants_MaxVertexBuffers];
        GFXIndexBufferHandle gfxIndexBufferHandle = { InvalidGfxHandle };
        shipUint32 vertexBufferStartSlot = 0;
        shipUint32 numVertexBuffers = 0;
        shipUint32* pVertexBufferOffsets = nullptr;
        shipUint32 indexCount = UseIndexBufferSize;
        shipUint32 startIndexLocation = 0;
        shipUint32 indexBufferOffset = 0;
        shipInt32 baseVertexLocation = 0;
    };

    enum class MapBufferType : shipUint8
    {
        Vertex,
        Index,
        Constant,
        Structured,
        Data
    };

    struct MapBufferCommand : BaseRenderCommand
    {
        union BufferHandle_t
        {
            GFXVertexBufferHandle gfxVertexBufferHandle;
            GFXIndexBufferHandle gfxIndexBufferHandle;
            GFXConstantBufferHandle gfxConstantBufferHandle;
            GFXByteBufferHandle gfxByteBufferHandle;
        } bufferHandle = { InvalidGfxHandle };

        MapBufferType mapBufferType = MapBufferType::Constant;
        MapFlag mapFlag = MapFlag::Write_Discard;
        size_t bufferOffset = 0;

        // Depending on MapFlag, this pointer will either be for writing at the next ExecuteCommandLists or for immediate reading.
        // This pointer is not owned by the user, but by the command list.
        void* pBuffer = nullptr;
    };

    struct DispatchCommand : BaseRenderCommand
    {
        GFXComputePipelineStateObjectHandle gfxPipelineStateObjectHandle = { InvalidGfxHandle };
        GFXRootSignatureHandle gfxRootSignatureHandle = { InvalidGfxHandle };
        GFXDescriptorSetHandle gfxDescriptorSetHandle = { InvalidGfxHandle };

        shipUint32 threadGroupCountX = 1;
        shipUint32 threadGroupCountY = 1;
        shipUint32 threadGroupCountZ = 1;
    };
}