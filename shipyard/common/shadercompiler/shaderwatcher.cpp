#include <common/shadercompiler/shaderwatcher.h>

#include <common/shadercompiler/shadercompiler.h>

#include <cassert>

#include <windows.h>

namespace Shipyard
{;

volatile bool ShaderWatcher::m_RunShaderWatcherThread = true;

ShaderWatcher::ShaderWatcher(ShaderCompiler* shaderCompiler, const String& shaderDirectoryName)
    : m_ShaderCompiler(shaderCompiler)
    , m_ShaderDirectoryName(shaderDirectoryName)
{
    assert(m_ShaderCompiler != nullptr);

    if (m_ShaderDirectoryName.back() != '\\')
    {
        m_ShaderDirectoryName += '\\';
    }

    if (m_ShaderDirectoryName.length() >= MAX_PATH)
    {
        MessageBox(NULL, "ShaderWatcher thread will not start, shader directory path is too long for Windows to open", "Error", MB_OK);
    }
    else
    {
        m_ShaderWatcherThread = thread(&ShaderWatcher::ShaderWatcherThreadFunction, this);
    }
}

ShaderWatcher::~ShaderWatcher()
{
    m_RunShaderWatcherThread = false;
    m_ShaderWatcherThread.join();
}

void ShaderWatcher::ShaderWatcherThreadFunction()
{
    while (m_RunShaderWatcherThread)
    {
        Array<String> modifiedFiles;
        GetModifiedFilesInDirectory(m_ShaderDirectoryName, modifiedFiles);

        if (modifiedFiles.empty())
        {
            continue;
        }

        m_ShaderCompiler->RequestCompilationFromShaderFiles(m_ShaderDirectoryName, modifiedFiles);
    }
}

void ShaderWatcher::GetModifiedFilesInDirectory(const String& directoryName, Array<String>& modifiedFiles)
{
    WIN32_FIND_DATAA findData;
    HANDLE findHandle = INVALID_HANDLE_VALUE;
    DWORD error = 0;

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
            modifiedFiles.push_back(filename);
        }

    } while (FindNextFileA(findHandle, &findData));
}

bool ShaderWatcher::FileWasModified(const String& filename, uint64_t lastWriteTimestamp)
{
    uint32_t idx = 0;
    for (; idx < m_WatchedShaderFiles.size(); idx++)
    {
        if (m_WatchedShaderFiles[idx].m_Filename == filename)
        {
            break;
        }
    }

    bool fileWasModified = false;

    if (idx == m_WatchedShaderFiles.size())
    {
        ShaderFile shaderFile;
        shaderFile.m_Filename = filename;
        shaderFile.m_LastWriteTimestamp = lastWriteTimestamp;

        m_WatchedShaderFiles.push_back(shaderFile);

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