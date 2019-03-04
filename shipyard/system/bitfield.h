#pragma once

#include <system/systemcommon.h>
#include <system/systemdebug.h>

namespace Shipyard
{
#if CPU_BITS == CPU_BITS_64
#   define NUM_BITS_PER_BITFIELD_ELEMENT 64
#elif CPU_BITS == CPU_BITS_32
#   define NUM_BITS_PER_BITFIELD_ELEMENT 32
#endif // #if CPU_BITS == CPU_BITS_64

    template <uint32_t NumBits>
    class Bitfield
    {
    private:

#if CPU_BITS == CPU_BITS_64
        using BitfieldType = uint64_t;
#elif CPU_BITS == CPU_BITS_32
        using BitfieldType = uint32_t;
#endif // #if CPU_BITS == CPU_BITS_64

        static constexpr uint32_t ms_NumElements = ((NumBits + NUM_BITS_PER_BITFIELD_ELEMENT - 1) / NUM_BITS_PER_BITFIELD_ELEMENT);

    public:
        Bitfield()
        {
            Clear();
        }

        void Clear()
        {
            for (uint32_t i = 0; i < ms_NumElements; i++)
            {
                m_BitField[i] = 0;
            }
        }

        bool IsClear() const
        {
            for (uint32_t i = 0; i < ms_NumElements; i++)
            {
                if (m_BitField[i] != 0)
                {
                    return false;
                }
            }

            return true;
        }

        void SetBit(uint32_t bitIndex)
        {
            SHIP_ASSERT(bitIndex < NumBits);

            uint32_t elementIndex = bitIndex / NUM_BITS_PER_BITFIELD_ELEMENT;
            uint32_t bit = bitIndex % NUM_BITS_PER_BITFIELD_ELEMENT;

            m_BitField[elementIndex] |= (BitfieldType(1) << BitfieldType(bit));
        }

        void UnsetBit(uint32_t bitIndex)
        {
            SHIP_ASSERT(bitIndex < NumBits);

            uint32_t elementIndex = bitIndex / NUM_BITS_PER_BITFIELD_ELEMENT;
            uint32_t bit = bitIndex % NUM_BITS_PER_BITFIELD_ELEMENT;

            m_BitField[elementIndex] &= ~(BitfieldType(1) << BitfieldType(bit));
        }

        bool IsBitSet(uint32_t bitIndex) const
        {
            SHIP_ASSERT(bitIndex < NumBits);

            uint32_t elementIndex = bitIndex / NUM_BITS_PER_BITFIELD_ELEMENT;
            uint32_t bit = bitIndex % NUM_BITS_PER_BITFIELD_ELEMENT;

            return ((m_BitField[elementIndex] & (BitfieldType(1) << BitfieldType(bit))) > 0);
        }

