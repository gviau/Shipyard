#pragma once

#include <system/systemcommon.h>

#include <math/mathutilities.h>

#include <system/array.h>
#include <system/bitfield.h>

namespace Shipyard
{
    // To be used for a maximum sized array where objects must be as contiguous as possible.
    // With the use of a bitfield, the container will prioritize empty locations that are closest
    // to the beginning of the internal array.
    //
    // Allocated size is floor(MaxElementsInPool / 64) * sizeof(Bitfield::BitfieldType) bytes due to the bitfield + MaxElementsInPool * sizeof(T) bytes.
    //
    // The rationale behind returning indices in the pool instead of pointers is that
    // shipUint32 can access the full range of the array without wasting another 4 bytes
    // from a 8 bytes pointer (on 64 bits platform).
    template<typename T, shipUint32 MaxElementsInPool, size_t alignment = 1>
    class DataPool
    {
    public:
        static const shipUint32 InvalidDataPoolIndex = shipUint32(-1);

    public:
        DataPool(BaseAllocator* pAllocator = nullptr)
            : m_pAllocator(pAllocator)
            , m_Datas(nullptr)
            , m_LastFreeIndex(0)
        {
            if (pAllocator == nullptr)
            {
                m_pAllocator = &GetGlobalAllocator();
            }

        }

        ~DataPool()
        {
            SHIP_ASSERT_MSG(m_FreePoolIndices.GetLongestRangeWithBitsSet(0) == MaxElementsInPool, "DataPool 0x%p elements were not all freed.", this);

            SHIP_FREE_EX(m_pAllocator, m_Datas);
        }

        shipBool Create()
        {
            m_FreePoolIndices.SetAllocator(m_pAllocator);

            constexpr shipBool setAllBits = true;
            if (!m_FreePoolIndices.Create(setAllBits))
            {
                return false;
            }

            size_t requiredSizeForPool = MaxElementsInPool * sizeof(T);

            m_Datas = reinterpret_cast<T*>(SHIP_ALLOC_EX(m_pAllocator, requiredSizeForPool, alignment));
            if (m_Datas == nullptr)
            {
                return false;
            }

            return true;
        }

        shipUint32 GetNewItemIndex()
        {
            SHIP_ASSERT_MSG(!m_FreePoolIndices.IsClear(), "Trying to get a new element from an empty data pool 0x%p with maximum size %u", this, MaxElementsInPool);

            shipUint32 newItemIndex = 0;
            if (m_FreePoolIndices.GetFirstBitSet(m_LastFreeIndex, newItemIndex))
            {
                m_FreePoolIndices.UnsetBit(newItemIndex);

                m_LastFreeIndex = newItemIndex;

                new (m_Datas + newItemIndex)T();

                return newItemIndex;
            }

            return InvalidDataPoolIndex;
        }

        shipBool GetFirstAllocatedIndex(shipUint32* firstAllocatedIndex) const
        {
            shipUint32 idx = m_FreePoolIndices.GetLongestRangeWithBitsSet(0);

            if (idx == MaxElementsInPool)
            {
                return false;
            }

            *firstAllocatedIndex = idx;

            return true;
        }

        shipBool GetNextAllocatedIndex(shipUint32 currentIndex, shipUint32* nextAllocatedIndex) const
        {
            shipUint32 nextIndex = currentIndex + 1;

            if (nextIndex == MaxElementsInPool)
            {
                return false;
            }

            nextIndex += m_FreePoolIndices.GetLongestRangeWithBitsSet(nextIndex);

            if (nextIndex == MaxElementsInPool)
            {
                return false;
            }

            *nextAllocatedIndex = nextIndex;

            return true;
        }

        void ReleaseItem(shipUint32 index)
        {
            SHIP_ASSERT_MSG(!m_FreePoolIndices.IsBitSet(index), "Releasing index %u more than once for data pool 0x%p", index, this);

            m_Datas[index].~T();

            m_FreePoolIndices.SetBit(index);
            m_LastFreeIndex = MIN(index, m_LastFreeIndex);
        }

