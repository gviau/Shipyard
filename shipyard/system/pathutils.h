#pragma once

#include <system/platform.h>

#include <system/string.h>

namespace Shipyard
{
    namespace PathUtils
    {
        SHIPYARD_SYSTEM_API void NormalizePath(StringT& path);
        SHIPYARD_SYSTEM_API void NormalizePath(const StringT& pathToNormalize, StringT* normalizedPath);
        SHIPYARD_SYSTEM_API void GetFileDirectory(StringT& path);
        SHIPYARD_SYSTEM_API void GetFileDirectory(const StringT& path, StringT* fileDirectory);

        SHIPYARD_SYSTEM_API shipBool DoesDirectoryExists(const shipChar* path);

        // Create all missing directories in path.
        SHIPYARD_SYSTEM_API void CreateDirectories(const shipChar* path);
        SHIPYARD_SYSTEM_API void GetWorkingDirectory(StringT& workingDirectory);
    }
}