#include <system/systemprecomp.h>

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

shipBool LinearAllocator::Create(void* pHeap, size_t heapSize)
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
            , const shipChar* pAllocationFilename
            , int allocationLineNumber
        #endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

        )
{
    size_t allocationOffset = 0;
    size_t newAllocationOffset = 0;
    size_t allocatedAddress = 0;

    do
    {
        allocationOffset = m_AllocationOffset;
        size_t allocationOffsetToUse = m_AllocationOffset;

        allocatedAddress = size_t(m_pHeap) + allocationOffsetToUse;

        if (!MemoryUtils::IsAddressAligned(allocatedAddress, alignment))
        {
            allocatedAddress = MemoryUtils::AlignAddress(allocatedAddress, alignment);

            allocationOffsetToUse = allocatedAddress - size_t(m_pHeap);
        }

        newAllocationOffset = allocationOffsetToUse + size;

        shipBool validAllocation = (newAllocationOffset <= m_HeapSize);
        if (!validAllocation)
        {
            return nullptr;
        }

    } while (AtomicOperations::CompareExchange(m_AllocationOffset, newAllocationOffset, allocationOffset) != allocationOffset);

    void* allocatedPtr = reinterpret_cast<void*>(allocatedAddress);

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