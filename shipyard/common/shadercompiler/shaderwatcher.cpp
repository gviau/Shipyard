#include <common/shadercompiler/shaderwatcher.h>

#include <common/shadercompiler/shadercompiler.h>

#include <common/shaderfamilies.h>

#include <algorithm>
#include <cassert>

#include <windows.h>

namespace Shipyard
{;

volatile bool ShaderWatcher::m_RunShaderWatcherThread = true;
extern StringA g_ShaderFamilyFilenames[uint8_t(ShaderFamily::Count)];

ShaderWatcher::ShaderWatcher()
{
    m_ShaderWatcherThread = thread(&ShaderWatcher::ShaderWatcherThreadFunction, this);
}

ShaderWatcher::~ShaderWatcher()
{
    m_RunShaderWatcherThread = false;
    m_ShaderWatcherThread.join();
}

uint64_t ShaderWatcher::GetTimestampForShaderKey(const ShaderKey& shaderKey) const
{
    ShaderFamily shaderFamily = shaderKey.GetShaderFamily();
    const StringA& shaderFamilyShaderFile = g_ShaderFamilyFilenames[uint8_t(shaderFamily)];

    size_t shaderFileSize = shaderFamilyShaderFile.Size();

    uint64_t lastModifiedTimestamp = 0;

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

void GetIncludeDirectives(const StringA& fileContent, Array<StringA>& includeDirectives)
{
    size_t currentOffset = 0;
    size_t findPosition = 0;

    // For now, it is assumed that all #include directives are one per line, never in a comment, and with a single whitespace between the #include and the file name
    static const StringA searchString = "#include ";

    while (true)
    {
        findPosition = fileContent.FindIndexOfFirstCaseInsensitive(searchString, currentOffset);
        if (findPosition == StringA::InvalidIndex)
        {
            break;
        }

        StringA includeFilename = "";
        char currentChar = '\0';

        currentOffset = findPosition + searchString.Size();

        bool startCollectingFilename = false;

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

bool CheckFileForHlslReference(const StringT& shaderDirectory, const StringA& filenameToCheck, const StringA& touchedHlslFilename)
{
    ifstream file(filenameToCheck.GetBuffer());
    if (!file.is_open())
    {
        return false;
    }

    StringA filenameToCheckContent;
    file.ignore((std::numeric_limits<std::streamsize>::max)());
    filenameToCheckContent.Resize((size_t)file.gcount());
    file.clear();
    file.seekg(0, std::ios::beg);

    file.read(&filenameToCheckContent[0], filenameToCheckContent.Size());

    Array<StringA> includeDirectives;
    GetIncludeDirectives(filenameToCheckContent, includeDirectives);

    bool immediateReference = includeDirectives.Exists(touchedHlslFilename);

    if (immediateReference)
    {
        return true;
    }

    for (const StringA& includeDirective : includeDirectives)
    {
        if (CheckFileForHlslReference(shaderDirectory, shaderDirectory + includeDirective, touchedHlslFilename))
        {
            return true;
        }
    }

    return false;
}

void AddShaderFamilyFilesIncludingThisHlslFile(const StringT& shaderDirectory, const StringA& hlslFilename, Array<StringA>& shaderFilesToUpdate)
{
    // For Hlsl files, we need to inspect every FX file and check whether they reference that Hlsl file. Moreoever, we also have to inspect
    // every Hlsl files to see if they reference the touched Hlsl file.

    for (uint32_t i = 0; i < uint32_t(ShaderFamily::Count); i++)
    {
        ShaderFamily shaderFamily = ShaderFamily(i);

        // The error shader family will never be recompiled at runtime, since it's supposed to be always available as a fallback
        if (shaderFamily == ShaderFamily::Error)
        {
            continue;
        }

        if (CheckFileForHlslReference(shaderDirectory, shaderDirectory + g_ShaderFamilyFilenames[i], hlslFilename))
        {
            shaderFilesToUpdate.Add(g_ShaderFamilyFilenames[i]);
        }
    }
}

void UpdateFileIfModified(const StringT& shaderDirectory, const StringA& filename, uint64_t lastWriteTimestamp, Array<ShaderWatcher::ShaderFile>& watchedShaderFiles, mutex& shaderWatcherLock)
{
    // Filter out filenames with unsupported extensions
    bool isFxFile = false;
    bool isHlslFile = false;

    if (filename.Substring(filename.Size() - 3, 3) == ".fx")
    {
        isFxFile = true;
    }
    else if (filename.Substring(filename.Size() - 5, 5) == ".hlsl")
    {
        isHlslFile = true;
    }

    bool isValidShaderFile = (isFxFile || isHlslFile);
    if (!isValidShaderFile)
    {
        return;
    }

    InplaceArray<StringA, 256> shaderFilesToUpdate;

    // For Hlsl file, we want to go through every shader family's file and update their timestamp if they include this Hlsl file.
    if (isHlslFile)
    {
        AddShaderFamilyFilesIncludingThisHlslFile(shaderDirectory, filename, shaderFilesToUpdate);
    }
    else
    {
        ShaderFamily shaderFamily = ShaderFamily::Count;

        for (uint32_t i = 0; i < uint32_t(ShaderFamily::Count); i++)
        {
            if (filename.FindIndexOfFirstCaseInsensitive(g_ShaderFamilyFilenames[i], 0) != StringA::InvalidIndex)
            {
                shaderFamily = ShaderFamily(i);
                break;
            }
        }

        // The error shader family is supposed to be always compiled, so don't include it.
        bool isValidShaderFamilyFile = (shaderFamily != ShaderFamily::Count && shaderFamily != ShaderFamily::Error);
        if (isValidShaderFamilyFile)
        {
            shaderFilesToUpdate.Add(filename);
        }
    }

    shaderWatcherLock.lock();

    for (const StringA& shaderFileToUpdate : shaderFilesToUpdate)
    {
        uint32_t idx = 0;
        for (; idx < watchedShaderFiles.Size(); idx++)
        {
            if (watchedShaderFiles[idx].m_Filename == shaderFileToUpdate)
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
        const StringT& directoryName,
        Array<ShaderWatcher::ShaderFile>& watchedShaderFiles,
        mutex& shaderWatcherLock)
{
    WIN32_FIND_DATAA findData;
    HANDLE findHandle = INVALID_HANDLE_VALUE;

    findHandle = FindFirstFileA((directoryName + '*').GetBuffer(), &findData);

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

        StringA filename = findData.cFileName;
        uint64_t lastWriteTimestamp = (uint64_t(findData.ftLastWriteTime.dwHighDateTime) << 32) | uint64_t(findData.ftLastWriteTime.dwLowDateTime);

        UpdateFileIfModified(directoryName, filename, lastWriteTimestamp, watchedShaderFiles, shaderWatcherLock);

    } while (FindNextFileA(findHandle, &findData));

    FindClose(findHandle);
}

void ShaderWatcher::ShaderWatcherThreadFunction()
{
    while (m_RunShaderWatcherThread)
    {
        GetModifiedFilesInDirectory(m_ShaderDirectoryName, m_WatchedShaderFiles, m_ShaderWatcherLock);
    }
}

}