#pragma once

#include <graphics/graphicstypes.h>

#include <graphics/shader/shaderhandler.h>

#include <graphics/wrapper/wrapper.h>

#include <graphics/graphicssingleton.h>

namespace Shipyard
{
    class SHIPYARD_GRAPHICS_API FullscreenHelper : public GraphicsSingleton<FullscreenHelper>
    {
    public:
        struct FullscreenQuadRenderContext
        {
            ShaderKey shaderKey;
            GFXRenderTargetHandle gfxRenderTargetHandle = { InvalidGfxHandle };
            GFXDepthStencilRenderTargetHandle gfxDepthStencilRenderTargetHandle = { InvalidGfxHandle };
            Array<ShaderInputProvider*> shaderInputProviders;

            // X & Y start on top-left. Values are relative to the used render target's size.
            shipFloat relativeScreenX = 0.0f;
            shipFloat relativeScreenY = 0.0f;
            shipFloat relativeScreenWidth = 1.0f;
            shipFloat relativeScreenHeight = 1.0f;
        };

    public:
        ~FullscreenHelper();

        shipBool CreateResources(GFXRenderDevice& gfxRenderDevice);
        void DestroyResources(GFXRenderDevice& gfxRenderDevice);

        void DrawFullscreenQuad(GFXRenderDevice& gfxRenderDevice, GFXDirectRenderCommandList& gfxRenderCommandList, const FullscreenQuadRenderContext& renderContext);

    private:
        GFXVertexBufferHandle m_FullscreenQuadVertexBufferHandle;
        GFXIndexBufferHandle m_FullscreenQuadIndexBufferHandle;
    };

    SHIPYARD_GRAPHICS_API FullscreenHelper& GetFullscreenHelper();
}