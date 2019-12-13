#include <system/systemprecomp.h>

#include <system/memory/debugallocator.h>

#ifdef SHIP_ALLOCATOR_DEBUG_INFO

#include <system/logger.h>
#include <system/systemdebug.h>

namespace Shipyard
{;

DebugAllocator::MemoryBreaks g_MemoryBreaks;

DebugAllocator::DebugAllocator()
    : m_pFirstFreeChunk(nullptr)
    , m_pFirstDebugAllocationInfo(nullptr)
    , m_pHeap(nullptr)
    , m_HeapSize(0)
    , m_NumAllocations(0)
{
}

DebugAllocator::~DebugAllocator()
{
    SHIP_ASSERT_MSG(m_pHeap == nullptr, "It is required to manually call Destroy on the DebugAllocator %p to control when to free the memory.", this);
}

shipBool DebugAllocator::Create(void* pHeap, size_t heapSize)
{
    if (pHeap == nullptr)
    {
        return false;
    }

    m_pHeap = pHeap;
    m_HeapSize = heapSize;

    m_NumAllocations = heapSize / sizeof(DebugAllocationInfo);

    m_pFirstFreeChunk = reinterpret_cast<FreeChunkHeader*>(m_pHeap);
    FreeChunkHeader* pCurrentFreeChunk = m_pFirstFreeChunk;
    FreeChunkHeader* pLastFreeChunk = nullptr;

    for (size_t i = 0; i < m_NumAllocations; i++)
    {
        pLastFreeChunk = pCurrentFreeChunk;

        size_t nextFreeChunkAddress = size_t(pCurrentFreeChunk) + sizeof(DebugAllocationInfo);

        FreeChunkHeader* pNextFreeChunk = reinterpret_cast<FreeChunkHeader*>(nextFreeChunkAddress);
        pCurrentFreeChunk->pNextFreeChunk = pNextFreeChunk;

        pCurrentFreeChunk = pNextFreeChunk;
    }

    pLastFreeChunk->pNextFreeChunk = nullptr;

    m_pFirstDebugAllocationInfo = nullptr;

    return true;
}

void DebugAllocator::Destroy()
{
    if (m_MemoryInfo.numAllocations != 0)
    {
        SHIP_ASSERT(m_pFirstDebugAllocationInfo != nullptr);

        SHIP_LOG_ERROR("***** Memory leaks detected! *****");

        // For convenience, let's pack the memory leaks by allocator. So we need to first gather the different allocators.
        BaseAllocator* pAllocators[32];
        shipUint32 numMemoryLeaksPerAllocator[32];
        size_t numAllocators = 0;

        for (DebugAllocationInfo* pCurrentMemoryLeak = m_pFirstDebugAllocationInfo; pCurrentMemoryLeak != nullptr; pCurrentMemoryLeak = pCurrentMemoryLeak->pNextDebugAllocationInfo)
        {
            size_t idx = 0;
            for (; idx < numAllocators; idx++)
            {
                if (pAllocators[idx] == pCurrentMemoryLeak->pAllocator)
                {
                    break;
                }
            }

            shipBool newAllocator = (idx == numAllocators);

            if (newAllocator)
            {
                pAllocators[idx] = pCurrentMemoryLeak->pAllocator;
                numMemoryLeaksPerAllocator[idx] = 1;

                numAllocators += 1;
            }
            else
            {
                numMemoryLeaksPerAllocator[idx] += 1;
            }
        }

        for (size_t i = 0; i < numAllocators; i++)
        {
            BaseAllocator* pAllocator = pAllocators[i];

            SHIP_LOG_ERROR("    %u Memory leaks for allocator %s at address 0x%p", numMemoryLeaksPerAllocator[i], pAllocator->GetAllocatorDebugName(), pAllocator);

            for (DebugAllocationInfo* pCurrentMemoryLeak = m_pFirstDebugAllocationInfo; pCurrentMemoryLeak != nullptr; pCurrentMemoryLeak = pCurrentMemoryLeak->pNextDebugAllocationInfo)
            {
                if (pCurrentMemoryLeak->pAllocator != pAllocator)
                {
                    continue;
                }

                SHIP_LOG_ERROR("        Memory leak of allocation #%llu of %llu bytes at address 0x%p --> \t%s (line %d)", pCurrentMemoryLeak->allocationId, pCurrentMemoryLeak->allocationSizeInBytes, pCurrentMemoryLeak->memoryAddress, pCurrentMemoryLeak->pAllocationFilename, pCurrentMemoryLeak->allocationLineNumber);
            }
        }
    }

    m_pHeap = nullptr;
    m_pFirstFreeChunk = nullptr;
    m_pFirstDebugAllocationInfo = nullptr;
}

void DebugAllocator::Allocate(BaseAllocator* pAllocator, void* pAllocatedMemory, size_t size, const shipChar* pAllocationFilename, int allocationLineNumber)
{
    if (m_pHeap == nullptr || pAllocatedMemory == nullptr)
    {
        return;
    }

    std::lock_guard<std::mutex> lock(m_Lock);

    if (m_pFirstFreeChunk == nullptr)
    {
        return;
    }

    DebugAllocationInfo* pDebugAllocationInfo = reinterpret_cast<DebugAllocationInfo*>(m_pFirstFreeChunk);

    m_pFirstFreeChunk = m_pFirstFreeChunk->pNextFreeChunk;

    pDebugAllocationInfo->memoryAddress = size_t(pAllocatedMemory);
    pDebugAllocationInfo->allocationSizeInBytes = size;
    pDebugAllocationInfo->pAllocator = pAllocator;
    pDebugAllocationInfo->pAllocationFilename = pAllocationFilename;
    pDebugAllocationInfo->allocationLineNumber = allocationLineNumber;

    static size_t s_AllocationId = 0;

    if (g_MemoryBreaks.allocationIdToBreak == s_AllocationId)
    {
        __debugbreak();
    }

    if (g_MemoryBreaks.allocationSizeToBreak == size)
    {
        __debugbreak();
    }

    pDebugAllocationInfo->allocationId = s_AllocationId;
    s_AllocationId += 1;

    pDebugAllocationInfo->pPreviousDebugAllocationInfo = nullptr;
    pDebugAllocationInfo->pNextDebugAllocationInfo = m_pFirstDebugAllocationInfo;

    if (m_pFirstDebugAllocationInfo != nullptr)
    {
        m_pFirstDebugAllocationInfo->pPreviousDebugAllocationInfo = pDebugAllocationInfo;
    }

    m_pFirstDebugAllocationInfo = pDebugAllocationInfo;

    m_MemoryInfo.numAllocations += 1;
}

void DebugAllocator::Deallocate(const void* memory)
{
    if (m_pHeap == nullptr)
    {
        return;
    }

    std::lock_guard<std::mutex> lock(m_Lock);

    SHIP_ASSERT(m_pFirstDebugAllocationInfo != nullptr);

    DebugAllocationInfo* pDebugAllocationInfo = nullptr;
    for (pDebugAllocationInfo = m_pFirstDebugAllocationInfo; pDebugAllocationInfo != nullptr; pDebugAllocationInfo = pDebugAllocationInfo->pNextDebugAllocationInfo)
    {
        if (pDebugAllocationInfo->memoryAddress == size_t(memory))
        {
            break;
        }
    }

    FreeChunkHeader* pNewFreeChunk = reinterpret_cast<FreeChunkHeader*>(pDebugAllocationInfo);
    pNewFreeChunk->pNextFreeChunk = m_pFirstFreeChunk;

    m_pFirstFreeChunk = pNewFreeChunk;

    if (m_pFirstDebugAllocationInfo == pDebugAllocationInfo)
    {
        m_pFirstDebugAllocationInfo = pDebugAllocationInfo->pNextDebugAllocationInfo;
    }

    if (pDebugAllocationInfo->pNextDebugAllocationInfo != nullptr)
    {
        pDebugAllocationInfo->pNextDebugAllocationInfo->pPreviousDebugAllocationInfo = pDebugAllocationInfo->pPreviousDebugAllocationInfo;
    }

    if (pDebugAllocationInfo->pPreviousDebugAllocationInfo != nullptr)
    {
        pDebugAllocationInfo->pPreviousDebugAllocationInfo->pNextDebugAllocationInfo = pDebugAllocationInfo->pNextDebugAllocationInfo;
    }

    SHIP_ASSERT(m_MemoryInfo.numAllocations > 0);

    m_MemoryInfo.numAllocations -= 1;
    m_MemoryInfo.numBytesAllocated -= pDebugAllocationInfo->allocationSizeInBytes;
}

}

#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO