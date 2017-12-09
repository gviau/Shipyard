#pragma once

#include <common/wrapper/renderdevicecontext.h>

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11RasterizerState;
struct ID3D11RenderTargetView;

namespace Shipyard
{
    class SHIPYARD_API DX11RenderDeviceContext : public RenderDeviceContext
    {
    public:
        DX11RenderDeviceContext(const GFXRenderDevice& renderDevice);
        ~DX11RenderDeviceContext();

        void SetRenderTargetView(uint32_t renderTarget, ID3D11RenderTargetView* renderTargetView);

        void ClearRenderTarget(float red, float green, float blue, float alpha, uint32_t renderTarget);
        void SetRasterizerState(const RasterizerState& rasterizerState);
        void SetViewport(float topLeftX, float topLeftY, float width, float height);

        void SetVertexShader(GFXVertexShader* vertexShader);
        void SetPixelShader(GFXPixelShader* pixelShader);

        void Draw(PrimitiveTopology primitiveTopology, const GFXVertexBuffer& vertexBuffer, uint32_t startVertexLocation);

    private:
        ID3D11Device* m_Device;
        ID3D11DeviceContext* m_ImmediateDeviceContext;

        ID3D11RasterizerState* m_RasterizerState;

        ID3D11RenderTargetView* m_RenderTargets[8];
    };
}