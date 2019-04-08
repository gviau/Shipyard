#include <system/logger.h>

#ifdef SHIP_ENABLE_LOGGING

#include <system/systemcommon.h>

namespace Shipyard
{;

Logger::Logger()
    : m_LogLevel(LogLevel(LogLevel_Error | LogLevel_Warning))
{
    OpenLog("shipyard_log.txt");
}

Logger::~Logger()
{
    CloseLog();
}

bool Logger::OpenLog(const char* pLogFilename)
{
    CloseLog();

    m_LogFile.open(pLogFilename, std::ios_base::ate | std::ios_base::out);
    if (!m_LogFile.is_open())
    {
        return false;
    }

    return true;
}

void Logger::CloseLog()
{
    m_LogFile.close();
}

void Logger::SetLogLevel(LogLevel logLevel)
{
    m_LogLevel = logLevel;
}

void GetCurrentTimeForLog(StringA& currentTimeForLog)
{
    currentTimeForLog.Reserve(32);

    GetCurrentTimeFullyFormatted(currentTimeForLog);

    currentTimeForLog += " - ";
}

void Log(std::ofstream& file, const char* pHeader, const char* pMessage, va_list argPtr)
{
    StringA currentTime;
    GetCurrentTimeForLog(currentTime);

    constexpr int maxCount = 4096;
    char pBuffer[maxCount];

    vsnprintf_s(pBuffer, maxCount, maxCount - 1, pMessage, argPtr);

    file << currentTime.GetBuffer() << pHeader << pBuffer << std::endl;

    char outputBuffer[8192];
    sprintf_s(outputBuffer, "%s%s\n", pHeader, pBuffer);

    OutputDebugString(outputBuffer);
}

void Logger::LogDebug(const char* pMessage, ...)
{
    if ((m_LogLevel & LogLevel::LogLevel_Debug) == 0)
    {
        return;
    }

    m_LoggerLock.lock();

    va_list argsPtr;
    va_start(argsPtr, pMessage);

    Log(m_LogFile, " [DEBUG]   ", pMessage, argsPtr);

    va_end(argsPtr);

    m_LoggerLock.unlock();
}

void Logger::LogInfo(const char* pMessage, ...)
{
    if ((m_LogLevel & LogLevel::LogLevel_Info) == 0)
    {
        return;
    }

    m_LoggerLock.lock();

    va_list argsPtr;
    va_start(argsPtr, pMessage);

    Log(m_LogFile, " [INFO]    ", pMessage, argsPtr);

    va_end(argsPtr);

    m_LoggerLock.unlock();
}

void Logger::LogWarning(const char* pMessage, ...)
{
    if ((m_LogLevel & LogLevel::LogLevel_Warning) == 0)
    {
        return;
    }

    m_LoggerLock.lock();

    va_list argsPtr;
    va_start(argsPtr, pMessage);

    Log(m_LogFile, " [WARNING] ", pMessage, argsPtr);

    va_end(argsPtr);

    m_LoggerLock.unlock();
}

void Logger::LogError(const char* pMessage, ...)
{
    if ((m_LogLevel & LogLevel::LogLevel_Error) == 0)
    {
        return;
    }

    m_LoggerLock.lock();

    va_list argsPtr;
    va_start(argsPtr, pMessage);

    Log(m_LogFile, " [ERROR]   ", pMessage, argsPtr);

    va_end(argsPtr);

    m_LoggerLock.unlock();
}

}

#endif // #ifdef SHIP_ENABLE_LOGGING