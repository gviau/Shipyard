#include <graphics/wrapper/dx11/dx11pipelinestateobject.h>

namespace Shipyard
{;

DX11PipelineStateObject::DX11PipelineStateObject(const PipelineStateObjectCreationParameters& creationParameters)
    : PipelineStateObject(creationParameters)
    , m_CreationParameters(creationParameters)
{

}

const PipelineStateObjectCreationParameters& DX11PipelineStateObject::GetCreationParameters() const
{
    return m_CreationParameters;
}

}