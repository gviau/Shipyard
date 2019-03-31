#pragma once

#include <system/platform.h>

#include <math/mathutilities.h>

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
}

namespace
{
    const size_t sizeForArraySizeHeader = sizeof(size_t) * 2;

    template <typename T>
    T* NewArray(Shipyard::BaseAllocator& allocator, size_t length, size_t alignment

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        , const char* pAllocationFilename
        , int allocationLineNumber
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

    )
    {
        // We need to allocate some extra space so that we can store how many objects we created.
        // Moreover, since we also need to respect alignment restrictions, we'll need to allocate a little
        // bit more space to be sure we can properly align the user pointer.
        const size_t sizeForArray = sizeof(T) * length;
        size_t requiredSize = sizeForArray + sizeForArraySizeHeader + alignment;

        // Since we're taking care of alignment manually here, no need to respect it at the allocation level.
        // Alignment is guaranteed to be respected since we allocated 'alignment' extra bytes, so we'll be able
        // to forward align the user pointer.
        constexpr size_t noAlignment = 1;
        void* pMem = allocator.Allocate(requiredSize, noAlignment
#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            , pAllocationFilename
            , allocationLineNumber
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO
        );

        if (pMem == nullptr)
        {
            return nullptr;
        }

        // Because some allocators might have a memory header before the returned user pointer, such as the FixedHeapAllocator for example,
        // we need to be careful about where to place the array size header. What we want to do to guarantee not to overwrite any information
        // directly before the returned user memory and to respect the alignment restriction is to align the user pointer to the next aligned
        // location that has an extra sizeof(size_t) bytes behind it so that we can place the header, even if the user pointer is already
        // aligned.
        size_t userPointerAddress = size_t(pMem);
        size_t forwardAlignedAddress = (userPointerAddress + (alignment - 1)) & (~(alignment - 1));

        size_t diffAddress = (forwardAlignedAddress - userPointerAddress);

        bool enoughSpaceForHeader = (diffAddress >= sizeForArraySizeHeader);

        bool needToAlignToNextBoundary = !enoughSpaceForHeader;
        if (needToAlignToNextBoundary)
        {
            forwardAlignedAddress += (sizeForArraySizeHeader - MAX(diffAddress, 1));
            forwardAlignedAddress = (forwardAlignedAddress + (alignment - 1)) & (~(alignment - 1));
        }

        // Required header so that we know how many objects to destroy when deleting the array.
        // Also need to know where is the real user pointer.
        size_t* pArraySizeHeader = reinterpret_cast<size_t*>(forwardAlignedAddress - sizeForArraySizeHeader);
        *pArraySizeHeader = length;
        *(pArraySizeHeader + 1) = userPointerAddress;

        T* pArray = reinterpret_cast<T*>(forwardAlignedAddress);

        for (size_t i = 0; i < length; i++)
        {
            new (&pArray)T;
        }

        return pArray;
    }

    template <typename T>
    void DestructorCall(T* memory)
    {
        memory->~T();
    }

    template <typename T>
    void DeleteArray(Shipyard::BaseAllocator& allocator, T* pArray)
    {
        // The small header before the user pointer contains the number of items that were created and the address of the user pointer
        // that we need to deallocate
        size_t* pArrayHeader = reinterpret_cast<size_t*>(size_t(pArray) - sizeForArraySizeHeader);
        size_t length = *pArrayHeader;
        size_t addressOfUserPointer = *(pArrayHeader + 1);

        for (size_t i = 0; i < length; i++)
        {
            pArray[i].~T();
        }

        allocator.Deallocate(reinterpret_cast<void*>(addressOfUserPointer));
    }
}

namespace Shipyard
{
#ifdef SHIP_ALLOCATOR_DEBUG_INFO

#define SHIP_ALLOC(allocator, size, alignment) allocator.Allocate(size, alignment, __FILE__, __LINE__)

#define SHIP_NEW(allocator, classX, alignment) new(allocator.Allocate(sizeof(classX), alignment, __FILE__, __LINE__))classX

#define SHIP_NEW_ARRAY(allocator, classX, length, alignment) NewArray<classX>(allocator, length, alignment, __FILE__, __LINE__)

#else

#define SHIP_ALLOC(allocator, size, alignment) allocator.Allocate(size, alignment)

#define SHIP_NEW(allocator, classX, alignment) new(allocator.Allocate(sizeof(classX), alignment))classX

#define SHIP_NEW_ARRAY(allocator, classX, length, alignment) NewArray<classX>(allocator, length, alignment)

#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

#define SHIP_FREE(allocator, memory) if (memory != nullptr) allocator.Deallocate(memory)

#define SHIP_DELETE(allocator, memory) if (memory != nullptr) { DestructorCall(memory); allocator.Deallocate(memory); }

#define SHIP_DELETE_ARRAY(allocator, memory) if (memory != nullptr) DeleteArray(allocator, memory)

}