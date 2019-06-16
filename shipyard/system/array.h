#pragma once

#include <math/mathutilities.h>

#include <system/memory.h>
#include <system/platform.h>
#include <system/systemdebug.h>

#include <cinttypes>
#include <stdlib.h>

#include <new>

namespace Shipyard
{
    template <typename T>
    class ArrayIterator
    {
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
    };

    // This array can hold a maximum of 16383 elements.
    template <typename T, size_t alignment = 1>
    class Array : public ArrayIterator<T>
    {
    protected:
        enum
        {
            BORROWED_MEMORY_FLAG = 0x80000000
        };

    public:
        Array(BaseAllocator* pAllocator = nullptr)
            : m_pAllocator(pAllocator)
            , m_Array(nullptr)
            , m_ArraySizeAndCapacity(0)
        {
            if (pAllocator == nullptr)
            {
                m_pAllocator = &GetGlobalAllocator();
            }

            Reserve(4);
        }

        Array(uint32_t initialCapacity, BaseAllocator* pAllocator = nullptr)
            : m_pAllocator(pAllocator)
            , m_Array(nullptr)
            , m_ArraySizeAndCapacity(0)
        {
            if (pAllocator == nullptr)
            {
                m_pAllocator = &GetGlobalAllocator();
            }

            if (initialCapacity > 0)
            {
                Reserve(initialCapacity);
            }
        }

        ~Array()
        {
            Clear();
        }

        Array(const Array& src)
            : m_pAllocator(src.m_pAllocator)
            , m_Array(nullptr)
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

                m_pAllocator = rhs.m_pAllocator;

                Reserve(rhs.Capacity());

                uint32_t srcSize = rhs.Size();

                m_ArraySizeAndCapacity |= srcSize;

                for (uint32_t i = 0; i < srcSize; i++)
                {
                    m_Array[i] = rhs.m_Array[i];
                }
            }

