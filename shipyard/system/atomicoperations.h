#pragma once

#include <system/platform.h>

#include <system/systemdebug.h>

#include <system/memory/memoryutils.h>

#if PLATFORM == PLATFORM_WINDOWS
#include <winnt.h>
#else
#error "Unsupported platform"
#endif // #if PLATFORM == PLATFORM_WINDOWS

namespace Shipyard
{
#if PLATFORM == PLATFORM_WINDOWS

    template <typename T, size_t typeSize = sizeof(T)>
    struct AtomicOperations
    {
    };

    template <typename T>
    struct AtomicOperations<T, 4>
    {
        // Returns the incremented value
        static T Increment(volatile T& val)
        {
            SHIP_ASSERT_MSG(MemoryUtils::IsAddressAligned(reinterpret_cast<size_t>(&val), 4), "Address of value must be aligned to 4 bytes boundary in AtomicOperations::Increment!");

            return InterlockedIncrement(reinterpret_cast<volatile long*>(&val));
        }

        // Returns the decremented value
        static T Decrement(volatile T& val)
        {
            SHIP_ASSERT_MSG(MemoryUtils::IsAddressAligned(reinterpret_cast<size_t>(&val), 4), "Address of value must be aligned to 4 bytes boundary in AtomicOperations::Decrement!");

            return InterlockedDecrement(reinterpret_cast<volatile long*>(&val));
        }

        // Equivalent to:
        // T oldDest = dest;
        // if (dest == comperand) dest = exchangeValue;
        // return oldDest;
        static T CompareExchange(volatile T& dest, T exchangeValue, T comperand)
        {
            SHIP_ASSERT_MSG(MemoryUtils::IsAddressAligned(reinterpret_cast<size_t>(&dest), 4), "Address of value must be aligned to 4 bytes boundary in AtomicOperations::CompareExchange!");

            return InterlockedCompareExchange(
                    reinterpret_cast<volatile long*>(&dest),
                    static_cast<const long>(exchangeValue),
                    static_cast<const long>(comperand));
        }

        // Returns the initial value of target
        static T Exchange(volatile T& target, T value)
        {
            SHIP_ASSERT_MSG(MemoryUtils::IsAddressAligned(reinterpret_cast<size_t>(&target), 4), "Address of value must be aligned to 4 bytes boundary in AtomicOperations::Exchange!");

            return InterlockedExchange(reinterpret_cast<volatile long*>(&dest), static_cast<const __int64>(value));
        }

        // Returns the initial value of target
        static T Add(volatile T& target, T value)
        {
            SHIP_ASSERT_MSG(MemoryUtils::IsAddressAligned(reinterpret_cast<size_t>(&target), 4), "Address of value must be aligned to 4 bytes boundary in AtomicOperations::Add!");

            return InterlockedExchangeAdd(reinterpret_cast<volatile long*>(&dest), static_cast<const __int64>(value));
        }

        // Returns the initial value of target
        static T Subtract(volatile T& target, T value)
        {
            SHIP_ASSERT_MSG(MemoryUtils::IsAddressAligned(reinterpret_cast<size_t>(&target), 4), "Address of value must be aligned to 4 bytes boundary in AtomicOperations::Subtract!");

            return InterlockedExchangeAdd(reinterpret_cast<volatile long*>(&dest), -static_cast<const __int64>(value));
        }
    };

    template <typename T>
    struct AtomicOperations<T, 8>
    {
        // Returns the incremented value
        static T Increment(volatile T& val)
        {
            SHIP_ASSERT_MSG(MemoryUtils::IsAddressAligned(reinterpret_cast<size_t>(&val), 8), "Address of value must be aligned to 8 bytes boundary in AtomicOperations::Increment!");

            return InterlockedIncrement64(reinterpret_cast<volatile __int64*>(&val));
        }

        // Returns the decremented value
        static T Decrement(volatile T& val)
        {
            SHIP_ASSERT_MSG(MemoryUtils::IsAddressAligned(reinterpret_cast<size_t>(&val), 8), "Address of value must be aligned to 8 bytes boundary in AtomicOperations::Decrement!");

            return InterlockedDecrement64(reinterpret_cast<volatile __int64*>(&val));
        }

        // Equivalent to:
        // T oldDest = dest;
        // if (dest == comperand) dest = exchangeValue;
        // return oldDest;
        static T CompareExchange(volatile T& dest, T exchangeValue, T comperand)
        {
            SHIP_ASSERT_MSG(MemoryUtils::IsAddressAligned(reinterpret_cast<size_t>(&dest), 8), "Address of value must be aligned to 8 bytes boundary in AtomicOperations::CompareExchange!");

            return InterlockedCompareExchange64(
                    reinterpret_cast<volatile __int64*>(&dest),
                    static_cast<const __int64>(exchangeValue),
                    static_cast<const __int64>(comperand));
        }

        // Returns the initial value of target
        static T Exchange(volatile T& target, T value)
        {
            SHIP_ASSERT_MSG(MemoryUtils::IsAddressAligned(reinterpret_cast<size_t>(&target), 8), "Address of value must be aligned to 8 bytes boundary in AtomicOperations::Exchange!");

            return InterlockedExchange64(reinterpret_cast<volatile __int64*>(&dest), static_cast<const __int64>(value));
        }

        // Returns the initial value of target
        static T Add(volatile T& target, T value)
        {
            SHIP_ASSERT_MSG(MemoryUtils::IsAddressAligned(reinterpret_cast<size_t>(&target), 8), "Address of value must be aligned to 8 bytes boundary in AtomicOperations::Add!");

            return InterlockedExchangeAdd64(reinterpret_cast<volatile __int64*>(&dest), static_cast<const __int64>(value));
        }

        // Returns the initial value of target
        static T Subtract(volatile T& target, T value)
        {
            SHIP_ASSERT_MSG(MemoryUtils::IsAddressAligned(reinterpret_cast<size_t>(&target), 8), "Address of value must be aligned to 8 bytes boundary in AtomicOperations::Subtract!");

            return InterlockedExchangeAdd64(reinterpret_cast<volatile __int64*>(&dest), - static_cast<const __int64>(value));
        }
    };

#else
#error "Unsupported platform"
#endif // #if PLATFORM == PLATFORM_WINDOWS
}