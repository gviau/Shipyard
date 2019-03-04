#include <system/systemcommon.h>

#if PLATFORM == PLATFORM_WINDOWS
#include <windows.h>
#endif // #if PLATFORM == PLATFORM_WINDOWS

namespace Shipyard
{;

#if COMPILER == COMPILER_MSVC
#pragma warning( disable: 4996 )
#endif // #if COMPILER == COMPILER_MSVC

// Returns the current time in a formatted output: year-month-day-hour-minutes-seconds-milliseconds.
SHIPYARD_API void GetCurrentTimeFullyFormatted(StringA& formattedOutput)
{
#if PLATFORM == PLATFORM_WINDOWS

    SYSTEMTIME localTime;
    GetLocalTime(&localTime);

    formattedOutput.Format("%04d-%02d-%02d-%02d-%02d-%02d-%04d", localTime.wYear, localTime.wMonth, localTime.wDay, localTime.wHour, localTime.wMinute, localTime.wSecond, localTime.wMilliseconds);

#else
#error To implement
#endif // #if PLATFORM == PLATFORM_WINDOWS
}

#if COMPILER == COMPILER_MSVC
#pragma warning( default: 4996 )
#endif // #if COMPILER == COMPILER_MSVC

}