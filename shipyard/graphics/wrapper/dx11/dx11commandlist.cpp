#include <graphics/wrapper/dx11/dx11commandlist.h>

#include <graphics/wrapper/dx11/dx11_common.h>
#include <graphics/wrapper/dx11/dx11buffer.h>
#include <graphics/wrapper/dx11/dx11descriptorset.h>
#include <graphics/wrapper/dx11/dx11pipelinestateobject.h>
#include <graphics/wrapper/dx11/dx11renderdevice.h>
#include <graphics/wrapper/dx11/dx11rendertarget.h>
#include <graphics/wrapper/dx11/dx11rootsignature.h>
#include <graphics/wrapper/dx11/dx11shader.h>
#include <graphics/wrapper/dx11/dx11texture.h>

#include <graphics/shaderhandler.h>
#include <graphics/vertexformat.h>

#include <system/array.h>
#include <system/logger.h>
#include <system/systemcommon.h>

#pragma warning( disable : 4005 )

#include <d3d11.h>

#pragma warning( default : 4005 )

namespace Shipyard
{;

DX11BaseRenderCommandList::DX11BaseRenderCommandList()
    : m_pCommandListHeap(nullptr)
    , m_CommandListHeapCurrentPointer(0)
{

}

DX11BaseRenderCommandList::~DX11BaseRenderCommandList()
{
    Destroy();
}

bool DX11BaseRenderCommandList::Create()
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

void DX11BaseRenderCommandList::Reset(GFXCommandListAllocator& gfxCommandListAllocator, GFXPipelineStateObject* pGfxPipelineStateObject)
{
    m_CommandListHeapCurrentPointer = 0;
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

    case RenderCommandType::DrawIndexed:
        renderCommandSize = sizeof(DrawIndexedCommand);
        break;

    case RenderCommandType::MapBuffer:
        renderCommandSize = sizeof(MapBufferCommand);
        break;

    default:
        SHIP_ASSERT(!"DX11BaseRenderCommandList::GetNewRenderCommand --> unsupported render command type");
    }

    if (m_CommandListHeapCurrentPointer + renderCommandSize > SHIP_COMMAND_LIST_HEAP_SIZE)
    {
        SHIP_ASSERT(!"No more space for command list heap! You need to increase the internal command list heap size");
    }

    BaseRenderCommand* pNewCommand = reinterpret_cast<BaseRenderCommand*>(size_t(m_pCommandListHeap) + m_CommandListHeapCurrentPointer);
    pNewCommand->renderCommandType = renderCommandType;

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

DrawIndexedCommand* DX11DirectRenderCommandList::DrawIndexed()
{
    BaseRenderCommand* pCmd = GetNewRenderCommand(RenderCommandType::DrawIndexed);

    return static_cast<DrawIndexedCommand*>(pCmd);
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

void* DX11DirectRenderCommandList::MapVertexBuffer(GFXVertexBufferHandle gfxVertexBufferhandle, MapFlag mapFlag)
{
    BaseRenderCommand* pCmd = GetNewRenderCommand(RenderCommandType::MapBuffer);

    MapBufferCommand* pMapBufferCommand = static_cast<MapBufferCommand*>(pCmd);
    pMapBufferCommand->mapFlag = mapFlag;
    pMapBufferCommand->mapBufferType = MapBufferType::Vertex;
    pMapBufferCommand->bufferHandle.gfxVertexBufferHandle = gfxVertexBufferhandle;

    GFXVertexBuffer& gfxVertexBuffer = m_RenderDevice.GetVertexBuffer(gfxVertexBufferhandle);

    pMapBufferCommand->pBuffer = GetMappedBuffer(gfxVertexBuffer, mapFlag, m_pDeviceContext);

    return pMapBufferCommand->pBuffer;
}

void* DX11DirectRenderCommandList::MapIndexBuffer(GFXIndexBufferHandle gfxIndexBufferHandle, MapFlag mapFlag)
{
    BaseRenderCommand* pCmd = GetNewRenderCommand(RenderCommandType::MapBuffer);

    MapBufferCommand* pMapBufferCommand = static_cast<MapBufferCommand*>(pCmd);
    pMapBufferCommand->mapFlag = mapFlag;
    pMapBufferCommand->mapBufferType = MapBufferType::Index;
    pMapBufferCommand->bufferHandle.gfxIndexBufferHandle = gfxIndexBufferHandle;

    GFXIndexBuffer& gfxIndexBuffer = m_RenderDevice.GetIndexBuffer(gfxIndexBufferHandle);

    pMapBufferCommand->pBuffer = GetMappedBuffer(gfxIndexBuffer, mapFlag, m_pDeviceContext);

    return pMapBufferCommand->pBuffer;
}

void* DX11DirectRenderCommandList::MapConstantBuffer(GFXConstantBufferHandle gfxConstantBufferHandle, MapFlag mapFlag)
{
    BaseRenderCommand* pCmd = GetNewRenderCommand(RenderCommandType::MapBuffer);

    MapBufferCommand* pMapBufferCommand = static_cast<MapBufferCommand*>(pCmd);
    pMapBufferCommand->mapFlag = mapFlag;
    pMapBufferCommand->mapBufferType = MapBufferType::Constant;
    pMapBufferCommand->bufferHandle.gfxConstantBufferHandle = gfxConstantBufferHandle;

    GFXConstantBuffer& gfxConstantBuffer = m_RenderDevice.GetConstantBuffer(gfxConstantBufferHandle);

    pMapBufferCommand->pBuffer = GetMappedBuffer(gfxConstantBuffer, mapFlag, m_pDeviceContext);

    return pMapBufferCommand->pBuffer;
}

}