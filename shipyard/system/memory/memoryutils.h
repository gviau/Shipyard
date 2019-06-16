#pragma once

#include <system/platform.h>

#include <cinttypes>

namespace Shipyard
{
    namespace MemoryUtils
    {
        SHIP_INLINE size_t AlignAddress(size_t address, size_t alignment)
        {
            return (address + (alignment - 1)) & (~(alignment - 1));
        }

        SHIP_INLINE bool IsAddressAligned(size_t address, size_t alignment)
        {
            return ((address & (alignment - 1)) == 0);
        }
    }
}