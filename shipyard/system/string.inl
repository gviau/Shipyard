#include <cstring>

#include <cstdarg>
#include <cstdio>

#include <system/systemdebug.h>

namespace Shipyard
{;

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif // #ifndef MIN

template <typename CharType>
String<CharType>::String(BaseAllocator* pAllocator)
    : m_pAllocator(pAllocator)
{
    if (pAllocator == nullptr)
    {
        m_pAllocator = &GlobalAllocator::GetInstance();
    }

    m_NumChars = 0;
    m_Capacity = DefaultStringCapacity;

    size_t requiredSize = sizeof(CharType) * m_Capacity;
    m_Buffer = reinterpret_cast<CharType*>(SHIP_ALLOC_EX(m_pAllocator, requiredSize, 1));

    m_Buffer[m_NumChars] = '\0';
}

template <typename CharType>
String<CharType>::String(const CharType* sz, BaseAllocator* pAllocator)
{
    if (pAllocator == nullptr)
    {
        m_pAllocator = &GlobalAllocator::GetInstance();
    }

    if (sz == nullptr)
    {
        m_NumChars = 0;
        m_Capacity = DefaultStringCapacity;

        size_t requiredSize = sizeof(CharType) * m_Capacity;
        m_Buffer = reinterpret_cast<CharType*>(SHIP_ALLOC_EX(m_pAllocator, requiredSize, 1));
    }
    else
    {
        m_NumChars = strlen(sz);
        m_Capacity = m_NumChars + 1;

        size_t requiredSize = sizeof(CharType) * m_Capacity;
        m_Buffer = reinterpret_cast<CharType*>(SHIP_ALLOC_EX(m_pAllocator, requiredSize, 1));

        memcpy(m_Buffer, sz, m_NumChars);
    }

    m_Buffer[m_NumChars] = '\0';
}

template <typename CharType>
String<CharType>::String(const CharType* sz, size_t numChars, BaseAllocator* pAllocator)
{
    if (pAllocator == nullptr)
    {
        m_pAllocator = &GlobalAllocator::GetInstance();
    }

    if (sz == nullptr || numChars == 0)
    {
        m_NumChars = 0;
        m_Capacity = 1;

        size_t requiredSize = sizeof(CharType) * m_Capacity;
        m_Buffer = reinterpret_cast<CharType*>(SHIP_ALLOC_EX(m_pAllocator, requiredSize, 1));
    }
    else
    {
        m_NumChars = numChars;
        m_Capacity = m_NumChars + 1;

        size_t requiredSize = sizeof(CharType) * m_Capacity;
        m_Buffer = reinterpret_cast<CharType*>(SHIP_ALLOC_EX(m_pAllocator, requiredSize, 1));

        memcpy(m_Buffer, sz, m_NumChars);
    }

    m_Buffer[m_NumChars] = '\0';
}

template <typename CharType>
String<CharType>::String(const String<CharType>& src)
    : m_pAllocator(src.m_pAllocator)
{
    m_NumChars = src.m_NumChars;
    m_Capacity = src.m_Capacity;

    size_t requiredSize = sizeof(CharType) * m_Capacity;
    m_Buffer = reinterpret_cast<CharType*>(SHIP_ALLOC_EX(m_pAllocator, requiredSize, 1));

    memcpy(m_Buffer, src.m_Buffer, m_NumChars);

    m_Buffer[m_NumChars] = '\0';
}

template <typename CharType>
String<CharType>::~String()
{
    SHIP_FREE_EX(m_pAllocator, m_Buffer);
}

template <typename CharType>
String<CharType>& String<CharType>::operator= (const String <CharType>& rhs)
{
    if (&rhs != this)
    {
        SHIP_FREE_EX(m_pAllocator, m_Buffer);

        m_pAllocator = rhs.m_pAllocator;

        m_NumChars = rhs.m_NumChars;
        m_Capacity = rhs.m_Capacity;

        size_t requiredSize = sizeof(CharType) * m_Capacity;
        m_Buffer = reinterpret_cast<CharType*>(SHIP_ALLOC_EX(m_pAllocator, requiredSize, 1));

        memcpy(m_Buffer, rhs.m_Buffer, m_NumChars);

        m_Buffer[m_NumChars] = '\0';
    }

    return *this;
}

template <typename CharType>
String<CharType>& String<CharType>::operator= (const CharType* rhs)
{
    SHIP_FREE_EX(m_pAllocator, m_Buffer);

    m_NumChars = strlen(rhs);
    m_Capacity = m_NumChars + 1;

    size_t requiredSize = sizeof(CharType) * m_Capacity;
    m_Buffer = reinterpret_cast<CharType*>(SHIP_ALLOC_EX(m_pAllocator, requiredSize, 1));

    memcpy(m_Buffer, rhs, m_NumChars);

    m_Buffer[m_NumChars] = '\0';
    
    return *this;
}

template <typename CharType>
String<CharType>& String<CharType>::operator= (CharType c)
{
    SHIP_FREE_EX(m_pAllocator, m_Buffer);

    m_NumChars = 1;
    m_Capacity = 2;

    size_t requiredSize = sizeof(CharType) * m_Capacity;
    m_Buffer = reinterpret_cast<CharType*>(SHIP_ALLOC_EX(m_pAllocator, requiredSize, 1));

    m_Buffer[0] = c;
    m_Buffer[1] = '\0';

    return *this;
}

template <typename CharType>
CharType& String<CharType>::operator[] (size_t index)
{
    SHIP_ASSERT(index < m_NumChars);
    return m_Buffer[index];
}

template <typename CharType>
const CharType& String<CharType>::operator[] (size_t index) const
{
    SHIP_ASSERT(index < m_NumChars);
    return m_Buffer[index];
}

template <typename CharType>
void String<CharType>::operator+= (const String<CharType>& rhs)
{
    size_t newSize = m_NumChars + rhs.m_NumChars;

    if (newSize >= m_Capacity)
    {
        m_Capacity = newSize + 1;

        size_t requiredSize = sizeof(CharType) * m_Capacity;
        CharType* newBuffer = reinterpret_cast<CharType*>(SHIP_ALLOC_EX(m_pAllocator, requiredSize, 1));

        memcpy(newBuffer, m_Buffer, m_NumChars);

        SHIP_FREE_EX(m_pAllocator, m_Buffer);
        m_Buffer = newBuffer;
    }

    memcpy(&m_Buffer[m_NumChars], rhs.m_Buffer, rhs.m_NumChars);

    m_NumChars = newSize;

    m_Buffer[m_NumChars] = '\0';
}

template <typename CharType>
void String<CharType>::operator+= (const CharType* rhs)
{
    size_t numCharsToAdd = strlen(rhs);

    size_t newSize = m_NumChars + numCharsToAdd;
    
    if (newSize >= m_Capacity)
    {
        m_Capacity = newSize + 1;

        size_t requiredSize = sizeof(CharType) * m_Capacity;
        CharType* newBuffer = reinterpret_cast<CharType*>(SHIP_ALLOC_EX(m_pAllocator, requiredSize, 1));

        memcpy(newBuffer, m_Buffer, m_NumChars);

        SHIP_FREE_EX(m_pAllocator, m_Buffer);
        m_Buffer = newBuffer;
    }

    memcpy(&m_Buffer[m_NumChars], rhs, numCharsToAdd);

    m_NumChars = newSize;

    m_Buffer[m_NumChars] = '\0';
}

template <typename CharType>
void String<CharType>::operator+= (CharType c)
{
    size_t newSize = m_NumChars + 1;

    if (newSize >= m_Capacity)
    {
        m_Capacity = newSize + 1;

        size_t requiredSize = sizeof(CharType) * m_Capacity;
        CharType* newBuffer = reinterpret_cast<CharType*>(SHIP_ALLOC_EX(m_pAllocator, requiredSize, 1));

        memcpy(newBuffer, m_Buffer, m_NumChars);

        SHIP_FREE_EX(m_pAllocator, m_Buffer);
        m_Buffer = newBuffer;
    }

    m_Buffer[m_NumChars] = c;
    
    m_NumChars += 1;

    m_Buffer[m_NumChars] = '\0';
}

template <typename CharType>
String<CharType> String<CharType>::operator+ (const String<CharType>& rhs) const
{
    size_t sizeOfNewString = m_NumChars + rhs.m_NumChars;

    String<CharType> result;

    if (sizeOfNewString == 0)
    {
        return result;
    }

    result.Resize(sizeOfNewString);

    memcpy(result.m_Buffer, m_Buffer, m_NumChars);
    memcpy(&result.m_Buffer[m_NumChars], rhs.m_Buffer, rhs.m_NumChars);

    result.m_Buffer[result.m_NumChars] = '\0';

    return result;
}

template <typename CharType>
String<CharType> String<CharType>::operator+ (const CharType* rhs) const
{
    size_t sizeOfRhsString = strlen(rhs);

    size_t sizeOfNewString = m_NumChars + sizeOfRhsString;

    String<CharType> result;

    if (sizeOfNewString == 0)
    {
        return result;
    }

    result.Resize(sizeOfNewString);

    memcpy(result.m_Buffer, m_Buffer, m_NumChars);
    memcpy(&result.m_Buffer[m_NumChars], rhs, sizeOfRhsString);

    result.m_Buffer[result.m_NumChars] = '\0';

    return result;
}

template <typename CharType>
String<CharType> String<CharType>::operator+ (CharType c) const
{
    size_t sizeOfNewString = m_NumChars + 1;

    String<CharType> result;

    if (sizeOfNewString == 0)
    {
        return result;
    }

    result.Resize(sizeOfNewString);

    memcpy(result.m_Buffer, m_Buffer, m_NumChars);
    result.m_Buffer[m_NumChars] = c;

    result.m_Buffer[result.m_NumChars] = '\0';

    return result;
}

template <typename CharType>
String<CharType> operator+ (const CharType* lhs, const String<CharType>& rhs)
{
    size_t sizeOfLhsString = strlen(lhs);

    size_t sizeOfNewString = sizeOfLhsString + rhs.Size();

    String<CharType> result;

    if (sizeOfNewString == 0)
    {
        return result;
    }

    result.Resize(sizeOfNewString);

    memcpy(&result[0], lhs, sizeOfLhsString);
    memcpy(&result[sizeOfLhsString], &rhs[0], rhs.Size());

    return result;
}

template <typename CharType>
String<CharType> operator+ (CharType c, const String<CharType>& rhs)
{
    size_t sizeOfNewString = 1 + rhs.Size();

    String<CharType> result;

    if (sizeOfNewString == 0)
    {
        return result;
    }

    result.Resize(sizeOfNewString);

    result[0] = c;
    memcpy(&result[1], &rhs[0], rhs.Size());

    return result;
}

template <typename CharType>
void String<CharType>::Assign(const CharType* sz, size_t numChars)
{
    SHIP_FREE_EX(m_pAllocator, m_Buffer);

    m_NumChars = numChars;
    m_Capacity = m_NumChars + 1;

    size_t requiredSize = sizeof(CharType) * m_Capacity;
    m_Buffer = reinterpret_cast<CharType*>(SHIP_ALLOC_EX(m_pAllocator, requiredSize, 1));

    memcpy(m_Buffer, sz, m_NumChars);

    m_Buffer[m_NumChars] = '\0';
}

template <typename CharType>
void String<CharType>::Assign(const CharType* sz)
{
    Assign(sz, strlen(sz));
}

template <typename CharType>
void String<CharType>::Append(const CharType* sz, size_t numChars)
{
    size_t newSize = m_NumChars + numChars;

    if (newSize >= m_Capacity)
    {
        m_Capacity = newSize + 1;

        size_t requiredSize = sizeof(CharType) * m_Capacity;
        CharType* newBuffer = reinterpret_cast<CharType*>(SHIP_ALLOC_EX(m_pAllocator, requiredSize, 1));

        memcpy(newBuffer, m_Buffer, m_NumChars);

        SHIP_FREE_EX(m_pAllocator, m_Buffer);
        m_Buffer = newBuffer;
    }

    memcpy(&m_Buffer[m_NumChars], sz, numChars);

    m_NumChars = newSize;

    m_Buffer[m_NumChars] = '\0';
}

template <typename CharType>
void String<CharType>::Append(const CharType* sz)
{
    Append(sz, strlen(sz));
}

template <typename CharType>
void String<CharType>::Insert(size_t pos, const String<CharType>& str)
{
    if (str.m_NumChars == 0)
    {
        return;
    }

    size_t numCharsToMove = m_NumChars - pos;
    size_t endInsertIdx = pos + str.m_NumChars;

    size_t newSize = m_NumChars + str.m_NumChars;

    if (newSize < m_Capacity)
    {
        if (numCharsToMove > 0)
        {
            memcpy(&m_Buffer[endInsertIdx], &m_Buffer[pos], numCharsToMove);
        }

        memcpy(&m_Buffer[pos], str.m_Buffer, str.m_NumChars);
    }
    else
    {
        m_Capacity = newSize + 1;

        size_t requiredSize = sizeof(CharType) * m_Capacity;
        CharType* newBuffer = reinterpret_cast<CharType*>(SHIP_ALLOC_EX(m_pAllocator, requiredSize, 1));

        if (pos > 0)
        {
            memcpy(newBuffer, m_Buffer, pos);
        }

        memcpy(&newBuffer[pos], str.m_Buffer, str.m_NumChars);

        if (numCharsToMove > 0)
        {
            memcpy(&newBuffer[endInsertIdx], &m_Buffer[pos], numCharsToMove);
        }

        SHIP_FREE_EX(m_pAllocator, m_Buffer);
        m_Buffer = newBuffer;
    }

    m_NumChars = newSize;
    m_Buffer[m_NumChars] = '\0';
}

template <typename CharType>
void String<CharType>::InsertSubstring(size_t pos, const String<CharType>& str, size_t substringPos, size_t substringLength)
{
    substringLength = MIN(substringLength, str.m_NumChars);

    if (substringLength == 0)
    {
        return;
    }

    size_t numCharsInSubstring = str.m_NumChars - substringPos;
    if (numCharsInSubstring == 0)
    {
        return;
    }

    size_t numCharsToMove = m_NumChars - pos;
    size_t endInsertIdx = pos + substringLength;

    size_t newSize = m_NumChars + substringLength;

    if (newSize < m_Capacity)
    {
        if (numCharsToMove > 0)
        {
            memcpy(&m_Buffer[endInsertIdx], &m_Buffer[pos], numCharsToMove);
        }

        memcpy(&m_Buffer[pos], &str.m_Buffer[substringPos], substringLength);
    }
    else
    {
        m_Capacity = newSize + 1;

        size_t requiredSize = sizeof(CharType) * m_Capacity;
        CharType* newBuffer = reinterpret_cast<CharType*>(SHIP_ALLOC_EX(m_pAllocator, requiredSize, 1));

        if (pos > 0)
        {
            memcpy(newBuffer, m_Buffer, pos);
        }

        memcpy(&newBuffer[pos], &str.m_Buffer[substringPos], substringLength);

        if (numCharsToMove > 0)
        {
            memcpy(&newBuffer[endInsertIdx], &m_Buffer[pos], numCharsToMove);
        }

        SHIP_FREE_EX(m_pAllocator, m_Buffer);
        m_Buffer = newBuffer;
    }

    m_NumChars = newSize;
    m_Buffer[m_NumChars] = '\0';
}

template <typename CharType>
void String<CharType>::Insert(size_t pos, const CharType* str, size_t numChars)
{
    if (numChars == 0)
    {
        return;
    }

    size_t numCharsToMove = m_NumChars - pos;
    size_t endInsertIdx = pos + numChars;

    size_t newSize = m_NumChars + numChars;

    if (newSize < m_Capacity)
    {
        if (numCharsToMove > 0)
        {
            memcpy(&m_Buffer[endInsertIdx], &m_Buffer[pos], numCharsToMove);
        }

        memcpy(&m_Buffer[pos], str, numChars);
    }
    else
    {
        m_Capacity = newSize + 1;

        size_t requiredSize = sizeof(CharType) * m_Capacity;
        CharType* newBuffer = reinterpret_cast<CharType*>(SHIP_ALLOC_EX(m_pAllocator, requiredSize, 1));

        if (pos > 0)
        {
            memcpy(newBuffer, m_Buffer, pos);
        }

        memcpy(&newBuffer[pos], str, numChars);

        if (numCharsToMove > 0)
        {
            memcpy(&newBuffer[endInsertIdx], &m_Buffer[pos], numCharsToMove);
        }

        SHIP_FREE_EX(m_pAllocator, m_Buffer);
        m_Buffer = newBuffer;
    }

    m_NumChars = newSize;
    m_Buffer[m_NumChars] = '\0';
}

template <typename CharType>
void String<CharType>::Insert(size_t pos, const CharType* str)
{
    Insert(pos, str, strlen(str));
}

template <typename CharType>
void String<CharType>::Erase(size_t pos, size_t length)
{
    if (length == 0)
    {
        return;
    }

    size_t endIndex = pos + length;

    bool needToMoveChars = (endIndex < m_NumChars);
    if (needToMoveChars)
    {
        size_t numCharsToMove = m_NumChars - length - pos;

        memcpy(&m_Buffer[pos], &m_Buffer[endIndex], numCharsToMove);
    }

    if (length > m_NumChars)
    {
        m_NumChars = 0;
    }
    else
    {
        m_NumChars -= length;
    }

    m_Buffer[m_NumChars] = '\0';
}

template <typename CharType>
CharType& String<CharType>::At(size_t index)
{
    SHIP_ASSERT(index < m_NumChars);
    return m_Buffer[index];
}

template <typename CharType>
const CharType& String<CharType>::At(size_t index) const
{
    SHIP_ASSERT(index < m_NumChars);
    return m_Buffer[index];
}

template <typename CharType>
CharType* String<CharType>::GetWriteBuffer()
{
    return m_Buffer;
}

template <typename CharType>
const CharType* String<CharType>::GetBuffer() const
{
    return m_Buffer;
}

template <typename CharType>
void String<CharType>::Resize(size_t newSize)
{
    if (newSize == m_NumChars)
    {
        return;
    }
    else if (newSize > m_NumChars)
    {
        if (newSize >= m_Capacity)
        {
            m_Capacity = newSize + 1;

            size_t requiredSize = sizeof(CharType) * m_Capacity;
            CharType* newBuffer = reinterpret_cast<CharType*>(SHIP_ALLOC_EX(m_pAllocator, requiredSize, 1));

            memcpy(newBuffer, m_Buffer, m_NumChars);

            SHIP_FREE_EX(m_pAllocator, m_Buffer);
            m_Buffer = newBuffer;
        }
    }

    m_NumChars = newSize;
    m_Buffer[m_NumChars] = '\0';
}

template <typename CharType>
void String<CharType>::Reserve(size_t newCapacity)
{
    if (newCapacity == m_Capacity)
    {
        return;
    }

    m_Capacity = newCapacity;

    size_t requiredSize = sizeof(CharType) * m_Capacity;
    CharType* newBuffer = reinterpret_cast<CharType*>(SHIP_ALLOC_EX(m_pAllocator, requiredSize, 1));
    
    size_t numCharsToCopy = MIN(newCapacity - 1, m_NumChars);
    memcpy(newBuffer, m_Buffer, numCharsToCopy);

    SHIP_FREE_EX(m_pAllocator, m_Buffer);
    m_Buffer = newBuffer;

    m_NumChars = MIN(m_NumChars, newCapacity - 1);

    m_Buffer[m_NumChars] = '\0';
}

template <typename CharType>
void String<CharType>::Clear()
{
    SHIP_FREE_EX(m_pAllocator, m_Buffer);
    
    m_Buffer = nullptr;
    m_NumChars = 0;
    m_Capacity = 0;
}

template <typename CharType>
size_t String<CharType>::Size() const
{
    return m_NumChars;
}

template <typename CharType>
size_t String<CharType>::Capacity() const
{
    return m_Capacity;
}

template <typename CharType>
bool String<CharType>::IsEmpty() const
{
    return (m_NumChars == 0);
}

template <typename CharType>
size_t String<CharType>::FindIndexOfFirst(const String<CharType>& strToFind, size_t startingPos) const
{
    if (strToFind.m_NumChars == 0 || m_NumChars == 0)
    {
        return InvalidIndex;
    }

    // Naive implementation for now
    size_t numCharsToIgnoreInSearch = (strToFind.m_NumChars + startingPos) - 1;
    if (numCharsToIgnoreInSearch > m_NumChars)
    {
        return InvalidIndex;
    }

    size_t numCharsToSearch = (m_NumChars - numCharsToIgnoreInSearch);

    for (size_t i = startingPos; i < numCharsToSearch; i++)
    {
        bool foundString = true;

        for (size_t j = 0; j < strToFind.m_NumChars; j++)
        {
            if (m_Buffer[i + j] != strToFind.m_Buffer[j])
            {
                foundString = false;
                break;
            }
        }

        if (foundString)
        {
            return i;
        }
    }

    return InvalidIndex;
}

template <typename CharType>
size_t String<CharType>::FindIndexOfFirst(const CharType* strToFind, size_t numChars, size_t startingPos) const
{
    if (numChars == 0 || m_NumChars == 0)
    {
        return InvalidIndex;
    }

    // Naive implementation for now
    size_t numCharsToIgnoreInSearch = (numChars + startingPos) - 1;
    if (numCharsToIgnoreInSearch > m_NumChars)
    {
        return InvalidIndex;
    }

    size_t numCharsToSearch = (m_NumChars - numCharsToIgnoreInSearch);

    for (size_t i = startingPos; i < numCharsToSearch; i++)
    {
        bool foundString = true;

        for (size_t j = 0; j < numChars; j++)
        {
            if (m_Buffer[i + j] != strToFind[j])
            {
                foundString = false;
                break;
            }
        }

        if (foundString)
        {
            return i;
        }
    }

    return InvalidIndex;
}

template <typename CharType>
size_t String<CharType>::FindIndexOfFirst(const CharType* strToFind, size_t startingPos) const
{
    return FindIndexOfFirst(strToFind, strlen(strToFind), startingPos);
}

template <typename CharType>
size_t String<CharType>::FindIndexOfFirst(CharType charToFind, size_t startingPos) const
{
    for (size_t i = startingPos; i < m_NumChars; i++)
    {
        if (m_Buffer[i] == charToFind)
        {
            return i;
        }
    }

    return InvalidIndex;
}

template <typename CharType>
size_t String<CharType>::FindIndexOfFirstReverse(const String<CharType>& strToFind, size_t startingPos) const
{
    if (strToFind.m_NumChars == 0 || m_NumChars == 0)
    {
        return InvalidIndex;
    }

    // Naive implementation for now
    for (size_t i = startingPos + 1; i > 0; i--)
    {
        size_t idx = i - 1;

        bool foundString = true;

        for (size_t j = 0; j < strToFind.m_NumChars; j++)
        {
            if (m_Buffer[idx + j] != strToFind.m_Buffer[j])
            {
                foundString = false;
                break;
            }
        }

        if (foundString)
        {
            return idx;
        }
    }

    return InvalidIndex;
}

template <typename CharType>
size_t String<CharType>::FindIndexOfFirstReverse(const CharType* strToFind, size_t numChars, size_t startingPos) const
{
    if (numChars == 0 || m_NumChars == 0)
    {
        return InvalidIndex;
    }

    // Naive implementation for now
    for (size_t i = startingPos + 1; i > 0; i--)
    {
        size_t idx = i - 1;

        bool foundString = true;

        for (size_t j = 0; j < numChars; j++)
        {
            if (m_Buffer[idx + j] != strToFind[j])
            {
                foundString = false;
                break;
            }
        }

        if (foundString)
        {
            return idx;
        }
    }

    return InvalidIndex;
}

template <typename CharType>
size_t String<CharType>::FindIndexOfFirstReverse(const CharType* strToFind, size_t startingPos) const
{
    return FindIndexOfFirstReverse(strToFind, strlen(strToFind), startingPos);
}

template <typename CharType>
size_t String<CharType>::FindIndexOfFirstReverse(CharType charToFind, size_t startingPos) const
{
    for (size_t i = startingPos + 1; i > 0; i--)
    {
        size_t idx = i - 1;

        if (m_Buffer[idx] == charToFind)
        {
            return idx;
        }
    }

    return InvalidIndex;
}

template <typename CharType>
size_t String<CharType>::FindIndexOfFirstCaseInsensitive(const String<CharType>& strToFind, size_t startingPos) const
{
    if (strToFind.m_NumChars == 0 || m_NumChars == 0)
    {
        return InvalidIndex;
    }

    // Naive implementation for now
    size_t numCharsToIgnoreInSearch = (strToFind.m_NumChars + startingPos) - 1;
    if (numCharsToIgnoreInSearch > m_NumChars)
    {
        return InvalidIndex;
    }

    size_t numCharsToSearch = (m_NumChars - numCharsToIgnoreInSearch);

    for (size_t i = startingPos; i < numCharsToSearch; i++)
    {
        bool foundString = true;

        for (size_t j = 0; j < strToFind.m_NumChars; j++)
        {
            if (tolower(m_Buffer[i + j]) != tolower(strToFind.m_Buffer[j]))
            {
                foundString = false;
                break;
            }
        }

        if (foundString)
        {
            return i;
        }
    }

    return InvalidIndex;
}

template <typename CharType>
size_t String<CharType>::FindIndexOfFirstCaseInsensitive(const CharType* strToFind, size_t numChars, size_t startingPos) const
{
    if (numChars == 0 || m_NumChars == 0)
    {
        return InvalidIndex;
    }

    // Naive implementation for now
    size_t numCharsToIgnoreInSearch = (numChars + startingPos) - 1;
    if (numCharsToIgnoreInSearch > m_NumChars)
    {
        return InvalidIndex;
    }

    size_t numCharsToSearch = (m_NumChars - numCharsToIgnoreInSearch);

    for (size_t i = startingPos; i < numCharsToSearch; i++)
    {
        bool foundString = true;

        for (size_t j = 0; j < numChars; j++)
        {
            if (tolower(m_Buffer[i + j]) != tolower(strToFind[j]))
            {
                foundString = false;
                break;
            }
        }

        if (foundString)
        {
            return i;
        }
    }

    return InvalidIndex;
}

template <typename CharType>
size_t String<CharType>::FindIndexOfFirstCaseInsensitive(const CharType* strToFind, size_t startingPos) const
{
    return FindIndexOfFirstCaseInsensitive(strToFind, strlen(strToFind), startingPos);
}

template <typename CharType>
size_t String<CharType>::FindIndexOfFirstCaseInsensitive(CharType charToFind, size_t startingPos) const
{
    int loweredCharToFind = tolower(charToFind);

    for (size_t i = startingPos; i < m_NumChars; i++)
    {
        if (tolower(m_Buffer[i]) == loweredCharToFind)
        {
            return i;
        }
    }

    return InvalidIndex;
}

template <typename CharType>
size_t String<CharType>::FindIndexOfFirstCaseInsensitiveReverse(const String<CharType>& strToFind, size_t startingPos) const
{
    if (strToFind.m_NumChars == 0 || m_NumChars == 0)
    {
        return InvalidIndex;
    }

    // Naive implementation for now
    for (size_t i = startingPos + 1; i > 0; i--)
    {
        size_t idx = i - 1;

        bool foundString = true;

        for (size_t j = 0; j < strToFind.m_NumChars; j++)
        {
            if (tolower(m_Buffer[idx + j]) != tolower(strToFind.m_Buffer[j]))
            {
                foundString = false;
                break;
            }
        }

        if (foundString)
        {
            return idx;
        }
    }

    return InvalidIndex;
}

template <typename CharType>
size_t String<CharType>::FindIndexOfFirstCaseInsensitiveReverse(const CharType* strToFind, size_t numChars, size_t startingPos) const
{
    if (numChars == 0 || m_NumChars == 0)
    {
        return InvalidIndex;
    }

    // Naive implementation for now
    for (size_t i = startingPos + 1; i > 0; i--)
    {
        size_t idx = i - 1;

        bool foundString = true;

        for (size_t j = 0; j < numChars; j++)
        {
            if (tolower(m_Buffer[idx + j]) != tolower(strToFind[j]))
            {
                foundString = false;
                break;
            }
        }

        if (foundString)
        {
            return idx;
        }
    }

    return InvalidIndex;
}

template <typename CharType>
size_t String<CharType>::FindIndexOfFirstCaseInsensitiveReverse(CharType charToFind, size_t startingPos) const
{
    int loweredCharToFind = tolower(charToFind);

    for (size_t i = startingPos + 1; i > 0; i--)
    {
        size_t idx = i - 1;

        if (tolower(m_Buffer[idx]) == loweredCharToFind)
        {
            return idx;
        }
    }

    return InvalidIndex;
}

template <typename CharType>
size_t String<CharType>::FindIndexOfFirstCaseInsensitiveReverse(const CharType* strToFind, size_t startingPos) const
{
    return FindIndexOfFirstCaseInsensitiveReverse(strToFind, strlen(strToFind), startingPos);
}

template <typename CharType>
String<CharType> String<CharType>::Substring(size_t pos, size_t lengthOfSubstring) const
{
    String<CharType> substring;

    if (pos >= m_NumChars)
    {
        return substring;
    }

    size_t sizeOfSubstring = MIN((m_NumChars - pos), lengthOfSubstring);

    if (sizeOfSubstring == 0)
    {
        return substring;
    }

    substring.Resize(sizeOfSubstring);

    memcpy(substring.m_Buffer, &m_Buffer[pos], sizeOfSubstring);

    return substring;
}

template <typename CharType>
int String<CharType>::Compare(const String<CharType>& str) const
{
    for (size_t idx = 0; true; idx++)
    {
        CharType first = m_Buffer[idx];
        CharType second = str.m_Buffer[idx];

        int diff = (int(first) - int(second));

        if (diff < 0)
        {
            return -1;
        }
        else if (diff > 0)
        {
            return 1;
        }
        else if (first == '\0')
        {
            return 0;
        }
    }
}

template <typename CharType>
int String<CharType>::Compare(const CharType* str) const
{
    for (size_t idx = 0; true; idx++)
    {
        CharType first = m_Buffer[idx];
        CharType second = str[idx];

        int diff = (int(first) - int(second));

        if (diff < 0)
        {
            return -1;
        }
        else if (diff > 0)
        {
            return 1;
        }
        else if (first == '\0')
        {
            return 0;
        }
    }
}

template <typename CharType>
int String<CharType>::CompareCaseInsensitive(const String<CharType>& str) const
{
    for (size_t idx = 0; true; idx++)
    {
        int first = tolower(m_Buffer[idx]);
        int second = tolower(str.m_Buffer[idx]);

        int diff = (first - second);

        if (diff < 0)
        {
            return -1;
        }
        else if (diff > 0)
        {
            return 1;
        }
        else if (first == '\0')
        {
            return 0;
        }
    }
}

template <typename CharType>
int String<CharType>::CompareCaseInsensitive(const CharType* str) const
{
    for (size_t idx = 0; true; idx++)
    {
        CharType first = tolower(m_Buffer[idx]);
        CharType second = tolower(str[idx]);

        int diff = (int(first) - int(second));

        if (diff < 0)
        {
            return -1;
        }
        else if (diff > 0)
        {
            return 1;
        }
        else if (first == '\0')
        {
            return 0;
        }
    }
}

template <typename CharType>
bool String<CharType>::EqualCaseInsensitive(const String<CharType>& str) const
{
    if (m_NumChars != str.m_NumChars)
    {
        return false;
    }

    for (size_t idx = 0; idx < m_NumChars; idx++)
    {
        int first = tolower(m_Buffer[idx]);
        int second = tolower(str.m_Buffer[idx]);

        int diff = (first - second);

        if (diff != 0)
        {
            return false;
        }
    }

    return true;
}

template <typename CharType>
bool String<CharType>::EqualCaseInsensitive(const CharType* str, size_t numChars) const
{
    if (m_NumChars != numChars)
    {
        return false;
    }

    for (size_t idx = 0; idx < m_NumChars; idx++)
    {
        int first = tolower(m_Buffer[idx]);
        int second = tolower(str[idx]);

        int diff = (first - second);

        if (diff != 0)
        {
            return false;
        }
    }

    return true;
}

template <typename CharType>
bool String<CharType>::EqualCaseInsensitive(const CharType* str) const
{
    return EqualCaseInsensitive(str, strlen(str));
}

template <typename CharType>
void String<CharType>::Format(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    int requiredLength = vsnprintf(nullptr, 0, format, args);
    va_end(args);

    if (requiredLength < 0)
    {
        return;
    }

    Resize(requiredLength);

    va_start(args, format);
    vsnprintf(m_Buffer, requiredLength + 1, format, args);
    va_end(args);
}

template <typename CharType>
bool String<CharType>::operator== (const String<CharType>& rhs) const
{
    if (m_NumChars != rhs.m_NumChars)
    {
        return false;
    }
    
    for (size_t i = 0; i < m_NumChars; i++)
    {
        if (m_Buffer[i] != rhs.m_Buffer[i])
        {
            return false;
        }
    }

    return true;
}

template <typename CharType>
bool String<CharType>::operator== (const CharType* rhs) const
{
    size_t numChars = strlen(rhs);

    if (m_NumChars != numChars)
    {
        return false;
    }

    for (size_t i = 0; i < m_NumChars; i++)
    {
        if (m_Buffer[i] != rhs[i])
        {
            return false;
        }
    }

    return true;
}

template <typename CharType>
bool String<CharType>::operator!= (const String<CharType>& rhs) const
{
    if (m_NumChars != rhs.m_NumChars)
    {
        return true;
    }

    for (size_t i = 0; i < m_NumChars; i++)
    {
        if (m_Buffer[i] != rhs.m_Buffer[i])
        {
            return true;
        }
    }

    return false;
}

template <typename CharType>
bool String<CharType>::operator!= (const CharType* rhs) const
{
    size_t numChars = strlen(rhs);

    if (m_NumChars != numChars)
    {
        return true;
    }

    for (size_t i = 0; i < m_NumChars; i++)
    {
        if (m_Buffer[i] != rhs[i])
        {
            return true;
        }
    }

    return false;
}

template <typename CharType>
void String<CharType>::SetAllocator(BaseAllocator* pAllocator)
{
    if (pAllocator == m_pAllocator)
    {
        return;
    }

    size_t requiredSize = sizeof(CharType) * m_Capacity;
    CharType* pNewArray = reinterpret_cast<CharType*>(SHIP_ALLOC_EX(pAllocator, requiredSize, 1));

    memcpy(pNewArray, m_Buffer, m_NumChars);

    SHIP_FREE_EX(m_pAllocator, m_Buffer);

    m_Buffer = pNewArray;
    m_Buffer[m_NumChars] = '\0';

    m_pAllocator = pAllocator;
}

}