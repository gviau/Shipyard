#include <shipyardunittestprecomp.h>

#include <extern/catch/catch.hpp>

#include <utils/unittestutils.h>

#include <system/memory/linearallocator.h>

#include <thread>

namespace
{
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

    size_t g_TestObjectCount = 0;

    class TestObject
    {
    public:
        TestObject()
        {
            g_TestObjectCount += 1;
        }

        ~TestObject()
        {
            g_TestObjectCount -= 1;
        }

    private:
        void* pDummy;
        uint32_t dummy1;
        float dummy2;
        bool dummy3;
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
        Shipyard::ScoppedBuffer scoppedBuffer(heapSize);

        fixedHeapAllocator.Create(scoppedBuffer.pBuffer, heapSize);
    }

    SECTION("simple allocation")
    {
        const size_t allocSize = 4;
        const size_t heapSize = minHeapSize + allocSize;
        Shipyard::ScoppedBuffer scoppedBuffer(heapSize);

        fixedHeapAllocator.Create(scoppedBuffer.pBuffer, heapSize);

        void* pAlloc = SHIP_ALLOC_EX(&fixedHeapAllocator, allocSize, 1);

        REQUIRE(pAlloc != nullptr);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 1);
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBytesUsed == heapSize);
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == allocSize);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

        SHIP_FREE_EX(&fixedHeapAllocator, pAlloc);

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
        Shipyard::ScoppedBuffer scoppedBuffer(heapSize);

        fixedHeapAllocator.Create(scoppedBuffer.pBuffer, heapSize);

        void* pAlloc = SHIP_ALLOC_EX(&fixedHeapAllocator, allocSize, alignment);

        REQUIRE(pAlloc != nullptr);
        REQUIRE(Shipyard::MemoryUtils::IsAddressAligned(size_t(pAlloc), alignment));

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 1);
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated >= allocSize);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

        SHIP_FREE_EX(&fixedHeapAllocator, pAlloc);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 0);
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == 0);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO
    }
    
    SECTION("two allocations same size")
    {
        const size_t allocSize = 4;
        const size_t heapSize = (minHeapSize + allocSize) * 2;
        Shipyard::ScoppedBuffer scoppedBuffer(heapSize);

        fixedHeapAllocator.Create(scoppedBuffer.pBuffer, heapSize);

        void* pAlloc1 = SHIP_ALLOC_EX(&fixedHeapAllocator, allocSize, 1);

        REQUIRE(pAlloc1 != nullptr);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 1);
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == allocSize);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

        void* pAlloc2 = SHIP_ALLOC_EX(&fixedHeapAllocator, allocSize, 1);

        REQUIRE(pAlloc2 != nullptr);

        REQUIRE(AllocsAreDontOverlap(pAlloc1, allocSize, pAlloc2, allocSize));

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 2);
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == (allocSize * 2));
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

        SHIP_FREE_EX(&fixedHeapAllocator, pAlloc1);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 1);
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == allocSize);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

        SHIP_FREE_EX(&fixedHeapAllocator, pAlloc2);

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
        Shipyard::ScoppedBuffer scoppedBuffer(heapSize);

        fixedHeapAllocator.Create(scoppedBuffer.pBuffer, heapSize);

        void* pAlloc1 = SHIP_ALLOC_EX(&fixedHeapAllocator, allocSize, alignment);

        REQUIRE(pAlloc1 != nullptr);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 1);
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated >= allocSize);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

        void* pAlloc2 = SHIP_ALLOC_EX(&fixedHeapAllocator, allocSize, alignment);

        REQUIRE(pAlloc2 != nullptr);

        REQUIRE(AllocsAreDontOverlap(pAlloc1, allocSize, pAlloc2, allocSize));

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 2);
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated >= (allocSize * 2));
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

        SHIP_FREE_EX(&fixedHeapAllocator, pAlloc1);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 1);
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated >= allocSize);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

        SHIP_FREE_EX(&fixedHeapAllocator, pAlloc2);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 0);
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == 0);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO
    }

    SECTION("two allocations same size realloc")
    {
        const size_t allocSize = 4;
        const size_t heapSize = (minHeapSize + allocSize) * 2;
        Shipyard::ScoppedBuffer scoppedBuffer(heapSize);

        fixedHeapAllocator.Create(scoppedBuffer.pBuffer, heapSize);

        for (uint32_t i = 0; i < 2; i++)
        {
            void* pAlloc1 = SHIP_ALLOC_EX(&fixedHeapAllocator, allocSize, 1);

            REQUIRE(pAlloc1 != nullptr);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 1);
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == allocSize);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            void* pAlloc2 = SHIP_ALLOC_EX(&fixedHeapAllocator, allocSize, 1);

            REQUIRE(pAlloc2 != nullptr);

            REQUIRE(AllocsAreDontOverlap(pAlloc1, allocSize, pAlloc2, allocSize));

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 2);
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == (allocSize * 2));
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            SHIP_FREE_EX(&fixedHeapAllocator, pAlloc1);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 1);
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == allocSize);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            SHIP_FREE_EX(&fixedHeapAllocator, pAlloc2);

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
        Shipyard::ScoppedBuffer scoppedBuffer(heapSize);

        fixedHeapAllocator.Create(scoppedBuffer.pBuffer, heapSize);

        void* pAlloc1 = SHIP_ALLOC_EX(&fixedHeapAllocator, allocSize, 1);

        REQUIRE(pAlloc1 != nullptr);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 1);
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == allocSize);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

        void* pAlloc2 = SHIP_ALLOC_EX(&fixedHeapAllocator, allocSize, 1);

        REQUIRE(pAlloc2 != nullptr);

        REQUIRE(AllocsAreDontOverlap(pAlloc1, allocSize, pAlloc2, allocSize));

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 2);
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == (allocSize * 2));
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

        SHIP_FREE_EX(&fixedHeapAllocator, pAlloc2);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 1);
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == allocSize);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

        SHIP_FREE_EX(&fixedHeapAllocator, pAlloc1);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 0);
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == 0);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO
    }

    SECTION("two allocations same size inversed free realloc")
    {
        const size_t allocSize = 4;
        const size_t heapSize = (minHeapSize + allocSize) * 2;
        Shipyard::ScoppedBuffer scoppedBuffer(heapSize);

        fixedHeapAllocator.Create(scoppedBuffer.pBuffer, heapSize);

        for (uint32_t i = 0; i < 2; i++)
        {
            void* pAlloc1 = SHIP_ALLOC_EX(&fixedHeapAllocator, allocSize, 1);

            REQUIRE(pAlloc1 != nullptr);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 1);
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == allocSize);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            void* pAlloc2 = SHIP_ALLOC_EX(&fixedHeapAllocator, allocSize, 1);

            REQUIRE(pAlloc2 != nullptr);

            REQUIRE(AllocsAreDontOverlap(pAlloc1, allocSize, pAlloc2, allocSize));

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 2);
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == (allocSize * 2));
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            SHIP_FREE_EX(&fixedHeapAllocator, pAlloc2);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 1);
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == allocSize);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            SHIP_FREE_EX(&fixedHeapAllocator, pAlloc1);

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
        Shipyard::ScoppedBuffer scoppedBuffer(heapSize);

        fixedHeapAllocator.Create(scoppedBuffer.pBuffer, heapSize);

        void* pAlloc1 = SHIP_ALLOC_EX(&fixedHeapAllocator, allocSize1, 1);

        REQUIRE(pAlloc1 != nullptr);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 1);
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == allocSize1);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

        void* pAlloc2 = SHIP_ALLOC_EX(&fixedHeapAllocator, allocSize2, 1);

        REQUIRE(pAlloc2 != nullptr);

        REQUIRE(AllocsAreDontOverlap(pAlloc1, allocSize1, pAlloc2, allocSize2));

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 2);
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == (allocSize1 + allocSize2));
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

        SHIP_FREE_EX(&fixedHeapAllocator, pAlloc1);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 1);
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == allocSize2);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

        SHIP_FREE_EX(&fixedHeapAllocator, pAlloc2);

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
        Shipyard::ScoppedBuffer scoppedBuffer(heapSize);

        fixedHeapAllocator.Create(scoppedBuffer.pBuffer, heapSize);

        for (uint32_t i = 0; i < 2; i++)
        {
            void* pAlloc1 = SHIP_ALLOC_EX(&fixedHeapAllocator, allocSize1, 1);

            REQUIRE(pAlloc1 != nullptr);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 1);
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == allocSize1);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            void* pAlloc2 = SHIP_ALLOC_EX(&fixedHeapAllocator, allocSize2, 1);

            REQUIRE(pAlloc2 != nullptr);

            REQUIRE(AllocsAreDontOverlap(pAlloc1, allocSize1, pAlloc2, allocSize2));

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 2);
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == (allocSize1 + allocSize2));
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            SHIP_FREE_EX(&fixedHeapAllocator, pAlloc1);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 1);
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == allocSize2);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            SHIP_FREE_EX(&fixedHeapAllocator, pAlloc2);

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
        Shipyard::ScoppedBuffer scoppedBuffer(heapSize);

        fixedHeapAllocator.Create(scoppedBuffer.pBuffer, heapSize);

        void* pAlloc1 = SHIP_ALLOC_EX(&fixedHeapAllocator, allocSize1, 1);

        REQUIRE(pAlloc1 != nullptr);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 1);
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == allocSize1);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

        void* pAlloc2 = SHIP_ALLOC_EX(&fixedHeapAllocator, allocSize2, 1);

        REQUIRE(pAlloc2 != nullptr);

        REQUIRE(AllocsAreDontOverlap(pAlloc1, allocSize1, pAlloc2, allocSize2));

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 2);
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == (allocSize1 + allocSize2));
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

        SHIP_FREE_EX(&fixedHeapAllocator, pAlloc2);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 1);
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == allocSize1);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

        SHIP_FREE_EX(&fixedHeapAllocator, pAlloc1);

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
        Shipyard::ScoppedBuffer scoppedBuffer(heapSize);

        fixedHeapAllocator.Create(scoppedBuffer.pBuffer, heapSize);

        for (uint32_t i = 0; i < 2; i++)
        {
            void* pAlloc1 = SHIP_ALLOC_EX(&fixedHeapAllocator, allocSize1, 1);

            REQUIRE(pAlloc1 != nullptr);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 1);
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == allocSize1);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            void* pAlloc2 = SHIP_ALLOC_EX(&fixedHeapAllocator, allocSize2, 1);

            REQUIRE(pAlloc2 != nullptr);

            REQUIRE(AllocsAreDontOverlap(pAlloc1, allocSize1, pAlloc2, allocSize2));

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 2);
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == (allocSize1 + allocSize2));
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            SHIP_FREE_EX(&fixedHeapAllocator, pAlloc2);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 1);
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == allocSize1);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            SHIP_FREE_EX(&fixedHeapAllocator, pAlloc1);

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
        Shipyard::ScoppedBuffer scoppedBuffer(heapSize);

        fixedHeapAllocator.Create(scoppedBuffer.pBuffer, heapSize);

        void* pAlloc1 = SHIP_ALLOC_EX(&fixedHeapAllocator, allocSize1, alignment);

        REQUIRE(pAlloc1 != nullptr);
        REQUIRE(Shipyard::MemoryUtils::IsAddressAligned(size_t(pAlloc1), alignment));

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 1);
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated >= allocSize1);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

        void* pAlloc2 = SHIP_ALLOC_EX(&fixedHeapAllocator, allocSize2, alignment);

        REQUIRE(pAlloc2 != nullptr);
        REQUIRE(Shipyard::MemoryUtils::IsAddressAligned(size_t(pAlloc2), alignment));

        REQUIRE(AllocsAreDontOverlap(pAlloc1, allocSize1, pAlloc2, allocSize2));

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 2);
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated >= (allocSize1 + allocSize2));
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

        SHIP_FREE_EX(&fixedHeapAllocator, pAlloc1);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 1);
        REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated >= allocSize2);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

        SHIP_FREE_EX(&fixedHeapAllocator, pAlloc2);

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
        Shipyard::ScoppedBuffer scoppedBuffer(heapSize);

        fixedHeapAllocator.Create(scoppedBuffer.pBuffer, heapSize);

        for (uint32_t i = 0; i < 2; i++)
        {
            void* pAlloc1 = SHIP_ALLOC_EX(&fixedHeapAllocator, allocSize1, alignment);

            REQUIRE(pAlloc1 != nullptr);
            REQUIRE(Shipyard::MemoryUtils::IsAddressAligned(size_t(pAlloc1), alignment));

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 1);
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated >= allocSize1);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            void* pAlloc2 = SHIP_ALLOC_EX(&fixedHeapAllocator, allocSize2, alignment);

            REQUIRE(pAlloc2 != nullptr);
            REQUIRE(Shipyard::MemoryUtils::IsAddressAligned(size_t(pAlloc2), alignment));

            REQUIRE(AllocsAreDontOverlap(pAlloc1, allocSize1, pAlloc2, allocSize2));

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 2);
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated >= (allocSize1 + allocSize2));
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            SHIP_FREE_EX(&fixedHeapAllocator, pAlloc1);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 1);
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated >= allocSize2);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            SHIP_FREE_EX(&fixedHeapAllocator, pAlloc2);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 0);
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == 0);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO
        }
    }
    SECTION("three allocations")
    {
        const size_t heapSize = (minHeapSize + 32) * 3;
        Shipyard::ScoppedBuffer scoppedBuffer(heapSize);

        fixedHeapAllocator.Create(scoppedBuffer.pBuffer, heapSize);

        auto deallocationPatternTestCase = [&fixedHeapAllocator](
                uint32_t first, uint32_t second, uint32_t third,
                uint32_t allocSize1, uint32_t allocSize2, uint32_t allocSize3)
        {
            void* pAlloc1 = SHIP_ALLOC_EX(&fixedHeapAllocator, allocSize1, 1);

            REQUIRE(pAlloc1 != nullptr);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 1);
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == allocSize1);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            void* pAlloc2 = SHIP_ALLOC_EX(&fixedHeapAllocator, allocSize2, 1);

            REQUIRE(pAlloc2 != nullptr);

            REQUIRE(AllocsAreDontOverlap(pAlloc1, allocSize1, pAlloc2, allocSize2));

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 2);
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == allocSize1 + allocSize2);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            void* pAlloc3 = SHIP_ALLOC_EX(&fixedHeapAllocator, allocSize3, 1);

            REQUIRE(pAlloc3 != nullptr);

            REQUIRE(AllocsAreDontOverlap(pAlloc1, allocSize1, pAlloc3, allocSize3));
            REQUIRE(AllocsAreDontOverlap(pAlloc2, allocSize2, pAlloc3, allocSize3));

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 3);
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == allocSize1 + allocSize2 + allocSize3);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            void* pAllocs[] = { pAlloc1, pAlloc2, pAlloc3 };
            size_t allocSizes[] = { allocSize1, allocSize2, allocSize3 };

            SHIP_FREE_EX(&fixedHeapAllocator, pAllocs[first]);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 2);
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == allocSizes[second] + allocSizes[third]);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            SHIP_FREE_EX(&fixedHeapAllocator, pAllocs[second]);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 1);
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == allocSizes[third]);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            SHIP_FREE_EX(&fixedHeapAllocator, pAllocs[third]);

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

        deallocationPatternTestCase(0, 1, 2, 4, 8, 16);
        deallocationPatternTestCase(1, 2, 0, 4, 8, 16);
        deallocationPatternTestCase(2, 1, 0, 4, 8, 16);
        deallocationPatternTestCase(2, 0, 1, 4, 8, 16);
        deallocationPatternTestCase(0, 2, 1, 4, 8, 16);
        deallocationPatternTestCase(1, 0, 2, 4, 8, 16);

        // One last time, to make sure we can properly realloc after last case
        deallocationPatternTestCase(0, 1, 2, 4, 8, 16);
    }

    SECTION("four allocations")
    {
        const size_t heapSize = (minHeapSize + 32) * 4;
        Shipyard::ScoppedBuffer scoppedBuffer(heapSize);

        fixedHeapAllocator.Create(scoppedBuffer.pBuffer, heapSize);

        auto deallocationPatternTestCase = [&fixedHeapAllocator](
                uint32_t first, uint32_t second, uint32_t third, uint32_t fourth,
                uint32_t allocSize1, uint32_t allocSize2, uint32_t allocSize3, uint32_t allocSize4)
        {
            void* pAlloc1 = SHIP_ALLOC_EX(&fixedHeapAllocator, allocSize1, 1);

            REQUIRE(pAlloc1 != nullptr);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 1);
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == allocSize1);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            void* pAlloc2 = SHIP_ALLOC_EX(&fixedHeapAllocator, allocSize2, 1);

            REQUIRE(pAlloc2 != nullptr);

            REQUIRE(AllocsAreDontOverlap(pAlloc1, allocSize1, pAlloc2, allocSize2));

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 2);
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == allocSize1 + allocSize2);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            void* pAlloc3 = SHIP_ALLOC_EX(&fixedHeapAllocator, allocSize3, 1);

            REQUIRE(pAlloc3 != nullptr);

            REQUIRE(AllocsAreDontOverlap(pAlloc1, allocSize1, pAlloc3, allocSize3));
            REQUIRE(AllocsAreDontOverlap(pAlloc2, allocSize2, pAlloc3, allocSize3));

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 3);
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == allocSize1 + allocSize2 + allocSize3);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            void* pAlloc4 = SHIP_ALLOC_EX(&fixedHeapAllocator, allocSize4, 1);

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

            SHIP_FREE_EX(&fixedHeapAllocator, pAllocs[first]);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 3);
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == allocSizes[second] + allocSizes[third] + allocSizes[fourth]);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            SHIP_FREE_EX(&fixedHeapAllocator, pAllocs[second]);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 2);
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == allocSizes[third] + allocSizes[fourth]);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            SHIP_FREE_EX(&fixedHeapAllocator, pAllocs[third]);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numBlocksAllocated == 1);
            REQUIRE(fixedHeapAllocator.GetMemoryInfo().numUserBytesAllocated == allocSizes[fourth]);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            SHIP_FREE_EX(&fixedHeapAllocator, pAllocs[fourth]);

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

        deallocationPatternTestCase(0, 1, 2, 3, 4, 8, 16, 32);
        deallocationPatternTestCase(1, 2, 3, 0, 4, 8, 16, 32);
        deallocationPatternTestCase(2, 3, 0, 1, 4, 8, 16, 32);
        deallocationPatternTestCase(3, 0, 1, 2, 4, 8, 16, 32);
        deallocationPatternTestCase(0, 2, 1, 3, 4, 8, 16, 32);
        deallocationPatternTestCase(2, 1, 3, 0, 4, 8, 16, 32);
        deallocationPatternTestCase(1, 3, 0, 2, 4, 8, 16, 32);
        deallocationPatternTestCase(3, 0, 2, 1, 4, 8, 16, 32);
        deallocationPatternTestCase(0, 1, 3, 2, 4, 8, 16, 32);
        deallocationPatternTestCase(1, 3, 2, 0, 4, 8, 16, 32);
        deallocationPatternTestCase(3, 2, 0, 1, 4, 8, 16, 32);
        deallocationPatternTestCase(2, 0, 1, 3, 4, 8, 16, 32);
        deallocationPatternTestCase(3, 1, 2, 0, 4, 8, 16, 32);
        deallocationPatternTestCase(1, 2, 0, 3, 4, 8, 16, 32);
        deallocationPatternTestCase(2, 0, 3, 1, 4, 8, 16, 32);
        deallocationPatternTestCase(0, 3, 1, 2, 4, 8, 16, 32);
        deallocationPatternTestCase(3, 2, 1, 0, 4, 8, 16, 32);
        deallocationPatternTestCase(2, 1, 0, 3, 4, 8, 16, 32);
        deallocationPatternTestCase(1, 0, 3, 2, 4, 8, 16, 32);
        deallocationPatternTestCase(0, 3, 2, 1, 4, 8, 16, 32);

        // One last time, to make sure we can properly realloc after last case
        deallocationPatternTestCase(0, 1, 2, 3, 4, 8, 16, 32);
    }

    SECTION("Array allocations")
    {
        const size_t arrayLength = 10;
        const size_t heapSize = (minHeapSize + 64 + sizeof(TestObject) * arrayLength) * 4;
        Shipyard::ScoppedBuffer scoppedBuffer(heapSize);

        fixedHeapAllocator.Create(scoppedBuffer.pBuffer, heapSize);

        auto deallocationPatternTestCase = [&fixedHeapAllocator, arrayLength](
            uint32_t first, uint32_t second, uint32_t third, uint32_t fourth,
            uint32_t alignment1, uint32_t alignment2, uint32_t alignment3, uint32_t alignment4)
        {
            TestObject* pAlloc1 = SHIP_NEW_ARRAY_EX(&fixedHeapAllocator, TestObject, arrayLength, alignment1);

            REQUIRE(pAlloc1 != nullptr);
            REQUIRE(g_TestObjectCount == arrayLength);

            TestObject* pAlloc2 = SHIP_NEW_ARRAY_EX(&fixedHeapAllocator, TestObject, arrayLength, alignment2);

            REQUIRE(pAlloc2 != nullptr);
            REQUIRE(g_TestObjectCount == arrayLength * 2);

            TestObject* pAlloc3 = SHIP_NEW_ARRAY_EX(&fixedHeapAllocator, TestObject, arrayLength, alignment3);

            REQUIRE(pAlloc3 != nullptr);
            REQUIRE(g_TestObjectCount == arrayLength * 3);

            TestObject* pAlloc4 = SHIP_NEW_ARRAY_EX(&fixedHeapAllocator, TestObject, arrayLength, alignment4);

            REQUIRE(pAlloc4 != nullptr);
            REQUIRE(g_TestObjectCount == arrayLength * 4);

            TestObject* pAllocs[] = { pAlloc1, pAlloc2, pAlloc3, pAlloc4 };

            SHIP_DELETE_ARRAY_EX(&fixedHeapAllocator, pAllocs[first]);
            SHIP_DELETE_ARRAY_EX(&fixedHeapAllocator, pAllocs[second]);
            SHIP_DELETE_ARRAY_EX(&fixedHeapAllocator, pAllocs[third]);
            SHIP_DELETE_ARRAY_EX(&fixedHeapAllocator, pAllocs[fourth]);

            REQUIRE(g_TestObjectCount == 0);
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

        deallocationPatternTestCase(0, 1, 2, 3, 4, 8, 16, 32);
        deallocationPatternTestCase(1, 2, 3, 0, 4, 8, 16, 32);
        deallocationPatternTestCase(2, 3, 0, 1, 4, 8, 16, 32);
        deallocationPatternTestCase(3, 0, 1, 2, 4, 8, 16, 32);
        deallocationPatternTestCase(0, 2, 1, 3, 4, 8, 16, 32);
        deallocationPatternTestCase(2, 1, 3, 0, 4, 8, 16, 32);
        deallocationPatternTestCase(1, 3, 0, 2, 4, 8, 16, 32);
        deallocationPatternTestCase(3, 0, 2, 1, 4, 8, 16, 32);
        deallocationPatternTestCase(0, 1, 3, 2, 4, 8, 16, 32);
        deallocationPatternTestCase(1, 3, 2, 0, 4, 8, 16, 32);
        deallocationPatternTestCase(3, 2, 0, 1, 4, 8, 16, 32);
        deallocationPatternTestCase(2, 0, 1, 3, 4, 8, 16, 32);
        deallocationPatternTestCase(3, 1, 2, 0, 4, 8, 16, 32);
        deallocationPatternTestCase(1, 2, 0, 3, 4, 8, 16, 32);
        deallocationPatternTestCase(2, 0, 3, 1, 4, 8, 16, 32);
        deallocationPatternTestCase(0, 3, 1, 2, 4, 8, 16, 32);
        deallocationPatternTestCase(3, 2, 1, 0, 4, 8, 16, 32);
        deallocationPatternTestCase(2, 1, 0, 3, 4, 8, 16, 32);
        deallocationPatternTestCase(1, 0, 3, 2, 4, 8, 16, 32);
        deallocationPatternTestCase(0, 3, 2, 1, 4, 8, 16, 32);

        // One last time, to make sure we can properly realloc after last case
        deallocationPatternTestCase(0, 1, 2, 3, 4, 8, 16, 32);

    }

    fixedHeapAllocator.Destroy();
}

