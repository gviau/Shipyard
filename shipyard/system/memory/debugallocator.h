#pragma once

#include <system/memory/baseallocator.h>

#ifdef SHIP_ALLOCATOR_DEBUG_INFO

#include <system/singleton.h>

#include <mutex>

namespace Shipyard
{
    // This allocator is not really an allocator, just a debugging class to log allocations. As such, it
    // doesn't have the same interface as the BaseAllocator, since we'd like to log the result of an other
    // allocation.
    class SHIPYARD_API DebugAllocator
    {
    public:
        struct MemoryBreaks
        {
            size_t allocationIdToBreak = size_t(-1);
            size_t allocationSizeToBreak = size_t(-1);
        };

    private:
        struct MemoryInfo
        {
            size_t numAllocations = 0;
            size_t numBytesAllocated = 0;
        };

    public:

        static DebugAllocator& GetInstance()
        {
            static DebugAllocator s_DebugAllocator;
            return s_DebugAllocator;
        }

        bool Create(void* pHeap, size_t heapSize);
        void Destroy();

        void Allocate(BaseAllocator* pAllocator, void* pAllocatedMemory, size_t size, const char* pAllocationFilename, int allocationLineNumber);

        void Deallocate(void* pAllocatedMemory);

        const MemoryInfo& GetMemoryInfo() const { return m_MemoryInfo; }

    private:
        struct FreeChunkHeader
        {
            FreeChunkHeader* pNextFreeChunk = nullptr;
        };

        struct DebugAllocationInfo
        {
            size_t memoryAddress = 0;
            size_t allocationSizeInBytes = 0;
            size_t allocationId = 0;

            BaseAllocator* pAllocator = nullptr;

            DebugAllocationInfo* pNextDebugAllocationInfo = nullptr;
            DebugAllocationInfo* pPreviousDebugAllocationInfo = nullptr;

            const char* pAllocationFilename = nullptr;
            int allocationLineNumber = 0;
        };

    private:
        DebugAllocator();
        ~DebugAllocator();

        DebugAllocator(const DebugAllocator& src) = delete;
        DebugAllocator& operator= (const DebugAllocator& rhs) = delete;
        DebugAllocator& operator= (const DebugAllocator&& rhs) = delete;

        FreeChunkHeader* m_pFirstFreeChunk;
        DebugAllocationInfo* m_pFirstDebugAllocationInfo;

        void* m_pHeap;
        size_t m_HeapSize;
        size_t m_NumAllocations;

        std::mutex m_Lock;

        MemoryInfo m_MemoryInfo;
    };
}

#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO