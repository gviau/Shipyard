#include <graphics/graphicsprecomp.h>

#include <graphics/shader/shaderoptions.h>

#include <system/string.h>

namespace Shipyard
{;

#define CREATE_OPTION(option, numBits) \
    numBits,

shipUint8 g_NumBitsForShaderOption[shipUint32(ShaderOption::Count)] =
{
#include <graphics/shader/shaderoptiondefinitions.h>
};

#define CREATE_OPTION(option, numBits) \
    #option,

const shipChar* g_ShaderOptionString[shipUint32(ShaderOption::Count)] =
{
#include <graphics/shader/shaderoptiondefinitions.h>
};

}