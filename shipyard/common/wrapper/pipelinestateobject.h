#pragma once

#include <common/wrapper/wrapper_common.h>

#include <common/vertexformat.h>

namespace Shipyard
{
    class RootSignature;
    
    class SHIPYARD_API PipelineStateObject
    {
    public:
        PipelineStateObject(const PipelineStateObjectCreationParameters& creationParameters);

#ifdef DEBUG_WRAPPER_INTERFACE_COMPILATION
#endif // #ifdef DEBUG_WRAPPER_INTERFACE_COMPILATION
    };
}