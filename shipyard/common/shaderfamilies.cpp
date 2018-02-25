#include <common/shaderfamilies.h>

#include <system/string.h>

namespace Shipyard
{;

String g_ShaderFamilyFilenames[uint8_t(ShaderFamily::Count)] =
{
    "error.fx",
    "generic.fx"
};

}