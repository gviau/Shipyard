#include <system/memory/fixedheapallocator.h>

#include <system/logger.h>
#include <system/systemdebug.h>

namespace Shipyard
{;

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
const size_t FixedHeapAllocator::FreeMemoryBlockSize = sizeof(FreeMemoryBlock);
const size_t FixedHeapAllocator::MemoryAllocationHeaderSize = sizeof(MemoryAllocationHeader);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

#define MAX(a, b) (((a) > (b)) ? (a) : (b))

FixedHeapAllocator::FixedHeapAllocator()
    : m_pFirstFreeMemoryBlock(nullptr)

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
    , m_pAllocatedBlockHead(nullptr)
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO
{
}

FixedHeapAllocator::~FixedHeapAllocator()
{
    SHIP_ASSERT_MSG(m_pHeap == nullptr, "It is required to manually call Destroy on the FixedHeapAllocator %p to control when to free the memory.", this);
}

#ifdef SHIP_ALLOCATOR_DEBUG_MEMORY_FILL
namespace
{
    enum FixedHeapAllocatorDebugConstants
    {
        FixedHeapAllocatorDebugConstants_NeverAllocatedMemory = 0xff,
        FixedHeapAllocatorDebugConstants_FreedMemory = 0xfe,
        FixedHeapAllocatorDebugConstants_AlignmentPaddingMemory = 0xfd
    };
}
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_MEMORY_FILL

bool FixedHeapAllocator::Create(void* pHeap, size_t heapSize)
{
    SHIP_ASSERT_MSG(pHeap != nullptr && heapSize > 0, "FixedHeapAllocator::Create --> Trying to initialize a fixed heap allocator with an invalid heap");

    m_pHeap = pHeap;
    m_HeapSize = heapSize;

#ifdef SHIP_ALLOCATOR_DEBUG_MEMORY_FILL
    memset(pHeap, FixedHeapAllocatorDebugConstants_NeverAllocatedMemory, heapSize);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_MEMORY_FILL

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
    m_MemoryInfo.heapSize = heapSize;
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

    FreeMemoryBlock* pFreeMemoryBlock = reinterpret_cast<FreeMemoryBlock*>(m_pHeap);
    pFreeMemoryBlock->sizeOfBlockInBytesIncludingThisHeader = heapSize;
    pFreeMemoryBlock->pNextFreeBlock = nullptr;
    pFreeMemoryBlock->pPreviousFreeBlock = nullptr;

    m_pFirstFreeMemoryBlock = pFreeMemoryBlock;

    return true;
}

void FixedHeapAllocator::Destroy()
{
#ifdef SHIP_ALLOCATOR_DEBUG_INFO
    if (m_MemoryInfo.numBlocksAllocated != 0)
    {
        SHIP_ASSERT(m_pAllocatedBlockHead != nullptr);

        SHIP_LOG_ERROR("***** Memory leaks detected! *****");

        for (MemoryAllocationHeader* pCurrentMemoryLeak = m_pAllocatedBlockHead; pCurrentMemoryLeak != nullptr; pCurrentMemoryLeak = pCurrentMemoryLeak->pNextHeader)
        {
            SHIP_LOG_ERROR("    Memory leak of %llu bytes --> \t%s (line %d)", pCurrentMemoryLeak->userAllocationRegionSizeInBytes, pCurrentMemoryLeak->pAllocationFilename, pCurrentMemoryLeak->allocationLineNumber);
        }
    }
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

    m_pHeap = nullptr;
    m_HeapSize = 0;
}

void* FixedHeapAllocator::Allocate(size_t size, size_t alignment

        #ifdef SHIP_ALLOCATOR_DEBUG_INFO
            , const char* pAllocationFilename
            , int allocationLineNumber
        #endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

        )
{
    SHIP_ASSERT_MSG(alignment > 0, "FixedHeapAllocator::Allocate --> alignment cannot be 0");
    SHIP_ASSERT_MSG( ( ((alignment - 1) & alignment) == 0 ), "FixedHeapAllocator::Allocate --> alignment %zu is not a power-of-2", alignment);

    if (m_pFirstFreeMemoryBlock == nullptr)
    {
        return nullptr;
    }

    // We need enough space for the user buffer + the required alignment + the MemoryAllocationHeader struct that comes before the allocation.
    //
    // Because the MemoryAllocationHeader comes exactly sizeof(MemoryAllocationHeader) bytes before the user buffer, we might need to pad some more
    // bytes to respect the alignment constraints.
    //
    // In practice, we need enough space for either one of the FreeMemoryBlock or MemoryAllocationHeader struct: since memory get aliased to one or the
    // other when allocating and deallocating, we need to plan for the maximum amount of memory to not otherwise overwrite values past the struct.
    const size_t minimalSpaceRequiredForHeader = MAX(sizeof(FreeMemoryBlock), sizeof(MemoryAllocationHeader));
    size_t requiredSize = size + minimalSpaceRequiredForHeader;
    
    for (FreeMemoryBlock* pCurrentFreeMemoryBlock = m_pFirstFreeMemoryBlock; pCurrentFreeMemoryBlock != nullptr; pCurrentFreeMemoryBlock = pCurrentFreeMemoryBlock->pNextFreeBlock)
    {
        bool isMemoryBlockCandidate = (pCurrentFreeMemoryBlock->sizeOfBlockInBytesIncludingThisHeader >= requiredSize);
        if (!isMemoryBlockCandidate)
        {
            continue;
        }

        size_t startingAddressOfBlock = size_t(pCurrentFreeMemoryBlock);

        size_t startingAddressOfUserBuffer = startingAddressOfBlock + minimalSpaceRequiredForHeader;

        bool isBlockBigEnough = IsAddressAligned(startingAddressOfUserBuffer, alignment);
        bool requiresAlignmentPadding = !isBlockBigEnough;

        if (requiresAlignmentPadding)
        {
            size_t alignedStartingAddressOfUserBuffer = AlignAddress(startingAddressOfUserBuffer, alignment);
            requiredSize = size + (alignedStartingAddressOfUserBuffer - startingAddressOfBlock);

            startingAddressOfUserBuffer = alignedStartingAddressOfUserBuffer;

            isBlockBigEnough = (pCurrentFreeMemoryBlock->sizeOfBlockInBytesIncludingThisHeader >= requiredSize);
        }

        if (!isBlockBigEnough)
        {
            continue;
        }

        // First, we want to create a new free memory block if possible and insert it in the linked list of free memory blocks.
        size_t remainingSizeForFreeBlock = pCurrentFreeMemoryBlock->sizeOfBlockInBytesIncludingThisHeader - requiredSize;

        constexpr size_t minimalAllocationSizeInBytes = 1;
        size_t minimalSizeForNewFreeBlock = MAX(sizeof(FreeMemoryBlock), sizeof(MemoryAllocationHeader) + minimalAllocationSizeInBytes);

        FreeMemoryBlock* pNewFreeMemoryBlock = nullptr;

        bool canCreateNewFreeBlock = (remainingSizeForFreeBlock >= minimalSizeForNewFreeBlock);
        if (canCreateNewFreeBlock)
        {
            pNewFreeMemoryBlock = reinterpret_cast<FreeMemoryBlock*>(startingAddressOfUserBuffer + size);

            pNewFreeMemoryBlock->sizeOfBlockInBytesIncludingThisHeader = remainingSizeForFreeBlock;
            pNewFreeMemoryBlock->pNextFreeBlock = nullptr;
            pNewFreeMemoryBlock->pPreviousFreeBlock = nullptr;
        }
        else
        {
            // If we're here, this means we have a small memory bubble of unallocatable memory, since we don't have enough space to have
            // both an allocation header and some space for the user buffer, or enough space for a free memory block header.
            //
            // In such a case, we'll instead claim the memory as part of the current allocation, so that it can be properly reclaimed
            // when freeing the memory.
            size += remainingSizeForFreeBlock;
        }

        // Insert in the list.
        if (pCurrentFreeMemoryBlock->pNextFreeBlock != nullptr)
        {
            if (canCreateNewFreeBlock)
            {
                pCurrentFreeMemoryBlock->pNextFreeBlock->pPreviousFreeBlock = pNewFreeMemoryBlock;
                pNewFreeMemoryBlock->pNextFreeBlock = pCurrentFreeMemoryBlock->pNextFreeBlock;
            }
            else
            {
                pCurrentFreeMemoryBlock->pNextFreeBlock->pPreviousFreeBlock = pCurrentFreeMemoryBlock->pPreviousFreeBlock;
            }
        }

        if (pCurrentFreeMemoryBlock->pPreviousFreeBlock != nullptr)
        {
            if (canCreateNewFreeBlock)
            {
                pCurrentFreeMemoryBlock->pPreviousFreeBlock->pNextFreeBlock = pNewFreeMemoryBlock;
                pNewFreeMemoryBlock->pPreviousFreeBlock = pCurrentFreeMemoryBlock->pPreviousFreeBlock;
            }
            else
            {
                pCurrentFreeMemoryBlock->pPreviousFreeBlock->pNextFreeBlock = pCurrentFreeMemoryBlock->pNextFreeBlock;
            }
        }

        if (m_pFirstFreeMemoryBlock == pCurrentFreeMemoryBlock)
        {
            if (canCreateNewFreeBlock)
            {
                m_pFirstFreeMemoryBlock = pNewFreeMemoryBlock;
            }
            else
            {
                m_pFirstFreeMemoryBlock = pCurrentFreeMemoryBlock->pNextFreeBlock;
            }
        }

        // Then we can create the allocated block with the header.
        size_t startingAddressOfMemoryBlockHeader = startingAddressOfUserBuffer - sizeof(MemoryAllocationHeader);

        MemoryAllocationHeader* pNewMemoryAllocationHeader = reinterpret_cast<MemoryAllocationHeader*>(startingAddressOfMemoryBlockHeader);
        pNewMemoryAllocationHeader->pStartOfMemoryAllocationHeaderIncludingAlignmentPadding = reinterpret_cast<void*>(pCurrentFreeMemoryBlock);
        pNewMemoryAllocationHeader->userAllocationRegionSizeInBytes = size;

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        pNewMemoryAllocationHeader->pAllocationFilename = pAllocationFilename;
        pNewMemoryAllocationHeader->allocationLineNumber = allocationLineNumber;

        pNewMemoryAllocationHeader->pPreviousHeader = nullptr;
        pNewMemoryAllocationHeader->pNextHeader = m_pAllocatedBlockHead;

        if (m_pAllocatedBlockHead != nullptr)
        {
            m_pAllocatedBlockHead->pPreviousHeader = pNewMemoryAllocationHeader;
        }

        m_pAllocatedBlockHead = pNewMemoryAllocationHeader;

        m_MemoryInfo.numBlocksAllocated += 1;
        m_MemoryInfo.numBytesUsed += requiredSize + ((canCreateNewFreeBlock) ? 0 : remainingSizeForFreeBlock);
        m_MemoryInfo.numUserBytesAllocated += size;
        m_MemoryInfo.maxAllocatedUserSize = MAX(m_MemoryInfo.maxAllocatedUserSize, size);
        m_MemoryInfo.minAllocatedUserSize = MIN(m_MemoryInfo.minAllocatedUserSize, size);
        m_MemoryInfo.peakUserBytesAllocated = MAX(m_MemoryInfo.maxAllocatedUserSize, m_MemoryInfo.numUserBytesAllocated);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

#ifdef SHIP_ALLOCATOR_DEBUG_MEMORY_FILL
        size_t alignmentPaddingSize = (startingAddressOfMemoryBlockHeader - startingAddressOfBlock);
        if (alignmentPaddingSize > 0)
        {
            void* pStartOfAlignmentPadding = reinterpret_cast<void*>(startingAddressOfBlock);
            memset(pStartOfAlignmentPadding, FixedHeapAllocatorDebugConstants_AlignmentPaddingMemory, alignmentPaddingSize);
        }
#endif // SHIP_ALLOCATOR_DEBUG_MEMORY_FILL

        return reinterpret_cast<void*>(startingAddressOfUserBuffer);
    }

    return nullptr;
}

void FixedHeapAllocator::Deallocate(void* memory)
{
    if (memory == nullptr)
    {
        return;
    }

    SHIP_ASSERT_MSG(size_t(m_pHeap) <= size_t(memory) && (size_t(m_pHeap) + size_t(m_HeapSize)) >= size_t(memory), "FixedHeapAllocator::Deallocate --> Memory address %p was not allocated from allocator %p", memory, this);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
    SHIP_ASSERT_MSG(m_pAllocatedBlockHead != nullptr, "FixedHeapAllocator::Deallocator --> Absolutely no blocks are allocated in allocator %p", this);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

    size_t addressOfMemoryAllocationHeader = size_t(memory) - sizeof(MemoryAllocationHeader);
    MemoryAllocationHeader* pMemoryAllocationHeader = reinterpret_cast<MemoryAllocationHeader*>(addressOfMemoryAllocationHeader);

    size_t sizeOfHeaderAndAlignmentPadding = (size_t(memory) - size_t(pMemoryAllocationHeader->pStartOfMemoryAllocationHeaderIncludingAlignmentPadding));
    
    FreeMemoryBlock* pNewFreeMemoryBlock = reinterpret_cast<FreeMemoryBlock*>(pMemoryAllocationHeader->pStartOfMemoryAllocationHeaderIncludingAlignmentPadding);
    size_t sizeOfNewFreeBlockInBytesIncludingThisHeader = sizeOfHeaderAndAlignmentPadding + pMemoryAllocationHeader->userAllocationRegionSizeInBytes;

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
    m_MemoryInfo.numBlocksAllocated -= 1;
    m_MemoryInfo.numBytesUsed -= sizeOfNewFreeBlockInBytesIncludingThisHeader;
    m_MemoryInfo.numUserBytesAllocated -= pMemoryAllocationHeader->userAllocationRegionSizeInBytes;

    if (m_pAllocatedBlockHead == pMemoryAllocationHeader)
    {
        m_pAllocatedBlockHead = pMemoryAllocationHeader->pNextHeader;
    }

    if (pMemoryAllocationHeader->pNextHeader != nullptr)
    {
        pMemoryAllocationHeader->pNextHeader->pPreviousHeader = pMemoryAllocationHeader->pPreviousHeader;
    }

    if (pMemoryAllocationHeader->pPreviousHeader != nullptr)
    {
        pMemoryAllocationHeader->pPreviousHeader->pNextHeader = pMemoryAllocationHeader->pNextHeader;
    }
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

    pNewFreeMemoryBlock->sizeOfBlockInBytesIncludingThisHeader = sizeOfNewFreeBlockInBytesIncludingThisHeader;
    pNewFreeMemoryBlock->pNextFreeBlock = nullptr;
    pNewFreeMemoryBlock->pPreviousFreeBlock = nullptr;

    size_t startAddressOfNewFreeBlock = size_t(pNewFreeMemoryBlock);
    size_t endAddressOfNewFreeBlock = startAddressOfNewFreeBlock + pNewFreeMemoryBlock->sizeOfBlockInBytesIncludingThisHeader;

    // We want to collapse free blocks into a single continuous block if they are right next to the free block we're about to create.
    // Otherwise, we want to insert it in the list of free blocks.
    bool isFirstFreeMemoryBlock = (m_pFirstFreeMemoryBlock == nullptr);
    if (isFirstFreeMemoryBlock)
    {
        m_pFirstFreeMemoryBlock = pNewFreeMemoryBlock;
    }
    else
    {
        FreeMemoryBlock* pCurrentFreeMemoryBlock = m_pFirstFreeMemoryBlock;

        size_t startAddressOfFirstFreeBlock = size_t(pCurrentFreeMemoryBlock);

        bool isNewFreeMemoryBlockBehindFirstFreeMemoryBlock = (startAddressOfFirstFreeBlock >= endAddressOfNewFreeBlock);

        if (isNewFreeMemoryBlockBehindFirstFreeMemoryBlock)
        {
            bool canCollapseFreeMemoryBlock = (startAddressOfFirstFreeBlock == endAddressOfNewFreeBlock);
            if (canCollapseFreeMemoryBlock)
            {
                pNewFreeMemoryBlock->sizeOfBlockInBytesIncludingThisHeader += pCurrentFreeMemoryBlock->sizeOfBlockInBytesIncludingThisHeader;

                pNewFreeMemoryBlock->pNextFreeBlock = pCurrentFreeMemoryBlock->pNextFreeBlock;
                pNewFreeMemoryBlock->pPreviousFreeBlock = nullptr;

                if (pCurrentFreeMemoryBlock->pNextFreeBlock != nullptr)
                {
                    pCurrentFreeMemoryBlock->pNextFreeBlock->pPreviousFreeBlock = pNewFreeMemoryBlock;
                }
            }
            else
            {
                pNewFreeMemoryBlock->pNextFreeBlock = pCurrentFreeMemoryBlock;
                pCurrentFreeMemoryBlock->pPreviousFreeBlock = pNewFreeMemoryBlock;
            }

            m_pFirstFreeMemoryBlock = pNewFreeMemoryBlock;
        }
        else
        {
            FreeMemoryBlock* pNextFreeMemoryBlock = pCurrentFreeMemoryBlock->pNextFreeBlock;

            while (pCurrentFreeMemoryBlock != nullptr)
            {
                size_t endAddressOfCurrentFreeBlock = size_t(pCurrentFreeMemoryBlock) + pCurrentFreeMemoryBlock->sizeOfBlockInBytesIncludingThisHeader;

                bool isNewFreeMemoryBlockAfterLastFreeMemoryBlock = (pNextFreeMemoryBlock == nullptr);

                if (isNewFreeMemoryBlockAfterLastFreeMemoryBlock)
                {
                    bool canCollapsePreviousMemoryBlock = (endAddressOfCurrentFreeBlock == startAddressOfNewFreeBlock);
                    if (canCollapsePreviousMemoryBlock)
                    {
                        pCurrentFreeMemoryBlock->sizeOfBlockInBytesIncludingThisHeader += pNewFreeMemoryBlock->sizeOfBlockInBytesIncludingThisHeader;
                    }
                    else
                    {
                        if (pCurrentFreeMemoryBlock->pNextFreeBlock != nullptr)
                        {
                            pCurrentFreeMemoryBlock->pNextFreeBlock->pPreviousFreeBlock = pNewFreeMemoryBlock;
                        }

                        pNewFreeMemoryBlock->pNextFreeBlock = pCurrentFreeMemoryBlock->pNextFreeBlock;

                        pCurrentFreeMemoryBlock->pNextFreeBlock = pNewFreeMemoryBlock;

                        pNewFreeMemoryBlock->pPreviousFreeBlock = pCurrentFreeMemoryBlock;
                    }

                    break;
                }
                else
                {
                    size_t startAddressOfNextFreeBlock = size_t(pNextFreeMemoryBlock);
                 
                    bool isBetweenTwoFreeMemoryBlocks = (startAddressOfNextFreeBlock >= endAddressOfNewFreeBlock);
                    if (isBetweenTwoFreeMemoryBlocks)
                    {
                        bool canCollapsePreviousFreeMemoryBlock = (endAddressOfCurrentFreeBlock == startAddressOfNewFreeBlock);
                        bool canCollapseNextFreeMemoryBlock = (startAddressOfNextFreeBlock == endAddressOfNewFreeBlock);

                        if (canCollapsePreviousFreeMemoryBlock && canCollapseNextFreeMemoryBlock)
                        {
                            pCurrentFreeMemoryBlock->sizeOfBlockInBytesIncludingThisHeader += pNewFreeMemoryBlock->sizeOfBlockInBytesIncludingThisHeader;
                            pCurrentFreeMemoryBlock->sizeOfBlockInBytesIncludingThisHeader += pNextFreeMemoryBlock->sizeOfBlockInBytesIncludingThisHeader;

                            pNewFreeMemoryBlock = pCurrentFreeMemoryBlock;

                            pNewFreeMemoryBlock->pNextFreeBlock = pNextFreeMemoryBlock->pNextFreeBlock;

                            if (pNextFreeMemoryBlock->pNextFreeBlock != nullptr)
                            {
                                pNextFreeMemoryBlock->pNextFreeBlock->pPreviousFreeBlock = pNewFreeMemoryBlock;
                            }
                        }
                        else if (canCollapsePreviousFreeMemoryBlock)
                        {
                            pCurrentFreeMemoryBlock->sizeOfBlockInBytesIncludingThisHeader += pNewFreeMemoryBlock->sizeOfBlockInBytesIncludingThisHeader;

                            pNewFreeMemoryBlock = pCurrentFreeMemoryBlock;
                        }
                        else if (canCollapseNextFreeMemoryBlock)
                        {
                            pNewFreeMemoryBlock->sizeOfBlockInBytesIncludingThisHeader += pNextFreeMemoryBlock->sizeOfBlockInBytesIncludingThisHeader;

                            pNewFreeMemoryBlock->pNextFreeBlock = pNextFreeMemoryBlock->pNextFreeBlock;
                            pNewFreeMemoryBlock->pPreviousFreeBlock = pNextFreeMemoryBlock->pPreviousFreeBlock;

                            if (pNewFreeMemoryBlock->pNextFreeBlock != nullptr)
                            {
                                pNewFreeMemoryBlock->pNextFreeBlock->pPreviousFreeBlock = pNewFreeMemoryBlock;
                            }

                            if (pNextFreeMemoryBlock->pPreviousFreeBlock != nullptr)
                            {
                                pNextFreeMemoryBlock->pPreviousFreeBlock->pNextFreeBlock = pNewFreeMemoryBlock;
                            }
                        }
                        else
                        {
                            pNewFreeMemoryBlock->pNextFreeBlock = pNextFreeMemoryBlock;
                            pNewFreeMemoryBlock->pPreviousFreeBlock = pNextFreeMemoryBlock->pPreviousFreeBlock;

                            if (pNextFreeMemoryBlock->pPreviousFreeBlock != nullptr)
                            {
                                pNextFreeMemoryBlock->pPreviousFreeBlock->pNextFreeBlock = pNewFreeMemoryBlock;
                            }
                        }

                        break;
                    }
                }

                pCurrentFreeMemoryBlock = pNextFreeMemoryBlock;

                if (pNextFreeMemoryBlock != nullptr)
                {
                    pNextFreeMemoryBlock = pNextFreeMemoryBlock->pNextFreeBlock;
                }
            }
        }
    }

#ifdef SHIP_ALLOCATOR_DEBUG_MEMORY_FILL
    void* pStartOfFreedMemory = reinterpret_cast<void*>(size_t(pNewFreeMemoryBlock) + sizeof(FreeMemoryBlock));
    size_t sizeOfFreedMemory = pNewFreeMemoryBlock->sizeOfBlockInBytesIncludingThisHeader - sizeof(FreeMemoryBlock);

    memset(pStartOfFreedMemory, FixedHeapAllocatorDebugConstants_FreedMemory, sizeOfFreedMemory);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_MEMORY_FILL
}

}