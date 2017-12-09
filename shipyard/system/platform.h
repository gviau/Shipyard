#pragma once

// This file is heavely inspired from Ogre3D's OgrePlatform.h file and
// OgrePlatformInformation.h files. Big thanks to them.
namespace Shipyard
{
#define PLATFORM_WIN32 1
#define PLATFORM_LINUX 2

#define COMPILER_MSVC 1
#define COMPILER_GNUC 2

    // Compiler
#if defined(_MSC_VER)
#   define COMPILER COMPILER_MSVC
#elif defined(__GNUC__)
#   define COMPILER COMPILER_GNUC
#else
#   pragma error "Unsupported compiler!"
#endif

    // Platform
#if defined(__WIN32__) || defined(_WIN32)
#   define PLATFORM PLATFORM_WIN32
#else
#   define PLATFORM PLATFORM_LINUX
#endif

    // DLL stuff
#if !defined(SHIPYARD_BUILD_STATIC)
#   if PLATFORM == PLATFORM_WIN32
#       define SHIPYARD_API_EXPORT __declspec(dllexport)
#       define SHIPYARD_API_IMPORT __declspec(dllimport)

    // TODO
    // Might want to properly investigate this so that we don't disable the warning
#       if COMPILER == COMPILER_MSVC
#           pragma warning(disable: 4251)
#       endif
#   elif COMPILER == COMPILER_GNUC
#       define SHIPYARD_API_EXPORT __attribute__ ((__visibility__ ("default")))
#       define SHIPYARD_API_IMPORT __attribute__ ((__visibility__ ("default")))
#   else
#       define SHIPYARD_API_EXPORT
#       define SHIPYARD_API_IMPORT
#   endif
#else
#   define SHIPYARD_API_EXPORT
#   define SHIPYARD_API_IMPORT
#endif

#if defined(SHIPYARD_NONCLIENT_BUILD)
#   define SHIPYARD_API SHIPYARD_API_EXPORT
#else
#   define SHIPYARD_API SHIPYARD_API_IMPORT
#endif
}