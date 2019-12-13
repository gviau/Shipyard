#include <system/systemprecomp.h>

#include "system/atomicoperations.h"

#include <system/systemdebug.h>

#include <system/memory/memoryutils.h>

#if PLATFORM == PLATFORM_WINDOWS
#include <winnt.h>
#else
#error "Unsupported platform"
#endif // #if PLATFORM == PLATFORM_WINDOWS

namespace Shipyard
{;

shipUint32 InternalAtomicIncrement(volatile shipUint32& val)
{
    SHIP_ASSERT_MSG(MemoryUtils::IsAddressAligned(reinterpret_cast<size_t>(&val), 4), "Address of value must be aligned to 4 bytes boundary in AtomicOperations::Increment!");

    return InterlockedIncrement(reinterpret_cast<volatile long*>(&val));
}

shipUint32 InternalAtomicDecrement(volatile shipUint32& val)
{
    SHIP_ASSERT_MSG(MemoryUtils::IsAddressAligned(reinterpret_cast<size_t>(&val), 4), "Address of value must be aligned to 4 bytes boundary in AtomicOperations::Decrement!");

    return InterlockedDecrement(reinterpret_cast<volatile long*>(&val));
}

shipUint32 InternalAtomicCompareExchange(volatile shipUint32& dest, shipUint32 exchangeValue, shipUint32 comperand)
{
    SHIP_ASSERT_MSG(MemoryUtils::IsAddressAligned(reinterpret_cast<size_t>(&dest), 4), "Address of value must be aligned to 4 bytes boundary in AtomicOperations::CompareExchange!");

    return InterlockedCompareExchange(
            reinterpret_cast<volatile long*>(&dest),
            static_cast<const long>(exchangeValue),
            static_cast<const long>(comperand));
}

shipUint32 InternalAtomicExchange(volatile shipUint32& target, shipUint32 value)
{
    SHIP_ASSERT_MSG(MemoryUtils::IsAddressAligned(reinterpret_cast<size_t>(&target), 4), "Address of value must be aligned to 4 bytes boundary in AtomicOperations::Exchange!");

    return InterlockedExchange(reinterpret_cast<volatile long*>(&target), static_cast<const long>(value));
}

shipUint32 InternalAtomicAdd(volatile shipUint32& target, shipUint32 value)
{
    SHIP_ASSERT_MSG(MemoryUtils::IsAddressAligned(reinterpret_cast<size_t>(&target), 4), "Address of value must be aligned to 4 bytes boundary in AtomicOperations::Add!");

    return InterlockedExchangeAdd(reinterpret_cast<volatile long*>(&target), static_cast<const long>(value));
}

shipUint32 InternalAtomicSubtract(volatile shipUint32& target, shipUint32 value)
{
    SHIP_ASSERT_MSG(MemoryUtils::IsAddressAligned(reinterpret_cast<size_t>(&target), 4), "Address of value must be aligned to 4 bytes boundary in AtomicOperations::Subtract!");

    return InterlockedExchangeAdd(reinterpret_cast<volatile long*>(&target), - static_cast<const long>(value));
}

shipUint64 InternalAtomicIncrement(volatile shipUint64& val)
{
    SHIP_ASSERT_MSG(MemoryUtils::IsAddressAligned(reinterpret_cast<size_t>(&val), 8), "Address of value must be aligned to 8 bytes boundary in AtomicOperations::Increment!");

    return InterlockedIncrement64(reinterpret_cast<volatile __int64*>(&val));
}

shipUint64 InternalAtomicDecrement(volatile shipUint64& val)
{
    SHIP_ASSERT_MSG(MemoryUtils::IsAddressAligned(reinterpret_cast<size_t>(&val), 8), "Address of value must be aligned to 8 bytes boundary in AtomicOperations::Decrement!");

    return InterlockedDecrement64(reinterpret_cast<volatile __int64*>(&val));
}

shipUint64 InternalAtomicCompareExchange(volatile shipUint64& dest, shipUint64 exchangeValue, shipUint64 comperand)
{
    SHIP_ASSERT_MSG(MemoryUtils::IsAddressAligned(reinterpret_cast<size_t>(&dest), 8), "Address of value must be aligned to 8 bytes boundary in AtomicOperations::CompareExchange!");

    return InterlockedCompareExchange64(
            reinterpret_cast<volatile __int64*>(&dest),
            static_cast<const __int64>(exchangeValue),
            static_cast<const __int64>(comperand));
}

shipUint64 InternalAtomicExchange(volatile shipUint64& target, shipUint64 value)
{
    SHIP_ASSERT_MSG(MemoryUtils::IsAddressAligned(reinterpret_cast<size_t>(&target), 8), "Address of value must be aligned to 8 bytes boundary in AtomicOperations::Exchange!");

    return InterlockedExchange64(reinterpret_cast<volatile __int64*>(&target), static_cast<const __int64>(value));
}

shipUint64 InternalAtomicAdd(volatile shipUint64& target, shipUint64 value)
{
    SHIP_ASSERT_MSG(MemoryUtils::IsAddressAligned(reinterpret_cast<size_t>(&target), 8), "Address of value must be aligned to 8 bytes boundary in AtomicOperations::Add!");

    return InterlockedExchangeAdd64(reinterpret_cast<volatile __int64*>(&target), static_cast<const __int64>(value));
}

shipUint64 InternalAtomicSubtract(volatile shipUint64& target, shipUint64 value)
{
    SHIP_ASSERT_MSG(MemoryUtils::IsAddressAligned(reinterpret_cast<size_t>(&target), 8), "Address of value must be aligned to 8 bytes boundary in AtomicOperations::Subtract!");

    return InterlockedExchangeAdd64(reinterpret_cast<volatile __int64*>(&target), - static_cast<const __int64>(value));
}

}