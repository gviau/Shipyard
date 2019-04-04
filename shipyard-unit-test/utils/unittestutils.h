#pragma once

#include <system/memory.h>
#include <system/memory/fixedheapallocator.h>
#include <system/memory/poolallocator.h>

namespace Shipyard
{
    class ScoppedGlobalAllocator
    {
    public:
        ScoppedGlobalAllocator()
            : m_pHeap(nullptr)
        {
            size_t totalHeapSize = 16 * 1024 * 1024;

            m_pHeap = malloc(totalHeapSize);

            size_t firstPoolAllocatorSize = 0;

            {
                size_t numChunks = 2048;
                size_t chunkSize = 16;

                firstPoolAllocatorSize = numChunks * chunkSize;

                void* pHeap = reinterpret_cast<void*>(Shipyard::AlignAddress(size_t(m_pHeap), chunkSize));

                m_FirstPoolAllocator.Create(pHeap, numChunks, chunkSize);
            }

            size_t secondPoolAllocatorSize = 0;

            {
                size_t numChunks = 2048;
                size_t chunkSize = 32;

                secondPoolAllocatorSize = numChunks * chunkSize;

                void* pHeap = reinterpret_cast<void*>(Shipyard::AlignAddress(size_t(m_pHeap) + firstPoolAllocatorSize, chunkSize));

                m_SecondPoolAllocator.Create(pHeap, numChunks, chunkSize);
            }

            {
                size_t previousSize = firstPoolAllocatorSize + secondPoolAllocatorSize;

                size_t heapStartAddress = size_t(m_pHeap) + previousSize;
                void* pHeap = reinterpret_cast<void*>(heapStartAddress);

                m_FixedHeapAllocator.Create(pHeap, totalHeapSize - previousSize);
            }

            Shipyard::GlobalAllocator::AllocatorInitEntry initEntries[3];
            initEntries[0].pAllocator = &m_FirstPoolAllocator;
            initEntries[0].maxAllocationSize = 16;
            initEntries[1].pAllocator = &m_SecondPoolAllocator;
            initEntries[1].maxAllocationSize = 32;
            initEntries[2].pAllocator = &m_FixedHeapAllocator;
            initEntries[2].maxAllocationSize = size_t(-1);

            uint32_t numInitEntries = uint32_t(sizeof(initEntries) / sizeof(initEntries[0]));

            Shipyard::GlobalAllocator::GetInstance().Create(initEntries, numInitEntries);
        }

        ~ScoppedGlobalAllocator()
        {
            m_FixedHeapAllocator.Destroy();
            m_SecondPoolAllocator.Destroy();
            m_FirstPoolAllocator.Destroy();

            Shipyard::GlobalAllocator::GetInstance().Destroy();

            free(m_pHeap);
        }

    private:
        Shipyard::FixedHeapAllocator m_FixedHeapAllocator;
        Shipyard::PoolAllocator m_FirstPoolAllocator;
        Shipyard::PoolAllocator m_SecondPoolAllocator;

        void* m_pHeap;
    };

    struct ScoppedBuffer
    {
        ScoppedBuffer(size_t size)
        {
            pInternalBuffer = malloc(size);
            pBuffer = pInternalBuffer;
        }

        ScoppedBuffer(size_t size, size_t alignment)
        {
            pInternalBuffer = malloc(size);
            pBuffer = reinterpret_cast<void*>(Shipyard::AlignAddress(size_t(pInternalBuffer), alignment));
        }

        ~ScoppedBuffer()
        {
            free(pInternalBuffer);
        }

        void* pBuffer = nullptr;
        void* pInternalBuffer = nullptr;
    };
}