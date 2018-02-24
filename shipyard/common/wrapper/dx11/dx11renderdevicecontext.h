#pragma once

#include <common/wrapper/renderdevicecontext.h>

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11DepthStencilState;
struct ID3D11RasterizerState;
struct ID3D11DepthStencilView;
struct ID3D11RenderTargetView;

namespace Shipyard
{
    class SHIPYARD_API DX11RenderDeviceContext : public RenderDeviceContext
    {
    public:
        DX11RenderDeviceContext(const GFXRenderDevice& renderDevice);
        ~DX11RenderDeviceContext();

        void SetRenderTargetView(uint32_t renderTarget, ID3D11RenderTargetView* renderTargetView);
        void SetDepthStencilView(ID3D11DepthStencilView* depthStencilView);

        void ClearRenderTarget(float red, float green, float blue, float alpha, uint32_t renderTarget);
        void ClearDepthStencil(bool clearDepth, bool clearStencil);

        void SetRasterizerState(const RasterizerState& rasterizerState);
        void SetDepthStencilState(const DepthStencilState& depthStencilState, uint8_t stencilRef);
        void SetViewport(float topLeftX, float topLeftY, float width, float height);

        void SetVertexShader(GFXVertexShader* vertexShader);
        void SetPixelShader(GFXPixelShader* pixelShader);

        void SetVertexShaderConstantBuffer(GFXConstantBuffer* constantBuffer, uint32_t slot);
        void SetPixelShaderConstantBuffer(GFXConstantBuffer* constantBuffer, uint32_t slot);
        void SetPixelShaderTexture(GFXTexture2D* texture, uint32_t slot);

        void Draw(PrimitiveTopology primitiveTopology, const GFXVertexBuffer& vertexBuffer, uint32_t startVertexLocation);
        void DrawIndexed(PrimitiveTopology primitiveTopology, const GFXVertexBuffer& vertexBuffer, const GFXIndexBuffer& indexBuffer, uint32_t startVertexLocation, uint32_t startIndexLocation);

    private:
        ID3D11Device* m_Device;
        ID3D11DeviceContext* m_ImmediateDeviceContext;

        ID3D11RasterizerState* m_RasterizerState;
        ID3D11DepthStencilState* m_DepthStencilState;

        ID3D11RenderTargetView* m_RenderTargets[8];
        ID3D11DepthStencilView* m_DepthStencilView;
    };
}