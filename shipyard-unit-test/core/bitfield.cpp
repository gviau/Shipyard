#include <shipyardunittestprecomp.h>

#include <extern/catch/catch.hpp>

#include <system/bitfield.h>

#include <utils/unittestutils.h>

TEST_CASE("Test bitfield 1 element", "[Bitfield]")
{
    Shipyard::ScoppedGlobalAllocator scoppedGlobalAllocator;

    Shipyard::Bitfield<NUM_BITS_PER_BITFIELD_ELEMENT> bitfield;

    bitfield.Create();

    SECTION("Set bit")
    {
        REQUIRE(bitfield.IsClear());

        bitfield.SetBit(0);

        REQUIRE(!bitfield.IsClear());
        REQUIRE(bitfield.IsBitSet(0));

        bitfield.UnsetBit(0);

        REQUIRE(bitfield.IsClear());
        REQUIRE(!bitfield.IsBitSet(0));

        uint32_t lastBitIndex = (NUM_BITS_PER_BITFIELD_ELEMENT - 1);
        bitfield.SetBit(lastBitIndex);

        REQUIRE(!bitfield.IsClear());
        REQUIRE(bitfield.IsBitSet(lastBitIndex));

        bitfield.UnsetBit(lastBitIndex);

        REQUIRE(bitfield.IsClear());
        REQUIRE(!bitfield.IsBitSet(lastBitIndex));

        bitfield.SetBit(10);

        REQUIRE(!bitfield.IsClear());
        REQUIRE(bitfield.IsBitSet(10));

        bitfield.Clear();

        REQUIRE(bitfield.IsClear());
    }

    SECTION("Set range of bits")
    {
        bitfield.SetRange(0, 3);

        for (uint32_t i = 0; i < 3; i++)
        {
            REQUIRE(bitfield.IsBitSet(i));
        }

        bitfield.UnsetRange(0, 3);

        REQUIRE(bitfield.IsClear());
    }

    SECTION("Get first bit set")
    {
        constexpr uint32_t numBits = 3;

        for (uint32_t i = 0; i < numBits; i++)
        {
            bitfield.SetBit(i);
        }

        uint32_t firstBitSet = 0;

        for (uint32_t i = 0; i < numBits; i++)
        {
            REQUIRE(bitfield.GetFirstBitSet(i, firstBitSet));
            REQUIRE(firstBitSet == i);
        }
        
        REQUIRE(!bitfield.GetFirstBitSet(numBits, firstBitSet));

        bitfield.SetBit(10);

        REQUIRE(bitfield.GetFirstBitSet(numBits, firstBitSet));
        REQUIRE(firstBitSet == 10);
    }

    SECTION("Count number of continous bits set")
    {
        constexpr uint32_t numBits = 3;

        for (uint32_t i = 0; i < numBits; i++)
        {
            bitfield.SetBit(i);
        }

        for (uint32_t i = 0; i < numBits; i++)
        {
            uint32_t expectedBitCount = (numBits - i);

            REQUIRE(bitfield.GetLongestRangeWithBitsSet(i) == expectedBitCount);
        }

        bitfield.Clear();

        bitfield.SetRange(0, NUM_BITS_PER_BITFIELD_ELEMENT - 1);

        REQUIRE(bitfield.GetLongestRangeWithBitsSet(0) == NUM_BITS_PER_BITFIELD_ELEMENT);
    }

    SECTION("Set All Bit")
    {
        bitfield.SetAllBits();

        for (uint32_t i = 0; i < NUM_BITS_PER_BITFIELD_ELEMENT; i++)
        {
            REQUIRE(bitfield.IsBitSet(i));
        }
    }
}