TEST_CASE("Test PoolAllocator", "[Allocator]")
{
    Shipyard::PoolAllocator poolAllocator;

    const size_t minChunkSize = sizeof(void*);
    const size_t minHeapSize = minChunkSize * 2;

    SECTION("creating allocator")
    {
        const size_t heapSize = 64;
        Shipyard::ScoppedBuffer scoppedBuffer(heapSize, minChunkSize);

        poolAllocator.Create(scoppedBuffer.pBuffer, 1, minChunkSize);
    }

    SECTION("simple allocation")
    {
        const size_t chunkSize = minChunkSize;
        const size_t heapSize = chunkSize * 2;
        Shipyard::ScoppedBuffer scoppedBuffer(heapSize, chunkSize);

        poolAllocator.Create(scoppedBuffer.pBuffer, 1, chunkSize);

        void* pAlloc = SHIP_ALLOC_EX(&poolAllocator, chunkSize, 1);

        REQUIRE(pAlloc != nullptr);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(poolAllocator.GetMemoryInfo().numBlocksAllocated == 1);
        REQUIRE(poolAllocator.GetMemoryInfo().numBytesUsed == chunkSize);
        REQUIRE(poolAllocator.GetMemoryInfo().numUserBytesAllocated == chunkSize);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

        SHIP_FREE_EX(&poolAllocator, pAlloc);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(poolAllocator.GetMemoryInfo().numBlocksAllocated == 0);
        REQUIRE(poolAllocator.GetMemoryInfo().numBytesUsed == 0);
        REQUIRE(poolAllocator.GetMemoryInfo().numUserBytesAllocated == 0);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO
    }

    SECTION("simple aligned allocation")
    {
        const size_t alignment = 16;
        const size_t chunkSize = 8;
        const size_t heapSize = alignment * 2 + chunkSize;
        Shipyard::ScoppedBuffer scoppedBuffer(heapSize, chunkSize);

        poolAllocator.Create(scoppedBuffer.pBuffer, 4, chunkSize);

        void* pAlloc = SHIP_ALLOC_EX(&poolAllocator, chunkSize, alignment);

        REQUIRE(pAlloc != nullptr);
        REQUIRE(Shipyard::MemoryUtils::IsAddressAligned(size_t(pAlloc), alignment));

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(poolAllocator.GetMemoryInfo().numBlocksAllocated == 1);
        REQUIRE(poolAllocator.GetMemoryInfo().numUserBytesAllocated == chunkSize);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

        SHIP_FREE_EX(&poolAllocator, pAlloc);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(poolAllocator.GetMemoryInfo().numBlocksAllocated == 0);
        REQUIRE(poolAllocator.GetMemoryInfo().numUserBytesAllocated == 0);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO
    }

    SECTION("two allocations same size")
    {
        const size_t chunkSize = minChunkSize;
        const size_t heapSize = chunkSize * 2 + chunkSize;
        Shipyard::ScoppedBuffer scoppedBuffer(heapSize, chunkSize);

        poolAllocator.Create(scoppedBuffer.pBuffer, 2, chunkSize);

        void* pAlloc1 = SHIP_ALLOC_EX(&poolAllocator, chunkSize, 1);

        REQUIRE(pAlloc1 != nullptr);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(poolAllocator.GetMemoryInfo().numBlocksAllocated == 1);
        REQUIRE(poolAllocator.GetMemoryInfo().numUserBytesAllocated == chunkSize);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

        void* pAlloc2 = SHIP_ALLOC_EX(&poolAllocator, chunkSize, 1);

        REQUIRE(pAlloc2 != nullptr);

        REQUIRE(AllocsAreDontOverlap(pAlloc1, chunkSize, pAlloc2, chunkSize));

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(poolAllocator.GetMemoryInfo().numBlocksAllocated == 2);
        REQUIRE(poolAllocator.GetMemoryInfo().numUserBytesAllocated == chunkSize * 2);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

        SHIP_FREE_EX(&poolAllocator, pAlloc1);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(poolAllocator.GetMemoryInfo().numBlocksAllocated == 1);
        REQUIRE(poolAllocator.GetMemoryInfo().numUserBytesAllocated == chunkSize);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

        SHIP_FREE_EX(&poolAllocator, pAlloc2);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(poolAllocator.GetMemoryInfo().numBlocksAllocated == 0);
        REQUIRE(poolAllocator.GetMemoryInfo().numUserBytesAllocated == 0);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO
    }

    SECTION("two aligned allocations same size")
    {
        const size_t chunkSize = minChunkSize;
        const size_t alignment = 16;
        const size_t heapSize = alignment * 4 + chunkSize;
        Shipyard::ScoppedBuffer scoppedBuffer(heapSize, chunkSize);

        poolAllocator.Create(scoppedBuffer.pBuffer, 8, chunkSize);

        void* pAlloc1 = SHIP_ALLOC_EX(&poolAllocator, chunkSize, alignment);

        REQUIRE(pAlloc1 != nullptr);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(poolAllocator.GetMemoryInfo().numBlocksAllocated == 1);
        REQUIRE(poolAllocator.GetMemoryInfo().numUserBytesAllocated == chunkSize);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

        void* pAlloc2 = SHIP_ALLOC_EX(&poolAllocator, chunkSize, alignment);

        REQUIRE(pAlloc2 != nullptr);

        REQUIRE(AllocsAreDontOverlap(pAlloc1, chunkSize, pAlloc2, chunkSize));

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(poolAllocator.GetMemoryInfo().numBlocksAllocated == 2);
        REQUIRE(poolAllocator.GetMemoryInfo().numUserBytesAllocated == chunkSize * 2);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

        SHIP_FREE_EX(&poolAllocator, pAlloc1);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(poolAllocator.GetMemoryInfo().numBlocksAllocated == 1);
        REQUIRE(poolAllocator.GetMemoryInfo().numUserBytesAllocated == chunkSize);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

        SHIP_FREE_EX(&poolAllocator, pAlloc2);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        REQUIRE(poolAllocator.GetMemoryInfo().numBlocksAllocated == 0);
        REQUIRE(poolAllocator.GetMemoryInfo().numUserBytesAllocated == 0);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO
    }

    SECTION("three allocations")
    {
        const size_t chunkSize = 32;
        const size_t heapSize = chunkSize * 3 + chunkSize;
        Shipyard::ScoppedBuffer scoppedBuffer(heapSize, chunkSize);

        poolAllocator.Create(scoppedBuffer.pBuffer, 3, chunkSize);

        auto deallocationPatternTestCase = [&poolAllocator](
            uint32_t first, uint32_t second, uint32_t third,
            size_t chunkSize)
        {
            void* pAlloc1 = SHIP_ALLOC_EX(&poolAllocator, chunkSize, 1);

            REQUIRE(pAlloc1 != nullptr);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(poolAllocator.GetMemoryInfo().numBlocksAllocated == 1);
            REQUIRE(poolAllocator.GetMemoryInfo().numUserBytesAllocated == chunkSize);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            void* pAlloc2 = SHIP_ALLOC_EX(&poolAllocator, chunkSize, 1);

            REQUIRE(pAlloc2 != nullptr);

            REQUIRE(AllocsAreDontOverlap(pAlloc1, chunkSize, pAlloc2, chunkSize));

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(poolAllocator.GetMemoryInfo().numBlocksAllocated == 2);
            REQUIRE(poolAllocator.GetMemoryInfo().numUserBytesAllocated == chunkSize + chunkSize);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            void* pAlloc3 = SHIP_ALLOC_EX(&poolAllocator, chunkSize, 1);

            REQUIRE(pAlloc3 != nullptr);

            REQUIRE(AllocsAreDontOverlap(pAlloc1, chunkSize, pAlloc3, chunkSize));
            REQUIRE(AllocsAreDontOverlap(pAlloc2, chunkSize, pAlloc3, chunkSize));

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(poolAllocator.GetMemoryInfo().numBlocksAllocated == 3);
            REQUIRE(poolAllocator.GetMemoryInfo().numUserBytesAllocated == chunkSize + chunkSize + chunkSize);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            void* pAllocs[] = { pAlloc1, pAlloc2, pAlloc3 };
            size_t allocSizes[] = { chunkSize, chunkSize, chunkSize };

            SHIP_FREE_EX(&poolAllocator, pAllocs[first]);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(poolAllocator.GetMemoryInfo().numBlocksAllocated == 2);
            REQUIRE(poolAllocator.GetMemoryInfo().numUserBytesAllocated == allocSizes[second] + allocSizes[third]);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            SHIP_FREE_EX(&poolAllocator, pAllocs[second]);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(poolAllocator.GetMemoryInfo().numBlocksAllocated == 1);
            REQUIRE(poolAllocator.GetMemoryInfo().numUserBytesAllocated == allocSizes[third]);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            SHIP_FREE_EX(&poolAllocator, pAllocs[third]);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(poolAllocator.GetMemoryInfo().numBlocksAllocated == 0);
            REQUIRE(poolAllocator.GetMemoryInfo().numUserBytesAllocated == 0);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO
        };

        deallocationPatternTestCase(0, 1, 2, chunkSize);
        deallocationPatternTestCase(1, 2, 0, chunkSize);
        deallocationPatternTestCase(2, 1, 0, chunkSize);
        deallocationPatternTestCase(2, 0, 1, chunkSize);
        deallocationPatternTestCase(0, 2, 1, chunkSize);
        deallocationPatternTestCase(1, 0, 2, chunkSize);

        // One last time, to make sure we can properly realloc after last case
        deallocationPatternTestCase(0, 1, 2, chunkSize);
    }

    SECTION("four allocations")
    {
        const size_t chunkSize = 16;
        const size_t heapSize = 16 * 4 + chunkSize;
        Shipyard::ScoppedBuffer scoppedBuffer(heapSize, chunkSize);

        poolAllocator.Create(scoppedBuffer.pBuffer, 4, chunkSize);

        auto deallocationPatternTestCase = [&poolAllocator](
            uint32_t first, uint32_t second, uint32_t third, uint32_t fourth,
            size_t chunkSize)
        {
            void* pAlloc1 = SHIP_ALLOC_EX(&poolAllocator, chunkSize, 1);

            REQUIRE(pAlloc1 != nullptr);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(poolAllocator.GetMemoryInfo().numBlocksAllocated == 1);
            REQUIRE(poolAllocator.GetMemoryInfo().numUserBytesAllocated == chunkSize);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            void* pAlloc2 = SHIP_ALLOC_EX(&poolAllocator, chunkSize, 1);

            REQUIRE(pAlloc2 != nullptr);

            REQUIRE(AllocsAreDontOverlap(pAlloc1, chunkSize, pAlloc2, chunkSize));

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(poolAllocator.GetMemoryInfo().numBlocksAllocated == 2);
            REQUIRE(poolAllocator.GetMemoryInfo().numUserBytesAllocated == chunkSize + chunkSize);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            void* pAlloc3 = SHIP_ALLOC_EX(&poolAllocator, chunkSize, 1);

            REQUIRE(pAlloc3 != nullptr);

            REQUIRE(AllocsAreDontOverlap(pAlloc1, chunkSize, pAlloc3, chunkSize));
            REQUIRE(AllocsAreDontOverlap(pAlloc2, chunkSize, pAlloc3, chunkSize));

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(poolAllocator.GetMemoryInfo().numBlocksAllocated == 3);
            REQUIRE(poolAllocator.GetMemoryInfo().numUserBytesAllocated == chunkSize + chunkSize + chunkSize);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            void* pAlloc4 = SHIP_ALLOC_EX(&poolAllocator, chunkSize, 1);

            REQUIRE(pAlloc4 != nullptr);

            REQUIRE(AllocsAreDontOverlap(pAlloc1, chunkSize, pAlloc4, chunkSize));
            REQUIRE(AllocsAreDontOverlap(pAlloc2, chunkSize, pAlloc4, chunkSize));
            REQUIRE(AllocsAreDontOverlap(pAlloc3, chunkSize, pAlloc4, chunkSize));

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(poolAllocator.GetMemoryInfo().numBlocksAllocated == 4);
            REQUIRE(poolAllocator.GetMemoryInfo().numUserBytesAllocated == chunkSize + chunkSize + chunkSize + chunkSize);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            void* pAllocs[] = { pAlloc1, pAlloc2, pAlloc3, pAlloc4 };
            size_t allocSizes[] = { chunkSize, chunkSize, chunkSize, chunkSize };

            SHIP_FREE_EX(&poolAllocator, pAllocs[first]);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(poolAllocator.GetMemoryInfo().numBlocksAllocated == 3);
            REQUIRE(poolAllocator.GetMemoryInfo().numUserBytesAllocated == allocSizes[second] + allocSizes[third] + allocSizes[fourth]);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            SHIP_FREE_EX(&poolAllocator, pAllocs[second]);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(poolAllocator.GetMemoryInfo().numBlocksAllocated == 2);
            REQUIRE(poolAllocator.GetMemoryInfo().numUserBytesAllocated == allocSizes[third] + allocSizes[fourth]);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            SHIP_FREE_EX(&poolAllocator, pAllocs[third]);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(poolAllocator.GetMemoryInfo().numBlocksAllocated == 1);
            REQUIRE(poolAllocator.GetMemoryInfo().numUserBytesAllocated == allocSizes[fourth]);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            SHIP_FREE_EX(&poolAllocator, pAllocs[fourth]);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            REQUIRE(poolAllocator.GetMemoryInfo().numBlocksAllocated == 0);
            REQUIRE(poolAllocator.GetMemoryInfo().numUserBytesAllocated == 0);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO
        };

        deallocationPatternTestCase(0, 1, 2, 3, chunkSize);
        deallocationPatternTestCase(1, 2, 3, 0, chunkSize);
        deallocationPatternTestCase(2, 3, 0, 1, chunkSize);
        deallocationPatternTestCase(3, 0, 1, 2, chunkSize);
        deallocationPatternTestCase(0, 2, 1, 3, chunkSize);
        deallocationPatternTestCase(2, 1, 3, 0, chunkSize);
        deallocationPatternTestCase(1, 3, 0, 2, chunkSize);
        deallocationPatternTestCase(3, 0, 2, 1, chunkSize);
        deallocationPatternTestCase(0, 1, 3, 2, chunkSize);
        deallocationPatternTestCase(1, 3, 2, 0, chunkSize);
        deallocationPatternTestCase(3, 2, 0, 1, chunkSize);
        deallocationPatternTestCase(2, 0, 1, 3, chunkSize);
        deallocationPatternTestCase(3, 1, 2, 0, chunkSize);
        deallocationPatternTestCase(1, 2, 0, 3, chunkSize);
        deallocationPatternTestCase(2, 0, 3, 1, chunkSize);
        deallocationPatternTestCase(0, 3, 1, 2, chunkSize);
        deallocationPatternTestCase(3, 2, 1, 0, chunkSize);
        deallocationPatternTestCase(2, 1, 0, 3, chunkSize);
        deallocationPatternTestCase(1, 0, 3, 2, chunkSize);
        deallocationPatternTestCase(0, 3, 2, 1, chunkSize);

        // One last time, to make sure we can properly realloc after last case
        deallocationPatternTestCase(0, 1, 2, 3, chunkSize);
    }

    SECTION("Array allocations")
    {
        const size_t arrayLength = 10;
        const size_t chunkSize = ((sizeof(TestObject) << 1) & (~sizeof(TestObject))) * 10;
        const size_t sizeForArraySizeHeader = sizeof(size_t) * 2;

        const size_t heapSize = (chunkSize * arrayLength + sizeForArraySizeHeader + 32) * 4 + chunkSize;
        Shipyard::ScoppedBuffer scoppedBuffer(heapSize, chunkSize);

        const size_t numChunks = heapSize / chunkSize;

        poolAllocator.Create(scoppedBuffer.pBuffer, numChunks, chunkSize);

        auto deallocationPatternTestCase = [&poolAllocator, arrayLength](
            uint32_t first, uint32_t second, uint32_t third, uint32_t fourth,
            uint32_t alignment1, uint32_t alignment2, uint32_t alignment3, uint32_t alignment4)
        {
            TestObject* pAlloc1 = SHIP_NEW_ARRAY_EX(&poolAllocator, TestObject, arrayLength, alignment1);

            REQUIRE(pAlloc1 != nullptr);
            REQUIRE(g_TestObjectCount == arrayLength);

            TestObject* pAlloc2 = SHIP_NEW_ARRAY_EX(&poolAllocator, TestObject, arrayLength, alignment2);

            REQUIRE(pAlloc2 != nullptr);
            REQUIRE(g_TestObjectCount == arrayLength * 2);

            TestObject* pAlloc3 = SHIP_NEW_ARRAY_EX(&poolAllocator, TestObject, arrayLength, alignment3);

            REQUIRE(pAlloc3 != nullptr);
            REQUIRE(g_TestObjectCount == arrayLength * 3);

            TestObject* pAlloc4 = SHIP_NEW_ARRAY_EX(&poolAllocator, TestObject, arrayLength, alignment4);

            REQUIRE(pAlloc4 != nullptr);
            REQUIRE(g_TestObjectCount == arrayLength * 4);

            TestObject* pAllocs[] = { pAlloc1, pAlloc2, pAlloc3, pAlloc4 };

            SHIP_DELETE_ARRAY_EX(&poolAllocator, pAllocs[first]);
            SHIP_DELETE_ARRAY_EX(&poolAllocator, pAllocs[second]);
            SHIP_DELETE_ARRAY_EX(&poolAllocator, pAllocs[third]);
            SHIP_DELETE_ARRAY_EX(&poolAllocator, pAllocs[fourth]);

            REQUIRE(g_TestObjectCount == 0);
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

        deallocationPatternTestCase(0, 1, 2, 3, 4, 8, 16, 32);
        deallocationPatternTestCase(1, 2, 3, 0, 4, 8, 16, 32);
        deallocationPatternTestCase(2, 3, 0, 1, 4, 8, 16, 32);
        deallocationPatternTestCase(3, 0, 1, 2, 4, 8, 16, 32);
        deallocationPatternTestCase(0, 2, 1, 3, 4, 8, 16, 32);
        deallocationPatternTestCase(2, 1, 3, 0, 4, 8, 16, 32);
        deallocationPatternTestCase(1, 3, 0, 2, 4, 8, 16, 32);
        deallocationPatternTestCase(3, 0, 2, 1, 4, 8, 16, 32);
        deallocationPatternTestCase(0, 1, 3, 2, 4, 8, 16, 32);
        deallocationPatternTestCase(1, 3, 2, 0, 4, 8, 16, 32);
        deallocationPatternTestCase(3, 2, 0, 1, 4, 8, 16, 32);
        deallocationPatternTestCase(2, 0, 1, 3, 4, 8, 16, 32);
        deallocationPatternTestCase(3, 1, 2, 0, 4, 8, 16, 32);
        deallocationPatternTestCase(1, 2, 0, 3, 4, 8, 16, 32);
        deallocationPatternTestCase(2, 0, 3, 1, 4, 8, 16, 32);
        deallocationPatternTestCase(0, 3, 1, 2, 4, 8, 16, 32);
        deallocationPatternTestCase(3, 2, 1, 0, 4, 8, 16, 32);
        deallocationPatternTestCase(2, 1, 0, 3, 4, 8, 16, 32);
        deallocationPatternTestCase(1, 0, 3, 2, 4, 8, 16, 32);
        deallocationPatternTestCase(0, 3, 2, 1, 4, 8, 16, 32);

        // One last time, to make sure we can properly realloc after last case
        deallocationPatternTestCase(0, 1, 2, 3, 4, 8, 16, 32);

    }

    poolAllocator.Destroy();
}

class LinearAllocatorTestThread
{
public:
    LinearAllocatorTestThread(Shipyard::LinearAllocator* pLinearAllocator, size_t numAllocationsToDo, size_t allocSize, size_t allocAlignment)
        : m_pLinearAllocator(pLinearAllocator)
        , m_NumAllocationsDoneAndValid(0)
        , m_NumAllocationsToDo(numAllocationsToDo)
        , m_AllocSize(allocSize)
        , m_AllocAlignment(allocAlignment)
    {
        m_LinearAllocatorTestThread = std::thread(&LinearAllocatorTestThread::LinearAllocatorTestThreadFunction, this);
    }

    bool ValidateAllocations()
    {
        return (m_NumAllocationsDoneAndValid == m_NumAllocationsToDo);
    }

    void WaitForThreadToFinish()
    {
        m_LinearAllocatorTestThread.join();
    }

private:
    Shipyard::LinearAllocator* m_pLinearAllocator;
    size_t m_NumAllocationsDoneAndValid;
    size_t m_NumAllocationsToDo;
    size_t m_AllocSize;
    size_t m_AllocAlignment;

    void LinearAllocatorTestThreadFunction()
    {
        for (size_t i = 0; i < m_NumAllocationsToDo; i++)
        {
            void* pAlloc = SHIP_ALLOC_EX(m_pLinearAllocator, m_AllocSize, m_AllocAlignment);

            if (pAlloc != nullptr && Shipyard::MemoryUtils::IsAddressAligned(size_t(pAlloc), m_AllocAlignment))
            {
                m_NumAllocationsDoneAndValid += 1;
            }
        }
    }

    std::thread m_LinearAllocatorTestThread;
};

TEST_CASE("Test LinearAllocator", "[Allocator]")
{
    Shipyard::LinearAllocator linearAllocator;

    SECTION("creating allocator")
    {
        const size_t heapSize = 64;
        Shipyard::ScoppedBuffer scoppedBuffer(heapSize);

        linearAllocator.Create(scoppedBuffer.pBuffer, heapSize);
    }

    SECTION("simple allocation")
    {
        const size_t allocSize = 4;
        const size_t heapSize = allocSize;
        Shipyard::ScoppedBuffer scoppedBuffer(heapSize);

        linearAllocator.Create(scoppedBuffer.pBuffer, heapSize);

        void* pAlloc = SHIP_ALLOC_EX(&linearAllocator, allocSize, 1);

        REQUIRE(pAlloc != nullptr);
    }

    SECTION("simple aligned allocation")
    {
        const size_t allocSize = 4;
        const size_t alignment = 16;
        const size_t heapSize = allocSize + alignment;
        Shipyard::ScoppedBuffer scoppedBuffer(heapSize);

        linearAllocator.Create(scoppedBuffer.pBuffer, heapSize);

        void* pAlloc = SHIP_ALLOC_EX(&linearAllocator, allocSize, alignment);

        REQUIRE(pAlloc != nullptr);
        REQUIRE(Shipyard::MemoryUtils::IsAddressAligned(size_t(pAlloc), alignment));
    }

    SECTION("Allocations from multiple threads")
    {
        const size_t numThreads = 8;

        const size_t allocSizes[] =
        {
            1,
            2,
            3,
            4,
            8,
            13,
            16,
            32
        };

        const size_t alignments[] =
        {
            1,
            2,
            1,
            4,
            8,
            16,
            16,
            16
        };

        const size_t numAllocationsForEachThread = 10000;

        // Enough space for biggest allocation & biggest alignment
        const size_t heapSize = numThreads * numAllocationsForEachThread * 32 * 16;

        Shipyard::ScoppedBuffer scoppedBuffer(heapSize);

        linearAllocator.Create(scoppedBuffer.pBuffer, heapSize);

        LinearAllocatorTestThread* linearAllocatorTestThreads[numThreads];

        for (size_t i = 0; i < numThreads; i++)
        {
            linearAllocatorTestThreads[i] = new LinearAllocatorTestThread(&linearAllocator, numAllocationsForEachThread, allocSizes[i], alignments[i]);
        }

        for (size_t i = 0; i < numThreads; i++)
        {
            linearAllocatorTestThreads[i]->WaitForThreadToFinish();
        }

        for (size_t i = 0; i < numThreads; i++)
        {
            REQUIRE(linearAllocatorTestThreads[i]->ValidateAllocations());

            delete linearAllocatorTestThreads[i];
        }
    }

    linearAllocator.Destroy();
}