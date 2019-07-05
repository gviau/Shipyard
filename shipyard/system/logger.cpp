#include <system/logger.h>

#ifdef SHIP_ENABLE_LOGGING

#include <system/systemcommon.h>

namespace Shipyard
{;

Logger::Logger()
    : m_LogLevel(LogLevel(LogLevel_Error | LogLevel_Warning))
    , m_IsLogOpen(false)
{
}

Logger::~Logger()
{
    CloseLog();
}

shipBool Logger::OpenLog(const shipChar* pLogFilename)
{
    CloseLog();

    m_LogFile.open(pLogFilename, std::ios_base::ate | std::ios_base::out);
    if (!m_LogFile.is_open())
    {
        return false;
    }

    m_IsLogOpen = true;

    return true;
}

void Logger::CloseLog()
{
    m_LogFile.close();

    m_IsLogOpen = false;
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

void Log(std::ofstream& file, const shipChar* pHeader, const shipChar* pMessage, va_list argPtr)
{
    StringA currentTime;
    GetCurrentTimeForLog(currentTime);

    constexpr int maxCount = 4096;
    shipChar pBuffer[maxCount];

    vsnprintf_s(pBuffer, maxCount, maxCount - 1, pMessage, argPtr);

    file << currentTime.GetBuffer() << pHeader << pBuffer << std::endl;

    shipChar outputBuffer[8192];
    sprintf_s(outputBuffer, "%s%s\n", pHeader, pBuffer);

    OutputDebugString(outputBuffer);
}

void Logger::LogDebug(const shipChar* pMessage, ...)
{
    if ((m_LogLevel & LogLevel::LogLevel_Debug) == 0 || !m_IsLogOpen)
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

void Logger::LogInfo(const shipChar* pMessage, ...)
{
    if ((m_LogLevel & LogLevel::LogLevel_Info) == 0 || !m_IsLogOpen)
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

void Logger::LogWarning(const shipChar* pMessage, ...)
{
    if ((m_LogLevel & LogLevel::LogLevel_Warning) == 0 || !m_IsLogOpen)
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

void Logger::LogError(const shipChar* pMessage, ...)
{
    if ((m_LogLevel & LogLevel::LogLevel_Error) == 0 || !m_IsLogOpen)
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

SHIPYARD_API Logger& GetLogger()
{
    return Logger::GetInstance();
}

}

#endif // #ifdef SHIP_ENABLE_LOGGING