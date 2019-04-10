#include <system/memory/poolallocator.h>

#include <system/logger.h>
#include <system/systemdebug.h>

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
#include <system/memory/debugallocator.h>
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

namespace Shipyard
{;

PoolAllocator::PoolAllocator()
    : m_pFirstFreeChunk(nullptr)
    , m_ChunkSize(0)
    , m_NumChunks(0)
{
}

PoolAllocator::~PoolAllocator()
{
    SHIP_ASSERT_MSG(m_pHeap == nullptr, "It is required to manually call Destroy on the PoolAllocator %p to control when to free the memory.", this);
}

#ifdef SHIP_ALLOCATOR_DEBUG_MEMORY_FILL
namespace
{
    enum FixedHeapAllocatorDebugConstants
    {
        FixedHeapAllocatorDebugConstants_NeverAllocatedMemory = 0xff,
        FixedHeapAllocatorDebugConstants_FreedMemory = 0xfe,
    };
}
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_MEMORY_FILL

bool PoolAllocator::Create(void* pHeap, size_t numChunks, size_t chunkSize)
{
    if (pHeap == nullptr)
    {
        return false;
    }

    SHIP_ASSERT_MSG(
            chunkSize >= sizeof(FreeChunkHeader),
            "PoolAllocator::Create --> Chunk size for pool allocator %p are set to be %zu bytes, but they need to be at least %zu bytes",
            this, chunkSize, sizeof(FreeChunkHeader));

    SHIP_ASSERT_MSG(
            IsAddressAligned(size_t(pHeap), chunkSize),
            "PoolAllocator::Create --> pHeap %p for pool allocator %p is assumed to be aligned to %zu bytes",
            pHeap, this, chunkSize);

#ifdef SHIP_ALLOCATOR_DEBUG_MEMORY_FILL
    memset(pHeap, FixedHeapAllocatorDebugConstants_NeverAllocatedMemory, heapSize);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_MEMORY_FILL

    m_pHeap = pHeap;

    m_ChunkSize = chunkSize;
    m_NumChunks = numChunks;

    m_HeapSize = chunkSize * numChunks;

    m_pFirstFreeChunk = reinterpret_cast<FreeChunkHeader*>(m_pHeap);
    FreeChunkHeader* pCurrentFreeChunk = m_pFirstFreeChunk;
    FreeChunkHeader* pLastFreeChunk = nullptr;

    for (size_t i = 0; i < m_NumChunks; i++)
    {
        pLastFreeChunk = pCurrentFreeChunk;

        size_t nextFreeChunkAddress = size_t(pCurrentFreeChunk) + chunkSize;

        FreeChunkHeader* pNextFreeChunk = reinterpret_cast<FreeChunkHeader*>(nextFreeChunkAddress);
        pCurrentFreeChunk->pNextFreeChunk = pNextFreeChunk;

        pCurrentFreeChunk = pNextFreeChunk;
    }

    pLastFreeChunk->pNextFreeChunk = nullptr;

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
    m_MemoryInfo.heapSize = chunkSize * numChunks;
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

    return true;
}

void PoolAllocator::Destroy()
{
    m_pHeap = nullptr;
    m_pFirstFreeChunk = nullptr;
    m_NumChunks = 0;
    m_ChunkSize = 0;
}

void* PoolAllocator::Allocate(size_t size, size_t alignment

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            , const char* pAllocationFilename
            , int allocationLineNumber
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

        )
{
    SHIP_ASSERT(size <= m_ChunkSize);
    SHIP_ASSERT_MSG(alignment > 0, "PoolAllocator::Allocate --> alignment cannot be 0");
    SHIP_ASSERT_MSG((((alignment - 1) & alignment) == 0), "PoolAllocator::Allocate --> alignment %zu is not a power-of-2", alignment);

    std::lock_guard<std::mutex> lock(m_Lock);

    if (m_pFirstFreeChunk == nullptr)
    {
        return nullptr;
    }

    FreeChunkHeader* pChunkCandidate = m_pFirstFreeChunk;

    if (IsAddressAligned(size_t(pChunkCandidate), alignment))
    {
        m_pFirstFreeChunk = m_pFirstFreeChunk->pNextFreeChunk;
    }
    else
    {
        // Find next chunk that respect alignment requirements.
        FreeChunkHeader* pPreviousFreeChunk = pChunkCandidate;
        FreeChunkHeader* pCurrentFreeChunk = pChunkCandidate->pNextFreeChunk;

        pChunkCandidate = nullptr;

        while (pCurrentFreeChunk != nullptr)
        {
            if (IsAddressAligned(size_t(pCurrentFreeChunk), alignment))
            {
                pPreviousFreeChunk->pNextFreeChunk = pCurrentFreeChunk->pNextFreeChunk;

                pChunkCandidate = pCurrentFreeChunk;

                break;
            }

            pPreviousFreeChunk = pCurrentFreeChunk;
            pCurrentFreeChunk = pCurrentFreeChunk->pNextFreeChunk;
        }

        if (pChunkCandidate == nullptr)
        {
            return nullptr;
        }
    }

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
    m_MemoryInfo.numBlocksAllocated += 1;
    m_MemoryInfo.numBytesUsed += m_ChunkSize;
    m_MemoryInfo.numUserBytesAllocated += m_ChunkSize;
    m_MemoryInfo.peakUserBytesAllocated = MAX(m_MemoryInfo.peakUserBytesAllocated, m_MemoryInfo.numUserBytesAllocated);

    DebugAllocator::GetInstance().Allocate(this, pChunkCandidate, m_ChunkSize, pAllocationFilename, allocationLineNumber);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

    return pChunkCandidate;
}

void PoolAllocator::Deallocate(const void* memory)
{
    std::lock_guard<std::mutex> lock(m_Lock);

    FreeChunkHeader* pNewFreeChunk = reinterpret_cast<FreeChunkHeader*>(const_cast<void*>(memory));

#ifdef SHIP_ALLOCATOR_DEBUG_MEMORY_FILL
    memset(pNewFreeChunk, FixedHeapAllocatorDebugConstants_FreedMemory, m_ChunkSize);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_MEMORY_FILL

    pNewFreeChunk->pNextFreeChunk = m_pFirstFreeChunk;

    m_pFirstFreeChunk = pNewFreeChunk;

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
    SHIP_ASSERT(m_MemoryInfo.numBlocksAllocated > 0);

    m_MemoryInfo.numBlocksAllocated -= 1;
    m_MemoryInfo.numBytesUsed -= m_ChunkSize;
    m_MemoryInfo.numUserBytesAllocated -= m_ChunkSize;

    DebugAllocator::GetInstance().Deallocate(memory);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

}

}