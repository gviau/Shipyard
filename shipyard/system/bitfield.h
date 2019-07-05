#pragma once

#include <system/systemcommon.h>
#include <system/systemdebug.h>

#include <system/memory.h>

namespace Shipyard
{
#if CPU_BITS == CPU_BITS_64
#   define NUM_BITS_PER_BITFIELD_ELEMENT 64
#elif CPU_BITS == CPU_BITS_32
#   define NUM_BITS_PER_BITFIELD_ELEMENT 32
#endif // #if CPU_BITS == CPU_BITS_64

    template <shipUint32 NumBits, size_t alignment = SHIP_CACHE_LINE_SIZE>
    class Bitfield
    {
    public:

#if CPU_BITS == CPU_BITS_64
        using BitfieldType = shipUint64;
#elif CPU_BITS == CPU_BITS_32
        using BitfieldType = shipUint32;
#endif // #if CPU_BITS == CPU_BITS_64

        static constexpr shipUint32 ms_NumElements = ((NumBits + NUM_BITS_PER_BITFIELD_ELEMENT - 1) / NUM_BITS_PER_BITFIELD_ELEMENT);

    public:
        Bitfield(BaseAllocator* pAllocator = nullptr)
            : m_pAllocator(pAllocator)
            , m_BitField(nullptr)
            , m_MemoryOwned(false)
        {
            if (pAllocator == nullptr)
            {
                m_pAllocator = &GetGlobalAllocator();
            }
        }

        ~Bitfield()
        {
            if (m_MemoryOwned)
            {
                SHIP_FREE_EX(m_pAllocator, m_BitField);
            }
        }

        shipBool Create(shipBool setAllBits = false)
        {
            size_t requiredSize = sizeof(BitfieldType) * ms_NumElements;

            m_BitField = reinterpret_cast<BitfieldType*>(SHIP_ALLOC_EX(m_pAllocator, requiredSize, alignment));
            if (m_BitField == nullptr)
            {
                return false;
            }

            m_MemoryOwned = true;

            if (setAllBits)
            {
                SetAllBits();
            }
            else
            {
                Clear();
            }

            return true;
        }

        void Clear()
        {
            for (shipUint32 i = 0; i < ms_NumElements; i++)
            {
                m_BitField[i] = 0;
            }
        }

        shipBool IsClear() const
        {
            for (shipUint32 i = 0; i < ms_NumElements; i++)
            {
                if (m_BitField[i] != 0)
                {
                    return false;
                }
            }

            return true;
        }

        void SetBit(shipUint32 bitIndex)
        {
            SHIP_ASSERT(bitIndex < NumBits);

            shipUint32 elementIndex = bitIndex / NUM_BITS_PER_BITFIELD_ELEMENT;
            shipUint32 bit = bitIndex % NUM_BITS_PER_BITFIELD_ELEMENT;

            m_BitField[elementIndex] |= (BitfieldType(1) << BitfieldType(bit));
        }

        void UnsetBit(shipUint32 bitIndex)
        {
            SHIP_ASSERT(bitIndex < NumBits);

            shipUint32 elementIndex = bitIndex / NUM_BITS_PER_BITFIELD_ELEMENT;
            shipUint32 bit = bitIndex % NUM_BITS_PER_BITFIELD_ELEMENT;

            m_BitField[elementIndex] &= ~(BitfieldType(1) << BitfieldType(bit));
        }

        shipBool IsBitSet(shipUint32 bitIndex) const
        {
            SHIP_ASSERT(bitIndex < NumBits);

            shipUint32 elementIndex = bitIndex / NUM_BITS_PER_BITFIELD_ELEMENT;
            shipUint32 bit = bitIndex % NUM_BITS_PER_BITFIELD_ELEMENT;

            return ((m_BitField[elementIndex] & (BitfieldType(1) << BitfieldType(bit))) > 0);
        }

        void SetRange(shipUint32 startingBitIndexInclusive, shipUint32 endingBitIndexInclusive)
        {
            SHIP_ASSERT(startingBitIndexInclusive < NumBits);
            SHIP_ASSERT(endingBitIndexInclusive < NumBits);

            shipUint32 startingElementIndex = startingBitIndexInclusive / NUM_BITS_PER_BITFIELD_ELEMENT;
            shipUint32 endingElementIndex = endingBitIndexInclusive / NUM_BITS_PER_BITFIELD_ELEMENT;

            shipBool rangeInSingleElement = (startingBitIndexInclusive == endingElementIndex);
            if (rangeInSingleElement)
            {
                BitfieldType maskOfBitsSetBeforeStart = ((BitfieldType(1) << BitfieldType(startingBitIndexInclusive)) - 1);

                BitfieldType bitfieldAtEndingBit = (BitfieldType(1) << BitfieldType(endingBitIndexInclusive));
                BitfieldType maskOfBitsSetBeforeEnd = ((bitfieldAtEndingBit - 1) | bitfieldAtEndingBit);

                BitfieldType maskOfBitsToSet = (maskOfBitsSetBeforeStart ^ maskOfBitsSetBeforeEnd);

                m_BitField[startingElementIndex] |= maskOfBitsToSet;
            }
            else
            {
                shipUint32 elementIndex = startingElementIndex;

                BitfieldType maskOfBitsToSet = ~((BitfieldType(1) << BitfieldType(startingBitIndexInclusive)) - 1);
                m_BitField[elementIndex] |= maskOfBitsToSet;

                elementIndex += 1;

                for (; elementIndex <= endingElementIndex; elementIndex++)
                {
                    shipBool setFullRange = (elementIndex < endingElementIndex);

                    if (setFullRange)
                    {
                        m_BitField[elementIndex] |= BitfieldType(-1);
                    }
                    else
                    {
                        BitfieldType bitfieldAtEndingBit = (BitfieldType(1) << BitfieldType(endingBitIndexInclusive));
                        maskOfBitsToSet = ((bitfieldAtEndingBit - 1) | bitfieldAtEndingBit);

                        m_BitField[elementIndex] |= maskOfBitsToSet;
                    }
                }
            }
        }

        void UnsetRange(shipUint32 startingBitIndexInclusive, shipUint32 endingBitIndexInclusive)
        {
            SHIP_ASSERT(startingBitIndexInclusive < NumBits);
            SHIP_ASSERT(endingBitIndexInclusive < NumBits);

            shipUint32 startingElementIndex = startingBitIndexInclusive / NUM_BITS_PER_BITFIELD_ELEMENT;
            shipUint32 endingElementIndex = endingBitIndexInclusive / NUM_BITS_PER_BITFIELD_ELEMENT;

            shipBool rangeInSingleElement = (startingBitIndexInclusive == endingElementIndex);
            if (rangeInSingleElement)
            {
                BitfieldType maskOfBitsSetBeforeStart = ((BitfieldType(1) << BitfieldType(startingBitIndexInclusive)) - 1);

                BitfieldType bitfieldAtEndingBit = (BitfieldType(1) << BitfieldType(endingBitIndexInclusive));
                BitfieldType maskOfBitsSetBeforeEnd = ((bitfieldAtEndingBit - 1) | bitfieldAtEndingBit);;

                BitfieldType maskOfBitsToSet = (maskOfBitsSetBeforeStart ^ maskOfBitsSetBeforeEnd);

                m_BitField[startingElementIndex] &= ~maskOfBitsToSet;
            }
            else
            {
                shipUint32 elementIndex = startingElementIndex;

                BitfieldType maskOfBitsToSet = ~((BitfieldType(1) << BitfieldType(startingBitIndexInclusive)) - 1);
                m_BitField[elementIndex] &= ~maskOfBitsToSet;

                elementIndex += 1;

                for (; elementIndex <= endingElementIndex; elementIndex++)
                {
                    shipBool unsetFullRange = (elementIndex < endingElementIndex);
                    if (unsetFullRange)
                    {
                        m_BitField[elementIndex] &= 0;
                    }
                    else
                    {
                        BitfieldType bitfieldAtEndingBit = (BitfieldType(1) << BitfieldType(endingBitIndexInclusive));
                        maskOfBitsToSet = ((bitfieldAtEndingBit - 1) | bitfieldAtEndingBit);

                        m_BitField[elementIndex] &= ~maskOfBitsToSet;
                    }
                }
            }
        }

        // Finds the index of the first bit set, starting from the specified bit index.
        // Returns false if a bit set to 1 after the starting bit index, including it, couldn't be found.
        shipBool GetFirstBitSet(shipUint32 startingBitIndex, shipUint32& firstBitSet) const
        {
            SHIP_ASSERT(startingBitIndex < NumBits);

            shipUint32 elementIndex = startingBitIndex / NUM_BITS_PER_BITFIELD_ELEMENT;
            
            BitfieldType maskOfEveryBitsBeforeCleared = ~((BitfieldType(1) << BitfieldType(startingBitIndex)) - 1);

            unsigned long bitScanResult = (unsigned long)startingBitIndex;

#if COMPILER == COMPILER_MSVC

            for (; elementIndex < ms_NumElements; elementIndex++)
            {
                BitfieldType maskForScan = (m_BitField[elementIndex] & maskOfEveryBitsBeforeCleared);

                unsigned char foundValue = FindFirstBitSet(&bitScanResult, maskForScan);

                shipBool foundBitSet = (foundValue != 0);

                if (foundBitSet)
                {
                    firstBitSet = shipUint32(bitScanResult) + elementIndex * NUM_BITS_PER_BITFIELD_ELEMENT;
                    return true;
                }
                else
                {
                    maskOfEveryBitsBeforeCleared = BitfieldType(-1);
                }
            }
#else

#pragma error "Implement using intrinsic for bit scanning for specific compiler"

#endif // #if COMPILER == COMPILER_MSVC

            return false;
        }

