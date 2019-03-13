#include <graphics/wrapper/dx11/dx11pipelinestateobject.h>

namespace Shipyard
{;

bool DX11PipelineStateObject::Create(const PipelineStateObjectCreationParameters& creationParameters)
{
    memcpy(&m_CreationParameters, &creationParameters, sizeof(creationParameters));

    return true;
}

void DX11PipelineStateObject::Destroy()
{

}

const PipelineStateObjectCreationParameters& DX11PipelineStateObject::GetCreationParameters() const
{
    return m_CreationParameters;
}

}