#pragma once

#include <system/platform.h>

namespace Shipyard
{
    shipUint32 SHIPYARD_API InternalAtomicIncrement(volatile shipUint32& val);
    shipUint32 SHIPYARD_API InternalAtomicDecrement(volatile shipUint32& val);
    shipUint32 SHIPYARD_API InternalAtomicCompareExchange(volatile shipUint32& dest, shipUint32 exchangeValue, shipUint32 comperand);
    shipUint32 SHIPYARD_API InternalAtomicExchange(volatile shipUint32& target, shipUint32 value);
    shipUint32 SHIPYARD_API InternalAtomicAdd(volatile shipUint32& target, shipUint32 value);
    shipUint32 SHIPYARD_API InternalAtomicSubtract(volatile shipUint32& target, shipUint32 value);

    shipUint64 SHIPYARD_API InternalAtomicIncrement(volatile shipUint64& val);
    shipUint64 SHIPYARD_API InternalAtomicDecrement(volatile shipUint64& val);
    shipUint64 SHIPYARD_API InternalAtomicCompareExchange(volatile shipUint64& dest, shipUint64 exchangeValue, shipUint64 comperand);
    shipUint64 SHIPYARD_API InternalAtomicExchange(volatile shipUint64& target, shipUint64 value);
    shipUint64 SHIPYARD_API InternalAtomicAdd(volatile shipUint64& target, shipUint64 value);
    shipUint64 SHIPYARD_API InternalAtomicSubtract(volatile shipUint64& target, shipUint64 value);

#if PLATFORM == PLATFORM_WINDOWS

    struct AtomicOperations
    {
        // Returns the incremented value
        template <typename T>
        static T Increment(volatile T& val)
        {
            return InternalAtomicIncrement(val);
        }

        // Returns the decremented value
        template <typename T>
        static T Decrement(volatile T& val)
        {
            return InternalAtomicDecrement(val);
        }

        // Equivalent to:
        // T oldDest = dest;
        // if (dest == comperand) dest = exchangeValue;
        // return oldDest;
        template <typename T>
        static T CompareExchange(volatile T& dest, T exchangeValue, T comperand)
        {
            return InternalAtomicCompareExchange(dest, exchangeValue, comperand);
        }

        // Returns the initial value of target
        template <typename T>
        static T Exchange(volatile T& target, T value)
        {
            return InternalAtomicExchange(target, value);
        }

        // Returns the initial value of target
        template <typename T>
        static T Add(volatile T& target, T value)
        {
            return InternalAtomicAdd(target, value);
        }

        // Returns the initial value of target
        template <typename T>
        static T Subtract(volatile T& target, T value)
        {
            return InternalAtomicSubtract(target, value);
        }
    };
#else
#error "Unsupported platform"
#endif // #if PLATFORM == PLATFORM_WINDOWS
}