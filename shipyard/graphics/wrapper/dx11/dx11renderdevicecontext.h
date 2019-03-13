#pragma once

#include <graphics/wrapper/renderdevicecontext.h>

#include <graphics/wrapper/dx11/dx11renderstatecache.h>

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11DepthStencilState;
struct ID3D11RasterizerState;
struct ID3D11DepthStencilView;
struct ID3D11RenderTargetView;

namespace Shipyard
{
    class GfxResource;

    class SHIPYARD_API DX11RenderDeviceContext : public RenderDeviceContext
    {
    public:
        DX11RenderDeviceContext(GFXRenderDevice& renderDevice);
        ~DX11RenderDeviceContext();

        void ClearFullRenderTarget(GFXRenderTargetHandle gfxRenderTargetHandle, float red, float green, float blue, float alpha);
        void ClearSingleRenderTarget(GFXRenderTargetHandle gfxRenderTargetHandle, uint32_t renderTargetIndex, float red, float green, float blue, float alpha);
        void ClearDepthStencilRenderTarget(GFXDepthStencilRenderTargetHandle gfxDepthStencilRenderTarget, DepthStencilClearFlag depthStencilClearFlag, float depthValue, uint8_t stencilValue);

        void Draw(
                const DrawItem& drawItem,
                GFXVertexBufferHandle* gfxVertexBufferHandles,
                uint32_t startSlot,
                uint32_t numVertexBuffers,
                uint32_t startVertexLocation,
                uint32_t* vertexBufferOffsets);
        void DrawIndexed(
                const DrawItem& drawItem,
                GFXVertexBufferHandle* gfxVertexBufferHandles,
                uint32_t startSlot,
                uint32_t numVertexBuffers,
                uint32_t* vertexBufferOffsets,
                GFXIndexBufferHandle gfxIndexBufferHandle,
                uint32_t startVertexLocation,
                uint32_t startIndexLocation,
                uint32_t indexBufferOffset);

    private:
        void PrepareNextDrawCalls(const DrawItem& drawItem, VertexFormatType vertexFormatType);

        ID3D11Device* m_Device;
        ID3D11DeviceContext* m_DeviceContext;

        ID3D11RenderTargetView* m_RenderTargets[8];
        ID3D11DepthStencilView* m_DepthStencilView;

        DX11RenderStateCache m_RenderStateCache;
    };
}