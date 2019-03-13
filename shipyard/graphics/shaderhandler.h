#pragma once

#include <graphics/shaderkey.h>

#include <graphics/wrapper/wrapper_common.h>

#include <graphics/wrapper/shader.h>

#include <system/platform.h>

namespace Shipyard
{
    class ShaderDatabase;
    class ShaderHandlerManager;

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

        const ShaderKey& GetShaderKey() const { return m_ShaderKey; }

    private:
        ShaderKey m_ShaderKey;

        GFXVertexShaderHandle m_VertexShaderHandle;
        GFXPixelShaderHandle m_PixelShaderHandle;

        RenderStateBlock m_RenderStateBlock;
    };
}