            return *this;
        }

        T& operator[] (uint32_t index)
        {
            SHIP_ASSERT(index < Size());
            return m_Array[index];
        }

        const T& operator[] (uint32_t index) const
        {
            SHIP_ASSERT(index < Size());
            return m_Array[index];
        }

        T& Front()
        {
            uint32_t currentSize = Size();
            SHIP_ASSERT(currentSize > 0);

            return m_Array[0];
        }

        const T& Front() const
        {
            uint32_t currentSize = Size();
            SHIP_ASSERT(currentSize > 0);

            return m_Array[0];
        }

        T& Back()
        {
            uint32_t currentSize = Size();
            SHIP_ASSERT(currentSize > 0);

            return m_Array[currentSize - 1];
        }

        const T& Back() const
        {
            uint32_t currentSize = Size();
            SHIP_ASSERT(currentSize > 0);

            return m_Array[currentSize - 1];
        }

        void Add(const T& element)
        {
            uint32_t currentSize = Size();
            SHIP_ASSERT(currentSize < 16383);

            uint32_t currentCapacity = Capacity();

            if ((currentSize + 1) > currentCapacity)
            {
                uint32_t newCapacity = MIN(currentCapacity * 2, 16383);
                Reserve(MAX(newCapacity, 4));
            }

            m_Array[currentSize] = element;

            m_ArraySizeAndCapacity += 1;
        }

        T& Grow()
        {
            Add(T());
            return Back();
        }

        void Pop()
        {
            uint32_t currentSize = Size();
            SHIP_ASSERT(currentSize > 0);

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
            SHIP_ASSERT(indexToRemove < currentSize);

            // Fast remove by swapping the index to remove with the last one.
            m_Array[indexToRemove] = m_Array[currentSize - 1];

            Pop();
        }

        void RemoveAtPreserveOrder(uint32_t indexToRemove)
        {
            uint32_t currentSize = Size();
            SHIP_ASSERT(indexToRemove < currentSize);

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
            if ((m_ArraySizeAndCapacity & BORROWED_MEMORY_FLAG) == 0)
            {
                uint32_t currentCapacity = Capacity();
                for (uint32_t i = 0; i < currentCapacity; i++)
                {
                    m_Array[i].~T();
                }

                SHIP_FREE_EX(m_pAllocator, m_Array);
            }

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
            SHIP_ASSERT(currentSize < 16384);

            uint32_t currentCapacity = Capacity();

            if ((currentSize + 1) > currentCapacity)
            {
                uint32_t newCapacity = MIN(currentCapacity * 2, 16383);
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
            SHIP_ASSERT(newCapacity > 0);
            SHIP_ASSERT(newCapacity <= 16383);

            if (newCapacity <= Capacity())
            {
                return;
            }

            size_t requiredSize = sizeof(T) * newCapacity;

            T* newArray = reinterpret_cast<T*>(SHIP_ALLOC_EX(m_pAllocator, requiredSize, alignment));

            for (uint32_t i = 0; i < newCapacity; i++)
            {
                new(newArray + i)T();
            }

            uint32_t currentSize = Size();

            for (uint32_t i = 0; i < currentSize; i++)
            {
                newArray[i] = m_Array[i];
            }

            if ((m_ArraySizeAndCapacity & BORROWED_MEMORY_FLAG) == 0)
            {
                uint32_t currentCapacity = Capacity();
                for (uint32_t i = 0; i < currentCapacity; i++)
                {
                    m_Array[i].~T();
                }

                SHIP_FREE_EX(m_pAllocator, m_Array);
            }

            m_Array = newArray;

            m_ArraySizeAndCapacity &= ~0xFFFC000;
            m_ArraySizeAndCapacity |= (newCapacity << 14);
        }

        void Resize(uint32_t newSize)
        {
            SHIP_ASSERT(newSize <= 16383);

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

        void SetUserPointer(T* userArray, uint32_t numElements, uint32_t startingSize)
        {
            SHIP_ASSERT(userArray != nullptr);
            SHIP_ASSERT(numElements > 0);
            SHIP_ASSERT(numElements < 16384);
            SHIP_ASSERT(startingSize <= numElements);

            Clear();

            m_Array = userArray;

            m_ArraySizeAndCapacity |= startingSize;
            m_ArraySizeAndCapacity |= (numElements << 14);
            m_ArraySizeAndCapacity |= BORROWED_MEMORY_FLAG;
        }

        void SetAllocator(BaseAllocator* pAllocator)
        {
            if (pAllocator == m_pAllocator)
            {
                return;
            }

            if ((m_ArraySizeAndCapacity & BORROWED_MEMORY_FLAG) == 0)
            {
                size_t capacity = Capacity();

                size_t requiredSize = sizeof(T) * capacity;
                T* pNewArray = reinterpret_cast<T*>(SHIP_ALLOC_EX(pAllocator, requiredSize, alignment));

                for (uint32_t i = 0; i < capacity; i++)
                {
                    new(pNewArray + i)T();
                }

                uint32_t numElements = Size();

                for (uint32_t i = 0; i < numElements; i++)
                {
                    pNewArray[i] = m_Array[i];
                }

                if ((m_ArraySizeAndCapacity & BORROWED_MEMORY_FLAG) == 0)
                {
                    uint32_t currentCapacity = Capacity();
                    for (uint32_t i = 0; i < currentCapacity; i++)
                    {
                        m_Array[i].~T();
                    }

                    SHIP_FREE_EX(m_pAllocator, m_Array);
                }

                m_Array = pNewArray;
            }

            m_pAllocator = pAllocator;
        }

    protected:
        BaseAllocator* m_pAllocator;
        T* m_Array;
        uint32_t m_ArraySizeAndCapacity;

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

    // Syntactic sugar for:
    //
    // ArrayType staticBuffer[100];
    // Array<ArrayType> myArray;
    // myArray.SetUserPoint(staticBuffer, 100, 0);
    //
    // Instead:
    //
    // InplaceArray<ArrayType, 100> myArray;
    template <typename T, uint32_t inplaceSize, size_t alignment = 1>
    class InplaceArray : public Array<T, alignment>
    {
    public:
        InplaceArray(BaseAllocator* pAllocator = nullptr)
            : Array(0, pAllocator)
        {
            this->SetUserPointer(m_StaticArray, inplaceSize, 0);
        }

    private:
        T m_StaticArray[inplaceSize];
    };

    // Array of number of elements larger than 16383, up to 4294967295 elements (2 ** 32 - 1)
    template <typename T, size_t alignment = 1>
    class BigArray : public ArrayIterator<T>
    {
    public:
        BigArray(BaseAllocator* pAllocator = nullptr)
            : m_pAllocator(pAllocator)
            , m_Array(nullptr)
            , m_Size(0)
            , m_Capacity(0)
        {
            if (pAllocator == nullptr)
            {
                m_pAllocator = &GetGlobalAllocator();
            }

            Reserve(4);
        }

        BigArray(uint32_t initialCapacity, BaseAllocator* pAllocator = nullptr)
            : m_pAllocator(pAllocator)
            , m_Array(nullptr)
            , m_Size(0)
            , m_Capacity(0)
        {
            if (pAllocator == nullptr)
            {
                m_pAllocator = &GetGlobalAllocator();
            }

            if (initialCapacity > 0)
            {
                Reserve(initialCapacity);
            }
        }

        ~BigArray()
        {
            Clear();
        }

        BigArray(const BigArray& src)
            : m_pAllocator(src.m_pAllocator)
            , m_Array(nullptr)
            , m_Size(0)
            , m_Capacity(0)
        {
            Reserve(src.m_Capacity);

            m_Size = src.m_Size;

            for (uint32_t i = 0; i < m_Size; i++)
            {
                m_Array[i] = src.m_Array[i];
            }
        }

        BigArray& operator= (const BigArray& rhs)
        {
            if (this != &rhs)
            {
                Clear();

                m_pAllocator = rhs.m_pAllocator;

                Reserve(rhs.m_Capacity);

                m_Size = rhs.m_Size;

                for (uint32_t i = 0; i < m_Size; i++)
                {
                    m_Array[i] = rhs.m_Array[i];
                }
            }

            return *this;
        }

        T& operator[] (uint32_t index)
        {
            SHIP_ASSERT(index < m_Size);
            return m_Array[index];
        }

        const T& operator[] (uint32_t index) const
        {
            SHIP_ASSERT(index < m_Size);
            return m_Array[index];
        }

        T& Front()
        {
            SHIP_ASSERT(m_Size > 0);

            return m_Array[0];
        }

        const T& Front() const
        {
            SHIP_ASSERT(m_Size > 0);

            return m_Array[0];
        }

        T& Back()
        {
            SHIP_ASSERT(m_Size > 0);

            return m_Array[m_Size - 1];
        }

        const T& Back() const
        {
            SHIP_ASSERT(m_Size > 0);

            return m_Array[m_Size - 1];
        }

        void Add(const T& element)
        {
            SHIP_ASSERT(m_Size < 0xFFFFFFFF);

            if ((m_Size + 1) > m_Capacity)
            {
                uint32_t newCapacity = uint32_t(MIN(uint64_t(m_Capacity) * 2, 0xFFFFFFFF));
                Reserve(MAX(newCapacity, 4));
            }

            m_Array[m_Size] = element;

            m_Size += 1;
        }

        T& Grow()
        {
            Add(T());
            return Back();
        }

        void Pop()
        {
            SHIP_ASSERT(m_Size > 0);

            m_Array[m_Size - 1].~T();

            m_Size -= 1;
        }

        void Remove(const T& elementToRemove)
        {
            if (m_Size == 0)
            {
                return;
            }

            for (uint32_t i = 0; i < m_Size; i++)
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
            SHIP_ASSERT(indexToRemove < m_Size);

            // Fast remove by swapping the index to remove with the last one.
            m_Array[indexToRemove] = m_Array[m_Size - 1];

            Pop();
        }

        void RemoveAtPreserveOrder(uint32_t indexToRemove)
        {
            SHIP_ASSERT(indexToRemove < m_Size);

            if (m_Size > 1)
            {
                m_Array[indexToRemove].~T();

                for (uint32_t i = indexToRemove; i < (m_Size - 1); i++)
                {
                    m_Array[i] = m_Array[i + 1];
                }

                m_Size -= 1;
            }
            else
            {
                Pop();
            }
        }

        void Clear()
        {
            for (uint32_t i = 0; i < m_Capacity; i++)
            {
                m_Array[i].~T();
            }

            SHIP_FREE_EX(m_pAllocator, m_Array);

            m_Array = nullptr;
            m_Size = 0;
            m_Capacity = 0;
        }

        bool Empty() const
        {
            return (m_Size == 0);
        }

        void InsertAt(uint32_t indexToInsertAt, const T& elementToInsert)
        {
            SHIP_ASSERT(m_Size < 0xFFFFFFFF);

            if ((m_Size + 1) > m_Capacity)
            {
                uint32_t newCapacity = uint32_t(MIN(uint64_t(m_Capacity) * 2, 0xFFFFFFFF));
                Reserve(MAX(newCapacity, 4));
            }

            // We have to go in reverse order, otherwise we'd shift the element right after the index at which we want
            // to insert at every other location.
            for (uint32_t i = m_Size + 1; i > (indexToInsertAt + 1); i--)
            {
                uint32_t currentIdx = i - 1;
                uint32_t previousIdx = i - 2;

                m_Array[currentIdx] = m_Array[previousIdx];
            }

            m_Array[indexToInsertAt] = elementToInsert;

            m_Size += 1;
        }

        void Reserve(uint32_t newCapacity)
        {
            SHIP_ASSERT(newCapacity > 0);
            SHIP_ASSERT(newCapacity <= 0xFFFFFFFF);

            if (newCapacity <= m_Capacity)
            {
                return;
            }

            size_t requiredSize = sizeof(T) * newCapacity;

            T* newArray = reinterpret_cast<T*>(SHIP_ALLOC_EX(m_pAllocator, requiredSize, alignment));

            for (uint32_t i = 0; i < newCapacity; i++)
            {
                new(newArray + i)T();
            }

            for (uint32_t i = 0; i < m_Size; i++)
            {
                newArray[i] = m_Array[i];
            }

            for (uint32_t i = 0; i < m_Capacity; i++)
            {
                m_Array[i].~T();
            }

            SHIP_FREE_EX(m_pAllocator, m_Array);

            m_Array = newArray;

            m_Capacity = newCapacity;
        }

        void Resize(uint32_t newSize)
        {
            SHIP_ASSERT(newSize <= 0xFFFFFFFF);

            if (newSize < m_Size)
            {
                uint32_t numElementsToRemove = (m_Size - newSize);
                for (uint32_t i = 0; i < numElementsToRemove; i++)
                {
                    Pop();
                }
            }
            else if (newSize > m_Size)
            {
                if (newSize > m_Capacity)
                {
                    Reserve(MAX(newSize, 4));
                }
            }

            m_Size = newSize;
        }

        uint32_t Size() const
        {
            return m_Size;
        }

        uint32_t Capacity() const
        {
            return m_Capacity;
        }

        uint32_t FindIndex(const T& elementToFind) const
        {
            for (uint32_t i = 0; i < m_Size; i++)
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

        void SetAllocator(BaseAllocator* pAllocator)
        {
            if (pAllocator == m_pAllocator)
            {
                return;
            }

            size_t requiredSize = sizeof(T) * m_Capacity;
            T* pNewArray = reinterpret_cast<T*>(SHIP_ALLOC_EX(pAllocator, requiredSize, alignment));

            for (uint32_t i = 0; i < m_Capacity; i++)
            {
                new(pNewArray + i)T();
            }

            for (uint32_t i = 0; i < m_Size; i++)
            {
                pNewArray[i] = m_Array[i];
            }

            for (uint32_t i = 0; i < m_Capacity; i++)
            {
                m_Array[i].~T();
            }

            SHIP_FREE_EX(m_pAllocator, m_Array);

            m_Array = pNewArray;
            m_pAllocator = pAllocator;
        }

    protected:
        BaseAllocator* m_pAllocator;
        T* m_Array;
        uint32_t m_Size;
        uint32_t m_Capacity;

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