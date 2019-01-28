#include <common/shadercompiler/shaderwatcher.h>

#include <common/shadercompiler/shadercompiler.h>

#include <common/shaderfamilies.h>

#include <algorithm>
#include <cassert>

#include <windows.h>

namespace Shipyard
{;

volatile bool ShaderWatcher::m_RunShaderWatcherThread = true;
extern String g_ShaderFamilyFilenames[uint8_t(ShaderFamily::Count)];

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
    const String& shaderFamilyShaderFile = g_ShaderFamilyFilenames[uint8_t(shaderFamily)];

    String loweredFamilyShaderFile = shaderFamilyShaderFile;
    std::transform(loweredFamilyShaderFile.begin(), loweredFamilyShaderFile.end(), loweredFamilyShaderFile.begin(), tolower);

    size_t shaderFileSize = shaderFamilyShaderFile.size();

    uint64_t lastModifiedTimestamp = 0;

    m_ShaderWatcherLock.lock();

    for (const ShaderFile& watchedShaderFile : m_WatchedShaderFiles)
    {
        size_t watchedShaderFileSize = watchedShaderFile.m_Filename.size();
        if (watchedShaderFileSize < shaderFileSize)
        {
            continue;
        }

        if (loweredFamilyShaderFile == watchedShaderFile.m_Filename)
        {
            lastModifiedTimestamp = watchedShaderFile.m_LastWriteTimestamp;
            break;
        }
    }

    m_ShaderWatcherLock.unlock();

    return lastModifiedTimestamp;
}

void ShaderWatcher::SetShaderDirectoryName(const String& shaderDirectoryName)
{
    m_ShaderDirectoryName = shaderDirectoryName;
}

void GetIncludeDirectives(const String& fileContent, Array<String>& includeDirectives)
{
    size_t currentOffset = 0;
    size_t findPosition = 0;

    // For now, it is assumed that all #include directives are one per line, never in a comment, and with a single whitespace between the #include and the file name
    static const String searchString = "#include ";

    while (true)
    {
        findPosition = fileContent.find(searchString, currentOffset);
        if (findPosition == String::npos)
        {
            break;
        }

        String includeFilename = "";
        char currentChar = '\0';

        currentOffset = findPosition + searchString.length();

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

bool CheckFileForHlslReference(const String& shaderDirectory, const String& filenameToCheck, const String& touchedHlslFilename)
{
    ifstream file(filenameToCheck);
    if (!file.is_open())
    {
        return false;
    }

    String filenameToCheckContent;
    file.ignore((std::numeric_limits<std::streamsize>::max)());
    filenameToCheckContent.resize((size_t)file.gcount());
    file.clear();
    file.seekg(0, std::ios::beg);

    file.read(&filenameToCheckContent[0], filenameToCheckContent.length());

    Array<String> includeDirectives;
    GetIncludeDirectives(filenameToCheckContent, includeDirectives);

    bool immediateReference = includeDirectives.Exists(touchedHlslFilename);

    if (immediateReference)
    {
        return true;
    }

    for (const String& includeDirective : includeDirectives)
    {
        if (CheckFileForHlslReference(shaderDirectory, shaderDirectory + includeDirective, touchedHlslFilename))
        {
            return true;
        }
    }

    return false;
}

void AddShaderFamilyFilesIncludingThisHlslFile(const String& shaderDirectory, const String& hlslFilename, Array<String>& shaderFilesToUpdate)
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

        String loweredShaderFamilyFilename = g_ShaderFamilyFilenames[i];
        std::transform(loweredShaderFamilyFilename.begin(), loweredShaderFamilyFilename.end(), loweredShaderFamilyFilename.begin(), tolower);

        if (CheckFileForHlslReference(shaderDirectory, shaderDirectory + loweredShaderFamilyFilename, hlslFilename))
        {
            shaderFilesToUpdate.Add(loweredShaderFamilyFilename);
        }
    }
}

void UpdateFileIfModified(const String& shaderDirectory, const String& filename, uint64_t lastWriteTimestamp, Array<ShaderWatcher::ShaderFile>& watchedShaderFiles, mutex& shaderWatcherLock)
{
    // Filter out filenames with unsupported extensions
    bool isFxFile = false;
    bool isHlslFile = false;

    if (filename.substr(filename.size() - 3) == ".fx")
    {
        isFxFile = true;
    }
    else if (filename.substr(filename.size() - 5) == ".hlsl")
    {
        isHlslFile = true;
    }

    bool isValidShaderFile = (isFxFile || isHlslFile);
    if (!isValidShaderFile)
    {
        return;
    }

    InplaceArray<String, 256> shaderFilesToUpdate;

    String loweredShaderFileToUpdate = filename;
    std::transform(loweredShaderFileToUpdate.begin(), loweredShaderFileToUpdate.end(), loweredShaderFileToUpdate.begin(), tolower);

    // For Hlsl file, we want to go through every shader family's file and update their timestamp if they include this Hlsl file.
    if (isHlslFile)
    {
        AddShaderFamilyFilesIncludingThisHlslFile(shaderDirectory, loweredShaderFileToUpdate, shaderFilesToUpdate);
    }
    else
    {
        ShaderFamily shaderFamily = ShaderFamily::Count;

        for (uint32_t i = 0; i < uint32_t(ShaderFamily::Count); i++)
        {
            String loweredShaderFamilyFile = g_ShaderFamilyFilenames[i];
            std::transform(loweredShaderFamilyFile.begin(), loweredShaderFamilyFile.end(), loweredShaderFamilyFile.begin(), tolower);

            if (loweredShaderFileToUpdate == loweredShaderFamilyFile)
            {
                shaderFamily = ShaderFamily(i);
                break;
            }
        }

        // The error shader family is supposed to be always compiled, so don't include it.
        bool isValidShaderFamilyFile = (shaderFamily != ShaderFamily::Count && shaderFamily != ShaderFamily::Error);
        if (isValidShaderFamilyFile)
        {
            shaderFilesToUpdate.Add(loweredShaderFileToUpdate);
        }
    }

    shaderWatcherLock.lock();

    for (const String& shaderFileToUpdate : shaderFilesToUpdate)
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
        const String& directoryName,
        Array<ShaderWatcher::ShaderFile>& watchedShaderFiles,
        mutex& shaderWatcherLock)
{
    WIN32_FIND_DATAA findData;
    HANDLE findHandle = INVALID_HANDLE_VALUE;

    findHandle = FindFirstFileA((directoryName + '*').c_str(), &findData);

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

        String filename = findData.cFileName;
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