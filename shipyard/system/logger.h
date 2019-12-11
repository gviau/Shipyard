#pragma once

#ifndef SHIP_MASTER
#define SHIP_ENABLE_LOGGING
#endif // #ifndef SHIP_MASTER

#ifdef SHIP_ENABLE_LOGGING

#include <system/string.h>

#include <fstream>

#include <mutex>

#include <stdarg.h>

namespace Shipyard
{
    enum LogLevel : shipUint8
    {
        LogLevel_Debug = 0x01,
        LogLevel_Info = 0x02,
        LogLevel_Warning = 0x04,
        LogLevel_Error = 0x08,

        LogLevel_FullLog = (LogLevel_Debug | LogLevel_Info | LogLevel_Warning | LogLevel_Error)
    };

    class SHIPYARD_API Logger
    {
    public:
        static Logger& GetInstance()
        {
            static Logger s_Logger;
            return s_Logger;
        }

        shipBool OpenLog(const shipChar* pLogFilename);
        void CloseLog();

        void SetLogLevel(LogLevel logLevel);

        void LogDebug(const shipChar* pMessage, ...);
        void LogInfo(const shipChar* pMessage, ...);
        void LogWarning(const shipChar* pMessage, ...);
        void LogError(const shipChar* pMessage, ...);

    private:
        Logger();
        ~Logger();

        Logger(const Logger& src) = delete;
        Logger(const Logger&& src) = delete;
        Logger& operator= (const Logger& rhs) = delete;

        std::ofstream m_LogFile;
        LogLevel m_LogLevel;

        std::mutex m_LoggerLock;

        shipBool m_IsLogOpen;
    };

    SHIPYARD_API Logger& GetLogger();
}

#define SHIP_LOG_DEBUG(msg, ...) Shipyard::GetLogger().LogDebug(msg, __VA_ARGS__)
#define SHIP_LOG_INFO(msg, ...) Shipyard::GetLogger().LogInfo(msg, __VA_ARGS__)
#define SHIP_LOG_WARNING(msg, ...) Shipyard::GetLogger().LogWarning(msg, __VA_ARGS__)
#define SHIP_LOG_ERROR(msg, ...) Shipyard::GetLogger().LogError(msg, __VA_ARGS__)

#else

#define SHIP_LOG_DEBUG(msg, ...)
#define SHIP_LOG_INFO(msg, ...)
#define SHIP_LOG_WARNING(msg, ...)
#define SHIP_LOG_ERROR(msg, ...)

#endif // #ifdef SHIP_ENABLE_LOGGING