        // Basically counts the number of bits set to 1 starting at the bit representing the given starting bit index.
        // The starting bit is assumed to be set to 1 in the bitfield.
        shipUint32 GetLongestRangeWithBitsSet(shipUint32 startingBitIndex) const
        {
            SHIP_ASSERT(startingBitIndex < NumBits);

            shipUint32 elementIndex = startingBitIndex / NUM_BITS_PER_BITFIELD_ELEMENT;
            
            BitfieldType maskOfEveryBitsBeforeSet = ((BitfieldType(1) << BitfieldType(startingBitIndex)) - 1);

            shipUint32 numBitsSet = 0;
            shipUint32 bitIndexInFirstElementToSearch = startingBitIndex;

#if COMPILER == COMPILER_MSVC
            for (; elementIndex < ms_NumElements; elementIndex++)
            {
                // Here, we reverse our bitfield, and scan for the first bit set to 1. This will be our first bit set to 0, which
                // will tell us the end of our continous range of bits set to 1.
                BitfieldType maskForScan = ~(m_BitField[elementIndex] | maskOfEveryBitsBeforeSet);

                unsigned long firstBitSet = 0;
                unsigned char foundValue = FindFirstBitSet(&firstBitSet, maskForScan);

                shipBool foundBitSet = (foundValue != 0);

                if (foundBitSet)
                {
                    numBitsSet = shipUint32(firstBitSet) + NUM_BITS_PER_BITFIELD_ELEMENT * elementIndex - bitIndexInFirstElementToSearch;
                    break;
                }
                else
                {
                    maskOfEveryBitsBeforeSet = 0;
                    numBitsSet += (NUM_BITS_PER_BITFIELD_ELEMENT - startingBitIndex);

                    startingBitIndex = 0;
                }
            }
#else

#pragma error "Implement using intrinsic for bit counting for specific compiler"

#endif // #if COMPILER == COMPILER_MSVC

            return numBitsSet;
        }

        void SetAllBits()
        {
            shipUint32 remainingBitsToSet = (NumBits - (ms_NumElements - 1) * NUM_BITS_PER_BITFIELD_ELEMENT);

            for (shipUint32 elementIndex = 0; elementIndex < ms_NumElements; elementIndex++)
            {
                if (elementIndex != (ms_NumElements - 1))
                {
                    m_BitField[elementIndex] = BitfieldType(-1);
                }
                else
                {
                    if (remainingBitsToSet == NUM_BITS_PER_BITFIELD_ELEMENT)
                    {
                        m_BitField[elementIndex] = BitfieldType(-1);
                    }
                    else
                    {
                        m_BitField[elementIndex] = ((BitfieldType(1) << remainingBitsToSet) - 1);
                    }
                }
            }
        }

        // Assumed to be at least sizeof(BitfieldType) * ms_NumElements bytes.
        void SetUserPointer(BitfieldType* pUserPtr)
        {
            if (m_MemoryOwned)
            {
                SHIP_FREE_EX(m_pAllocator, m_BitField);
            }

            m_BitField = pUserPtr;

            m_MemoryOwned = false;
        }

        void SetAllocator(BaseAllocator* pAllocator)
        {
            if (pAllocator == m_pAllocator)
            {
                return;
            }

            if (m_MemoryOwned)
            {
                size_t requiredSize = sizeof(BitfieldType) * ms_NumElements;
                BitfieldType* pNewBitfield = reinterpret_cast<BitfieldType*>(SHIP_ALLOC_EX(pAllocator, requiredSize, alignment));

                for (shipUint32 i = 0; i < ms_NumElements; i++)
                {
                    pNewBitfield[i] = m_BitField[i];
                }

                SHIP_FREE_EX(m_pAllocator, m_BitField);

                m_BitField = pNewBitfield;
            }

            m_pAllocator = pAllocator;
        }

    private:
        BaseAllocator* m_pAllocator;
        BitfieldType* m_BitField;
        shipBool m_MemoryOwned;
    };

    template <shipUint32 NumBits, size_t alignment = SHIP_CACHE_LINE_SIZE>
    class InplaceBitfield : public Bitfield<NumBits, alignment>
    {
    public:
        InplaceBitfield(shipBool setAllBits = false)
        {
            this->SetUserPointer(m_StackBitfield);

            if (setAllBits)
            {
                SetAllBits();
            }
            else
            {
                Clear();
            }
        }

    private:
        BitfieldType m_StackBitfield[ms_NumElements];
    };
}