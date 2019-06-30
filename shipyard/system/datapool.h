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
    // uint32_t can access the full range of the array without wasting another 4 bytes
    // from a 8 bytes pointer (on 64 bits platform).
    template<typename T, uint32_t MaxElementsInPool, size_t alignment = 1>
    class DataPool
    {
    public:
        static const uint32_t InvalidDataPoolIndex = uint32_t(-1);

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

        bool Create()
        {
            m_FreePoolIndices.SetAllocator(m_pAllocator);

            constexpr bool setAllBits = true;
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

        uint32_t GetNewItemIndex()
        {
            SHIP_ASSERT_MSG(!m_FreePoolIndices.IsClear(), "Trying to get a new element from an empty data pool 0x%p with maximum size %u", this, MaxElementsInPool);

            uint32_t newItemIndex = 0;
            if (m_FreePoolIndices.GetFirstBitSet(m_LastFreeIndex, newItemIndex))
            {
                m_FreePoolIndices.UnsetBit(newItemIndex);

                m_LastFreeIndex = newItemIndex;

                new (m_Datas + newItemIndex)T();

                return newItemIndex;
            }

            return InvalidDataPoolIndex;
        }

        void ReleaseItem(uint32_t index)
        {
            SHIP_ASSERT_MSG(!m_FreePoolIndices.IsBitSet(index), "Releasing index %u more than once for data pool 0x%p", index, this);

            m_Datas[index].~T();

            m_FreePoolIndices.SetBit(index);
            m_LastFreeIndex = MIN(index, m_LastFreeIndex);
        }

        T& GetItem(uint32_t index)
        {
            SHIP_ASSERT_MSG(!m_FreePoolIndices.IsBitSet(index), "Accessing index %u that was not previously allocated for data pool 0x%p", index, this);
            return m_Datas[index];
        }

        const T& GetItem(uint32_t index) const
        {
            SHIP_ASSERT_MSG(!m_FreePoolIndices.IsBitSet(index), "Accessing index %u that was not previously allocated for data pool 0x%p", index, this);
            return m_Datas[index];
        }

        T* GetItemPtr(uint32_t index)
        {
            SHIP_ASSERT_MSG(!m_FreePoolIndices.IsBitSet(index), "Accessing index %u that was not previously allocated for data pool 0x%p", index, this);
            return &m_Datas[index];
        }

        const T* GetItemPtr(uint32_t index) const
        {
            SHIP_ASSERT_MSG(!m_FreePoolIndices.IsBitSet(index), "Accessing index %u that was not previously allocated for data pool 0x%p", index, this);
            return &m_Datas[index];
        }

        bool GetFirstAllocatedIndex(uint32_t* firstAllocatedIndex)
        {
            uint32_t idx = 0;
            while (idx < MaxElementsInPool && m_FreePoolIndices.IsBitSet(idx))
            {
                idx += 1;
            }

            if (idx == MaxElementsInPool)
            {
                return false;
            }

            *firstAllocatedIndex = idx;

            return true;
        }

        bool GetNextAllocatedIndex(uint32_t currentIndex, uint32_t* nextAllocatedIndex)
        {
            if (currentIndex == MaxElementsInPool)
            {
                return false;
            }

            uint32_t nextIndex = currentIndex;

            if (m_FreePoolIndices.IsBitSet(currentIndex))
            {
                nextIndex = currentIndex + m_FreePoolIndices.GetLongestRangeWithBitsSet(currentIndex);

            }
            else
            {
                nextIndex = currentIndex + 1;
            }

            if (nextIndex == MaxElementsInPool)
            {
                return false;
            }

            *nextAllocatedIndex = nextIndex;

            return true;
        }

        uint32_t GetMaxElementsInPool() const
        {
            return MaxElementsInPool;
        }

    private:
        BaseAllocator* m_pAllocator;
        Bitfield<MaxElementsInPool> m_FreePoolIndices;
        T* m_Datas;
        uint32_t m_LastFreeIndex;
    };
}