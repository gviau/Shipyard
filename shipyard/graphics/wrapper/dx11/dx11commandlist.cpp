#include <graphics/graphicsprecomp.h>

#include <graphics/wrapper/dx11/dx11commandlist.h>

#include <graphics/shader/shaderhandler.h>

#include <graphics/wrapper/dx11/dx11_common.h>
#include <graphics/wrapper/dx11/dx11buffer.h>
#include <graphics/wrapper/dx11/dx11descriptorset.h>
#include <graphics/wrapper/dx11/dx11pipelinestateobject.h>
#include <graphics/wrapper/dx11/dx11renderdevice.h>
#include <graphics/wrapper/dx11/dx11rendertarget.h>
#include <graphics/wrapper/dx11/dx11rootsignature.h>
#include <graphics/wrapper/dx11/dx11shader.h>
#include <graphics/wrapper/dx11/dx11texture.h>

#include <graphics/vertexformat.h>

#include <system/array.h>
#include <system/logger.h>
#include <system/systemcommon.h>

#ifdef SHIP_RENDER_COMMANDS_DEBUG_INFO
#include <system/atomicoperations.h>
#endif // #ifdef SHIP_RENDER_COMMANDS_DEBUG_INFO

#pragma warning( disable : 4005 )

#include <d3d11.h>

#pragma warning( default : 4005 )

