#pragma once

#include <system/platform.h>

#include <system/string.h>

namespace Shipyard
{
    SHIPYARD_API void NormalizePath(StringT& path);
    SHIPYARD_API void NormalizePath(const StringT& pathToNormalize, StringT* normalizedPath);
}