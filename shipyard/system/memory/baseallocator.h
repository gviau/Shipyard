#pragma once

#include <system/platform.h>

#include <math/mathutilities.h>

#include <system/memory/memoryutils.h>

#ifndef SHIP_OPTIMIZED

// This define allows logging of some information about allocation and deallocation.
#define SHIP_ALLOCATOR_DEBUG_INFO

// This define enables memsetting regions of memory with pre-defined values for debugging.
// #define SHIP_ALLOCATOR_DEBUG_MEMORY_FILL

#endif // #ifndef SHIP_OPTIMIZED

namespace Shipyard
{
    class SHIPYARD_API BaseAllocator
    {
    public:
        BaseAllocator()
            : m_pHeap(nullptr)
            , m_HeapSize(0)

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
            , m_pAllocatorDebugName("Unammed allocator")
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO
        {
        }

        // Alignment must be a power of 2 and non-zero.
        virtual void* Allocate(size_t size, size_t alignment
        
                #ifdef SHIP_ALLOCATOR_DEBUG_INFO
                    , const shipChar* pAllocationFilename
                    , int allocationLineNumber
                #endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            ) = 0;

        // Memory must come from the allocator that allocated it.
        virtual void Deallocate(const void* memory) = 0;

        const void* GetHeap() const { return m_pHeap; }
        size_t GetHeapSize() const { return m_HeapSize; }

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        void SetAllocatorDebugName(const shipChar* pAllocatorDebugName) { m_pAllocatorDebugName = pAllocatorDebugName; }
        const shipChar* GetAllocatorDebugName() const { return m_pAllocatorDebugName; }
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

    protected:
        void* m_pHeap;
        size_t m_HeapSize;

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        const shipChar* m_pAllocatorDebugName;
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO
    };
}

namespace
{
    template <typename T>
    T* NewArray(Shipyard::BaseAllocator* pAllocator, size_t length, size_t alignment

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
        const size_t sizeForArraySizeHeader = sizeof(size_t) * 2;
        size_t requiredSize = sizeForArray + sizeForArraySizeHeader + alignment;

        // Since we're taking care of alignment manually here, no need to respect it at the allocation level.
        // Alignment is guaranteed to be respected since we allocated 'alignment' extra bytes, so we'll be able
        // to forward align the user pointer.
        constexpr size_t noAlignment = 1;
        void* pMem = pAllocator->Allocate(requiredSize, noAlignment
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
        size_t forwardAlignedAddress = Shipyard::MemoryUtils::AlignAddress(userPointerAddress, alignment);

        size_t diffAddress = (forwardAlignedAddress - userPointerAddress);

        bool enoughSpaceForHeader = (diffAddress >= sizeForArraySizeHeader);

        bool needToAlignToNextBoundary = !enoughSpaceForHeader;
        if (needToAlignToNextBoundary)
        {
            forwardAlignedAddress += (sizeForArraySizeHeader - MAX(diffAddress, 1));
            forwardAlignedAddress = Shipyard::MemoryUtils::AlignAddress(forwardAlignedAddress, alignment);
        }

        // Required header so that we know how many objects to destroy when deleting the array.
        // Also need to know where is the real user pointer.
        size_t* pArraySizeHeader = reinterpret_cast<size_t*>(forwardAlignedAddress - sizeForArraySizeHeader);
        *pArraySizeHeader = length;
        *(pArraySizeHeader + 1) = userPointerAddress;

        T* pArray = reinterpret_cast<T*>(forwardAlignedAddress);

        for (size_t i = 0; i < length; i++)
        {
            new(pArray + i)T;
        }

        return pArray;
    }

    template <typename T>
    void DestructorCall(const T* memory)
    {
        memory->~T();
    }

    template <typename T>
    void DeleteArray(Shipyard::BaseAllocator* pAllocator, const T* pArray)
    {
        // The small header before the user pointer contains the number of items that were created and the address of the user pointer
        // that we need to deallocate
        const size_t sizeForArraySizeHeader = sizeof(size_t) * 2;

        size_t* pArrayHeader = reinterpret_cast<size_t*>(size_t(pArray) - sizeForArraySizeHeader);
        size_t length = *pArrayHeader;
        size_t addressOfUserPointer = *(pArrayHeader + 1);

        for (size_t i = 0; i < length; i++)
        {
            pArray[i].~T();
        }

        pAllocator->Deallocate(reinterpret_cast<void*>(addressOfUserPointer));
    }
}

namespace Shipyard
{
#ifdef SHIP_ALLOCATOR_DEBUG_INFO

#define SHIP_ALLOC_EX(pAllocator, size, alignment) (pAllocator)->Allocate(size, alignment, __FILE__, __LINE__)

#define SHIP_NEW_EX(pAllocator, classX, alignment) new((pAllocator)->Allocate(sizeof(classX), alignment, __FILE__, __LINE__))classX

#define SHIP_NEW_ARRAY_EX(pAllocator, classX, length, alignment) NewArray<classX>(pAllocator, length, alignment, __FILE__, __LINE__)

#else

#define SHIP_ALLOC_EX(pAllocator, size, alignment) (pAllocator)->Allocate(size, alignment)

#define SHIP_NEW_EX(pAllocator, classX, alignment) new((pAllocator)->Allocate(sizeof(classX), alignment))classX

#define SHIP_NEW_ARRAY_EX(pAllocator, classX, length, alignment) NewArray<classX>(pAllocator, length, alignment)

#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

#define SHIP_FREE_EX(pAllocator, memory) if (memory != nullptr) (pAllocator)->Deallocate(memory)

#define SHIP_DELETE_EX(pAllocator, memory) if (memory != nullptr) { DestructorCall(memory); (pAllocator)->Deallocate(memory); }

#define SHIP_DELETE_ARRAY_EX(pAllocator, memory) if (memory != nullptr) DeleteArray(pAllocator, memory)

}