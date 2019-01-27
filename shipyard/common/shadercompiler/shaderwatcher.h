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
        ShaderWatcher();
        ~ShaderWatcher();

    public:
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

        thread m_ShaderWatcherThread;
        static volatile bool m_RunShaderWatcherThread;

        String m_ShaderDirectoryName;

        Array<ShaderFile> m_WatchedShaderFiles;
    };
}