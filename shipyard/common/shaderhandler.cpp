#include <common/shaderhandler.h>

#include <common/wrapper/wrapper.h>

namespace Shipyard
{;

ShaderHandler::ShaderHandler(ShaderKey shaderKey)
    : m_ShaderKey(shaderKey)
{
}

void ShaderHandler::ApplyShader(PipelineStateObjectCreationParameters& pipelineStateObjectCreationParameters)
{
    pipelineStateObjectCreationParameters.vertexShader = m_VertexShader.get();
    pipelineStateObjectCreationParameters.pixelShader = m_PixelShader.get();
}

}