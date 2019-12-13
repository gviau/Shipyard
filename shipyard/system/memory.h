#pragma once

#include <system/memory/baseallocator.h>

#include <mutex>

namespace Shipyard
{
    class SHIPYARD_SYSTEM_API GlobalAllocator : public BaseAllocator
    {
    public:
        static const shipUint32 ms_MaxNumAllocators = 32;

        struct AllocatorInitEntry
        {
            BaseAllocator* pAllocator = nullptr;

            // maxAllocationSize is the size in bytes for up to which the allocator will be considered.
            size_t maxAllocationSize = 0;
        };

    public:

        static GlobalAllocator& GetInstance()
        {
            static GlobalAllocator s_GlobalAllocator;
            return s_GlobalAllocator;
        }

        // Allocators are all assumed to be created beforehand.
        // During allocations, first matching allocator found will be used. Therefore, allocators are assumed to
        // be in increasing maximum size. Last allocator's maximum size is ignored since its assumed to be the fallback allocator.
        shipBool Create(AllocatorInitEntry* pInitEntries, shipUint32 numAllocators);

        void Destroy();

        // Alignment must be a power of 2 and non-zero.
        virtual void* Allocate(size_t size, size_t alignment

                #ifdef SHIP_ALLOCATOR_DEBUG_INFO
                    , const shipChar* pAllocationFilename
                    , int allocationLineNumber
                #endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

            ) override;

        // Memory must come from the allocator that allocated it.
        virtual void Deallocate(const void* memory) override;

    private:
        struct AllocatorAddressRange
        {
            BaseAllocator* pAllocator = nullptr;
            size_t startingAddressBytes = 0;
            size_t endingAddressBytes = 0;
        };

    private:
        GlobalAllocator();
        ~GlobalAllocator();

        GlobalAllocator(const GlobalAllocator& src) = delete;
        GlobalAllocator& operator= (const GlobalAllocator& rhs) = delete;
        GlobalAllocator& operator= (const GlobalAllocator&& rhs) = delete;

        size_t m_MaxAllocationSizes[ms_MaxNumAllocators];
        AllocatorAddressRange m_pAllocators[ms_MaxNumAllocators];
        shipUint32 m_NumAllocators;

        std::mutex m_Lock;

#ifdef SHIP_DEBUG
        // Used to assert when we forget to initialize this guy before usage.
        shipBool m_Initialized;
#endif // #ifdef SHIP_DEBUG
    };

    SHIPYARD_SYSTEM_API GlobalAllocator& GetGlobalAllocator();
}

// Those macros use the GlobalAllocator instead of specifying one. Be sure not to mismatch allocators!
#ifdef SHIP_ALLOCATOR_DEBUG_INFO

#define SHIP_ALLOC(size, alignment) Shipyard::GetGlobalAllocator().Allocate(size, alignment, __FILE__, __LINE__)

#define SHIP_NEW(classX, alignment) new(Shipyard::GetGlobalAllocator().Allocate(sizeof(classX), alignment, __FILE__, __LINE__))classX

#define SHIP_NEW_ARRAY(classX, length, alignment) NewArray<classX>(&Shipyard::GetGlobalAllocator(), length, alignment, __FILE__, __LINE__)

#else

#define SHIP_ALLOC(size, alignment) Shipyard::GetGlobalAllocator().Allocate(size, alignment)

#define SHIP_NEW(classX, alignment) new(Shipyard::GetGlobalAllocator().Allocate(sizeof(classX), alignment))classX

#define SHIP_NEW_ARRAY(classX, length, alignment) NewArray<classX>(&Shipyard::GetGlobalAllocator(), length, alignment)

#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

#define SHIP_FREE(memory) if (memory != nullptr) Shipyard::GetGlobalAllocator().Deallocate(memory)

#define SHIP_DELETE(memory) if (memory != nullptr) { DestructorCall(memory); Shipyard::GetGlobalAllocator().Deallocate(memory); }

#define SHIP_DELETE_ARRAY(memory) if (memory != nullptr) DeleteArray(&Shipyard::GetGlobalAllocator(), memory)