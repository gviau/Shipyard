#include <graphics/graphicsprecomp.h>

#include <graphics/wrapper/dx11/dx11pipelinestateobject.h>

namespace Shipyard
{;

shipBool DX11GraphicsPipelineStateObject::Create(const GraphicsPipelineStateObjectCreationParameters& creationParameters)
{
    memcpy(&m_CreationParameters, &creationParameters, sizeof(creationParameters));

    return true;
}

void DX11GraphicsPipelineStateObject::Destroy()
{

}

const GraphicsPipelineStateObjectCreationParameters& DX11GraphicsPipelineStateObject::GetCreationParameters() const
{
    return m_CreationParameters;
}

shipBool DX11ComputePipelineStateObject::Create(const ComputePipelineStateObjectCreationParameters& creationParameters)
{
    memcpy(&m_CreationParameters, &creationParameters, sizeof(creationParameters));

    return true;
}

void DX11ComputePipelineStateObject::Destroy()
{

}

const ComputePipelineStateObjectCreationParameters& DX11ComputePipelineStateObject::GetCreationParameters() const
{
    return m_CreationParameters;
}

}