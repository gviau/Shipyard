#pragma once

#include <graphics/wrapper/commandlist.h>

// Uncomment this define to allow the command list to dynamically grow its internal heap
// if there isn't enough space to record a command. Useful to determine whether or not one
// should increase the static heap size. This define is disabled in master builds.
#ifndef SHIP_MASTER
#define SHIP_COMMAND_LIST_GROWABLE_HEAP
#endif // #ifndef SHIP_MASTER

struct ID3D11DeviceContext;

namespace Shipyard
{
    class SHIPYARD_API DX11BaseRenderCommandList : BaseRenderCommandList
    {
    public:
        DX11BaseRenderCommandList();
        ~DX11BaseRenderCommandList();

        bool Create();
        void Destroy();

        void Close();
        void Reset(GFXCommandListAllocator& gfxCommandListAllocator, GFXPipelineStateObject* pGfxPipelineStateObject);

        BaseRenderCommand* GetNewRenderCommand(RenderCommandType renderCommandType);

        void* GetCommandListHeap() const;
        void* GetCommandListEnd() const;

    protected:
        void* m_pCommandListHeap;
        size_t m_CommandListHeapCurrentPointer;

#ifdef SHIP_COMMAND_LIST_GROWABLE_HEAP
        size_t m_CommandListHeapSize;

        void* GrowCommandListHeap(size_t newCommandListHeapSize);
#endif // #ifdef SHIP_COMMAND_LIST_GROWABLE_HEAP
    };

    class SHIPYARD_API DX11DirectRenderCommandList : public DX11BaseRenderCommandList, public DirectRenderCommandList
    {
    public:
        DX11DirectRenderCommandList(GFXRenderDevice& gfxRenderDevice);
        ~DX11DirectRenderCommandList();

        ClearFullRenderTargetCommand* ClearFullRenderTarget();
        ClearSingleRenderTargetCommand* ClearSingleRenderTarget();
        ClearDepthStencilRenderTargetCommand* ClearDepthStencilRenderTarget();

        DrawCommand* Draw();
        DrawIndexedCommand* DrawIndexed();

        void* MapVertexBuffer(GFXVertexBufferHandle gfxVertexBufferhandle, MapFlag mapFlag, size_t bufferOffset);
        void* MapIndexBuffer(GFXIndexBufferHandle gfxIndexBufferHandle, MapFlag mapFlag, size_t bufferOffset);
        void* MapConstantBuffer(GFXConstantBufferHandle gfxConstantBufferHandle, MapFlag mapFlag, size_t bufferOffset);
        void* MapByteBuffer(GFXByteBufferHandle gfxByteBufferHandle, MapFlag mapFlag, size_t bufferOffset);
        
    protected:
        ID3D11DeviceContext* m_pDeviceContext;
    };
}