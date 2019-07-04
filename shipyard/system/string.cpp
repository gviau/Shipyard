#include "string.h"

#include <stdarg.h>

namespace Shipyard
{;

#define STRING_FORMAT_THREAD_LOCAL_BUFFER_SIZE 32768

thread_local char g_StringFormatThreadLocalBuffer[STRING_FORMAT_THREAD_LOCAL_BUFFER_SIZE];
thread_local uint32_t g_StringFormatThreadLocalBufferOffset = 0;

char* const GetStringFormatBuffer(uint32_t stringFormatBufferOffset)
{
    return &g_StringFormatThreadLocalBuffer[stringFormatBufferOffset];
}

const char* StringFormat(const char* fmt, ...)
{
    va_list argsPtr;
    va_start(argsPtr, fmt);

    constexpr int maxCount = 8192;
    char* const pBuffer = GetStringFormatBuffer(g_StringFormatThreadLocalBufferOffset);

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