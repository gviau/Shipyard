#include <graphics/shader/shaderfamilies.h>

#include <system/string.h>

namespace Shipyard
{;

#define START_SHADER_KEY(shaderFamily) #shaderFamily".fx",

const shipChar* g_ShaderFamilyFilenames[shipUint8(ShaderFamily::Count)] =
{
#include <graphics/shader/shaderkeydefinitions.h>
};

}