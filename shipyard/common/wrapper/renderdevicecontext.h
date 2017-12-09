#pragma once

#include <common/wrapper/wrapper_common.h>

#include <common/common.h>

namespace Shipyard
{
    class SHIPYARD_API RenderDeviceContext
    {
    public:
        RenderDeviceContext(const GFXRenderDevice& renderDevice);
        virtual ~RenderDeviceContext();

        void ClearFirstRenderTarget(float red, float green, float blue, float alpha);
        void ClearRenderTargets(float red, float green, float blue, float alpha, uint32_t start, uint32_t count);

#ifdef DEBUG_WRAPPER_INTERFACE_COMPILATION
        virtual void ClearRenderTarget(float red, float green, float blue, float alpha, uint32_t renderTarget) = 0;
        virtual void SetRasterizerState(const RasterizerState& rasterizerState) = 0;
        virtual void SetViewport(float topLeftX, float topLeftY, float width, float height) = 0;

        virtual void SetVertexShader(GFXVertexShader* vertexShader) = 0;
        virtual void SetPixelShader(GFXPixelShader* pixelShader) = 0;

        virtual void Draw(PrimitiveTopology primitiveTopology, const GFXVertexBuffer& vertexBuffer, uint32_t startVertexLocation) = 0;
        virtual void DrawIndexed(PrimitiveTopology primitiveTopology, const GFXVertexBuffer& vertexBuffer, const GFXIndexBuffer& indexBuffer, uint32_t startVertexLocation, uint32_t startIndexLocation) = 0;
#endif // #ifdef DEBUG_WRAPPER_INTERFACE_COMPILATION
    };
}