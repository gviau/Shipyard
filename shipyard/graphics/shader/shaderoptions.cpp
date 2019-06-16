#include <graphics/shader/shaderoptions.h>

#include <system/string.h>

namespace Shipyard
{;

#define CREATE_OPTION(option, numBits) \
    numBits,

uint8_t g_NumBitsForShaderOption[uint32_t(ShaderOption::Count)] =
{
#include <graphics/shader/shaderoptiondefinitions.h>
};

#define CREATE_OPTION(option, numBits) \
    #option,

const char* g_ShaderOptionString[uint32_t(ShaderOption::Count)] =
{
#include <graphics/shader/shaderoptiondefinitions.h>
};

}