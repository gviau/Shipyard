#pragma once

#include <system/platform.h>

namespace Shipyard
{
    namespace MemoryUtils
    {
        SHIP_INLINE size_t AlignAddress(size_t address, size_t alignment)
        {
            return (address + (alignment - 1)) & (~(alignment - 1));
        }

        SHIP_INLINE shipBool IsAddressAligned(size_t address, size_t alignment)
        {
            return ((address & (alignment - 1)) == 0);
        }
    }
}