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
#endif // #ifdef DEBUG_WRAPPER_INTERFACE_COMPILATION
    };
}