        T& GetItem(shipUint32 index)
        {
            SHIP_ASSERT_MSG(!m_FreePoolIndices.IsBitSet(index), "Accessing index %u that was not previously allocated for data pool 0x%p", index, this);
            return m_Datas[index];
        }

        const T& GetItem(shipUint32 index) const
        {
            SHIP_ASSERT_MSG(!m_FreePoolIndices.IsBitSet(index), "Accessing index %u that was not previously allocated for data pool 0x%p", index, this);
            return m_Datas[index];
        }

        T* GetItemPtr(shipUint32 index)
        {
            SHIP_ASSERT_MSG(!m_FreePoolIndices.IsBitSet(index), "Accessing index %u that was not previously allocated for data pool 0x%p", index, this);
            return &m_Datas[index];
        }

        const T* GetItemPtr(shipUint32 index) const
        {
            SHIP_ASSERT_MSG(!m_FreePoolIndices.IsBitSet(index), "Accessing index %u that was not previously allocated for data pool 0x%p", index, this);
            return &m_Datas[index];
        }

        shipUint32 GetMaxElementsInPool() const
        {
            return MaxElementsInPool;
        }

    protected:
        BaseAllocator* m_pAllocator;
        Bitfield<MaxElementsInPool> m_FreePoolIndices;
        T* m_Datas;
        shipUint32 m_LastFreeIndex;
    };

    template<typename T, shipUint32 MaxElementsInPool, size_t alignment = 1>
    class GenerationalDataPool
    {
    public:
        static const shipUint32 InvalidDataPoolIndex = shipUint32(-1);
        using GenerationType = shipUint16;

    public:
        GenerationalDataPool(BaseAllocator* pAllocator = nullptr)
            : m_pAllocator(pAllocator)
            , m_Datas(nullptr)
            , m_LastFreeIndex(0)
        {
            if (pAllocator == nullptr)
            {
                m_pAllocator = &GetGlobalAllocator();
            }

        }

        ~GenerationalDataPool()
        {
            SHIP_ASSERT_MSG(m_FreePoolIndices.GetLongestRangeWithBitsSet(0) == MaxElementsInPool, "DataPool 0x%p elements were not all freed.", this);

            SHIP_FREE_EX(m_pAllocator, m_Generations);
            SHIP_FREE_EX(m_pAllocator, m_Datas);
        }

        shipBool Create()
        {
            m_FreePoolIndices.SetAllocator(m_pAllocator);

            constexpr shipBool setAllBits = true;
            if (!m_FreePoolIndices.Create(setAllBits))
            {
                return false;
            }

            size_t requiredSizeForPool = MaxElementsInPool * sizeof(T);

            m_Datas = reinterpret_cast<T*>(SHIP_ALLOC_EX(m_pAllocator, requiredSizeForPool, alignment));
            if (m_Datas == nullptr)
            {
                return false;
            }

            m_Generations = reinterpret_cast<shipUint16*>(SHIP_ALLOC_EX(m_pAllocator, MaxElementsInPool * sizeof(GenerationType), alignment));
            if (m_Generations == nullptr)
            {
                return false;
            }

            for (shipUint32 i = 0; i < MaxElementsInPool; i++)
            {
                m_Generations[i] = 0;
            }

            return true;
        }

        shipUint32 GetNewItemIndex(GenerationType* newItemGeneration)
        {
            SHIP_ASSERT_MSG(!m_FreePoolIndices.IsClear(), "Trying to get a new element from an empty data pool 0x%p with maximum size %u", this, MaxElementsInPool);

            shipUint32 newItemIndex = 0;
            if (m_FreePoolIndices.GetFirstBitSet(m_LastFreeIndex, newItemIndex))
            {
                m_FreePoolIndices.UnsetBit(newItemIndex);

                m_LastFreeIndex = newItemIndex;

                new (m_Datas + newItemIndex)T();

                *newItemGeneration = m_Generations[newItemIndex];

                return newItemIndex;
            }

            return InvalidDataPoolIndex;
        }

