#include <graphics/shaderfamilies.h>

#include <system/string.h>

namespace Shipyard
{;

#define START_SHADER_KEY(shaderFamily) #shaderFamily".fx",

StringA g_ShaderFamilyFilenames[uint8_t(ShaderFamily::Count)] =
{
#include <graphics/shaderkeydefinitions.h>
};

}