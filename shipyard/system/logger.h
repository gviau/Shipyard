#pragma once

#include <system/singleton.h>

#include <system/string.h>

#include <fstream>

#include <mutex>

namespace Shipyard
{
    enum LogLevel : uint8_t
    {
        LogLevel_Debug = 0x01,
        LogLevel_Info = 0x02,
        LogLevel_Warning = 0x04,
        LogLevel_Error = 0x08,

        LogLevel_FullLog = (LogLevel_Debug | LogLevel_Info | LogLevel_Warning | LogLevel_Error)
    };

    class SHIPYARD_API Logger : public Singleton<Logger>
    {
        friend class Singleton<Logger>;

    public:
        Logger();
        virtual ~Logger();

        bool OpenLog(const StringA& logFilename);
        void CloseLog();

        void SetLogLevel(LogLevel logLevel);

        void LogDebug(const StringA& message);
        void LogInfo(const StringA& message);
        void LogWarning(const StringA& message);
        void LogError(const StringA& message);

    private:
        std::ofstream m_LogFile;
        LogLevel m_LogLevel;

        std::mutex m_LoggerLock;
    };
}

#define SHIP_LOG_DEBUG(msg) Shipyard::Logger::GetInstance().LogDebug(msg)
#define SHIP_LOG_INFO(msg) Shipyard::Logger::GetInstance().LogInfo(msg)
#define SHIP_LOG_WARNING(msg) Shipyard::Logger::GetInstance().LogWarning(msg)
#define SHIP_LOG_ERROR(msg) Shipyard::Logger::GetInstance().LogError(msg)