        shipBool GetFirstAllocatedIndex(shipUint32* firstAllocatedIndex, GenerationType* firstAllocatedGeneration) const
        {
            shipUint32 idx = m_FreePoolIndices.GetLongestRangeWithBitsSet(0);

            if (idx == MaxElementsInPool)
            {
                return false;
            }

            *firstAllocatedIndex = idx;
            *firstAllocatedGeneration = m_Generations[idx];

            return true;
        }

        shipBool GetNextAllocatedIndex(shipUint32 currentIndex, shipUint32* nextAllocatedIndex, GenerationType* nextAllocationGeneration) const
        {
            shipUint32 nextIndex = currentIndex + 1;

            if (nextIndex == MaxElementsInPool)
            {
                return false;
            }

            nextIndex += m_FreePoolIndices.GetLongestRangeWithBitsSet(nextIndex);

            if (nextIndex == MaxElementsInPool)
            {
                return false;
            }

            *nextAllocatedIndex = nextIndex;
            *nextAllocationGeneration = m_Generations[nextIndex];

            return true;
        }

        void ReleaseItem(shipUint32 index)
        {
            SHIP_ASSERT_MSG(!m_FreePoolIndices.IsBitSet(index), "Releasing index %u more than once for data pool 0x%p", index, this);

            m_Datas[index].~T();
            m_Generations[index] += 1;

            m_FreePoolIndices.SetBit(index);
            m_LastFreeIndex = MIN(index, m_LastFreeIndex);
        }

        T& GetItem(shipUint32 index, GenerationType generation)
        {
            SHIP_ASSERT_MSG(!m_FreePoolIndices.IsBitSet(index), "Accessing index %u that was not previously allocated for data pool 0x%p", index, this);
            SHIP_ASSERT_MSG(m_Generations[index] == generation, "Accessing index %u that was already deleted! Current generation %u, requested generation %u", index, m_Generations[index], generation);
            return m_Datas[index];
        }

        const T& GetItem(shipUint32 index, GenerationType generation) const
        {
            SHIP_ASSERT_MSG(!m_FreePoolIndices.IsBitSet(index), "Accessing index %u that was not previously allocated for data pool 0x%p", index, this);
            SHIP_ASSERT_MSG(m_Generations[index] == generation, "Accessing index %u that was already deleted! Current generation %u, requested generation %u", index, m_Generations[index], generation);
            return m_Datas[index];
        }

        T* GetItemPtr(shipUint32 index, GenerationType generation)
        {
            SHIP_ASSERT_MSG(!m_FreePoolIndices.IsBitSet(index), "Accessing index %u that was not previously allocated for data pool 0x%p", index, this);
            SHIP_ASSERT_MSG(m_Generations[index] == generation, "Accessing index %u that was already deleted! Current generation %u, requested generation %u", index, m_Generations[index], generation);
            return &m_Datas[index];
        }

        const T* GetItemPtr(shipUint32 index, GenerationType generation) const
        {
            SHIP_ASSERT_MSG(!m_FreePoolIndices.IsBitSet(index), "Accessing index %u that was not previously allocated for data pool 0x%p", index, this);
            SHIP_ASSERT_MSG(m_Generations[index] == generation, "Accessing index %u that was already deleted! Current generation %u, requested generation %u", index, m_Generations[index], generation);
            return &m_Datas[index];
        }

        shipUint32 GetMaxElementsInPool() const
        {
            return MaxElementsInPool;
        }

    protected:
        BaseAllocator* m_pAllocator;
        Bitfield<MaxElementsInPool> m_FreePoolIndices;
        GenerationType* m_Generations;
        T* m_Datas;
        shipUint32 m_LastFreeIndex;
    };
}