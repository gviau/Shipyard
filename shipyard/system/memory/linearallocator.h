#pragma once

#include <system/memory/baseallocator.h>

namespace Shipyard
{
    // Allocator that allocates by always increasing linearly a pointer in its heap. Single deallocation is not possible,
    // only full allocator's deallocation is possible.
    class SHIPYARD_API LinearAllocator : public BaseAllocator
    {
    public:
        LinearAllocator();
        ~LinearAllocator();

        bool Create(void* pHeap, size_t heapSize);
        void Destroy();

        // Alignment must be a power of 2 and non-zero.
        virtual void* Allocate(size_t size, size_t alignment

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
                    , const char* pAllocationFilename
                    , int allocationLineNumber
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

                ) override;

        // Single deallocation is not permitted with the LinearAllocator.
        virtual void Deallocate(const void* memory) override;

        // Not thread-safe.
        void FreeEverything();

    private:
        SHIP_ALIGN(8) size_t m_AllocationOffset;
    };
}