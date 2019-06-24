#pragma once

#include <graphics/shader/shaderkey.h>

#include <graphics/wrapper/wrapper_common.h>

#include <graphics/wrapper/shader.h>

#include <system/platform.h>

namespace Shipyard
{
    class ShaderDatabase;
    class ShaderHandlerManager;
    class ShaderInputProvider;
    class ShaderResourceBinder;

    struct RenderStateBlock;

    struct PipelineStateObjectCreationParameters;

    class SHIPYARD_API ShaderHandler
    {
        friend class ShaderDatabase;
        friend class ShaderHandlerManager;

    public:
        ShaderHandler(ShaderKey shaderKey);
        ~ShaderHandler();

        void ApplyShader(PipelineStateObjectCreationParameters& pipelineStateObjectCreationParameters) const;

        void ApplyShaderInputProviders(
                GFXRenderDevice& gfxRenderDevice,
                GFXDirectRenderCommandList& gfxDirectRenderCommandList,
                const ShaderResourceBinder& shaderResourceBinder,
                const Array<ShaderInputProvider*>& shaderInputProviders);

        const ShaderKey& GetShaderKey() const { return m_ShaderKey; }

    private:
        ShaderKey m_ShaderKey;

        GFXVertexShaderHandle m_VertexShaderHandle;
        GFXPixelShaderHandle m_PixelShaderHandle;

        RenderStateBlock m_RenderStateBlock;
    };
}