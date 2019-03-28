#pragma once

#include <system/memory/baseallocator.h>

namespace Shipyard
{
    // Use this class to allocate variably sized chunk of memory from a fixed heap.
    // The FixedHeapAllocator does not take ownership of the memory. It is the responsability of the user to properly free it after
    // calling FixedHeapAllocator::Destroy()
    class SHIPYARD_API FixedHeapAllocator : public BaseAllocator
    {
#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        struct MemoryInfo;
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

    public:
        FixedHeapAllocator();
        ~FixedHeapAllocator();

        bool Create(void* pHeap, size_t heapSize);
        void Destroy();

        // Alignment must be a power of 2 and non-zero.
        virtual void* Allocate(size_t size, size_t alignment
        
                #ifdef SHIP_ALLOCATOR_DEBUG_INFO
                    , const char* pAllocationFilename
                    , int allocationLineNumber
                #endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

                ) override;

        // Memory must come from the allocator that allocated it.
        virtual void Deallocate(void* memory) override;

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        const MemoryInfo& GetMemoryInfo() const { return m_MemoryInfo; }
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

    private:
        struct FreeMemoryBlock
        {
            size_t sizeOfBlockInBytesIncludingThisHeader = 0;

            FreeMemoryBlock* pNextFreeBlock = nullptr;
            FreeMemoryBlock* pPreviousFreeBlock = nullptr;
        };

        // Each MemoryAllocationHeader are at sizeof(MemoryAllocationHeader) bytes before the user allocated region.
        struct MemoryAllocationHeader
        {
            void* pStartOfMemoryAllocationHeaderIncludingAlignmentPadding = nullptr;
            size_t userAllocationRegionSizeInBytes = 0;

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            MemoryAllocationHeader* pNextHeader = nullptr;
            MemoryAllocationHeader* pPreviousHeader = nullptr;

            const char* pAllocationFilename = nullptr;
            int allocationLineNumber = 0;

            char pad[4];
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO
        };

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        struct MemoryInfo
        {
            uint64_t numBlocksAllocated = 0;
            size_t heapSize = 0;
            size_t numBytesUsed = 0;
            size_t numUserBytesAllocated = 0;
            size_t maxAllocatedUserSize = 0;
            size_t minAllocatedUserSize = size_t(-1);
        };

    public:
        // Mainly used for testing purposes.
        static const size_t FreeMemoryBlockSize;
        static const size_t MemoryAllocationHeaderSize;
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

    private:
        void* m_pHeap;
        size_t m_HeapSize;

        FreeMemoryBlock* m_pFirstFreeMemoryBlock;

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        MemoryAllocationHeader* m_pAllocatedBlockHead;

        MemoryInfo m_MemoryInfo;
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO
    };
}