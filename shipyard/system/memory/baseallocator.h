#pragma once

#include <system/platform.h>

#include <stdint.h>

#ifndef SHIP_OPTIMIZED

// This define allows loogging of some information about allocation and deallocation.
#define SHIP_ALLOCATOR_DEBUG_INFO

// This define enables memsetting regions of memory with pre-defined values for debugging.
// #define SHIP_ALLOCATOR_DEBUG_MEMORY_FILL

#endif // #ifndef SHIP_OPTIMIZED

namespace Shipyard
{
    class SHIPYARD_API BaseAllocator
    {
    public:
        // Alignment must be a power of 2 and non-zero.
        virtual void* Allocate(size_t size, size_t alignment
        
                #ifdef SHIP_ALLOCATOR_DEBUG_INFO
                    , const char* pAllocationFilename
                    , int allocationLineNumber
                #endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            ) = 0;

        // Memory must come from the allocator that allocated it.
        virtual void Deallocate(void* memory) = 0;
    };

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
#define SHIP_ALLOC(allocator, size, alignment) allocator.Allocate(size, alignment, __FILE__, __LINE__)
#else
#define SHIP_ALLOC(allocator, size, alignment) allocator.Allocate(size, alignment)
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

#define SHIP_FREE(allocator, memory) allocator.Deallocate(memory)

}