#include <catch.hpp>

#include <system/memory/fixedheapallocator.h>

namespace
{
    size_t AlignAddress(size_t address, size_t alignment)
    {
        return (address + (alignment - 1)) & (~(alignment - 1));
    }

    bool IsAddressAligned(size_t address, size_t alignment)
    {
        return ((address & (alignment - 1)) == 0);
    }

    bool AllocsAreDontOverlap(void* pAlloc1, size_t allocSize1, void* pAlloc2, size_t allocSize2)
    {
        size_t start1 = size_t(pAlloc1);
        size_t end1 = start1 + allocSize1;
        size_t start2 = size_t(pAlloc2);
        size_t end2 = start2 + allocSize2;

        return ( (start1 <= start2 && end1 <= start2) ||
                 (start1 >= end2 && end1 >= end2) ||
                 (start2 <= start1 && end2 <= start1) ||
                 (start2 >= end1 && end2 >= end1) );
    }

    struct ScoppedBuffer
    {
        ScoppedBuffer(size_t size)
        {
            pBuffer = malloc(size);
        }

        ~ScoppedBuffer()
        {
            free(pBuffer);
        }

        void* pBuffer = nullptr;
    };
}

TEST_CASE("Test FixedHeapAllocator", "[Allocator]")
{
    Shipyard::FixedHeapAllocator fixedHeapAllocator;

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
    const size_t minHeapSize = ((Shipyard::FixedHeapAllocator::FreeMemoryBlockSize > Shipyard::FixedHeapAllocator::MemoryAllocationHeaderSize) ? Shipyard::FixedHeapAllocator::FreeMemoryBlockSize : Shipyard::FixedHeapAllocator::MemoryAllocationHeaderSize);
#else
    const size_t minHeapSize = 32;
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

    SECTION("creating allocator")
    {
        const size_t heapSize = 64;
        ScoppedBuffer scoppedBuffer(heapSize);

        fixedHeapAllocator.Create(scoppedBuffer.pBuffer, heapSize);
    }

    SECTION("simple allocation")
    {
        const size_t allocSize = 4;
        const size_t heapSize = minHeapSize + allocSize;
        ScoppedBuffer scoppedBuffer(heapSize);

        fixedHeapAllocator.Create(scoppedBuffer.pBuffer, heapSize);

        void* pAlloc = SHIP_ALLOC(fixedHeapAllocator, allocSize, 1);

        REQUIRE(pAlloc != nullptr);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 1);
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBytesUsed == heapSize);
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == allocSize);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

        SHIP_FREE(fixedHeapAllocator, pAlloc);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 0);
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBytesUsed == 0);
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == 0);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO
    }

    SECTION("simple aligned allocation")
    {
        const size_t allocSize = 4;
        const size_t alignment = 16;
        const size_t heapSize = minHeapSize + allocSize + alignment;
        ScoppedBuffer scoppedBuffer(heapSize);

        fixedHeapAllocator.Create(scoppedBuffer.pBuffer, heapSize);

        void* pAlloc = SHIP_ALLOC(fixedHeapAllocator, allocSize, alignment);

        REQUIRE(pAlloc != nullptr);
        REQUIRE(IsAddressAligned(size_t(pAlloc), alignment));

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 1);
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated >= allocSize);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

        SHIP_FREE(fixedHeapAllocator, pAlloc);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 0);
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == 0);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO
    }
    
    SECTION("two allocations same size")
    {
        const size_t allocSize = 4;
        const size_t heapSize = (minHeapSize + allocSize) * 2;
        ScoppedBuffer scoppedBuffer(heapSize);

        fixedHeapAllocator.Create(scoppedBuffer.pBuffer, heapSize);

        void* pAlloc1 = SHIP_ALLOC(fixedHeapAllocator, allocSize, 1);

        REQUIRE(pAlloc1 != nullptr);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 1);
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == allocSize);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

        void* pAlloc2 = SHIP_ALLOC(fixedHeapAllocator, allocSize, 1);

        REQUIRE(pAlloc2 != nullptr);

        REQUIRE(AllocsAreDontOverlap(pAlloc1, allocSize, pAlloc2, allocSize));

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 2);
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == (allocSize * 2));
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

        SHIP_FREE(fixedHeapAllocator, pAlloc1);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 1);
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == allocSize);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

        SHIP_FREE(fixedHeapAllocator, pAlloc2);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 0);
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == 0);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO
    }

    SECTION("two aligned allocations same size")
    {
        const size_t allocSize = 4;
        const size_t alignment = 16;
        const size_t heapSize = (minHeapSize + allocSize + alignment) * 2;
        ScoppedBuffer scoppedBuffer(heapSize);

        fixedHeapAllocator.Create(scoppedBuffer.pBuffer, heapSize);

        void* pAlloc1 = SHIP_ALLOC(fixedHeapAllocator, allocSize, alignment);

        REQUIRE(pAlloc1 != nullptr);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 1);
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated >= allocSize);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

        void* pAlloc2 = SHIP_ALLOC(fixedHeapAllocator, allocSize, alignment);

        REQUIRE(pAlloc2 != nullptr);

        REQUIRE(AllocsAreDontOverlap(pAlloc1, allocSize, pAlloc2, allocSize));

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 2);
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated >= (allocSize * 2));
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

        SHIP_FREE(fixedHeapAllocator, pAlloc1);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 1);
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated >= allocSize);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

        SHIP_FREE(fixedHeapAllocator, pAlloc2);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 0);
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == 0);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO
    }

    SECTION("two allocations same size realloc")
    {
        const size_t allocSize = 4;
        const size_t heapSize = (minHeapSize + allocSize) * 2;
        ScoppedBuffer scoppedBuffer(heapSize);

        fixedHeapAllocator.Create(scoppedBuffer.pBuffer, heapSize);

        for (uint32_t i = 0; i < 2; i++)
        {
            void* pAlloc1 = SHIP_ALLOC(fixedHeapAllocator, allocSize, 1);

            REQUIRE(pAlloc1 != nullptr);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 1);
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == allocSize);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            void* pAlloc2 = SHIP_ALLOC(fixedHeapAllocator, allocSize, 1);

            REQUIRE(pAlloc2 != nullptr);

            REQUIRE(AllocsAreDontOverlap(pAlloc1, allocSize, pAlloc2, allocSize));

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 2);
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == (allocSize * 2));
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            SHIP_FREE(fixedHeapAllocator, pAlloc1);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 1);
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == allocSize);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            SHIP_FREE(fixedHeapAllocator, pAlloc2);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 0);
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == 0);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO
        }
    }

    SECTION("two allocations same size inversed free")
    {
        const size_t allocSize = 4;
        const size_t heapSize = (minHeapSize + allocSize) * 2;
        ScoppedBuffer scoppedBuffer(heapSize);

        fixedHeapAllocator.Create(scoppedBuffer.pBuffer, heapSize);

        void* pAlloc1 = SHIP_ALLOC(fixedHeapAllocator, allocSize, 1);

        REQUIRE(pAlloc1 != nullptr);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 1);
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == allocSize);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

        void* pAlloc2 = SHIP_ALLOC(fixedHeapAllocator, allocSize, 1);

        REQUIRE(pAlloc2 != nullptr);

        REQUIRE(AllocsAreDontOverlap(pAlloc1, allocSize, pAlloc2, allocSize));

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 2);
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == (allocSize * 2));
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

        SHIP_FREE(fixedHeapAllocator, pAlloc2);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 1);
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == allocSize);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

        SHIP_FREE(fixedHeapAllocator, pAlloc1);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 0);
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == 0);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO
    }

    SECTION("two allocations same size inversed free realloc")
    {
        const size_t allocSize = 4;
        const size_t heapSize = (minHeapSize + allocSize) * 2;
        ScoppedBuffer scoppedBuffer(heapSize);

        fixedHeapAllocator.Create(scoppedBuffer.pBuffer, heapSize);

        for (uint32_t i = 0; i < 2; i++)
        {
            void* pAlloc1 = SHIP_ALLOC(fixedHeapAllocator, allocSize, 1);

            REQUIRE(pAlloc1 != nullptr);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 1);
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == allocSize);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            void* pAlloc2 = SHIP_ALLOC(fixedHeapAllocator, allocSize, 1);

            REQUIRE(pAlloc2 != nullptr);

            REQUIRE(AllocsAreDontOverlap(pAlloc1, allocSize, pAlloc2, allocSize));

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 2);
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == (allocSize * 2));
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            SHIP_FREE(fixedHeapAllocator, pAlloc2);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 1);
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == allocSize);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            SHIP_FREE(fixedHeapAllocator, pAlloc1);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 0);
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == 0);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO
        }
    }

    SECTION("two allocations different size")
    {
        const size_t allocSize1 = 4;
        const size_t allocSize2 = 12;
        const size_t heapSize = minHeapSize + minHeapSize + allocSize1 + allocSize2;
        ScoppedBuffer scoppedBuffer(heapSize);

        fixedHeapAllocator.Create(scoppedBuffer.pBuffer, heapSize);

        void* pAlloc1 = SHIP_ALLOC(fixedHeapAllocator, allocSize1, 1);

        REQUIRE(pAlloc1 != nullptr);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 1);
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == allocSize1);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

        void* pAlloc2 = SHIP_ALLOC(fixedHeapAllocator, allocSize2, 1);

        REQUIRE(pAlloc2 != nullptr);

        REQUIRE(AllocsAreDontOverlap(pAlloc1, allocSize1, pAlloc2, allocSize2));

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 2);
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == (allocSize1 + allocSize2));
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

        SHIP_FREE(fixedHeapAllocator, pAlloc1);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 1);
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == allocSize2);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

        SHIP_FREE(fixedHeapAllocator, pAlloc2);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 0);
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == 0);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO
    }

    SECTION("two allocations different size realloc")
    {
        const size_t allocSize1 = 4;
        const size_t allocSize2 = 12;
        const size_t heapSize = minHeapSize + minHeapSize + allocSize1 + allocSize2;
        ScoppedBuffer scoppedBuffer(heapSize);

        fixedHeapAllocator.Create(scoppedBuffer.pBuffer, heapSize);

        for (uint32_t i = 0; i < 2; i++)
        {
            void* pAlloc1 = SHIP_ALLOC(fixedHeapAllocator, allocSize1, 1);

            REQUIRE(pAlloc1 != nullptr);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 1);
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == allocSize1);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            void* pAlloc2 = SHIP_ALLOC(fixedHeapAllocator, allocSize2, 1);

            REQUIRE(pAlloc2 != nullptr);

            REQUIRE(AllocsAreDontOverlap(pAlloc1, allocSize1, pAlloc2, allocSize2));

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 2);
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == (allocSize1 + allocSize2));
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            SHIP_FREE(fixedHeapAllocator, pAlloc1);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 1);
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == allocSize2);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            SHIP_FREE(fixedHeapAllocator, pAlloc2);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 0);
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == 0);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO
        }
    }

    SECTION("two allocations different size inversed free")
    {
        const size_t allocSize1 = 4;
        const size_t allocSize2 = 12;
        const size_t heapSize = minHeapSize + minHeapSize + allocSize1 + allocSize2;
        ScoppedBuffer scoppedBuffer(heapSize);

        fixedHeapAllocator.Create(scoppedBuffer.pBuffer, heapSize);

        void* pAlloc1 = SHIP_ALLOC(fixedHeapAllocator, allocSize1, 1);

        REQUIRE(pAlloc1 != nullptr);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 1);
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == allocSize1);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

        void* pAlloc2 = SHIP_ALLOC(fixedHeapAllocator, allocSize2, 1);

        REQUIRE(pAlloc2 != nullptr);

        REQUIRE(AllocsAreDontOverlap(pAlloc1, allocSize1, pAlloc2, allocSize2));

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 2);
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == (allocSize1 + allocSize2));
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

        SHIP_FREE(fixedHeapAllocator, pAlloc2);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 1);
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == allocSize1);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

        SHIP_FREE(fixedHeapAllocator, pAlloc1);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 0);
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == 0);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO
    }

    SECTION("two allocations different size inversed free realloc")
    {
        const size_t allocSize1 = 4;
        const size_t allocSize2 = 12;
        const size_t heapSize = minHeapSize + minHeapSize + allocSize1 + allocSize2;
        ScoppedBuffer scoppedBuffer(heapSize);

        fixedHeapAllocator.Create(scoppedBuffer.pBuffer, heapSize);

        for (uint32_t i = 0; i < 2; i++)
        {
            void* pAlloc1 = SHIP_ALLOC(fixedHeapAllocator, allocSize1, 1);

            REQUIRE(pAlloc1 != nullptr);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 1);
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == allocSize1);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            void* pAlloc2 = SHIP_ALLOC(fixedHeapAllocator, allocSize2, 1);

            REQUIRE(pAlloc2 != nullptr);

            REQUIRE(AllocsAreDontOverlap(pAlloc1, allocSize1, pAlloc2, allocSize2));

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 2);
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == (allocSize1 + allocSize2));
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            SHIP_FREE(fixedHeapAllocator, pAlloc2);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 1);
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == allocSize1);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            SHIP_FREE(fixedHeapAllocator, pAlloc1);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 0);
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == 0);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO
        }
    }

    SECTION("two aligned allocations different size")
    {
        const size_t allocSize1 = 4;
        const size_t allocSize2 = 12;
        const size_t alignment = 32;
        const size_t heapSize = minHeapSize + minHeapSize + allocSize1 + allocSize2 + alignment + alignment;
        ScoppedBuffer scoppedBuffer(heapSize);

        fixedHeapAllocator.Create(scoppedBuffer.pBuffer, heapSize);

        void* pAlloc1 = SHIP_ALLOC(fixedHeapAllocator, allocSize1, alignment);

        REQUIRE(pAlloc1 != nullptr);
        REQUIRE(IsAddressAligned(size_t(pAlloc1), alignment));

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 1);
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated >= allocSize1);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

        void* pAlloc2 = SHIP_ALLOC(fixedHeapAllocator, allocSize2, alignment);

        REQUIRE(pAlloc2 != nullptr);
        REQUIRE(IsAddressAligned(size_t(pAlloc2), alignment));

        REQUIRE(AllocsAreDontOverlap(pAlloc1, allocSize1, pAlloc2, allocSize2));

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 2);
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated >= (allocSize1 + allocSize2));
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

        SHIP_FREE(fixedHeapAllocator, pAlloc1);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 1);
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated >= allocSize2);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

        SHIP_FREE(fixedHeapAllocator, pAlloc2);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 0);
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == 0);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO
    }

    SECTION("two aligned allocations different size realloc")
    {
        const size_t allocSize1 = 4;
        const size_t allocSize2 = 12;
        const size_t alignment = 32;
        const size_t heapSize = minHeapSize + minHeapSize + allocSize1 + allocSize2 + alignment + alignment;
        ScoppedBuffer scoppedBuffer(heapSize);

        fixedHeapAllocator.Create(scoppedBuffer.pBuffer, heapSize);

        for (uint32_t i = 0; i < 2; i++)
        {
            void* pAlloc1 = SHIP_ALLOC(fixedHeapAllocator, allocSize1, alignment);

            REQUIRE(pAlloc1 != nullptr);
            REQUIRE(IsAddressAligned(size_t(pAlloc1), alignment));

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 1);
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated >= allocSize1);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            void* pAlloc2 = SHIP_ALLOC(fixedHeapAllocator, allocSize2, alignment);

            REQUIRE(pAlloc2 != nullptr);
            REQUIRE(IsAddressAligned(size_t(pAlloc2), alignment));

            REQUIRE(AllocsAreDontOverlap(pAlloc1, allocSize1, pAlloc2, allocSize2));

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 2);
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated >= (allocSize1 + allocSize2));
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            SHIP_FREE(fixedHeapAllocator, pAlloc1);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 1);
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated >= allocSize2);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            SHIP_FREE(fixedHeapAllocator, pAlloc2);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 0);
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == 0);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO
        }
    }
    SECTION("three allocations")
    {
        const size_t heapSize = (minHeapSize + 32) * 3;
        ScoppedBuffer scoppedBuffer(heapSize);

        fixedHeapAllocator.Create(scoppedBuffer.pBuffer, heapSize);

        auto deallocationPatternTestCase = [&fixedHeapAllocator](
                uint32_t first, uint32_t second, uint32_t third,
                uint32_t allocSize1, uint32_t allocSize2, uint32_t allocSize3)
        {
            void* pAlloc1 = SHIP_ALLOC(fixedHeapAllocator, allocSize1, 1);

            REQUIRE(pAlloc1 != nullptr);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 1);
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == allocSize1);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            void* pAlloc2 = SHIP_ALLOC(fixedHeapAllocator, allocSize2, 1);

            REQUIRE(pAlloc2 != nullptr);

            REQUIRE(AllocsAreDontOverlap(pAlloc1, allocSize1, pAlloc2, allocSize2));

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 2);
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == allocSize1 + allocSize2);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            void* pAlloc3 = SHIP_ALLOC(fixedHeapAllocator, allocSize3, 1);

            REQUIRE(pAlloc3 != nullptr);

            REQUIRE(AllocsAreDontOverlap(pAlloc1, allocSize1, pAlloc3, allocSize3));
            REQUIRE(AllocsAreDontOverlap(pAlloc2, allocSize2, pAlloc3, allocSize3));

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 3);
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == allocSize1 + allocSize2 + allocSize3);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            void* pAllocs[] = { pAlloc1, pAlloc2, pAlloc3 };
            size_t allocSizes[] = { allocSize1, allocSize2, allocSize3 };

            SHIP_FREE(fixedHeapAllocator, pAllocs[first]);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 2);
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == allocSizes[second] + allocSizes[third]);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            SHIP_FREE(fixedHeapAllocator, pAllocs[second]);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 1);
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == allocSizes[third]);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            SHIP_FREE(fixedHeapAllocator, pAllocs[third]);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 0);
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == 0);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO
        };

        deallocationPatternTestCase(0, 1, 2, 4, 4, 4);
        deallocationPatternTestCase(1, 2, 0, 4, 4, 4);
        deallocationPatternTestCase(2, 1, 0, 4, 4, 4);
        deallocationPatternTestCase(2, 0, 1, 4, 4, 4);
        deallocationPatternTestCase(0, 2, 1, 4, 4, 4);
        deallocationPatternTestCase(1, 0, 2, 4, 4, 4);

        // One last time, to make sure we can properly realloc after last case
        deallocationPatternTestCase(0, 1, 2, 4, 4, 4);

        deallocationPatternTestCase(0, 1, 2, 4, 5, 13);
        deallocationPatternTestCase(1, 2, 0, 4, 5, 13);
        deallocationPatternTestCase(2, 1, 0, 4, 5, 13);
        deallocationPatternTestCase(2, 0, 1, 4, 5, 13);
        deallocationPatternTestCase(0, 2, 1, 4, 5, 13);
        deallocationPatternTestCase(1, 0, 2, 4, 5, 13);

        // One last time, to make sure we can properly realloc after last case
        deallocationPatternTestCase(0, 1, 2, 4, 5, 13);
    }

    SECTION("four allocations")
    {
        const size_t heapSize = (minHeapSize + 32) * 4;
        ScoppedBuffer scoppedBuffer(heapSize);

        fixedHeapAllocator.Create(scoppedBuffer.pBuffer, heapSize);

        auto deallocationPatternTestCase = [&fixedHeapAllocator](
                uint32_t first, uint32_t second, uint32_t third, uint32_t fourth,
                uint32_t allocSize1, uint32_t allocSize2, uint32_t allocSize3, uint32_t allocSize4)
        {
            void* pAlloc1 = SHIP_ALLOC(fixedHeapAllocator, allocSize1, 1);

            REQUIRE(pAlloc1 != nullptr);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 1);
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == allocSize1);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            void* pAlloc2 = SHIP_ALLOC(fixedHeapAllocator, allocSize2, 1);

            REQUIRE(pAlloc2 != nullptr);

            REQUIRE(AllocsAreDontOverlap(pAlloc1, allocSize1, pAlloc2, allocSize2));

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 2);
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == allocSize1 + allocSize2);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            void* pAlloc3 = SHIP_ALLOC(fixedHeapAllocator, allocSize3, 1);

            REQUIRE(pAlloc3 != nullptr);

            REQUIRE(AllocsAreDontOverlap(pAlloc1, allocSize1, pAlloc3, allocSize3));
            REQUIRE(AllocsAreDontOverlap(pAlloc2, allocSize2, pAlloc3, allocSize3));

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 3);
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == allocSize1 + allocSize2 + allocSize3);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            void* pAlloc4 = SHIP_ALLOC(fixedHeapAllocator, allocSize4, 1);

            REQUIRE(pAlloc4 != nullptr);

            REQUIRE(AllocsAreDontOverlap(pAlloc1, allocSize1, pAlloc4, allocSize4));
            REQUIRE(AllocsAreDontOverlap(pAlloc2, allocSize2, pAlloc4, allocSize4));
            REQUIRE(AllocsAreDontOverlap(pAlloc3, allocSize3, pAlloc4, allocSize4));

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 4);
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == allocSize1 + allocSize2 + allocSize3 + allocSize4);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            void* pAllocs[] = { pAlloc1, pAlloc2, pAlloc3, pAlloc4 };
            uint32_t allocSizes[] = { allocSize1, allocSize2, allocSize3, allocSize4 };

            SHIP_FREE(fixedHeapAllocator, pAllocs[first]);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 3);
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == allocSizes[second] + allocSizes[third] + allocSizes[fourth]);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            SHIP_FREE(fixedHeapAllocator, pAllocs[second]);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 2);
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == allocSizes[third] + allocSizes[fourth]);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            SHIP_FREE(fixedHeapAllocator, pAllocs[third]);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 1);
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == allocSizes[fourth]);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            SHIP_FREE(fixedHeapAllocator, pAllocs[fourth]);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 0);
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == 0);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO
        };

        deallocationPatternTestCase(0, 1, 2, 3, 4, 4, 4, 4);
        deallocationPatternTestCase(1, 2, 3, 0, 4, 4, 4, 4);
        deallocationPatternTestCase(2, 3, 0, 1, 4, 4, 4, 4);
        deallocationPatternTestCase(3, 0, 1, 2, 4, 4, 4, 4);
        deallocationPatternTestCase(0, 2, 1, 3, 4, 4, 4, 4);
        deallocationPatternTestCase(2, 1, 3, 0, 4, 4, 4, 4);
        deallocationPatternTestCase(1, 3, 0, 2, 4, 4, 4, 4);
        deallocationPatternTestCase(3, 0, 2, 1, 4, 4, 4, 4);
        deallocationPatternTestCase(0, 1, 3, 2, 4, 4, 4, 4);
        deallocationPatternTestCase(1, 3, 2, 0, 4, 4, 4, 4);
        deallocationPatternTestCase(3, 2, 0, 1, 4, 4, 4, 4);
        deallocationPatternTestCase(2, 0, 1, 3, 4, 4, 4, 4);
        deallocationPatternTestCase(3, 1, 2, 0, 4, 4, 4, 4);
        deallocationPatternTestCase(1, 2, 0, 3, 4, 4, 4, 4);
        deallocationPatternTestCase(2, 0, 3, 1, 4, 4, 4, 4);
        deallocationPatternTestCase(0, 3, 1, 2, 4, 4, 4, 4);
        deallocationPatternTestCase(3, 2, 1, 0, 4, 4, 4, 4);
        deallocationPatternTestCase(2, 1, 0, 3, 4, 4, 4, 4);
        deallocationPatternTestCase(1, 0, 3, 2, 4, 4, 4, 4);
        deallocationPatternTestCase(0, 3, 2, 1, 4, 4, 4, 4);

        // One last time, to make sure we can properly realloc after last case
        deallocationPatternTestCase(0, 1, 2, 3, 4, 4, 4, 4);

        deallocationPatternTestCase(0, 1, 2, 3, 4, 5, 13, 31);
        deallocationPatternTestCase(1, 2, 3, 0, 4, 5, 13, 31);
        deallocationPatternTestCase(2, 3, 0, 1, 4, 5, 13, 31);
        deallocationPatternTestCase(3, 0, 1, 2, 4, 5, 13, 31);
        deallocationPatternTestCase(0, 2, 1, 3, 4, 5, 13, 31);
        deallocationPatternTestCase(2, 1, 3, 0, 4, 5, 13, 31);
        deallocationPatternTestCase(1, 3, 0, 2, 4, 5, 13, 31);
        deallocationPatternTestCase(3, 0, 2, 1, 4, 5, 13, 31);
        deallocationPatternTestCase(0, 1, 3, 2, 4, 5, 13, 31);
        deallocationPatternTestCase(1, 3, 2, 0, 4, 5, 13, 31);
        deallocationPatternTestCase(3, 2, 0, 1, 4, 5, 13, 31);
        deallocationPatternTestCase(2, 0, 1, 3, 4, 5, 13, 31);
        deallocationPatternTestCase(3, 1, 2, 0, 4, 5, 13, 31);
        deallocationPatternTestCase(1, 2, 0, 3, 4, 5, 13, 31);
        deallocationPatternTestCase(2, 0, 3, 1, 4, 5, 13, 31);
        deallocationPatternTestCase(0, 3, 1, 2, 4, 5, 13, 31);
        deallocationPatternTestCase(3, 2, 1, 0, 4, 5, 13, 31);
        deallocationPatternTestCase(2, 1, 0, 3, 4, 5, 13, 31);
        deallocationPatternTestCase(1, 0, 3, 2, 4, 5, 13, 31);
        deallocationPatternTestCase(0, 3, 2, 1, 4, 5, 13, 31);

        // One last time, to make sure we can properly realloc after last case
        deallocationPatternTestCase(0, 1, 2, 3, 4, 5, 13, 31);
    }

    fixedHeapAllocator.Destroy();
}