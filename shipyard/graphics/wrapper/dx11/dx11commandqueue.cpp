#include <graphics/wrapper/dx11/dx11commandqueue.h>

#include <graphics/shader/shaderhandler.h>
#include <graphics/shader/shaderhandlermanager.h>

#include <graphics/wrapper/dx11/dx11commandlist.h>
#include <graphics/wrapper/dx11/dx11renderdevice.h>

#include <system/logger.h>

namespace Shipyard
{;

DX11CommandQueue::DX11CommandQueue(GFXRenderDevice& gfxRenderDevice, CommandQueueType commandQueueType)
    : CommandQueue(gfxRenderDevice, commandQueueType)
    , m_Device(gfxRenderDevice.GetDevice())
    , m_DeviceContext(gfxRenderDevice.GetImmediateDeviceContext())
    , m_DepthStencilView(nullptr)
    , m_RenderStateCache(m_Device, m_DeviceContext)
{
    for (int i = 0; i < 8; i++)
    {
        m_RenderTargets[i] = nullptr;
    }
}

DX11CommandQueue::~DX11CommandQueue()
{
    Destroy();
}

shipBool DX11CommandQueue::Create()
{
    return true;
}

void DX11CommandQueue::Destroy()
{

}

void DX11CommandQueue::ExecuteCommandLists(GFXRenderCommandList** ppRenderCommandLists, shipUint32 numRenderCommandLists)
{
    for (shipUint32 i = 0; i < numRenderCommandLists; i++)
    {
        GFXRenderCommandList* pRenderCommandList = ppRenderCommandLists[i];

        void* pCommandListStart = pRenderCommandList->GetCommandListHeap();
        void* pCommandListEnd = pRenderCommandList->GetCommandListEnd();

        SHIP_ASSERT(pCommandListStart != pCommandListEnd);

        void* pCurrentCommandListPtr = pCommandListStart;
        while (pCurrentCommandListPtr != pCommandListEnd)
        {
            BaseRenderCommand* pBaseRenderCommand = reinterpret_cast<BaseRenderCommand*>(pCurrentCommandListPtr);

            size_t processedRenderCommandSize = 0;

            switch (pBaseRenderCommand->renderCommandType)
            {
            case RenderCommandType::ClearFullRenderTarget:
                processedRenderCommandSize = ClearFullRenderTarget(pBaseRenderCommand);
                break;

            case RenderCommandType::ClearSingleRenderTarget:
                processedRenderCommandSize = ClearSingleRenderTarget(pBaseRenderCommand);
                break;

            case RenderCommandType::ClearDepthStencilRenderTarget:
                processedRenderCommandSize = ClearDepthStencilRenderTarget(pBaseRenderCommand);
                break;

            case RenderCommandType::Draw:
                processedRenderCommandSize = Draw(pBaseRenderCommand);
                break;

            case RenderCommandType::DrawIndexed:
                processedRenderCommandSize = DrawIndexed(pBaseRenderCommand);
                break;

            case RenderCommandType::MapBuffer:
                processedRenderCommandSize = MapBuffer(pBaseRenderCommand);
                break;

            default:
                SHIP_ASSERT_MSG(false, "DX11CommandQueue::ExecuteCommandLists --> This render command type is not handled");
                break;
            }

            pCurrentCommandListPtr = reinterpret_cast<void*>(size_t(pCurrentCommandListPtr) + processedRenderCommandSize);
        }
    }
}

size_t DX11CommandQueue::ClearFullRenderTarget(BaseRenderCommand* pCmd)
{
    ClearFullRenderTargetCommand& clearFullRenderTargetCommand = *static_cast<ClearFullRenderTargetCommand*>(pCmd);

    SHIP_ASSERT(clearFullRenderTargetCommand.gfxRenderTargetHandle.handle != InvalidGfxHandle);

    const GFXRenderTarget& gfxRenderTarget = m_RenderDevice.GetRenderTarget(clearFullRenderTargetCommand.gfxRenderTargetHandle);

    ID3D11RenderTargetView* const * renderTargetViews = gfxRenderTarget.GetRenderTargetViews();

    for (shipUint32 i = 0; i < GfxConstants::GfxConstants_MaxRenderTargetsBound; i++)
    {
        if (renderTargetViews[i] == nullptr)
        {
            continue;
        }

        // This only works because the read, green, blue and alpha values are sequential in memory.
        m_DeviceContext->ClearRenderTargetView(renderTargetViews[i], &clearFullRenderTargetCommand.red);
    }

    return sizeof(ClearFullRenderTargetCommand);
}

size_t DX11CommandQueue::ClearSingleRenderTarget(BaseRenderCommand* pCmd)
{
    ClearSingleRenderTargetCommand& clearSingleRenderTargetCommand = *static_cast<ClearSingleRenderTargetCommand*>(pCmd);

    SHIP_ASSERT(clearSingleRenderTargetCommand.gfxRenderTargetHandle.handle != InvalidGfxHandle);
    SHIP_ASSERT(clearSingleRenderTargetCommand.renderTargetIndex < GfxConstants::GfxConstants_MaxRenderTargetsBound);

    const GFXRenderTarget& gfxRenderTarget = m_RenderDevice.GetRenderTarget(clearSingleRenderTargetCommand.gfxRenderTargetHandle);

    ID3D11RenderTargetView* const * renderTargetViews = gfxRenderTarget.GetRenderTargetViews();

    if (renderTargetViews[clearSingleRenderTargetCommand.renderTargetIndex] == nullptr)
    {
        return sizeof(ClearSingleRenderTargetCommand);
    }

    // This only works because the read, green, blue and alpha values are sequential in memory.
    m_DeviceContext->ClearRenderTargetView(renderTargetViews[clearSingleRenderTargetCommand.renderTargetIndex], &clearSingleRenderTargetCommand.red);

    return sizeof(ClearSingleRenderTargetCommand);
}

size_t DX11CommandQueue::ClearDepthStencilRenderTarget(BaseRenderCommand* pCmd)
{
    ClearDepthStencilRenderTargetCommand& clearDepthStencilRenderTargetCommand = *static_cast<ClearDepthStencilRenderTargetCommand*>(pCmd);

    SHIP_ASSERT(clearDepthStencilRenderTargetCommand.gfxDepthStencilRenderTargetHandle.handle != InvalidGfxHandle);

    const GFXDepthStencilRenderTarget& gfxDepthStencilRenderTarget = m_RenderDevice.GetDepthStencilRenderTarget(clearDepthStencilRenderTargetCommand.gfxDepthStencilRenderTargetHandle);

    ID3D11DepthStencilView* const depthStencilView = gfxDepthStencilRenderTarget.GetDepthStencilView();

    uint clearFlag = ((clearDepthStencilRenderTargetCommand.depthStencilClearFlag == DepthStencilClearFlag::Depth) ? D3D11_CLEAR_DEPTH :
                      (clearDepthStencilRenderTargetCommand.depthStencilClearFlag == DepthStencilClearFlag::Stencil) ? D3D11_CLEAR_STENCIL :
                      (D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL));

    m_DeviceContext->ClearDepthStencilView(depthStencilView, clearFlag, clearDepthStencilRenderTargetCommand.depthValue, clearDepthStencilRenderTargetCommand.stencilValue);

    return sizeof(ClearDepthStencilRenderTargetCommand);
}

size_t DX11CommandQueue::Draw(BaseRenderCommand* pCmd)
{
    DrawCommand& drawCommand = *static_cast<DrawCommand*>(pCmd);

    DrawItem drawItem(
            drawCommand.gfxRenderTargetHandle,
            drawCommand.gfxDepthStencilRenderTargetHandle,
            drawCommand.gfxViewport,
            drawCommand.gfxPipelineStateObjectHandle,
            drawCommand.gfxRootSignatureHandle,
            drawCommand.gfxDescriptorSetHandle);

    PrepareNextDrawCalls(drawItem);

    SHIP_ASSERT(drawCommand.numVertexBuffers < GfxConstants_MaxVertexBuffers);
    GFXVertexBuffer* gfxVertexBuffers[GfxConstants_MaxVertexBuffers];
    for (shipUint32 i = drawCommand.startSlot; i < (drawCommand.startSlot + drawCommand.numVertexBuffers); i++)
    {
        gfxVertexBuffers[i] = &m_RenderDevice.GetVertexBuffer(drawCommand.pGfxVertexBufferHandles[i]);
    }

    m_RenderStateCache.SetVertexBuffers(gfxVertexBuffers, drawCommand.startSlot, drawCommand.numVertexBuffers, drawCommand.pVertexBufferOffsets);

    m_RenderStateCache.CommitStateChangesForGraphics(m_RenderDevice);

    shipUint32 numVertices = ((drawCommand.numVertexBuffers > 0) ? m_RenderDevice.GetVertexBuffer(drawCommand.pGfxVertexBufferHandles[drawCommand.startSlot]).GetNumVertices() : 1);
    m_DeviceContext->Draw(numVertices, drawCommand.startVertexLocation);

    return sizeof(DrawCommand);
}

size_t DX11CommandQueue::DrawIndexed(BaseRenderCommand* pCmd)
{
    DrawIndexedCommand& drawIndexedCommand = *static_cast<DrawIndexedCommand*>(pCmd);

    SHIP_ASSERT(drawIndexedCommand.gfxIndexBufferHandle.handle != InvalidGfxHandle);
    const GFXIndexBuffer& gfxIndexBuffer = m_RenderDevice.GetIndexBuffer(drawIndexedCommand.gfxIndexBufferHandle);

    m_RenderStateCache.SetIndexBuffer(gfxIndexBuffer, drawIndexedCommand.indexBufferOffset);

    DrawItem drawItem(
            drawIndexedCommand.gfxRenderTargetHandle,
            drawIndexedCommand.gfxDepthStencilRenderTargetHandle,
            drawIndexedCommand.gfxViewport,
            drawIndexedCommand.gfxPipelineStateObjectHandle,
            drawIndexedCommand.gfxRootSignatureHandle,
            drawIndexedCommand.gfxDescriptorSetHandle);

    PrepareNextDrawCalls(drawItem);

    SHIP_ASSERT(drawIndexedCommand.numVertexBuffers < GfxConstants_MaxVertexBuffers);
    GFXVertexBuffer* gfxVertexBuffers[GfxConstants_MaxVertexBuffers];
    for (shipUint32 i = drawIndexedCommand.startSlot; i < (drawIndexedCommand.startSlot + drawIndexedCommand.numVertexBuffers); i++)
    {
        gfxVertexBuffers[i] = &m_RenderDevice.GetVertexBuffer(drawIndexedCommand.pGfxVertexBufferHandles[i]);
    }

    m_RenderStateCache.SetVertexBuffers(gfxVertexBuffers, drawIndexedCommand.startSlot, drawIndexedCommand.numVertexBuffers, drawIndexedCommand.pVertexBufferOffsets);

    m_RenderStateCache.CommitStateChangesForGraphics(m_RenderDevice);

    m_DeviceContext->DrawIndexed(gfxIndexBuffer.GetNumIndices(), drawIndexedCommand.startIndexLocation, drawIndexedCommand.startVertexLocation);

    return sizeof(DrawIndexedCommand);
}

size_t DX11CommandQueue::MapBuffer(BaseRenderCommand* pCmd)
{
    MapBufferCommand* pMapBufferCommand = static_cast<MapBufferCommand*>(pCmd);

    DX11BaseBuffer* pBaseBuffer = nullptr;

    switch (pMapBufferCommand->mapBufferType)
    {
    case MapBufferType::Vertex:
        pBaseBuffer = &m_RenderDevice.GetVertexBuffer(pMapBufferCommand->bufferHandle.gfxVertexBufferHandle);
        break;

    case MapBufferType::Index:
        pBaseBuffer = &m_RenderDevice.GetIndexBuffer(pMapBufferCommand->bufferHandle.gfxIndexBufferHandle);
        break;

    case MapBufferType::Constant:
        pBaseBuffer = &m_RenderDevice.GetConstantBuffer(pMapBufferCommand->bufferHandle.gfxConstantBufferHandle);
        break;

    case MapBufferType::Data:
        pBaseBuffer = &m_RenderDevice.GetByteBuffer(pMapBufferCommand->bufferHandle.gfxByteBufferHandle);
        break;

    default:
        SHIP_ASSERT_MSG(false, "DX11CommandQueue::MapBuffer --> Unsupported buffer type");
    }

    ID3D11Buffer* pDx11Buffer = pBaseBuffer->GetBuffer();

    if (pMapBufferCommand->mapFlag == MapFlag::Read || pMapBufferCommand->mapFlag == MapFlag::Read_Write)
    {
        // If we're reading or reading & writing, we only have to unmap the buffer.
        m_DeviceContext->Unmap(pDx11Buffer, 0);
    }
    else
    {
        // Otherwise, we have to map a buffer, copy the user buffer we have, unmap and free the user buffer.
        D3D11_MAPPED_SUBRESOURCE mappedSubresource;
        HRESULT hr = m_DeviceContext->Map(pDx11Buffer, 0, ConvertShipyardMapFlagToDX11(pMapBufferCommand->mapFlag), 0, &mappedSubresource);
        if (FAILED(hr))
        {
            SHIP_LOG_ERROR("DX11CommandQueue::MapBuffer --> Failed to map buffer");
            return sizeof(MapBufferCommand);
        }

        void* offsetedWritedBuffer = reinterpret_cast<void*>(size_t(mappedSubresource.pData) + pMapBufferCommand->bufferOffset);

        memcpy(offsetedWritedBuffer, pMapBufferCommand->pBuffer, pBaseBuffer->GetSize());

        m_DeviceContext->Unmap(pDx11Buffer, 0);

        SHIP_FREE(pMapBufferCommand->pBuffer);
    }

    return sizeof(MapBufferCommand);
}

void DX11CommandQueue::PrepareNextDrawCalls(const DrawItem& drawItem)
{
    if (drawItem.renderTargetHandle.handle != InvalidGfxHandle)
    {
        const GFXRenderTarget& gfxRenderTarget = m_RenderDevice.GetRenderTarget(drawItem.renderTargetHandle);

        m_RenderStateCache.BindRenderTarget(gfxRenderTarget);
    }

    if (drawItem.depthStencilRenderTargetHandle.handle != InvalidGfxHandle)
    {
        const GFXDepthStencilRenderTarget& gfxDepthStencilRenderTarget = m_RenderDevice.GetDepthStencilRenderTarget(drawItem.depthStencilRenderTargetHandle);

        m_RenderStateCache.BindDepthStencilRenderTarget(gfxDepthStencilRenderTarget);
    }

    GFXPipelineStateObject& gfxPipelineStateObject = m_RenderDevice.GetPipelineStateObject(drawItem.pipelineStateObjetHandle);
    GFXRootSignature& gfxRootSignature = m_RenderDevice.GetRootSignature(drawItem.rootSignatureHandle);
    GFXDescriptorSet& gfxDescriptorSet = m_RenderDevice.GetDescriptorSet(drawItem.descriptorSetHandle);

    m_RenderStateCache.BindRootSignature(gfxRootSignature);
    m_RenderStateCache.BindPipelineStateObject(gfxPipelineStateObject);
    m_RenderStateCache.BindDescriptorSet(gfxDescriptorSet, gfxRootSignature);

    m_RenderStateCache.SetViewport(drawItem.viewport);
}

}