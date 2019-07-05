#pragma once

#include <system/array.h>
#include <system/platform.h>
#include <system/string.h>
#include <system/singleton.h>

#include <graphics/shader/shaderkey.h>

#include <mutex>
#include <thread>

namespace Shipyard
{
    class SHIPYARD_API ShaderWatcher : public Singleton<ShaderWatcher>
    {
        friend class Singleton<ShaderWatcher>;

    public:
        ShaderWatcher();
        ~ShaderWatcher();

        void StopThread();

        shipUint64 GetTimestampForShaderKey(const ShaderKey& shaderKey) const;

        void SetShaderDirectoryName(const StringT& shaderDirectoryName);

    public:
        struct ShaderFile
        {
            ShaderFile()
                : m_Filename("")
                , m_LastWriteTimestamp(0)
            {}

            StringA m_Filename;
            shipUint64 m_LastWriteTimestamp;
        };

    private:
        void ShaderWatcherThreadFunction();

        std::thread m_ShaderWatcherThread;
        static volatile shipBool m_RunShaderWatcherThread;

        mutable std::mutex m_ShaderWatcherLock;

        SmallInplaceStringT m_ShaderDirectoryName;

        Array<ShaderFile> m_WatchedShaderFiles;

        StringA m_FileToCheckContent;
    };
}