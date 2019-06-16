#pragma once

#include <cinttypes>

namespace Shipyard
{
#define CREATE_OPTION(option, numBits) \
    ShaderOption_##option,

    enum class ShaderOption : uint32_t
    {
#include <graphics/shader/shaderoptiondefinitions.h>
        Count
    };
}