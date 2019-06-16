#include <graphics/shader/shaderhandler.h>

#include <graphics/wrapper/wrapper.h>

namespace Shipyard
{;

ShaderHandler::ShaderHandler(ShaderKey shaderKey)
    : m_ShaderKey(shaderKey)
{
}

ShaderHandler::~ShaderHandler()
{
}

void ShaderHandler::ApplyShader(PipelineStateObjectCreationParameters& pipelineStateObjectCreationParameters) const
{
    pipelineStateObjectCreationParameters.vertexShaderHandle = m_VertexShaderHandle;
    pipelineStateObjectCreationParameters.pixelShaderHandle = m_PixelShaderHandle;

    pipelineStateObjectCreationParameters.renderStateBlock = m_RenderStateBlock;
}

}