TEST_CASE("Test bitfield 2 elements", "[Bitfield]")
{
    Shipyard::ScoppedGlobalAllocator scoppedGlobalAllocator;

    constexpr uint32_t bitfieldSize = NUM_BITS_PER_BITFIELD_ELEMENT * 2;
    Shipyard::Bitfield<bitfieldSize> bitfield;

    bitfield.Create();

    SECTION("Set bit")
    {
        REQUIRE(bitfield.IsClear());

        bitfield.SetBit(0);

        REQUIRE(!bitfield.IsClear());
        REQUIRE(bitfield.IsBitSet(0));

        bitfield.UnsetBit(0);

        REQUIRE(bitfield.IsClear());
        REQUIRE(!bitfield.IsBitSet(0));

        uint32_t lastBitIndex = (bitfieldSize - 1);
        bitfield.SetBit(lastBitIndex);

        REQUIRE(!bitfield.IsClear());
        REQUIRE(bitfield.IsBitSet(lastBitIndex));

        bitfield.UnsetBit(lastBitIndex);

        REQUIRE(bitfield.IsClear());
        REQUIRE(!bitfield.IsBitSet(lastBitIndex));

        bitfield.SetBit(10);

        REQUIRE(!bitfield.IsClear());
        REQUIRE(bitfield.IsBitSet(10));

        bitfield.Clear();

        REQUIRE(bitfield.IsClear());
    }

    SECTION("Set range of bits accross elements")
    {
        bitfield.SetRange(NUM_BITS_PER_BITFIELD_ELEMENT - 2, NUM_BITS_PER_BITFIELD_ELEMENT + 2);

        for (uint32_t i = NUM_BITS_PER_BITFIELD_ELEMENT - 2; i < NUM_BITS_PER_BITFIELD_ELEMENT + 2; i++)
        {
            REQUIRE(bitfield.IsBitSet(i));
        }
    }

    SECTION("Get first bit set")
    {
        constexpr uint32_t numBits = 3;

        for (uint32_t i = 0; i < numBits; i++)
        {
            uint32_t bitIndex = i + NUM_BITS_PER_BITFIELD_ELEMENT;

            bitfield.SetBit(bitIndex);
        }

        uint32_t firstBitSet = 0;

        for (uint32_t i = 0; i < numBits; i++)
        {
            REQUIRE(bitfield.GetFirstBitSet(i, firstBitSet));
            REQUIRE(firstBitSet == NUM_BITS_PER_BITFIELD_ELEMENT);
        }

        for (uint32_t i = 0; i < numBits; i++)
        {
            uint32_t bitIndex = i + NUM_BITS_PER_BITFIELD_ELEMENT;

            REQUIRE(bitfield.GetFirstBitSet(bitIndex, firstBitSet));
            REQUIRE(firstBitSet == bitIndex);
        }

        REQUIRE(!bitfield.GetFirstBitSet(numBits + NUM_BITS_PER_BITFIELD_ELEMENT, firstBitSet));

        uint32_t bitIndex = 10 + NUM_BITS_PER_BITFIELD_ELEMENT;
        bitfield.SetBit(bitIndex);

        REQUIRE(bitfield.GetFirstBitSet(numBits + NUM_BITS_PER_BITFIELD_ELEMENT, firstBitSet));
        REQUIRE(firstBitSet == bitIndex);
    }

    SECTION("Count number of continous bits set")
    {
        constexpr uint32_t numBits = 3;

        for (uint32_t i = 0; i < numBits; i++)
        {
            uint32_t bitIndex = i + NUM_BITS_PER_BITFIELD_ELEMENT;
            bitfield.SetBit(bitIndex);
        }

        for (uint32_t i = 0; i < numBits; i++)
        {
            uint32_t expectedBitCount = (numBits - i);

            uint32_t bitIndex = i + NUM_BITS_PER_BITFIELD_ELEMENT;
            REQUIRE(bitfield.GetLongestRangeWithBitsSet(bitIndex) == expectedBitCount);
        }

        bitfield.Clear();

        bitfield.SetRange(0, bitfieldSize - 1);

        REQUIRE(bitfield.GetLongestRangeWithBitsSet(0) == bitfieldSize);
    }

    SECTION("Count number of continous bits set accross bitfield elements")
    {
        bitfield.SetBit(NUM_BITS_PER_BITFIELD_ELEMENT - 2);
        bitfield.SetBit(NUM_BITS_PER_BITFIELD_ELEMENT - 1);
        bitfield.SetBit(NUM_BITS_PER_BITFIELD_ELEMENT    );
        bitfield.SetBit(NUM_BITS_PER_BITFIELD_ELEMENT + 1);

        uint32_t startingBitIndex = NUM_BITS_PER_BITFIELD_ELEMENT - 2;
        for (uint32_t i = 0; i < 4; i++)
        {
            uint32_t expectedBitCount = (4 - i);

            uint32_t bitIndex = startingBitIndex + i;
            REQUIRE(bitfield.GetLongestRangeWithBitsSet(bitIndex) == expectedBitCount);
        }
    }

    SECTION("Set All Bit")
    {
        bitfield.SetAllBits();

        for (uint32_t i = 0; i < bitfieldSize; i++)
        {
            REQUIRE(bitfield.IsBitSet(i));
        }
    }
}

