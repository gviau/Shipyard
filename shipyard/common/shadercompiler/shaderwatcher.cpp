#include <common/shadercompiler/shaderwatcher.h>

#include <common/shadercompiler/shadercompiler.h>

#include <cassert>

#include <windows.h>

namespace Shipyard
{;

volatile bool ShaderWatcher::m_RunShaderWatcherThread = true;

void GetModifiedFilesInDirectory(
        const String& directoryName,
        Array<ShaderWatcher::ShaderFile>& watchedShaderFiles,
        Array<ShaderCompiler::ShaderFileCompilationRequest>& modifiedFiles);

bool FileWasModified(const String& filename, uint64_t lastWriteTimestamp, Array<ShaderWatcher::ShaderFile>& watchedShaderFiles);

ShaderWatcher::ShaderWatcher()
{
    m_ShaderWatcherThread = thread(&ShaderWatcher::ShaderWatcherThreadFunction, this);
}

ShaderWatcher::~ShaderWatcher()
{
    m_RunShaderWatcherThread = false;
    m_ShaderWatcherThread.join();
}

void ShaderWatcher::ShaderWatcherThreadFunction()
{
    InplaceArray<ShaderCompiler::ShaderFileCompilationRequest, 8> modifiedFiles;

    while (m_RunShaderWatcherThread)
    {
        ShaderCompiler& shaderCompiler = ShaderCompiler::GetInstance();

        GetModifiedFilesInDirectory(shaderCompiler.GetShaderDirectoryName(), m_WatchedShaderFiles,  modifiedFiles);

        if (modifiedFiles.Empty())
        {
            continue;
        }

        shaderCompiler.RequestCompilationFromShaderFiles(modifiedFiles);

        modifiedFiles.Resize(0);
    }
}

void GetModifiedFilesInDirectory(
        const String& directoryName,
        Array<ShaderWatcher::ShaderFile>& watchedShaderFiles,
        Array<ShaderCompiler::ShaderFileCompilationRequest>& modifiedFiles)
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

        if (FileWasModified(filename, lastWriteTimestamp, watchedShaderFiles))
        {
            ShaderCompiler::ShaderFileCompilationRequest& newShaderFileCompilationRequest = modifiedFiles.Grow();
            newShaderFileCompilationRequest.shaderFilename = filename;
            newShaderFileCompilationRequest.modificationTimestamp = lastWriteTimestamp;
        }

    } while (FindNextFileA(findHandle, &findData));

    FindClose(findHandle);
}

bool FileWasModified(const String& filename, uint64_t lastWriteTimestamp, Array<ShaderWatcher::ShaderFile>& watchedShaderFiles)
{
    bool isValidShaderFile = false;

    // Filter out filenames with unsupported extensions
    if (filename.substr(filename.size() - 3) == ".fx")
    {
        isValidShaderFile = true;
    }
    else if (filename.substr(filename.size() - 5) == ".hlsl")
    {
        isValidShaderFile = true;
    }

    if (!isValidShaderFile)
    {
        return false;
    }

    uint32_t idx = 0;
    for (; idx < watchedShaderFiles.Size(); idx++)
    {
        if (watchedShaderFiles[idx].m_Filename == filename)
        {
            break;
        }
    }

    bool fileWasModified = false;

    if (idx == watchedShaderFiles.Size())
    {
        ShaderWatcher::ShaderFile& newWatchedShaderFile = watchedShaderFiles.Grow();
        newWatchedShaderFile.m_Filename = filename;
        newWatchedShaderFile.m_LastWriteTimestamp = lastWriteTimestamp;

        fileWasModified = true;
    }
    else if (lastWriteTimestamp > watchedShaderFiles[idx].m_LastWriteTimestamp)
    {
        watchedShaderFiles[idx].m_LastWriteTimestamp = lastWriteTimestamp;

        fileWasModified = true;
    }

    return fileWasModified;
}

}