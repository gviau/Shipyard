#pragma once

#include <system/memory/baseallocator.h>

namespace Shipyard
{
    // Use this class to allocate fixed sized chunk of memory.
    // The PoolAllocator does not take ownership of the memory. It is the responsability of the user to properly free it after
    // calling PoolAllocator::Destroy()
    class SHIPYARD_API PoolAllocator : public BaseAllocator
    {
#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        struct MemoryInfo;
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

    public:
        PoolAllocator();
        ~PoolAllocator();

        // pHeap is assumed to be aligned to chunkSize bytes.
        // chunkSize is assumed to be at least sizeof(void*) bytes, because free chunks are used
        // as pointers to next free chunks.
        bool Create(void* pHeap, size_t numChunks, size_t chunkSize);
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
        struct FreeChunkHeader
        {
            FreeChunkHeader* pNextFreeChunk;
        };

    private:
        FreeChunkHeader* m_pFirstFreeChunk;
        size_t m_ChunkSize;
        size_t m_NumChunks;

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        struct MemoryInfo
        {
            uint64_t numBlocksAllocated = 0;
            size_t heapSize = 0;
            size_t numBytesUsed = 0;
            size_t numUserBytesAllocated = 0;
            size_t peakUserBytesAllocated = 0;
        };

        MemoryInfo m_MemoryInfo;
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO
    };
}