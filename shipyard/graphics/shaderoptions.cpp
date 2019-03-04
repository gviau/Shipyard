#include <graphics/shaderoptions.h>

#include <system/string.h>

namespace Shipyard
{;

#define CREATE_OPTION(option, numBits) \
    numBits,

uint8_t g_NumBitsForShaderOption[uint32_t(ShaderOption::Count)] =
{
#include <graphics/shaderoptiondefinitions.h>
};

#define CREATE_OPTION(option, numBits) \
    #option,

StringA g_ShaderOptionString[uint32_t(ShaderOption::Count)] =
{
#include <graphics/shaderoptiondefinitions.h>
};

}