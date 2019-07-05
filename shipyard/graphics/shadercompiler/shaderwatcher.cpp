#include <graphics/shadercompiler/shaderwatcher.h>

#include <graphics/shadercompiler/shadercompiler.h>

#include <graphics/shader/shaderfamilies.h>

#include <system/pathutils.h>
#include <system/systemcommon.h>

#include <algorithm>

#include <windows.h>

namespace Shipyard
{;

volatile shipBool ShaderWatcher::m_RunShaderWatcherThread = true;
extern const shipChar* g_ShaderFamilyFilenames[shipUint8(ShaderFamily::Count)];

ShaderWatcher::ShaderWatcher()
    : m_ShaderDirectoryName(".\\shaders\\")
    , m_FileToCheckContent(nullptr, nullptr)
{
    m_ShaderWatcherThread = std::thread(&ShaderWatcher::ShaderWatcherThreadFunction, this);

    m_FileToCheckContent.Reserve(4096);
}

ShaderWatcher::~ShaderWatcher()
{
    StopThread();
}

void ShaderWatcher::StopThread()
{
    m_RunShaderWatcherThread = false;

    if (m_ShaderWatcherThread.native_handle() != nullptr)
    {
        m_ShaderWatcherThread.join();
    }
}

shipUint64 ShaderWatcher::GetTimestampForShaderKey(const ShaderKey& shaderKey) const
{
    ShaderFamily shaderFamily = shaderKey.GetShaderFamily();
    const TinyInplaceStringA& shaderFamilyShaderFile = g_ShaderFamilyFilenames[shipUint8(shaderFamily)];

    size_t shaderFileSize = shaderFamilyShaderFile.Size();

    shipUint64 lastModifiedTimestamp = 0;

    m_ShaderWatcherLock.lock();

    for (const ShaderFile& watchedShaderFile : m_WatchedShaderFiles)
    {
        size_t watchedShaderFileSize = watchedShaderFile.m_Filename.Size();
        if (watchedShaderFileSize < shaderFileSize)
        {
            continue;
        }

        if (shaderFamilyShaderFile.CompareCaseInsensitive(watchedShaderFile.m_Filename) == 0)
        {
            lastModifiedTimestamp = watchedShaderFile.m_LastWriteTimestamp;
            break;
        }
    }

    m_ShaderWatcherLock.unlock();

    return lastModifiedTimestamp;
}

void ShaderWatcher::SetShaderDirectoryName(const StringT& shaderDirectoryName)
{
    m_ShaderDirectoryName = shaderDirectoryName;
}

void GetIncludeDirectives(const StringA& fileContent, Array<TinyInplaceStringT>& includeDirectives)
{
    size_t currentOffset = 0;
    size_t findPosition = 0;

    // For now, it is assumed that all #include directives are one per line, never in a comment, and with a single whitespace between the #include and the file name
    static const shipChar* searchString = "#include ";
    size_t searchStringLength = strlen(searchString);

    while (true)
    {
        findPosition = fileContent.FindIndexOfFirstCaseInsensitive(searchString, currentOffset);
        if (findPosition == StringA::InvalidIndex)
        {
            break;
        }

        SmallInplaceStringT includeFilename = "";
        shipChar currentChar = '\0';

        currentOffset = findPosition + searchStringLength;

        shipBool startCollectingFilename = false;

        do
        {
            currentChar = fileContent[currentOffset++];
            if (currentChar == '\"')
            {
                if (startCollectingFilename)
                {
                    startCollectingFilename = false;
                    break;
                }
                else
                {
                    startCollectingFilename = true;
                }
            }
            else if (startCollectingFilename)
            {
                includeFilename += currentChar;
            }

        } while (currentChar != '\n');

        if (!startCollectingFilename)
        {
            includeDirectives.Add(includeFilename);
        }
    }
}

shipBool CheckFileForHlslReference(const StringT& shaderDirectory, const StringT& filenameToCheck, const StringA& touchedHlslFilename, StringT& fileToCheckContent)
{
    FileHandler file(filenameToCheck, FileHandlerOpenFlag::FileHandlerOpenFlag_Read);
    if (!file.IsOpen())
    {
        return false;
    }

    size_t fileSize = file.Size();
    if (fileSize > fileToCheckContent.Capacity())
    {
        fileToCheckContent.Resize(0);
        fileToCheckContent.Reserve(fileSize * 2);
    }
    file.ReadWholeFile(fileToCheckContent);

    InplaceArray<TinyInplaceStringT, 32> includeDirectives;
    GetIncludeDirectives(fileToCheckContent, includeDirectives);

    for (TinyInplaceStringT& includeDirective : includeDirectives)
    {
        NormalizePath(includeDirective);
    }

    TinyInplaceStringT normalizedTouchedHlslFilename;
    NormalizePath(touchedHlslFilename, &normalizedTouchedHlslFilename);

    shipBool immediateReference = includeDirectives.Exists(normalizedTouchedHlslFilename);

    if (immediateReference)
    {
        return true;
    }

    for (const TinyInplaceStringA& includeDirective : includeDirectives)
    {
        SmallInplaceStringT includePath = shaderDirectory;
        includePath += includeDirective;

        if (CheckFileForHlslReference(shaderDirectory, includePath, touchedHlslFilename, fileToCheckContent))
        {
            return true;
        }
    }

    return false;
}

void AddShaderFamilyFilesIncludingThisHlslFile(const StringT& shaderDirectory, const StringT& fileDirectory, const StringA& hlslFilename, Array<TinyInplaceStringA>& shaderFilesToUpdate, StringT& fileToCheckContent)
{
    // For Hlsl files, we need to inspect every FX file and check whether they reference that Hlsl file. Moreoever, we also have to inspect
    // every Hlsl files to see if they reference the touched Hlsl file.

    for (shipUint32 i = 0; i < shipUint32(ShaderFamily::Count); i++)
    {
        ShaderFamily shaderFamily = ShaderFamily(i);

        // The error shader family will never be recompiled at runtime, since it's supposed to be always available as a fallback
        if (shaderFamily == ShaderFamily::Error)
        {
            continue;
        }

        SmallInplaceStringT shaderFilePath = shaderDirectory;
        shaderFilePath += g_ShaderFamilyFilenames[i];

        if (CheckFileForHlslReference(shaderDirectory, shaderFilePath, fileDirectory + hlslFilename, fileToCheckContent))
        {
            shaderFilesToUpdate.Add(g_ShaderFamilyFilenames[i]);
        }
    }
}

void UpdateFileIfModified(
        const StringT& shaderDirectory,
        const StringT& fileDirectory,
        const StringA& filename,
        shipUint64 lastWriteTimestamp,
        Array<ShaderWatcher::ShaderFile>& watchedShaderFiles,
        StringT& fileToCheckContent,
        std::mutex& shaderWatcherLock)
{
    // Filter out filenames with unsupported extensions
    shipBool isFxFile = false;
    shipBool isHlslFile = false;

    if (filename.FindIndexOfFirstCaseInsensitive(".fx", 3, filename.Size() - 3) != StringT::InvalidIndex)
    {
        isFxFile = true;
    }
    else if (filename.FindIndexOfFirstCaseInsensitive(".hlsl", 5, filename.Size() - 5) != StringT::InvalidIndex)
    {
        isHlslFile = true;
    }

    shipBool isValidShaderFile = (isFxFile || isHlslFile);
    if (!isValidShaderFile)
    {
        return;
    }

    InplaceArray<TinyInplaceStringA, 128> shaderFilesToUpdate;

    // For Hlsl file, we want to go through every shader family's file and update their timestamp if they include this Hlsl file.
    if (isHlslFile)
    {
        AddShaderFamilyFilesIncludingThisHlslFile(shaderDirectory, fileDirectory, filename, shaderFilesToUpdate, fileToCheckContent);
    }
    else
    {
        ShaderFamily shaderFamily = ShaderFamily::Count;

        for (shipUint32 i = 0; i < shipUint32(ShaderFamily::Count); i++)
        {
            if (filename.FindIndexOfFirstCaseInsensitive(g_ShaderFamilyFilenames[i], 0) != StringA::InvalidIndex)
            {
                shaderFamily = ShaderFamily(i);
                break;
            }
        }

        // The error shader family is supposed to be always compiled, so don't include it.
        shipBool isValidShaderFamilyFile = (shaderFamily != ShaderFamily::Count && shaderFamily != ShaderFamily::Error);
        if (isValidShaderFamilyFile)
        {
            shaderFilesToUpdate.Add(filename);
        }
    }

    shaderWatcherLock.lock();

    for (const StringA& shaderFileToUpdate : shaderFilesToUpdate)
    {
        shipUint32 idx = 0;
        for (; idx < watchedShaderFiles.Size(); idx++)
        {
            if (watchedShaderFiles[idx].m_Filename.EqualCaseInsensitive(shaderFileToUpdate))
            {
                break;
            }
        }

        if (idx == watchedShaderFiles.Size())
        {
            ShaderWatcher::ShaderFile& newWatchedShaderFile = watchedShaderFiles.Grow();
            newWatchedShaderFile.m_Filename = shaderFileToUpdate;
            newWatchedShaderFile.m_LastWriteTimestamp = lastWriteTimestamp;
        }
        else if (lastWriteTimestamp > watchedShaderFiles[idx].m_LastWriteTimestamp)
        {
            watchedShaderFiles[idx].m_LastWriteTimestamp = lastWriteTimestamp;
        }
    }

    shaderWatcherLock.unlock();
}

void GetModifiedFilesInDirectory(
        const SmallInplaceStringT& shaderDirectory,
        const SmallInplaceStringT& directoryName,
        Array<ShaderWatcher::ShaderFile>& watchedShaderFiles,
        StringT& fileToCheckContent,
        std::mutex& shaderWatcherLock,
        shipBool recursiveSearch)
{
    WIN32_FIND_DATAA findData;
    HANDLE findHandle = INVALID_HANDLE_VALUE;

    TinyInplaceStringA fileRegex = shaderDirectory + directoryName;
    fileRegex += '*';

    findHandle = FindFirstFileA(fileRegex.GetBuffer(), &findData);

    if (findHandle == INVALID_HANDLE_VALUE)
    {
        return;
    }

    do
    {
        if (strcmp(findData.cFileName, ".") == 0)
        {
            continue;
        }

        if (strcmp(findData.cFileName, "..") == 0)
        {
            continue;
        }

        if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) > 0 && recursiveSearch)
        {
            GetModifiedFilesInDirectory(shaderDirectory, directoryName + findData.cFileName + "\\", watchedShaderFiles, fileToCheckContent, shaderWatcherLock, recursiveSearch);
            continue;
        }

        TinyInplaceStringA filename = findData.cFileName;
        shipUint64 lastWriteTimestamp = (shipUint64(findData.ftLastWriteTime.dwHighDateTime) << 32) | shipUint64(findData.ftLastWriteTime.dwLowDateTime);

        UpdateFileIfModified(shaderDirectory, directoryName, filename, lastWriteTimestamp, watchedShaderFiles, fileToCheckContent, shaderWatcherLock);

    } while (FindNextFileA(findHandle, &findData));

    FindClose(findHandle);
}

void ShaderWatcher::ShaderWatcherThreadFunction()
{
    while (m_RunShaderWatcherThread)
    {
        Sleep(100);

        constexpr shipBool recursiveSearch = true;
        GetModifiedFilesInDirectory(m_ShaderDirectoryName, "", m_WatchedShaderFiles, m_FileToCheckContent, m_ShaderWatcherLock, recursiveSearch);
    }
}

}