namespace Shipyard
{;

DX11BaseRenderCommandList::DX11BaseRenderCommandList()
    : m_pCommandListHeap(nullptr)
    , m_CommandListHeapCurrentPointer(0)

#ifdef SHIP_COMMAND_LIST_GROWABLE_HEAP
    , m_CommandListHeapSize(SHIP_COMMAND_LIST_HEAP_SIZE)
#endif // #ifdef SHIP_COMMAND_LIST_GROWABLE_HEAP

#ifdef SHIP_RENDER_COMMANDS_DEBUG_INFO
    , m_pLastAllocatedRenderCommand(nullptr)
#endif // #ifdef SHIP_RENDER_COMMANDS_DEBUG_INFO
{

}

DX11BaseRenderCommandList::~DX11BaseRenderCommandList()
{
    Destroy();
}

shipBool DX11BaseRenderCommandList::Create()
{
    m_pCommandListHeap = SHIP_ALLOC(SHIP_COMMAND_LIST_HEAP_SIZE, SHIP_CACHE_LINE_SIZE);

    return (m_pCommandListHeap != nullptr);
}

void DX11BaseRenderCommandList::Destroy()
{
    SHIP_FREE(m_pCommandListHeap);
    m_pCommandListHeap = nullptr;
}

void DX11BaseRenderCommandList::Close()
{

}

void DX11BaseRenderCommandList::Reset(GFXCommandListAllocator& gfxCommandListAllocator, PipelineStateObject* pPipelineStateObject)
{
    m_CommandListHeapCurrentPointer = 0;

#ifdef SHIP_RENDER_COMMANDS_DEBUG_INFO
    m_pLastAllocatedRenderCommand = nullptr;
#endif // #ifdef SHIP_RENDER_COMMANDS_DEBUG_INFO
}

BaseRenderCommand* DX11BaseRenderCommandList::GetNewRenderCommand(RenderCommandType renderCommandType)
{
    size_t renderCommandSize = 0;

    switch (renderCommandType)
    {
    case RenderCommandType::ClearFullRenderTarget:
        renderCommandSize = sizeof(ClearFullRenderTargetCommand);
        break;

    case RenderCommandType::ClearSingleRenderTarget:
        renderCommandSize = sizeof(ClearSingleRenderTargetCommand);
        break;

    case RenderCommandType::ClearDepthStencilRenderTarget:
        renderCommandSize = sizeof(ClearDepthStencilRenderTargetCommand);
        break;

    case RenderCommandType::Draw:
        renderCommandSize = sizeof(DrawCommand);
        break;

    case RenderCommandType::DrawSeveralVertexBuffers:
        renderCommandSize = sizeof(DrawSeveralVertexBuffersCommand);
        break;

    case RenderCommandType::DrawIndexed:
        renderCommandSize = sizeof(DrawIndexedCommand);
        break;

    case RenderCommandType::DrawIndexedSeveralVertexBuffers:
        renderCommandSize = sizeof(DrawIndexedSeveralVertexBuffersCommand);
        break;

    case RenderCommandType::MapBuffer:
        renderCommandSize = sizeof(MapBufferCommand);
        break;

    case RenderCommandType::Dispatch:
        renderCommandSize = sizeof(DispatchCommand);
        break;

    default:
        SHIP_ASSERT(!"DX11BaseRenderCommandList::GetNewRenderCommand --> unsupported render command type");
    }

#ifdef SHIP_COMMAND_LIST_GROWABLE_HEAP

    shipBool notEnoughSpaceToStoreThisRenderCommand = ((m_CommandListHeapCurrentPointer + renderCommandSize) > m_CommandListHeapSize);
    if (notEnoughSpaceToStoreThisRenderCommand)
    {
        m_pCommandListHeap = GrowCommandListHeap(m_CommandListHeapSize * 2);
    }

#else

    if (m_CommandListHeapCurrentPointer + renderCommandSize > SHIP_COMMAND_LIST_HEAP_SIZE)
    {
        SHIP_ASSERT(!"No more space for command list heap! You need to increase the internal command list heap size");
    }

#endif // #ifdef SHIP_COMMAND_LIST_GROWABLE_HEAP

    BaseRenderCommand* pNewCommand = reinterpret_cast<BaseRenderCommand*>(size_t(m_pCommandListHeap) + m_CommandListHeapCurrentPointer);

#ifdef SHIP_USE_DRAW_COMMANDS_DEFAULT_CONSTRUCTOR
    switch (renderCommandType)
    {
    case RenderCommandType::ClearFullRenderTarget:
        {
            ClearFullRenderTargetCommand initRenderCommandData;
            memcpy(pNewCommand, &initRenderCommandData, sizeof(initRenderCommandData));
        }
        break;

    case RenderCommandType::ClearSingleRenderTarget:
        {
            ClearSingleRenderTargetCommand initRenderCommandData;
            memcpy(pNewCommand, &initRenderCommandData, sizeof(initRenderCommandData));
        }
        break;

    case RenderCommandType::ClearDepthStencilRenderTarget:
        {
            ClearDepthStencilRenderTargetCommand initRenderCommandData;
            memcpy(pNewCommand, &initRenderCommandData, sizeof(initRenderCommandData));
        }
        break;

    case RenderCommandType::Draw:
        {
            DrawCommand initRenderCommandData;
            memcpy(pNewCommand, &initRenderCommandData, sizeof(initRenderCommandData));
        }
        break;
        
    case RenderCommandType::DrawSeveralVertexBuffers:
        {
            DrawSeveralVertexBuffersCommand initRenderCommandData;
            memcpy(pNewCommand, &initRenderCommandData, sizeof(initRenderCommandData));
        }
        break;

    case RenderCommandType::DrawIndexed:
        {
            DrawIndexedCommand initRenderCommandData;
            memcpy(pNewCommand, &initRenderCommandData, sizeof(initRenderCommandData));
        }
        break;

    case RenderCommandType::DrawIndexedSeveralVertexBuffers:
        {
            DrawIndexedSeveralVertexBuffersCommand initRenderCommandData;
            memcpy(pNewCommand, &initRenderCommandData, sizeof(initRenderCommandData));
        }
        break;

    case RenderCommandType::MapBuffer:
        {
            MapBufferCommand initRenderCommandData;
            memcpy(pNewCommand, &initRenderCommandData, sizeof(initRenderCommandData));
        }
        break;

    case RenderCommandType::Dispatch:
        {
            DispatchCommand initRenderCommandData;
            memcpy(pNewCommand, &initRenderCommandData, sizeof(initRenderCommandData));
        }
        break;

    default:
        SHIP_ASSERT(!"DX11BaseRenderCommandList::GetNewRenderCommand --> unsupported render command type");
    }
#endif // #ifdef SHIP_USE_DRAW_COMMANDS_DEFAULT_CONSTRUCTOR

    pNewCommand->renderCommandType = renderCommandType;

#ifdef SHIP_RENDER_COMMANDS_DEBUG_INFO
    pNewCommand->pPreviousRenderCommand = m_pLastAllocatedRenderCommand;

    if (pNewCommand->pPreviousRenderCommand != nullptr)
    {
        pNewCommand->pPreviousRenderCommand->pNextRenderCommand = pNewCommand;
    }

    m_pLastAllocatedRenderCommand = pNewCommand;
#endif // SHIP_RENDER_COMMANDS_DEBUG_INFO

    m_CommandListHeapCurrentPointer += renderCommandSize;

    return pNewCommand;
}

void* DX11BaseRenderCommandList::GetCommandListHeap() const
{
    return m_pCommandListHeap;
}

void* DX11BaseRenderCommandList::GetCommandListEnd() const
{
    return reinterpret_cast<void*>(size_t(m_pCommandListHeap) + m_CommandListHeapCurrentPointer);
}

#ifdef SHIP_COMMAND_LIST_GROWABLE_HEAP
void* DX11BaseRenderCommandList::GrowCommandListHeap(size_t newCommandListHeapSize)
{
    SHIP_LOG_WARNING(
            "DX11BaseRenderCommandList::GrowCommandListHeap --> Growing command list 0x%p from size %llu to new size %llu",
            this, m_CommandListHeapSize, newCommandListHeapSize);

    m_CommandListHeapSize = newCommandListHeapSize;

    void* pNewCommandListHeap = SHIP_ALLOC(newCommandListHeapSize, SHIP_CACHE_LINE_SIZE);

    memcpy(pNewCommandListHeap, m_pCommandListHeap, m_CommandListHeapCurrentPointer);

    SHIP_FREE(m_pCommandListHeap);

    return pNewCommandListHeap;
}
#endif // #ifdef SHIP_COMMAND_LIST_GROWABLE_HEAP

DX11DirectRenderCommandList::DX11DirectRenderCommandList(GFXRenderDevice& gfxRenderDevice)
    : DirectRenderCommandList(gfxRenderDevice)
{
    m_pDeviceContext = gfxRenderDevice.GetImmediateDeviceContext();
}


DX11DirectRenderCommandList::~DX11DirectRenderCommandList()
{

}

ClearFullRenderTargetCommand* DX11DirectRenderCommandList::ClearFullRenderTarget()
{
    BaseRenderCommand* pCmd = GetNewRenderCommand(RenderCommandType::ClearFullRenderTarget);
    
    return static_cast<ClearFullRenderTargetCommand*>(pCmd);
}

ClearSingleRenderTargetCommand* DX11DirectRenderCommandList::ClearSingleRenderTarget()
{
    BaseRenderCommand* pCmd = GetNewRenderCommand(RenderCommandType::ClearSingleRenderTarget);
    
    return static_cast<ClearSingleRenderTargetCommand*>(pCmd);
}

ClearDepthStencilRenderTargetCommand* DX11DirectRenderCommandList::ClearDepthStencilRenderTarget()
{
    BaseRenderCommand* pCmd = GetNewRenderCommand(RenderCommandType::ClearDepthStencilRenderTarget);
    
    return static_cast<ClearDepthStencilRenderTargetCommand*>(pCmd);
}

DrawCommand* DX11DirectRenderCommandList::Draw()
{
    BaseRenderCommand* pCmd = GetNewRenderCommand(RenderCommandType::Draw);
    
    return static_cast<DrawCommand*>(pCmd);
}

DrawSeveralVertexBuffersCommand* DX11DirectRenderCommandList::DrawSeveralVertexBuffers()
{
    BaseRenderCommand* pCmd = GetNewRenderCommand(RenderCommandType::DrawSeveralVertexBuffers);

    return static_cast<DrawSeveralVertexBuffersCommand*>(pCmd);
}

DrawIndexedCommand* DX11DirectRenderCommandList::DrawIndexed()
{
    BaseRenderCommand* pCmd = GetNewRenderCommand(RenderCommandType::DrawIndexed);

    return static_cast<DrawIndexedCommand*>(pCmd);
}

DrawIndexedSeveralVertexBuffersCommand* DX11DirectRenderCommandList::DrawIndexedSeveralVertexBuffers()
{
    BaseRenderCommand* pCmd = GetNewRenderCommand(RenderCommandType::DrawIndexedSeveralVertexBuffers);

    return static_cast<DrawIndexedSeveralVertexBuffersCommand*>(pCmd);
}

DispatchCommand* DX11DirectRenderCommandList::Dispatch()
{
    BaseRenderCommand* pCmd = GetNewRenderCommand(RenderCommandType::Dispatch);

    return static_cast<DispatchCommand*>(pCmd);
}

void* GetMappedBuffer(DX11BaseBuffer& dx11BaseBuffer, MapFlag mapFlag, ID3D11DeviceContext* pDx11DeviceContext)
{
    if (mapFlag == MapFlag::Read || mapFlag == MapFlag::Read_Write)
    {
        // If we're reading, then we need to map on the d3d11 immediate context now
        ID3D11Buffer* pBuffer = dx11BaseBuffer.GetBuffer();

        D3D11_MAPPED_SUBRESOURCE mappedResource;
        HRESULT hr = pDx11DeviceContext->Map(pBuffer, 0, ConvertShipyardMapFlagToDX11(mapFlag), 0, &mappedResource);

        if (FAILED(hr))
        {
            SHIP_LOG_ERROR("DX11CommandQueue::MapBuffer --> Failed to map buffer");
            return nullptr;
        }

        return mappedResource.pData;
    }

    // If we're only writing, we defer the write until we execute the command list.
    return  SHIP_ALLOC(dx11BaseBuffer.GetSize(), 16);
}

void* DX11DirectRenderCommandList::MapVertexBuffer(GFXVertexBufferHandle gfxVertexBufferhandle, MapFlag mapFlag, size_t bufferOffset)
{
    SHIP_ASSERT_MSG(bufferOffset == 0 || mapFlag != MapFlag::Write_Discard, "Cannot Map a buffer with Write_Discard with a non-zero offset. Using an offset indicates a partial update of the buffer, but Write_Discard will throw away the whole buffer.");

    BaseRenderCommand* pCmd = GetNewRenderCommand(RenderCommandType::MapBuffer);

    MapBufferCommand* pMapBufferCommand = static_cast<MapBufferCommand*>(pCmd);
    pMapBufferCommand->mapFlag = mapFlag;
    pMapBufferCommand->mapBufferType = MapBufferType::Vertex;
    pMapBufferCommand->bufferHandle.gfxVertexBufferHandle = gfxVertexBufferhandle;
    pMapBufferCommand->bufferOffset = bufferOffset;

    GFXVertexBuffer& gfxVertexBuffer = m_RenderDevice.GetVertexBuffer(gfxVertexBufferhandle);

    pMapBufferCommand->pBuffer = GetMappedBuffer(gfxVertexBuffer, mapFlag, m_pDeviceContext);
    SHIP_ASSERT(pMapBufferCommand->pBuffer != nullptr);

    // If we're reading, then we want to offset the buffer.
    if (mapFlag == MapFlag::Read || mapFlag == MapFlag::Read_Write)
    {
        return reinterpret_cast<void*>(size_t(pMapBufferCommand->pBuffer) + bufferOffset);
    }

    // If we're writing, since we allocated a fresh temporary buffer, we don't offset here.
    return pMapBufferCommand->pBuffer;
}

void* DX11DirectRenderCommandList::MapIndexBuffer(GFXIndexBufferHandle gfxIndexBufferHandle, MapFlag mapFlag, size_t bufferOffset)
{
    SHIP_ASSERT_MSG(bufferOffset == 0 || mapFlag != MapFlag::Write_Discard, "Cannot Map a buffer with Write_Discard with a non-zero offset. Using an offset indicates a partial update of the buffer, but Write_Discard will throw away the whole buffer.");

    BaseRenderCommand* pCmd = GetNewRenderCommand(RenderCommandType::MapBuffer);

    MapBufferCommand* pMapBufferCommand = static_cast<MapBufferCommand*>(pCmd);
    pMapBufferCommand->mapFlag = mapFlag;
    pMapBufferCommand->mapBufferType = MapBufferType::Index;
    pMapBufferCommand->bufferHandle.gfxIndexBufferHandle = gfxIndexBufferHandle;
    pMapBufferCommand->bufferOffset = bufferOffset;

    GFXIndexBuffer& gfxIndexBuffer = m_RenderDevice.GetIndexBuffer(gfxIndexBufferHandle);

    pMapBufferCommand->pBuffer = GetMappedBuffer(gfxIndexBuffer, mapFlag, m_pDeviceContext);
    SHIP_ASSERT(pMapBufferCommand->pBuffer != nullptr);

    // If we're reading, then we want to offset the buffer.
    if (mapFlag == MapFlag::Read || mapFlag == MapFlag::Read_Write)
    {
        return reinterpret_cast<void*>(size_t(pMapBufferCommand->pBuffer) + bufferOffset);
    }

    // If we're writing, since we allocated a fresh temporary buffer, we don't offset here.
    return pMapBufferCommand->pBuffer;
}

void* DX11DirectRenderCommandList::MapConstantBuffer(GFXConstantBufferHandle gfxConstantBufferHandle, MapFlag mapFlag, size_t bufferOffset)
{
    SHIP_ASSERT_MSG(bufferOffset == 0 || mapFlag != MapFlag::Write_Discard, "Cannot Map a buffer with Write_Discard with a non-zero offset. Using an offset indicates a partial update of the buffer, but Write_Discard will throw away the whole buffer.");

    BaseRenderCommand* pCmd = GetNewRenderCommand(RenderCommandType::MapBuffer);

    MapBufferCommand* pMapBufferCommand = static_cast<MapBufferCommand*>(pCmd);
    pMapBufferCommand->mapFlag = mapFlag;
    pMapBufferCommand->mapBufferType = MapBufferType::Constant;
    pMapBufferCommand->bufferHandle.gfxConstantBufferHandle = gfxConstantBufferHandle;
    pMapBufferCommand->bufferOffset = bufferOffset;

    GFXConstantBuffer& gfxConstantBuffer = m_RenderDevice.GetConstantBuffer(gfxConstantBufferHandle);

    pMapBufferCommand->pBuffer = GetMappedBuffer(gfxConstantBuffer, mapFlag, m_pDeviceContext);
    SHIP_ASSERT(pMapBufferCommand->pBuffer != nullptr);

    // If we're reading, then we want to offset the buffer.
    if (mapFlag == MapFlag::Read || mapFlag == MapFlag::Read_Write)
    {
        return reinterpret_cast<void*>(size_t(pMapBufferCommand->pBuffer) + bufferOffset);
    }

    // If we're writing, since we allocated a fresh temporary buffer, we don't offset here.
    return pMapBufferCommand->pBuffer;
}

void* DX11DirectRenderCommandList::MapByteBuffer(GFXByteBufferHandle gfxByteBufferHandle, MapFlag mapFlag, size_t bufferOffset)
{
    SHIP_ASSERT_MSG(bufferOffset == 0 || mapFlag != MapFlag::Write_Discard, "Cannot Map a buffer with Write_Discard with a non-zero offset. Using an offset indicates a partial update of the buffer, but Write_Discard will throw away the whole buffer.");

    BaseRenderCommand* pCmd = GetNewRenderCommand(RenderCommandType::MapBuffer);

    MapBufferCommand* pMapBufferCommand = static_cast<MapBufferCommand*>(pCmd);
    pMapBufferCommand->mapFlag = mapFlag;
    pMapBufferCommand->mapBufferType = MapBufferType::Data;
    pMapBufferCommand->bufferHandle.gfxByteBufferHandle = gfxByteBufferHandle;
    pMapBufferCommand->bufferOffset = bufferOffset;

    GFXByteBuffer& gfxByteBuffer = m_RenderDevice.GetByteBuffer(gfxByteBufferHandle);

    pMapBufferCommand->pBuffer = GetMappedBuffer(gfxByteBuffer, mapFlag, m_pDeviceContext);
    SHIP_ASSERT(pMapBufferCommand->pBuffer != nullptr);

    // If we're reading, then we want to offset the buffer.
    if (mapFlag == MapFlag::Read || mapFlag == MapFlag::Read_Write)
    {
        return reinterpret_cast<void*>(size_t(pMapBufferCommand->pBuffer) + bufferOffset);
    }

    // If we're writing, since we allocated a fresh temporary buffer, we don't offset here.
    return pMapBufferCommand->pBuffer;
}

}