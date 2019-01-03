#pragma once

#include <system/platform.h>

#if PLATFORM == PLATFORM_WINDOWS
#include <intrin.h>
#endif // #if PLATFORM == PLATFORM_WINDOWS

namespace Shipyard
{
    // Sone intrinsics as macros for convenience
#if COMPILER == COMPILER_MSVC
#   if CPU_BITS == CPU_BITS_64
#       define FindFirstBitSet(pOutFirstBitSet, scanMask) _BitScanForward64(pOutFirstBitSet, scanMask)
#   elif CPU_BITS == CPU_BITS_32
#       define FindFirstBitSet(pOutFirstBitSet, scanMask) _BitScanForward(pOutFirstBitSet, scanMask)
#   endif // #if CPU_BITS == CPU_BITS_64
#endif // #if COMPILER == COMPILER_MSVC
}