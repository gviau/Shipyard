#include <graphics/graphicsprecomp.h>

#include <graphics/utils/fullscreenhelper.h>

#include <graphics/shader/shaderfamilies.h>
#include <graphics/shader/shaderhandlermanager.h>

#include <system/systemdebug.h>

namespace Shipyard
{;

template SHIPYARD_GRAPHICS_API class GraphicsSingleton<FullscreenHelper>;

FullscreenHelper::~FullscreenHelper()
{
    SHIP_ASSERT_MSG(
            !m_FullscreenQuadVertexBufferHandle.IsValid() && !m_FullscreenQuadIndexBufferHandle.IsValid(),
            "FullscreenHelper::DestroyResources must be called before destroying the object!");
}

shipBool FullscreenHelper::CreateResources(GFXRenderDevice& gfxRenderDevice)
{
    Vertex_Pos_UV fullscreenQuadVertices[4] =
    {
        { {-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f} },
        { {-1.0f,  1.0f, 0.0f}, {0.0f, 1.0f} },
        { { 1.0f,  1.0f, 0.0f}, {1.0f, 1.0f} },
        { { 1.0f, -1.0f, 0.0f}, {1.0f, 0.0f} },
    };

    constexpr shipBool dynamic = false;
    m_FullscreenQuadVertexBufferHandle = gfxRenderDevice.CreateVertexBuffer(4, VertexFormatType::Pos_UV, dynamic, fullscreenQuadVertices);

    shipUint16 fullscreenQuadIndices[6] =
    {
        0, 1, 2,
        0, 2, 3
    };

    constexpr shipBool uses2BytesPerIndex = true;
    m_FullscreenQuadIndexBufferHandle = gfxRenderDevice.CreateIndexBuffer(6, uses2BytesPerIndex, dynamic, fullscreenQuadIndices);

    return (m_FullscreenQuadVertexBufferHandle.IsValid() && m_FullscreenQuadIndexBufferHandle.IsValid());
}

void FullscreenHelper::DestroyResources(GFXRenderDevice& gfxRenderDevice)
{
    gfxRenderDevice.DestroyVertexBuffer(m_FullscreenQuadVertexBufferHandle);
    gfxRenderDevice.DestroyIndexBuffer(m_FullscreenQuadIndexBufferHandle);
}

void FullscreenHelper::DrawFullscreenQuad(GFXRenderDevice& gfxRenderDevice, GFXDirectRenderCommandList& gfxRenderCommandList, const FullscreenQuadRenderContext& renderContext)
{
    ShaderHandler* pShaderHandler = GetShaderHandlerManager().GetShaderHandlerForShaderKey(renderContext.shaderKey);
    if (pShaderHandler->GetShaderKey().GetShaderFamily() == ShaderFamily::Error)
    {
        return;
    }

    pShaderHandler->ApplyShaderInputProvidersForGraphics(gfxRenderDevice, gfxRenderCommandList, renderContext.shaderInputProviders);

    ShaderHandler::RenderState renderState;
    renderState.GfxRenderTargetHandle = renderContext.gfxRenderTargetHandle;
    renderState.GfxDepthStencilRenderTargetHandle = renderContext.gfxDepthStencilRenderTargetHandle;
    renderState.PrimitiveTopologyToRender = PrimitiveTopology::TriangleList;
    renderState.VertexFormatTypeToRender = VertexFormatType::Pos_UV;
    renderState.OptionalRenderStateBlockStateOverride = nullptr;

    const ShaderHandler::ShaderRenderElementsForGraphics& shaderRenderElements = pShaderHandler->GetShaderRenderElementsForGraphicsCommands(gfxRenderDevice, renderState);

    const GFXRenderTarget& gfxRenderTarget = gfxRenderDevice.GetRenderTarget(renderContext.gfxRenderTargetHandle);
    shipUint32 renderTargetWidth = gfxRenderTarget.GetWidth();
    shipUint32 renderTargetHeight = gfxRenderTarget.GetHeight();

    GfxViewport gfxViewport;
    gfxViewport.topLeftX = renderContext.relativeScreenX * renderTargetWidth;
    gfxViewport.topLeftY = renderContext.relativeScreenY * renderTargetHeight;
    gfxViewport.width = renderContext.relativeScreenWidth * renderTargetWidth;
    gfxViewport.height = renderContext.relativeScreenHeight * renderTargetHeight;
    gfxViewport.minDepth = 0.0f;
    gfxViewport.maxDepth = 1.0f;

    DrawIndexedCommand* pDrawIndexedCommand = gfxRenderCommandList.DrawIndexed();
    pDrawIndexedCommand->gfxViewport = gfxViewport;
    pDrawIndexedCommand->gfxRenderTargetHandle = renderContext.gfxRenderTargetHandle;
    pDrawIndexedCommand->gfxDepthStencilRenderTargetHandle = renderContext.gfxDepthStencilRenderTargetHandle;
    pDrawIndexedCommand->gfxPipelineStateObjectHandle = shaderRenderElements.GfxGraphicsPipelineStateObjectHandle;
    pDrawIndexedCommand->gfxRootSignatureHandle = shaderRenderElements.GfxRootSignatureHandle;
    pDrawIndexedCommand->gfxDescriptorSetHandle = shaderRenderElements.GfxDescriptorSetHandle;
    pDrawIndexedCommand->gfxVertexBufferHandle = m_FullscreenQuadVertexBufferHandle;
    pDrawIndexedCommand->gfxIndexBufferHandle = m_FullscreenQuadIndexBufferHandle;
    pDrawIndexedCommand->vertexBufferOffset = 0;
    pDrawIndexedCommand->startIndexLocation = 0;
    pDrawIndexedCommand->indexBufferOffset = 0;
    pDrawIndexedCommand->baseVertexLocation = 0;
}

FullscreenHelper& GetFullscreenHelper()
{
    return FullscreenHelper::GetInstance();
}

}