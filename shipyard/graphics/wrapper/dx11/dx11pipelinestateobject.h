#pragma once

#include <graphics/wrapper/pipelinestateobject.h>

#include <graphics/graphicscommon.h>

namespace Shipyard
{
    class SHIPYARD_GRAPHICS_API DX11PipelineStateObject : public PipelineStateObject
    {
    public:
        shipBool Create(const PipelineStateObjectCreationParameters& creationParameters);
        void Destroy();

        const PipelineStateObjectCreationParameters& GetCreationParameters() const;

    private:
        PipelineStateObjectCreationParameters m_CreationParameters;
    };
}