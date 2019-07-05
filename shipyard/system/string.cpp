#include "string.h"

#include <stdarg.h>

namespace Shipyard
{;

#define STRING_FORMAT_THREAD_LOCAL_BUFFER_SIZE 32768

thread_local shipChar g_StringFormatThreadLocalBuffer[STRING_FORMAT_THREAD_LOCAL_BUFFER_SIZE];
thread_local shipUint32 g_StringFormatThreadLocalBufferOffset = 0;

shipChar* const GetStringFormatBuffer(shipUint32 stringFormatBufferOffset)
{
    return &g_StringFormatThreadLocalBuffer[stringFormatBufferOffset];
}

const shipChar* StringFormat(const shipChar* fmt, ...)
{
    va_list argsPtr;
    va_start(argsPtr, fmt);

    constexpr int maxCount = 8192;
    shipChar* const pBuffer = GetStringFormatBuffer(g_StringFormatThreadLocalBufferOffset);

    int numChars = vsnprintf_s(pBuffer, maxCount, maxCount - 1, fmt, argsPtr);

    g_StringFormatThreadLocalBufferOffset += numChars;
    if (g_StringFormatThreadLocalBufferOffset > STRING_FORMAT_THREAD_LOCAL_BUFFER_SIZE)
    {
        g_StringFormatThreadLocalBufferOffset = 0;
    }
    
    va_end(argsPtr);

    return pBuffer;
}

}