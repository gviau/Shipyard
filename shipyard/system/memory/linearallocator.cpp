#include <system/memory/linearallocator.h>

#include <system/systemdebug.h>

#include <system/atomicoperations.h>

namespace Shipyard
{;

LinearAllocator::LinearAllocator()
    : m_AllocationOffset(0)
{

}

LinearAllocator::~LinearAllocator()
{
    SHIP_ASSERT_MSG(m_pHeap == nullptr, "It is required to manually call Destroy on the LinearAllocator %p to control when to free the memory.", this);
}

bool LinearAllocator::Create(void* pHeap, size_t heapSize)
{
    m_pHeap = pHeap;
    m_HeapSize = heapSize;

    return true;
}

void LinearAllocator::Destroy()
{
    m_pHeap = nullptr;
    m_HeapSize = 0;
}

void* LinearAllocator::Allocate(size_t size, size_t alignment 

        #ifdef SHIP_ALLOCATOR_DEBUG_INFO
            , const char* pAllocationFilename
            , int allocationLineNumber
        #endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

        )
{
    size_t allocationOffsetToUse = 0;

    do
    {
        allocationOffsetToUse = m_AllocationOffset;

        if (!MemoryUtils::IsAddressAligned(allocationOffsetToUse, alignment))
        {
            allocationOffsetToUse = MemoryUtils::AlignAddress(allocationOffsetToUse, alignment);
        }

        bool validAllocation = (allocationOffsetToUse + size <= m_HeapSize);
        if (!validAllocation)
        {
            return nullptr;
        }

    } while (AtomicOperations<size_t>::CompareExchange(m_AllocationOffset, allocationOffsetToUse, m_AllocationOffset) != m_AllocationOffset);

    void* allocatedPtr = reinterpret_cast<void*>(reinterpret_cast<size_t>(m_pHeap) + allocationOffsetToUse);

    return allocatedPtr;
}

void LinearAllocator::Deallocate(const void* memory)
{
    SHIP_ASSERT_MSG(false, "LinearAllocator doesn't support individual deallocation, you must go through LinearAllocator::FreeEverything instead.");
}

void LinearAllocator::FreeEverything()
{
    m_AllocationOffset = 0;
}

}