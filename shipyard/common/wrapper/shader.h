#pragma once

#include <common/wrapper/wrapper_common.h>

#include <system/string.h>

namespace Shipyard
{
    class SHIPYARD_API BaseShader
    {
    public:
        virtual ~BaseShader();

#ifdef DEBUG_WRAPPER_INTERFACE_COMPILATION
#endif // #ifdef DEBUG_WRAPPER_INTERFACE_COMPILATION
    };

    class SHIPYARD_API VertexShader
    {
    public:
        VertexShader(void* shaderData, uint64_t shaderDataSize);
    };

    class SHIPYARD_API PixelShader
    {
    public:
        PixelShader(void* shaderData, uint64_t shaderDataSize);
    };
}