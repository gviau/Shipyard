#pragma once

#include <system/platform.h>

namespace Shipyard
{
#define CREATE_OPTION(option, numBits) \
    ShaderOption_##option,

    enum class ShaderOption : shipUint32
    {
#include <graphics/shader/shaderoptiondefinitions.h>
        Count
    };
}