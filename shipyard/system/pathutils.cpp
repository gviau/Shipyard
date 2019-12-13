#include <system/systemprecomp.h>

#include <system/pathutils.h>

#if PLATFORM == PLATFORM_WINDOWS
#include <direct.h>
#include <windows.h>
#else
#error "Unsupported platform"
#endif // #if PLATFORM == PLATFORM_WINDOWS

namespace Shipyard
{;

namespace PathUtils
{;

void NormalizePath(StringT& path)
{
    shipChar* pBuffer = path.GetWriteBuffer();

    while (*pBuffer != '\0')
    {
        if (*pBuffer == '\\')
        {
            *pBuffer = '/';
        }

        pBuffer++;
    }
}

void NormalizePath(const StringT& pathToNormalize, StringT* normalizedPath)
{
    SHIP_ASSERT(normalizedPath != nullptr);

    normalizedPath->Resize(pathToNormalize.Size());

    const shipChar* pBuffer = pathToNormalize.GetBuffer();
    shipChar* pOutBuffer = normalizedPath->GetWriteBuffer();

    while (*pBuffer != '\0')
    {
        if (*pBuffer == '\\')
        {
            *pOutBuffer = '/';
        }
        else
        {
            *pOutBuffer = *pBuffer;
        }

        pBuffer++;
        pOutBuffer++;
    }
}

void GetFileDirectory(StringT& path)
{
    size_t pathLength = path.Size();
    if (pathLength == 0)
    {
        return;
    }

    const shipChar* pBuffer = path.GetBuffer();

    size_t endOfDirectoryIndex = pathLength;

    for (size_t i = pathLength; i > 0; i--)
    {
        size_t idx = i - 1;
        if (pBuffer[idx] == '\\' || pBuffer[idx] == '/')
        {
            endOfDirectoryIndex = idx;
            break;
        }
    }

    if (endOfDirectoryIndex != pathLength)
    {
        path.Resize(endOfDirectoryIndex + 1);
    }
}

void GetFileDirectory(const StringT& path, StringT* fileDirectory)
{
    SHIP_ASSERT(fileDirectory != nullptr);

    size_t pathLength = path.Size();
    if (pathLength == 0)
    {
        return;
    }

    const shipChar* pBuffer = path.GetBuffer();

    size_t endOfDirectoryIndex = pathLength;

    for (size_t i = pathLength; i > 0; i--)
    {
        size_t idx = i - 1;
        if (pBuffer[idx] == '\\' || pBuffer[idx] == '/')
        {
            endOfDirectoryIndex = idx;
            break;
        }
    }

    if (endOfDirectoryIndex != pathLength)
    {
        *fileDirectory = path.Substring(0, endOfDirectoryIndex + 1);
    }
}

shipBool DoesDirectoryExists(const shipChar* path)
{
#if PLATFORM == PLATFORM_WINDOWS
    DWORD fileAttributes = GetFileAttributes(path);
    if (fileAttributes == INVALID_FILE_ATTRIBUTES)
    {
        return false;
    }
    
    if ((fileAttributes & FILE_ATTRIBUTE_DIRECTORY) > 0)
    {
        return true;
    }
#else
#error "Unsupported platform"
#endif // #if PLATFORM == PLATFORM_WINDOWS

    return false;
}

void CreateDirectories(const shipChar* path)
{
    StringT normalizedPath;
    PathUtils::NormalizePath(path, &normalizedPath);

    InplaceArray<StringT, 32> directories;
    StringSplit(normalizedPath.GetBuffer(), '/', directories);

    StringT currentDirectoryToCreate;
    currentDirectoryToCreate.Reserve(normalizedPath.Size());

    for (const StringT& directory : directories)
    {
        currentDirectoryToCreate += directory;

        if (!DoesDirectoryExists(currentDirectoryToCreate.GetBuffer()))
        {
#if PLATFORM == PLATFORM_WINDOWS
            CreateDirectory(currentDirectoryToCreate.GetBuffer(), NULL);
#else
#error "Unsupported platform"
#endif // #if PLATFORM == PLATFORM_WINDOWS
        }

        currentDirectoryToCreate += '/';
    }
}

void GetWorkingDirectory(StringT& workingDirectory)
{
#if PLATFORM == PLATFORM_WINDOWS
    char buffer[MAX_PATH];
    _getcwd(buffer, MAX_PATH);

    workingDirectory = buffer;
#else
#error "Unsupported platform"
#endif // #if PLATFORM == PLATFORM_WINDOWS
}

}

}