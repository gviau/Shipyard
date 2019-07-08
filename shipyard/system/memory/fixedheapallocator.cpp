#include <system/memory/fixedheapallocator.h>

#include <system/logger.h>
#include <system/systemdebug.h>

#include <system/memory/memoryutils.h>

namespace Shipyard
{;

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
const size_t FixedHeapAllocator::FreeMemoryBlockSize = sizeof(FreeMemoryBlock);
const size_t FixedHeapAllocator::MemoryAllocationHeaderSize = sizeof(MemoryAllocationHeader);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

#define MAX(a, b) (((a) > (b)) ? (a) : (b))

FixedHeapAllocator::FixedHeapAllocator()
    : m_pFirstFreeMemoryBlock(nullptr)
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

shipBool FixedHeapAllocator::Create(void* pHeap, size_t heapSize)
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
    m_pHeap = nullptr;
    m_HeapSize = 0;
}

void* FixedHeapAllocator::Allocate(size_t size, size_t alignment

        #ifdef SHIP_ALLOCATOR_DEBUG_INFO
            , const shipChar* pAllocationFilename
            , int allocationLineNumber
        #endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

        )
{
    SHIP_ASSERT_MSG(alignment > 0, "FixedHeapAllocator::Allocate --> alignment cannot be 0");
    SHIP_ASSERT_MSG( ( ((alignment - 1) & alignment) == 0 ), "FixedHeapAllocator::Allocate --> alignment %zu is not a power-of-2", alignment);

    std::lock_guard<std::mutex> lock(m_Lock);

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
        shipBool isMemoryBlockCandidate = (pCurrentFreeMemoryBlock->sizeOfBlockInBytesIncludingThisHeader >= requiredSize);
        if (!isMemoryBlockCandidate)
        {
            continue;
        }

        size_t startingAddressOfBlock = size_t(pCurrentFreeMemoryBlock);

        size_t startingAddressOfUserBuffer = startingAddressOfBlock + minimalSpaceRequiredForHeader;

        shipBool isBlockBigEnough = MemoryUtils::IsAddressAligned(startingAddressOfUserBuffer, alignment);
        shipBool requiresAlignmentPadding = !isBlockBigEnough;

        if (requiresAlignmentPadding)
        {
            size_t alignedStartingAddressOfUserBuffer = MemoryUtils::AlignAddress(startingAddressOfUserBuffer, alignment);
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

        shipBool canCreateNewFreeBlock = (remainingSizeForFreeBlock >= minimalSizeForNewFreeBlock);
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

void FixedHeapAllocator::Deallocate(const void* memory)
{
    if (memory == nullptr)
    {
        return;
    }

    std::lock_guard<std::mutex> lock(m_Lock);

    SHIP_ASSERT_MSG(size_t(m_pHeap) <= size_t(memory) && (size_t(m_pHeap) + size_t(m_HeapSize)) >= size_t(memory), "FixedHeapAllocator::Deallocate --> Memory address %p was not allocated from allocator %p", memory, this);

    size_t addressOfMemoryAllocationHeader = size_t(memory) - sizeof(MemoryAllocationHeader);
    MemoryAllocationHeader* pMemoryAllocationHeader = reinterpret_cast<MemoryAllocationHeader*>(addressOfMemoryAllocationHeader);

    size_t sizeOfHeaderAndAlignmentPadding = (size_t(memory) - size_t(pMemoryAllocationHeader->pStartOfMemoryAllocationHeaderIncludingAlignmentPadding));
    
    FreeMemoryBlock* pNewFreeMemoryBlock = reinterpret_cast<FreeMemoryBlock*>(pMemoryAllocationHeader->pStartOfMemoryAllocationHeaderIncludingAlignmentPadding);
    size_t sizeOfNewFreeBlockInBytesIncludingThisHeader = sizeOfHeaderAndAlignmentPadding + pMemoryAllocationHeader->userAllocationRegionSizeInBytes;

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
    m_MemoryInfo.numBlocksAllocated -= 1;
    m_MemoryInfo.numBytesUsed -= sizeOfNewFreeBlockInBytesIncludingThisHeader;
    m_MemoryInfo.numUserBytesAllocated -= pMemoryAllocationHeader->userAllocationRegionSizeInBytes;
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

    pNewFreeMemoryBlock->sizeOfBlockInBytesIncludingThisHeader = sizeOfNewFreeBlockInBytesIncludingThisHeader;
    pNewFreeMemoryBlock->pNextFreeBlock = nullptr;
    pNewFreeMemoryBlock->pPreviousFreeBlock = nullptr;

    size_t startAddressOfNewFreeBlock = size_t(pNewFreeMemoryBlock);
    size_t endAddressOfNewFreeBlock = startAddressOfNewFreeBlock + pNewFreeMemoryBlock->sizeOfBlockInBytesIncludingThisHeader;

    // We want to collapse free blocks into a single continuous block if they are right next to the free block we're about to create.
    // Otherwise, we want to insert it in the list of free blocks.
    shipBool isFirstFreeMemoryBlock = (m_pFirstFreeMemoryBlock == nullptr);
    if (isFirstFreeMemoryBlock)
    {
        m_pFirstFreeMemoryBlock = pNewFreeMemoryBlock;
    }
    else
    {
        FreeMemoryBlock* pCurrentFreeMemoryBlock = m_pFirstFreeMemoryBlock;

        size_t startAddressOfFirstFreeBlock = size_t(pCurrentFreeMemoryBlock);

        shipBool isNewFreeMemoryBlockBehindFirstFreeMemoryBlock = (startAddressOfFirstFreeBlock >= endAddressOfNewFreeBlock);

        if (isNewFreeMemoryBlockBehindFirstFreeMemoryBlock)
        {
            shipBool canCollapseFreeMemoryBlock = (startAddressOfFirstFreeBlock == endAddressOfNewFreeBlock);
            if (canCollapseFreeMemoryBlock)
            {
                pNewFreeMemoryBlock->sizeOfBlockInBytesIncludingThisHeader += pCurrentFreeMemoryBlock->sizeOfBlockInBytesIncludingThisHeader;

                pNewFreeMemoryBlock->pNextFreeBlock = pCurrentFreeMemoryBlock->pNextFreeBlock;
                
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
            while (pCurrentFreeMemoryBlock != nullptr)
            {
                FreeMemoryBlock* pNextFreeMemoryBlock = pCurrentFreeMemoryBlock->pNextFreeBlock;

                size_t endAddressOfCurrentFreeBlock = size_t(pCurrentFreeMemoryBlock) + pCurrentFreeMemoryBlock->sizeOfBlockInBytesIncludingThisHeader;

                SHIP_ASSERT(startAddressOfNewFreeBlock >= endAddressOfCurrentFreeBlock);

                // The rationale is that if it isn't behind the current block (above branch), then if there are no more block after the current
                // then the new free block must be after the current one.
                shipBool isNewFreeMemoryBlockAfterLastFreeMemoryBlock = (pNextFreeMemoryBlock == nullptr);

                if (isNewFreeMemoryBlockAfterLastFreeMemoryBlock)
                {
                    shipBool canCollapsePreviousMemoryBlock = (endAddressOfCurrentFreeBlock == startAddressOfNewFreeBlock);
                    if (canCollapsePreviousMemoryBlock)
                    {
                        pCurrentFreeMemoryBlock->sizeOfBlockInBytesIncludingThisHeader += pNewFreeMemoryBlock->sizeOfBlockInBytesIncludingThisHeader;
                    }
                    else
                    {
                        pCurrentFreeMemoryBlock->pNextFreeBlock = pNewFreeMemoryBlock;

                        pNewFreeMemoryBlock->pPreviousFreeBlock = pCurrentFreeMemoryBlock;
                    }

                    break;
                }
                else
                {
                    size_t startAddressOfNextFreeBlock = size_t(pNextFreeMemoryBlock);
                 
                    shipBool isBetweenTwoFreeMemoryBlocks = (startAddressOfNextFreeBlock >= endAddressOfNewFreeBlock);
                    if (isBetweenTwoFreeMemoryBlocks)
                    {
                        shipBool canCollapsePreviousFreeMemoryBlock = (endAddressOfCurrentFreeBlock == startAddressOfNewFreeBlock);
                        shipBool canCollapseNextFreeMemoryBlock = (startAddressOfNextFreeBlock == endAddressOfNewFreeBlock);

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
                            pNewFreeMemoryBlock->pPreviousFreeBlock = pCurrentFreeMemoryBlock;

                            pCurrentFreeMemoryBlock->pNextFreeBlock = pNewFreeMemoryBlock;

                            if (pNextFreeMemoryBlock->pNextFreeBlock != nullptr)
                            {
                                pNextFreeMemoryBlock->pNextFreeBlock->pPreviousFreeBlock = pNewFreeMemoryBlock;
                            }
                        }
                        else
                        {
                            pNewFreeMemoryBlock->pNextFreeBlock = pNextFreeMemoryBlock;
                            pNewFreeMemoryBlock->pPreviousFreeBlock = pCurrentFreeMemoryBlock;

                            pCurrentFreeMemoryBlock->pNextFreeBlock = pNewFreeMemoryBlock;

                            if (pNextFreeMemoryBlock != nullptr)
                            {
                                pNextFreeMemoryBlock->pPreviousFreeBlock = pNewFreeMemoryBlock;
                            }
                        }

                        break;
                    }
                }

                pCurrentFreeMemoryBlock = pNextFreeMemoryBlock;
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