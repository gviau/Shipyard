#pragma once

#ifndef SHIP_MASTER
#define SHIP_ENABLE_LOGGING
#endif // #ifndef SHIP_MASTER

#ifdef SHIP_ENABLE_LOGGING

#include <system/singleton.h>

#include <system/string.h>

#include <fstream>

#include <mutex>

#include <stdarg.h>

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

        void LogDebug(const char* pMessage, ...);
        void LogInfo(const char* pMessage, ...);
        void LogWarning(const char* pMessage, ...);
        void LogError(const char* pMessage, ...);

    private:
        std::ofstream m_LogFile;
        LogLevel m_LogLevel;

        std::mutex m_LoggerLock;
    };
}

#define SHIP_LOG_DEBUG(msg, ...) Shipyard::Logger::GetInstance().LogDebug(msg, __VA_ARGS__)
#define SHIP_LOG_INFO(msg, ...) Shipyard::Logger::GetInstance().LogInfo(msg, __VA_ARGS__)
#define SHIP_LOG_WARNING(msg, ...) Shipyard::Logger::GetInstance().LogWarning(msg, __VA_ARGS__)
#define SHIP_LOG_ERROR(msg, ...) Shipyard::Logger::GetInstance().LogError(msg, __VA_ARGS__)

#else

#define SHIP_LOG_DEBUG(msg, ...)
#define SHIP_LOG_INFO(msg, ...)
#define SHIP_LOG_WARNING(msg, ...)
#define SHIP_LOG_ERROR(msg, ...)

#endif // #ifdef SHIP_ENABLE_LOGGING