TEST_CASE("Test bitfield 3 elements", "[Bitfield]")
{
    Shipyard::ScoppedGlobalAllocator scoppedGlobalAllocator;

    constexpr uint32_t bitfieldSize = NUM_BITS_PER_BITFIELD_ELEMENT * 3;
    Shipyard::Bitfield<bitfieldSize> bitfield;

    bitfield.Create();

    SECTION("Set bit")
    {
        REQUIRE(bitfield.IsClear());

        bitfield.SetBit(0);

        REQUIRE(!bitfield.IsClear());
        REQUIRE(bitfield.IsBitSet(0));

        bitfield.UnsetBit(0);

        REQUIRE(bitfield.IsClear());
        REQUIRE(!bitfield.IsBitSet(0));

        uint32_t lastBitIndex = (bitfieldSize - 1);
        bitfield.SetBit(lastBitIndex);

        REQUIRE(!bitfield.IsClear());
        REQUIRE(bitfield.IsBitSet(lastBitIndex));

        bitfield.UnsetBit(lastBitIndex);

        REQUIRE(bitfield.IsClear());
        REQUIRE(!bitfield.IsBitSet(lastBitIndex));

        bitfield.SetBit(10);

        REQUIRE(!bitfield.IsClear());
        REQUIRE(bitfield.IsBitSet(10));

        bitfield.Clear();

        REQUIRE(bitfield.IsClear());
    }

    SECTION("Set range of bits accross elements")
    {
        bitfield.SetRange(NUM_BITS_PER_BITFIELD_ELEMENT - 2, NUM_BITS_PER_BITFIELD_ELEMENT + 2);

        for (uint32_t i = NUM_BITS_PER_BITFIELD_ELEMENT - 2; i < NUM_BITS_PER_BITFIELD_ELEMENT + 2; i++)
        {
            REQUIRE(bitfield.IsBitSet(i));
        }

        bitfield.Clear();

        uint32_t endingBitIndex = (NUM_BITS_PER_BITFIELD_ELEMENT * 2 + 2);
        bitfield.SetRange(0, endingBitIndex - 1);

        for (uint32_t i = 0; i < endingBitIndex; i++)
        {
            REQUIRE(bitfield.IsBitSet(i));
        }
    }

    SECTION("Get first bit set")
    {
        constexpr uint32_t numBits = 3;

        for (uint32_t i = 0; i < numBits; i++)
        {
            uint32_t bitIndex = i + NUM_BITS_PER_BITFIELD_ELEMENT;

            bitfield.SetBit(bitIndex);
        }

        uint32_t firstBitSet = 0;

        for (uint32_t i = 0; i < numBits; i++)
        {
            REQUIRE(bitfield.GetFirstBitSet(i, firstBitSet));
            REQUIRE(firstBitSet == NUM_BITS_PER_BITFIELD_ELEMENT);
        }

        for (uint32_t i = 0; i < numBits; i++)
        {
            uint32_t bitIndex = i + NUM_BITS_PER_BITFIELD_ELEMENT;

            REQUIRE(bitfield.GetFirstBitSet(bitIndex, firstBitSet));
            REQUIRE(firstBitSet == bitIndex);
        }

        REQUIRE(!bitfield.GetFirstBitSet(numBits + NUM_BITS_PER_BITFIELD_ELEMENT, firstBitSet));

        uint32_t bitIndex = 10 + NUM_BITS_PER_BITFIELD_ELEMENT;
        bitfield.SetBit(bitIndex);

        REQUIRE(bitfield.GetFirstBitSet(numBits + NUM_BITS_PER_BITFIELD_ELEMENT, firstBitSet));
        REQUIRE(firstBitSet == bitIndex);
    }

    SECTION("Count number of continous bits set")
    {
        constexpr uint32_t numBits = 3;

        for (uint32_t i = 0; i < numBits; i++)
        {
            uint32_t bitIndex = i + NUM_BITS_PER_BITFIELD_ELEMENT;
            bitfield.SetBit(bitIndex);
        }

        for (uint32_t i = 0; i < numBits; i++)
        {
            uint32_t expectedBitCount = (numBits - i);

            uint32_t bitIndex = i + NUM_BITS_PER_BITFIELD_ELEMENT;
            REQUIRE(bitfield.GetLongestRangeWithBitsSet(bitIndex) == expectedBitCount);
        }

        bitfield.Clear();

        bitfield.SetRange(0, bitfieldSize - 1);

        REQUIRE(bitfield.GetLongestRangeWithBitsSet(0) == bitfieldSize);
    }

    SECTION("Count number of continous bits set accross bitfield elements")
    {
        bitfield.SetBit(NUM_BITS_PER_BITFIELD_ELEMENT - 2);
        bitfield.SetBit(NUM_BITS_PER_BITFIELD_ELEMENT - 1);
        bitfield.SetBit(NUM_BITS_PER_BITFIELD_ELEMENT);
        bitfield.SetBit(NUM_BITS_PER_BITFIELD_ELEMENT + 1);

        uint32_t startingBitIndex = NUM_BITS_PER_BITFIELD_ELEMENT - 2;
        for (uint32_t i = 0; i < 4; i++)
        {
            uint32_t expectedBitCount = (4 - i);

            uint32_t bitIndex = startingBitIndex + i;
            REQUIRE(bitfield.GetLongestRangeWithBitsSet(bitIndex) == expectedBitCount);
        }
    }

    SECTION("Set All Bit")
    {
        bitfield.SetAllBits();

        for (uint32_t i = 0; i < bitfieldSize; i++)
        {
            REQUIRE(bitfield.IsBitSet(i));
        }
    }
}

