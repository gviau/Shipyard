#pragma once

#include <graphics/wrapper/pipelinestateobject.h>

#include <graphics/graphicscommon.h>

namespace Shipyard
{
    class SHIPYARD_GRAPHICS_API DX11GraphicsPipelineStateObject : public GraphicsPipelineStateObject
    {
    public:
        shipBool Create(const GraphicsPipelineStateObjectCreationParameters& creationParameters);
        void Destroy();

        const GraphicsPipelineStateObjectCreationParameters& GetCreationParameters() const;

    private:
        GraphicsPipelineStateObjectCreationParameters m_CreationParameters;
    };

    class SHIPYARD_GRAPHICS_API DX11ComputePipelineStateObject : public ComputePipelineStateObject
    {
    public:
        shipBool Create(const ComputePipelineStateObjectCreationParameters& creationParameters);
        void Destroy();

        const ComputePipelineStateObjectCreationParameters& GetCreationParameters() const;

    private:
        ComputePipelineStateObjectCreationParameters m_CreationParameters;
    };
}