#pragma once

#include <graphics/wrapper/wrapper_common.h>

#include <graphics/shader/shaderhandler.h>

namespace Shipyard
{
    enum class RenderCommandType : uint8_t
    {
        ClearFullRenderTarget,
        ClearSingleRenderTarget,
        ClearDepthStencilRenderTarget,
        Draw,
        DrawIndexed,
        MapBuffer,
    };

    struct BaseRenderCommand
    {
        RenderCommandType renderCommandType;
    };

    struct ClearFullRenderTargetCommand : BaseRenderCommand
    {
        GFXRenderTargetHandle gfxRenderTargetHandle = { InvalidGfxHandle };
        float red = 0.0f;
        float green = 0.0f;
        float blue = 0.0f;
        float alpha = 0.0f;
    };

    struct ClearSingleRenderTargetCommand : BaseRenderCommand
    {
        GFXRenderTargetHandle gfxRenderTargetHandle = { InvalidGfxHandle };
        uint32_t renderTargetIndex = 0;
        float red = 0.0f;
        float green = 0.0f;
        float blue = 0.0f;
        float alpha = 0.0f;
    };

    struct ClearDepthStencilRenderTargetCommand : BaseRenderCommand
    {
        GFXDepthStencilRenderTargetHandle gfxDepthStencilRenderTargetHandle = { InvalidGfxHandle };
        DepthStencilClearFlag depthStencilClearFlag = DepthStencilClearFlag::Depth;
        float depthValue = 1.0f;
        uint8_t stencilValue = 0;
    };

    struct DrawCommand : BaseRenderCommand
    {
        GfxViewport gfxViewport;
        ShaderHandler* pShaderHandler = nullptr;

        GFXRenderTargetHandle gfxRenderTargetHandle = { InvalidGfxHandle };
        GFXDepthStencilRenderTargetHandle gfxDepthStencilRenderTargetHandle = { InvalidGfxHandle };
        GFXRootSignatureHandle gfxRootSignatureHandle = { InvalidGfxHandle };
        GFXDescriptorSetHandle gfxDescriptorSetHandle = { InvalidGfxHandle };
        PrimitiveTopology primitiveTopologyToUse = PrimitiveTopology::TriangleList;
        RenderStateBlockStateOverride* pRenderStateBlockStateOverride = nullptr;
        GFXVertexBufferHandle* pGfxVertexBufferHandles = nullptr;
        uint32_t startSlot = 0;
        uint32_t numVertexBuffers = 0;
        uint32_t startVertexLocation = 0;
        uint32_t* pVertexBufferOffsets = nullptr;
    };

    struct DrawIndexedCommand : BaseRenderCommand
    {
        GfxViewport gfxViewport;
        ShaderHandler* pShaderHandler = nullptr;

        GFXRenderTargetHandle gfxRenderTargetHandle = { InvalidGfxHandle };
        GFXDepthStencilRenderTargetHandle gfxDepthStencilRenderTargetHandle = { InvalidGfxHandle };
        GFXRootSignatureHandle gfxRootSignatureHandle = { InvalidGfxHandle };
        GFXDescriptorSetHandle gfxDescriptorSetHandle = { InvalidGfxHandle };
        PrimitiveTopology primitiveTopologyToUse = PrimitiveTopology::TriangleList;
        RenderStateBlockStateOverride* pRenderStateBlockStateOverride = nullptr;
        GFXVertexBufferHandle* pGfxVertexBufferHandles = nullptr;
        GFXIndexBufferHandle gfxIndexBufferHandle = { InvalidGfxHandle };
        uint32_t startSlot = 0;
        uint32_t numVertexBuffers = 0;
        uint32_t startVertexLocation = 0;
        uint32_t* pVertexBufferOffsets = nullptr;
        uint32_t startIndexLocation = 0;
        uint32_t indexBufferOffset = 0;
    };

    enum class MapBufferType : uint8_t
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
        } bufferHandle;

        MapBufferType mapBufferType;
        MapFlag mapFlag;

        // Depending on MapFlag, this pointer will either be for writing at the next ExecuteCommandLists or for immediate reading.
        // This pointer is not owned by the user, but by the command list.
        void* pBuffer;
    };
}