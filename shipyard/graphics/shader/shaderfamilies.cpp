#include <graphics/shader/shaderfamilies.h>

#include <system/string.h>

namespace Shipyard
{;

#define START_SHADER_KEY(shaderFamily) #shaderFamily".fx",

const char* g_ShaderFamilyFilenames[uint8_t(ShaderFamily::Count)] =
{
#include <graphics/shader/shaderkeydefinitions.h>
};

}