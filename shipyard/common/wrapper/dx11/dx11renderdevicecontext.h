#pragma once

#include <common/wrapper/renderdevicecontext.h>

#include <common/wrapper/dx11/dx11renderstatecache.h>

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
        DX11RenderDeviceContext(const GFXRenderDevice& renderDevice);
        ~DX11RenderDeviceContext();

        void SetRenderTargetView(uint32_t renderTarget, ID3D11RenderTargetView* renderTargetView);
        void SetDepthStencilView(ID3D11DepthStencilView* depthStencilView);

        void ClearRenderTarget(float red, float green, float blue, float alpha, uint32_t renderTarget);
        void ClearDepthStencil(bool clearDepth, bool clearStencil);

        void Draw(const DrawItem& drawItem, const GFXVertexBuffer& vertexBuffer, uint32_t startVertexLocation);
        void DrawIndexed(const DrawItem& drawItem, const GFXVertexBuffer& vertexBuffer, const GFXIndexBuffer& indexBuffer, uint32_t startVertexLocation, uint32_t startIndexLocation);

        void SetViewport(float topLeftX, float topLeftY, float width, float height);

    private:
        void PrepareNextDrawCalls(const DrawItem& drawItem, VertexFormatType vertexFormatType);

        ID3D11Device* m_Device;
        ID3D11DeviceContext* m_DeviceContext;

        ID3D11RenderTargetView* m_RenderTargets[8];
        ID3D11DepthStencilView* m_DepthStencilView;

        DX11RenderStateCache m_RenderStateCache;
    };
}