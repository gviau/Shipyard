#include <common/shadercompiler/shaderwatcher.h>

#include <common/shadercompiler/shadercompiler.h>

#include <cassert>

#include <windows.h>

namespace Shipyard
{;

volatile bool ShaderWatcher::m_RunShaderWatcherThread = true;

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
    InplaceArray<String, 8> modifiedFiles;

    while (m_RunShaderWatcherThread)
    {
        ShaderCompiler& shaderCompiler = ShaderCompiler::GetInstance();

        GetModifiedFilesInDirectory(shaderCompiler.GetShaderDirectoryName(), modifiedFiles);

        if (modifiedFiles.Empty())
        {
            continue;
        }

        shaderCompiler.RequestCompilationFromShaderFiles(modifiedFiles);

        modifiedFiles.Resize(0);
    }
}

void ShaderWatcher::GetModifiedFilesInDirectory(const String& directoryName, Array<String>& modifiedFiles)
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

        if (FileWasModified(filename, lastWriteTimestamp))
        {
            modifiedFiles.Add(filename);
        }

    } while (FindNextFileA(findHandle, &findData));

    FindClose(findHandle);
}

bool ShaderWatcher::FileWasModified(const String& filename, uint64_t lastWriteTimestamp)
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
    for (; idx < m_WatchedShaderFiles.Size(); idx++)
    {
        if (m_WatchedShaderFiles[idx].m_Filename == filename)
        {
            break;
        }
    }

    bool fileWasModified = false;

    if (idx == m_WatchedShaderFiles.Size())
    {
        ShaderFile shaderFile;
        shaderFile.m_Filename = filename;
        shaderFile.m_LastWriteTimestamp = lastWriteTimestamp;

        m_WatchedShaderFiles.Add(shaderFile);

        fileWasModified = true;
    }
    else if (lastWriteTimestamp > m_WatchedShaderFiles[idx].m_LastWriteTimestamp)
    {
        m_WatchedShaderFiles[idx].m_LastWriteTimestamp = lastWriteTimestamp;

        fileWasModified = true;
    }

    return fileWasModified;
}

}