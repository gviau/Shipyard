#pragma once

#include <system/array.h>
#include <system/platform.h>
#include <system/string.h>
#include <system/singleton.h>

#include <common/shaderkey.h>

#include <mutex>
#include <thread>
using namespace std;

namespace Shipyard
{
    class SHIPYARD_API ShaderWatcher : public Singleton<ShaderWatcher>
    {
        friend class Singleton<ShaderWatcher>;

    public:
        ShaderWatcher();
        ~ShaderWatcher();

        uint64_t GetTimestampForShaderKey(const ShaderKey& shaderKey) const;

        void SetShaderDirectoryName(const String& shaderDirectoryName);

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

        mutable mutex m_ShaderWatcherLock;

        String m_ShaderDirectoryName;

        Array<ShaderFile> m_WatchedShaderFiles;
    };
}