TEST_CASE("Test bitfield less than an element", "[Bitfield]")
{
    Shipyard::ScoppedGlobalAllocator scoppedGlobalAllocator;

    constexpr uint32_t bitfieldSize = 16;
    Shipyard::Bitfield<bitfieldSize> bitfield;

    bitfield.Create();

    SECTION("Set bit")
    {
        REQUIRE(bitfield.IsClear());

        bitfield.SetBit(0);

        REQUIRE(!bitfield.IsClear());
        REQUIRE(bitfield.IsBitSet(0));

        bitfield.UnsetBit(0);

        REQUIRE(bitfield.IsClear());
        REQUIRE(!bitfield.IsBitSet(0));

        uint32_t lastBitIndex = (bitfieldSize - 1);
        bitfield.SetBit(lastBitIndex);

        REQUIRE(!bitfield.IsClear());
        REQUIRE(bitfield.IsBitSet(lastBitIndex));

        bitfield.UnsetBit(lastBitIndex);

        REQUIRE(bitfield.IsClear());
        REQUIRE(!bitfield.IsBitSet(lastBitIndex));

        bitfield.SetBit(10);

        REQUIRE(!bitfield.IsClear());
        REQUIRE(bitfield.IsBitSet(10));

        bitfield.Clear();

        REQUIRE(bitfield.IsClear());
    }

    SECTION("Get first bit set")
    {
        constexpr uint32_t numBits = 3;

        for (uint32_t i = 0; i < numBits; i++)
        {
            bitfield.SetBit(i);
        }

        uint32_t firstBitSet = 0;

        for (uint32_t i = 0; i < numBits; i++)
        {
            REQUIRE(bitfield.GetFirstBitSet(i, firstBitSet));
            REQUIRE(firstBitSet == i);
        }

        REQUIRE(!bitfield.GetFirstBitSet(numBits, firstBitSet));

        uint32_t bitIndex = 10;
        bitfield.SetBit(bitIndex);

        REQUIRE(bitfield.GetFirstBitSet(numBits, firstBitSet));
        REQUIRE(firstBitSet == bitIndex);
    }

    SECTION("Count number of continous bits set")
    {
        constexpr uint32_t numBits = 3;

        for (uint32_t i = 0; i < numBits; i++)
        {
            bitfield.SetBit(i);
        }

        for (uint32_t i = 0; i < numBits; i++)
        {
            uint32_t expectedBitCount = (numBits - i);

            REQUIRE(bitfield.GetLongestRangeWithBitsSet(i) == expectedBitCount);
        }

        bitfield.Clear();

        bitfield.SetRange(0, bitfieldSize - 1);

        REQUIRE(bitfield.GetLongestRangeWithBitsSet(0) == bitfieldSize);
    }

    SECTION("Set All Bit")
    {
        bitfield.SetAllBits();

        for (uint32_t i = 0; i < bitfieldSize; i++)
        {
            REQUIRE(bitfield.IsBitSet(i));
        }
    }
}