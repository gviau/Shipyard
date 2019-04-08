#include <system/memory.h>

#include <system/systemdebug.h>

namespace Shipyard
{;

GlobalAllocator::GlobalAllocator()
    : m_NumAllocators(0)

#ifdef SHIP_DEBUG
    , m_Initialized(false)
#endif // #ifdef SHIP_DEBUG
{

}

GlobalAllocator::~GlobalAllocator()
{
    Destroy();
}

bool GlobalAllocator::Create(AllocatorInitEntry* pInitEntries, uint32_t numAllocators)
{
    SHIP_ASSERT(pInitEntries != nullptr);
    SHIP_ASSERT(numAllocators > 0);

    size_t lastSize = 0;

    for (uint32_t i = 0; i < numAllocators ; i++)
    {
        const AllocatorInitEntry& allocatorInitEntry = pInitEntries[i];

        SHIP_ASSERT(allocatorInitEntry.pAllocator != nullptr);

        bool lastAllocator = (i == (numAllocators - 1));

        if (lastAllocator)
        {
            m_MaxAllocationSizes[i] = size_t(-1);
        }
        else
        {
            SHIP_ASSERT_MSG(lastSize < allocatorInitEntry.maxAllocationSize, "GlobalAllocator::Create --> Allocators are assumed to be in increasing order");

            lastSize = allocatorInitEntry.maxAllocationSize;

            m_MaxAllocationSizes[i] = allocatorInitEntry.maxAllocationSize;
        }

        AllocatorAddressRange& allocatorAddressRange = m_pAllocators[i];
        allocatorAddressRange.pAllocator = allocatorInitEntry.pAllocator;
        allocatorAddressRange.startingAddressBytes = size_t(allocatorInitEntry.pAllocator->GetHeap());
        allocatorAddressRange.endingAddressBytes = allocatorAddressRange.startingAddressBytes + allocatorInitEntry.pAllocator->GetHeapSize();

        SHIP_ASSERT(allocatorAddressRange.startingAddressBytes < allocatorAddressRange.endingAddressBytes);
    }

    m_NumAllocators = numAllocators;

#ifdef SHIP_DEBUG
    m_Initialized = true;
#endif // #ifdef SHIP_DEBUG

    return true;
}

void GlobalAllocator::Destroy()
{
    m_NumAllocators = 0;

#ifdef SHIP_DEBUG
    m_Initialized = false;
#endif // #ifdef SHIP_DEBUG
}

void* GlobalAllocator::Allocate(size_t size, size_t alignment

        #ifdef SHIP_ALLOCATOR_DEBUG_INFO
            , const char* pAllocationFilename
            , int allocationLineNumber
        #endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

        )
{
#ifdef SHIP_DEBUG
    SHIP_ASSERT_MSG(m_Initialized, "The GlobalAllocator needs to be initialized before using it for allocations!");
#endif // #ifdef SHIP_DEBUG

    std::lock_guard<std::mutex> lock(m_Lock);

    uint32_t allocatorIndexToUse = 0;

    for (; allocatorIndexToUse < m_NumAllocators; allocatorIndexToUse++)
    {
        if (size > m_MaxAllocationSizes[allocatorIndexToUse])
        {
            continue;
        }

        void* pAllocatedPtr = m_pAllocators[allocatorIndexToUse].pAllocator->Allocate(size, alignment

                #ifdef SHIP_ALLOCATOR_DEBUG_INFO
                    , pAllocationFilename
                    , allocationLineNumber
                #endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

                );

        // If the allocator is out of memory then we need to continue trying with the next one.
        if (pAllocatedPtr != nullptr)
        {
            return pAllocatedPtr;
        }
    }

    // If we're here, then we we're out of memory in this allocator.
    return nullptr;
}

void GlobalAllocator::Deallocate(void* memory)
{
    if (memory == nullptr)
    {
        return;
    }

#ifdef SHIP_DEBUG
    SHIP_ASSERT_MSG(m_Initialized, "The GlobalAllocator needs to be initialized before using it for freeing memory!");
#endif // #ifdef SHIP_DEBUG

    std::lock_guard<std::mutex> lock(m_Lock);

    size_t memoryAddress = size_t(memory);

    for (uint32_t i = 0; i < m_NumAllocators; i++)
    {
        AllocatorAddressRange& allocatorAddressRange = m_pAllocators[i];

        if (memoryAddress >= allocatorAddressRange.startingAddressBytes && memoryAddress < allocatorAddressRange.endingAddressBytes)
        {
            allocatorAddressRange.pAllocator->Deallocate(memory);
            return;
        }
    }

    SHIP_ASSERT(false);
}

}