        void SetRange(uint32_t startingBitIndexInclusive, uint32_t endingBitIndexInclusive)
        {
            SHIP_ASSERT(startingBitIndexInclusive < NumBits);
            SHIP_ASSERT(endingBitIndexInclusive < NumBits);

            uint32_t startingElementIndex = startingBitIndexInclusive / NUM_BITS_PER_BITFIELD_ELEMENT;
            uint32_t endingElementIndex = endingBitIndexInclusive / NUM_BITS_PER_BITFIELD_ELEMENT;

            bool rangeInSingleElement = (startingBitIndexInclusive == endingElementIndex);
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
                uint32_t elementIndex = startingElementIndex;

                BitfieldType maskOfBitsToSet = ~((BitfieldType(1) << BitfieldType(startingBitIndexInclusive)) - 1);
                m_BitField[elementIndex] |= maskOfBitsToSet;

                elementIndex += 1;

                for (; elementIndex <= endingElementIndex; elementIndex++)
                {
                    bool setFullRange = (elementIndex < endingElementIndex);

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

        void UnsetRange(uint32_t startingBitIndexInclusive, uint32_t endingBitIndexInclusive)
        {
            SHIP_ASSERT(startingBitIndexInclusive < NumBits);
            SHIP_ASSERT(endingBitIndexInclusive < NumBits);

            uint32_t startingElementIndex = startingBitIndexInclusive / NUM_BITS_PER_BITFIELD_ELEMENT;
            uint32_t endingElementIndex = endingBitIndexInclusive / NUM_BITS_PER_BITFIELD_ELEMENT;

            bool rangeInSingleElement = (startingBitIndexInclusive == endingElementIndex);
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
                uint32_t elementIndex = startingElementIndex;

                BitfieldType maskOfBitsToSet = ~((BitfieldType(1) << BitfieldType(startingBitIndexInclusive)) - 1);
                m_BitField[elementIndex] &= ~maskOfBitsToSet;

                elementIndex += 1;

                for (; elementIndex <= endingElementIndex; elementIndex++)
                {
                    bool unsetFullRange = (elementIndex < endingElementIndex);
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
        bool GetFirstBitSet(uint32_t startingBitIndex, uint32_t& firstBitSet) const
        {
            SHIP_ASSERT(startingBitIndex < NumBits);

            uint32_t elementIndex = startingBitIndex / NUM_BITS_PER_BITFIELD_ELEMENT;
            
            BitfieldType maskOfEveryBitsBeforeCleared = ~((BitfieldType(1) << BitfieldType(startingBitIndex)) - 1);

            unsigned long bitScanResult = (unsigned long)startingBitIndex;

#if COMPILER == COMPILER_MSVC

            for (; elementIndex < ms_NumElements; elementIndex++)
            {
                BitfieldType maskForScan = (m_BitField[elementIndex] & maskOfEveryBitsBeforeCleared);

                unsigned char foundValue = FindFirstBitSet(&bitScanResult, maskForScan);

                bool foundBitSet = (foundValue != 0);

                if (foundBitSet)
                {
                    firstBitSet = uint32_t(bitScanResult) + elementIndex * NUM_BITS_PER_BITFIELD_ELEMENT;
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
        uint32_t GetLongestRangeWithBitsSet(uint32_t startingBitIndex) const
        {
            SHIP_ASSERT(startingBitIndex < NumBits);

            uint32_t elementIndex = startingBitIndex / NUM_BITS_PER_BITFIELD_ELEMENT;
            
            BitfieldType maskOfEveryBitsBeforeSet = ((BitfieldType(1) << BitfieldType(startingBitIndex)) - 1);

            uint32_t numBitsSet = 0;

#if COMPILER == COMPILER_MSVC
            for (; elementIndex < ms_NumElements; elementIndex++)
            {
                // Here, we reverse our bitfield, and scan for the first bit set to 1. This will be our first bit set to 0, which
                // will tell us the end of our continous range of bits set to 1.
                BitfieldType maskForScan = ~(m_BitField[elementIndex] | maskOfEveryBitsBeforeSet);

                unsigned long firstBitSet = 0;
                unsigned char foundValue = FindFirstBitSet(&firstBitSet, maskForScan);

                bool foundBitSet = (foundValue != 0);

                if (foundBitSet)
                {
                    numBitsSet = uint32_t(firstBitSet) + NUM_BITS_PER_BITFIELD_ELEMENT * elementIndex - startingBitIndex;
                    break;
                }
                else
                {
                    maskOfEveryBitsBeforeSet = 0;
                    numBitsSet += (NUM_BITS_PER_BITFIELD_ELEMENT - startingBitIndex);
                }
            }
#else

#pragma error "Implement using intrinsic for bit counting for specific compiler"

#endif // #if COMPILER == COMPILER_MSVC

            return numBitsSet;
        }

        void SetAllBits()
        {
            uint32_t remainingBitsToSet = (NumBits - (ms_NumElements - 1) * NUM_BITS_PER_BITFIELD_ELEMENT);

            for (uint32_t elementIndex = 0; elementIndex < ms_NumElements; elementIndex++)
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

    private:
        BitfieldType m_BitField[ms_NumElements];
    };
}