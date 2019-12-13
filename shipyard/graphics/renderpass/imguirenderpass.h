#pragma once

#include <graphics/renderpass/renderpass.h>

#include <graphics/shader/shaderinputprovider.h>

#include <math/mathtypes.h>

namespace Shipyard
{
    struct ImGuiShaderInputProvider : public BaseShaderInputProvider<ImGuiShaderInputProvider>
    {
        shipMat4x4 OrthographicProjectionMatrix;
        GFXTexture2DHandle ImGuiTexture;
    };

    class SHIPYARD_GRAPHICS_API ImGuiRenderPass : public RenderPass
    {
    public:
        void Execute(RenderContext& renderContext) override;

    private:
        ImGuiShaderInputProvider m_ImGuiShaderInputProvider;
    };

    DEFINE_RENDER_PASS_HANDLE(ImGuiRenderPass);
}