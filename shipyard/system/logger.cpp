#include <system/logger.h>

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

bool Logger::OpenLog(const StringA& logFilename)
{
    CloseLog();

    m_LogFile.open(logFilename.GetBuffer(), std::ios_base::ate | std::ios_base::out);
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

void Logger::LogDebug(const StringA& message)
{
    if ((m_LogLevel & LogLevel::LogLevel_Debug) == 0)
    {
        return;
    }

    m_LoggerLock.lock();

    StringA currentTime;
    GetCurrentTimeForLog(currentTime);

    m_LogFile << currentTime.GetBuffer() << " [DEBUG]   " << message.GetBuffer() << std::endl;

    m_LoggerLock.unlock();
}

void Logger::LogInfo(const StringA& message)
{
    if ((m_LogLevel & LogLevel::LogLevel_Info) == 0)
    {
        return;
    }

    m_LoggerLock.lock();

    StringA currentTime;
    GetCurrentTimeForLog(currentTime);

    m_LogFile << currentTime.GetBuffer() << " [INFO]    " << message.GetBuffer() << std::endl;

    m_LoggerLock.unlock();
}

void Logger::LogWarning(const StringA& message)
{
    if ((m_LogLevel & LogLevel::LogLevel_Warning) == 0)
    {
        return;
    }

    m_LoggerLock.lock();

    StringA currentTime;
    GetCurrentTimeForLog(currentTime);

    m_LogFile << currentTime.GetBuffer() << " [WARNING] " << message.GetBuffer() << std::endl;

    m_LoggerLock.unlock();
}

void Logger::LogError(const StringA& message)
{
    if ((m_LogLevel & LogLevel::LogLevel_Error) == 0)
    {
        return;
    }

    m_LoggerLock.lock();

    StringA currentTime;
    GetCurrentTimeForLog(currentTime);

    m_LogFile << currentTime.GetBuffer() << " [ERROR]   " << message.GetBuffer() << std::endl;

    m_LoggerLock.unlock();
}

}