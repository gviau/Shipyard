#pragma once

#include <common/wrapper/wrapper_common.h>

#include <system/string.h>

namespace Shipyard
{
    class BaseShader
    {
    public:
        virtual ~BaseShader();

#ifdef DEBUG_WRAPPER_INTERFACE_COMPILATION
#endif // #ifdef DEBUG_WRAPPER_INTERFACE_COMPILATION
    };

    class VertexShader
    {
    public:
        VertexShader(const String& source);
    };

    class PixelShader
    {
    public:
        PixelShader(const String& source);
    };
}