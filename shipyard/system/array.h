#pragma once

#include <system/common.h>
#include <system/memory.h>
#include <system/platform.h>

#include <cassert>
#include <cinttypes>
#include <stdlib.h>

namespace Shipyard
{
    // This array can hold a maximum of 16383 elements.
    template <typename T>
    class Array
    {
    public:
        Array()
            : m_Array(nullptr)
            , m_ArraySizeAndCapacity(0)
        {
            Reserve(4);
        }

        ~Array()
        {
            Clear();
        }

        Array(const Array& src)
            : m_Array(nullptr)
            , m_ArraySizeAndCapacity(0)
        {
            Reserve(src.Capacity());

            uint32_t srcSize = src.Size();

            m_ArraySizeAndCapacity |= srcSize;

            for (uint32_t i = 0; i < srcSize; i++)
            {
                m_Array[i] = src.m_Array[i];
            }
        }

        Array& operator= (const Array& rhs)
        {
            if (this != &rhs)
            {
                Clear();

                Reserve(src.Capacity());

                uint32_t srcSize = src.size();

                m_ArraySizeAndCapacity |= srcSize;

                for (uint32_t i = 0; i < srcSize; i++)
                {
                    m_Array[i] = src.m_Array[i];
                }
            }

            return *this;
        }

        T& operator[] (uint32_t index)
        {
            assert(index < Size());
            return m_Array[index];
        }

        const T& operator[] (uint32_t index) const
        {
            assert(index < Size());
            return m_Array[index];
        }

        T& Front()
        {
            uint32_t currentSize = Size();
            assert(currentSize > 0);

            return m_Array[0];
        }

        const T& Front() const
        {
            uint32_t currentSize = Size();
            assert(currentSize > 0);

            return m_Array[0];
        }

        T& Back()
        {
            uint32_t currentSize = Size();
            assert(currentSize > 0);

            return m_Array[currentSize - 1];
        }

        const T& Back() const
        {
            uint32_t currentSize = Size();
            assert(currentSize > 0);

            return m_Array[currentSize - 1];
        }

        void Add(const T& element)
        {
            uint32_t currentSize = Size();
            assert(currentSize < 16383);

            uint32_t currentCapacity = Capacity();

            if ((currentSize + 1) > currentCapacity)
            {
                uint32_t newCapacity = currentCapacity * 2;
                Reserve(MAX(newCapacity, 4));
            }

            m_Array[currentSize] = element;

            m_ArraySizeAndCapacity += 1;
        }

        void Pop()
        {
            uint32_t currentSize = Size();
            assert(currentSize > 0);

            m_Array[currentSize - 1].~T();

            m_ArraySizeAndCapacity -= 1;
        }

        void Remove(const T& elementToRemove)
        {
            uint32_t currentSize = Size();
            if (currentSize == 0)
            {
                return;
            }

            for (uint32_t i = 0; i < currentSize; i++)
            {
                if (m_Array[i] == elementToRemove)
                {
                    RemoveAt(i);
                    break;
                }
            }
        }

        void RemoveAt(uint32_t indexToRemove)
        {
            uint32_t currentSize = Size();
            assert(indexToRemove < currentSize);

            // Fast remove by swapping the index to remove with the last one.
            m_Array[indexToRemove] = m_Array[currentSize - 1];

            Pop();
        }

        void RemoveAtPreserveOrder(uint32_t indexToRemove)
        {
            uint32_t currentSize = Size();
            assert(indexToRemove < currentSize);

            if (currentSize > 1)
            {
                m_Array[indexToRemove].~T();

                for (uint32_t i = indexToRemove; i < (currentSize - 1); i++)
                {
                    m_Array[i] = m_Array[i + 1];
                }

                m_ArraySizeAndCapacity -= 1;
            }
            else
            {
                Pop();
            }
        }

        void Clear()
        {
            MemArrayFree(m_Array);

            m_Array = nullptr;
            m_ArraySizeAndCapacity = 0;
        }

        bool Empty() const
        {
            return (Size() == 0);
        }

        void InsertAt(uint32_t indexToInsertAt, const T& elementToInsert)
        {
            uint32_t currentSize = Size();
            assert(currentSize < 16384);

            uint32_t currentCapacity = Capacity();

            if ((currentSize + 1) > currentCapacity)
            {
                uint32_t newCapacity = currentCapacity * 2;
                Reserve(MAX(newCapacity, 4));
            }

            // We have to go in reverse order, otherwise we'd shift the element right after the index at which we want
            // to insert at every other location.
            for (uint32_t i = currentSize + 1; i > (indexToInsertAt + 1); i--)
            {
                uint32_t currentIdx = i - 1;
                uint32_t previousIdx = i - 2;

                m_Array[currentIdx] = m_Array[previousIdx];
            }

            m_Array[indexToInsertAt] = elementToInsert;

            m_ArraySizeAndCapacity += 1;
        }

