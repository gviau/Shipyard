#pragma once

#include <common/wrapper/pipelinestateobject.h>

namespace Shipyard
{
    class SHIPYARD_API DX11PipelineStateObject : public PipelineStateObject
    {
    public:
        DX11PipelineStateObject(const PipelineStateObjectCreationParameters& creationParameters);

        const PipelineStateObjectCreationParameters& GetCreationParameters() const;

    private:
        PipelineStateObjectCreationParameters m_CreationParameters;
    };
}