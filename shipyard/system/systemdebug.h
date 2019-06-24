#pragma once

#include <system/platform.h>

#if PLATFORM == PLATFORM_WINDOWS
#include <cassert>
#include <windows.h>
#include <stdio.h>
#endif // #if PLATFORM == PLATFORM_WINDOWS

namespace Shipyard
{

#ifndef SHIP_OPTIMIZED
#define SHIP_ENABLE_ASSERTS
#endif // #ifndef SHIP_OPTIMIZED

#if PLATFORM == PLATFORM_WINDOWS
#ifdef SHIP_ENABLE_ASSERTS
#define SHIP_ASSERT(cond) \
    if (!(cond)) \
    { \
        constexpr int bufferSize = 4096; \
        char buf[bufferSize]; \
        sprintf_s(buf, bufferSize - 1, "The condition %s failed", #cond); \
        int ret = MessageBox(NULL, buf, "Shipyard assert", MB_ABORTRETRYIGNORE | MB_ICONERROR); \
        if (ret == IDABORT) \
        { \
            abort(); \
        } \
        else if (ret == IDRETRY) \
        { \
            __debugbreak(); \
        } \
    }

#define SHIP_ASSERT_MSG(cond, fmt, ...) \
    if (!(cond)) \
    { \
        constexpr int bufferSize = 4096; \
        char buf[bufferSize]; \
        int startOfMsg = sprintf_s(buf, bufferSize - 1, "The condition %s failed with message: ", #cond); \
        if (startOfMsg < bufferSize - 1) \
        { \
            sprintf_s(buf + startOfMsg, bufferSize - 1 - startOfMsg, fmt, __VA_ARGS__); \
        } \
        int ret = MessageBox(NULL, buf, "Shipyard assert", MB_ABORTRETRYIGNORE | MB_ICONERROR); \
        if (ret == IDABORT) \
        { \
            abort(); \
        } \
        else if (ret == IDRETRY) \
        { \
            __debugbreak(); \
        } \
    }
#else
#    define SHIP_ASSERT(cond) (void)(cond)
#    define SHIP_ASSERT_MSG(cond, fmt, ...) (void)(cond)
#endif // #ifdef SHIP_ENABLE_ASSERTS

#else
#error Unsupported platform
#endif // #if PLATFORM == PLATFORM_WINDOWS

#define SHIP_STATIC_ASSERT(cond) static_assert(cond, #cond)
#define SHIP_STATIC_ASSERT_MSG(cond, msg) static_assert(cond, msg)
}