        void Reserve(uint32_t newCapacity)
        {
            assert(newCapacity > 0);
            assert(newCapacity <= 16383);

            if (newCapacity <= Capacity())
            {
                return;
            }

            T* newArray = MemArrayAlloc(T, newCapacity);

            uint32_t currentSize = Size();

            for (uint32_t i = 0; i < currentSize; i++)
            {
                newArray[i] = m_Array[i];
            }

            MemArrayFree(m_Array);
            m_Array = newArray;

            m_ArraySizeAndCapacity &= ~0xFFFC000;
            m_ArraySizeAndCapacity |= (newCapacity << 14);
        }

        void Resize(uint32_t newSize)
        {
            assert(newSize <= 16383);

            uint32_t currentSize = Size();

            if (newSize < currentSize)
            {
                uint32_t numElementsToRemove = (currentSize - newSize);
                for (uint32_t i = 0; i < numElementsToRemove; i++)
                {
                    Pop();
                }
            }
            else if (newSize > currentSize)
            {
                uint32_t currentCapacity = Capacity();

                if (newSize > currentCapacity)
                {
                    Reserve(MAX(newSize, 4));
                }

                m_ArraySizeAndCapacity &= ~0x3FFF;
                m_ArraySizeAndCapacity |= newSize;
            }
        }

        uint32_t Size() const
        {
            return (m_ArraySizeAndCapacity & 0x3FFF);
        }

        uint32_t Capacity() const
        {
            return ((m_ArraySizeAndCapacity >> 14) & 0x3FFF);
        }

        uint32_t FindIndex(const T& elementToFind) const
        {
            uint32_t currentSize = Size();
            
            for (uint32_t i = 0; i < currentSize; i++)
            {
                if (m_Array[i] == elementToFind)
                {
                    return i;
                }
            }

            return uint32_t(-1);
        }

        bool Exists(const T& elementToFind) const
        {
            return (FindIndex(elementToFind) != uint32_t(-1));
        }

    private:
        T* m_Array;
        uint32_t m_ArraySizeAndCapacity;

    public:
        class ConstIterator
        {
        public:
            ConstIterator()
                : m_IteratorPtr(nullptr)
            {

            }

            ConstIterator(T* iteratorPtr)
                : m_IteratorPtr(iteratorPtr)
            {

            }
            
            ConstIterator(const ConstIterator& src)
                : m_IteratorPtr(src.m_IteratorPtr)
            {

            }

            ConstIterator& operator= (const ConstIterator& rhs)
            {
                m_IteratorPtr = rhs.m_IteratorPtr;
            }

            bool operator== (const ConstIterator& rhs) const
            {
                return (m_IteratorPtr == rhs.m_IteratorPtr);
            }

            bool operator!= (const ConstIterator& rhs) const
            {
                return (m_IteratorPtr != rhs.m_IteratorPtr);
            }

            bool operator< (const ConstIterator& rhs)  const
            {
                return (m_IteratorPtr < rhs.m_IteratorPtr);
            }

            bool operator<= (const ConstIterator& rhs) const
            {
                return (m_IteratorPtr <= rhs.m_IteratorPtr);
            }

            bool operator> (const ConstIterator& rhs) const
            {
                return (m_IteratorPtr > rhs.m_IteratorPtr);
            }

            bool operator>= (const ConstIterator& rhs) const
            {
                return (m_IteratorPtr >= rhs.m_IteratorPtr);
            }

            const T& operator* () const
            {
                return *m_IteratorPtr;
            }

            const T& operator-> () const
            {
                return m_IteratorPtr;
            }

            ConstIterator& operator++ ()
            {
                ++m_IteratorPtr;
                return (*this);
            }

            ConstIterator operator++ (int)
            {
                ConstIterator oldIterator = *this;

                ++*this;

                return oldIterator;
            }

            ConstIterator& operator-- ()
            {
                --m_IteratorPtr;
                return (*this);
            }

            ConstIterator operator-- (int)
            {
                ConstIterator oldIterator = *this;

                --*this;

                return oldIterator;
            }
            
        protected:
            T* m_IteratorPtr;
        };

        class Iterator : public ConstIterator
        {
        public:
            Iterator()
            {

            }

            Iterator(T* iteratorPtr)
                : ConstIterator(iteratorPtr)
            {

            }

            Iterator(const Iterator& src)
                : ConstIterator(src)
            {

            }

            T& operator* ()
            {
                return *m_IteratorPtr;
            }

            T& operator-> ()
            {
                return m_IteratorPtr;
            }
        };

    public:
        Iterator begin()
        {
            return Iterator(&m_Array[0]);
        }

        ConstIterator begin() const
        {
            return ConstIterator(&m_Array[0]);
        }

        Iterator end()
        {
            return Iterator(&m_Array[Size()]);
        }

        ConstIterator end() const
        {
            return ConstIterator(&m_Array[Size()]);
        }
    };
}