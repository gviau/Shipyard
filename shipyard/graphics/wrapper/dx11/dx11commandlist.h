#pragma once

#include <graphics/wrapper/commandlist.h>

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

        void* MapVertexBuffer(GFXVertexBufferHandle gfxVertexBufferhandle, MapFlag mapFlag);
        void* MapIndexBuffer(GFXIndexBufferHandle gfxIndexBufferHandle, MapFlag mapFlag);
        void* MapConstantBuffer(GFXConstantBufferHandle gfxConstantBufferHandle, MapFlag mapFlag);
        
    protected:
        ID3D11DeviceContext* m_pDeviceContext;
    };
}