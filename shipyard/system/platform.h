#pragma once

// This file is heavely inspired from Ogre3D's OgrePlatform.h file and
// OgrePlatformInformation.h files. Big thanks to them.
namespace Shipyard
{
#define PLATFORM_WINDOWS 1
#define PLATFORM_LINUX 2

#define COMPILER_MSVC 1
#define COMPILER_GNUC 2

#define CPU_BITS_32 1
#define CPU_BITS_64 2

    // Compiler
#if defined(_MSC_VER)
#   define COMPILER COMPILER_MSVC
#elif defined(__GNUC__)
#   define COMPILER COMPILER_GNUC
#else
#   pragma error "Unreconized compiler"
#endif // #if defined(_MSC_VER)

    // Platform
#if defined(WIN32) || defined(WIN64)
#   define PLATFORM PLATFORM_WINDOWS
#else
#   define PLATFORM PLATFORM_LINUX
#endif // #if defined(WIN32) || defined(WIN64)

    // CPU bits
#if COMPILER == COMPILER_MSVC

#   if defined(WIN64)
#       define CPU_BITS CPU_BITS_64
#   elif defined(WIN32)
#       define CPU_BITS CPU_BITS_32
#   endif // #if defined(WIN32)

#elif COMPILER == COMPILER_GNUC

#   if __x86_64__ || __ppc64__
#       define CPU_BITS CPU_BITS_64
#   else
#       define CPU_BITS CPU_BITS_32
#   endif // #if __x86_64__ || __ppc64__

#else
#   pragma error "Unreconized compiler"
#endif // #if COMPILER == COMPILER_MSVC

    // DLL stuff
#if !defined(SHIPYARD_BUILD_STATIC)
#   if COMPILER == COMPILER_MSVC
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
#endif // #if !defined(SHIPYARD_BUILD_STATIC)

    // Inline
#if COMPILER == COMPILER_MSVC
#   define SHIP_INLINE inline
#else
#   pragma error "Unreconized compiler"
#endif // #if COMPILER == COMPILER_MSVC

#if defined(SHIPYARD_NONCLIENT_BUILD)
#   define SHIPYARD_API SHIPYARD_API_EXPORT
#elif defined(SHIPYARD_STATIC_LINK)
#   define SHIPYARD_API
#else
#   define SHIPYARD_API SHIPYARD_API_IMPORT
#endif // #if defined(SHIPYARD_NONCLIENT_BUILD)

    // Uncomment the following to have virtual methods in each of the wrappers' base class to make sure that your implementation is valid
#define DEBUG_WRAPPER_INTERFACE_COMPILATION
}