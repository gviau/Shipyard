#include <common/shaderhandler.h>

#include <common/wrapper/wrapper.h>

namespace Shipyard
{;

ShaderHandler::ShaderHandler(ShaderKey shaderKey)
    : m_ShaderKey(shaderKey)
{
}

void ShaderHandler::ApplyShader(GFXRenderDeviceContext& gfxRenderDeviceContext)
{
    gfxRenderDeviceContext.SetVertexShader(m_VertexShader.get());
    gfxRenderDeviceContext.SetPixelShader(m_PixelShader.get());
}

}