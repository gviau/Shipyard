#pragma once

#include <common/shaderkey.h>

#include <common/wrapper/wrapper_common.h>

#include <system/platform.h>

#include <memory>
using namespace std;

namespace Shipyard
{
    class ShaderDatabase;
    class ShaderHandlerManager;

    struct PipelineStateObjectCreationParameters;

    class SHIPYARD_API ShaderHandler
    {
        friend class ShaderDatabase;
        friend class ShaderHandlerManager;

    public:
        ShaderHandler(ShaderKey shaderKey);

        void ApplyShader(PipelineStateObjectCreationParameters& pipelineStateObjectCreationParameters);

        const ShaderKey& GetShaderKey() const { return m_ShaderKey; }

    private:
        ShaderKey m_ShaderKey;

        shared_ptr<GFXVertexShader> m_VertexShader;
        shared_ptr<GFXPixelShader> m_PixelShader;
    };
}