#include <graphics/shaderhandler.h>

#include <graphics/wrapper/wrapper.h>

namespace Shipyard
{;

ShaderHandler::ShaderHandler(ShaderKey shaderKey)
    : m_ShaderKey(shaderKey)
{
}

void ShaderHandler::ApplyShader(PipelineStateObjectCreationParameters& pipelineStateObjectCreationParameters) const
{
    pipelineStateObjectCreationParameters.vertexShader = m_VertexShader.get();
    pipelineStateObjectCreationParameters.pixelShader = m_PixelShader.get();

    pipelineStateObjectCreationParameters.renderStateBlock = m_RenderStateBlock;
}

}