#pragma once

#include <system/array.h>
#include <system/platform.h>
#include <system/string.h>

#include <thread>
using namespace std;

namespace Shipyard
{
    class ShaderCompiler;

    class SHIPYARD_API ShaderWatcher
    {
    public:
        ShaderWatcher(const String& shaderDirectoryName);
        ~ShaderWatcher();

    private:
        struct ShaderFile
        {
            ShaderFile()
                : m_Filename("")
                , m_LastWriteTimestamp(0)
            {}

            String m_Filename;
            uint64_t m_LastWriteTimestamp;
        };

    private:
        void ShaderWatcherThreadFunction();

        void GetModifiedFilesInDirectory(const String& directoryName, Array<String>& modifiedFiles);
        bool FileWasModified(const String& filename, uint64_t lastWriteTimestamp);

        thread m_ShaderWatcherThread;
        static volatile bool m_RunShaderWatcherThread;

        String m_ShaderDirectoryName;

        Array<ShaderFile> m_WatchedShaderFiles;
    };
}