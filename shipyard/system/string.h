#pragma once

#include <stdint.h>

namespace Shipyard
{
    // Null-terminated string implementation
    template <typename CharType>
    class String
    {
    public:
        static const size_t InvalidIndex = size_t(-1);

    public:
        String();
        String(const CharType* sz);

        // It is the caller's responsability to make sure that numChars <= strlen(sz)
        String(const CharType* sz, size_t numChars);

        String(const String& src);
        ~String();

        String& operator= (const String& rhs);
        String& operator= (const CharType* rhs);
        String& operator= (CharType c);

        CharType& operator[] (size_t index);
        const CharType& operator[] (size_t index) const;

        void operator+= (const String& rhs);
        void operator+= (const CharType* rhs);
        void operator+= (CharType c);

        String operator+ (const String& rhs) const;
        String operator+ (const CharType* rhs) const;
        String operator+ (CharType c) const;

        template <typename CharType>
        friend String operator+ (const CharType* lhs, const String& rhs);

        template <typename CharType>
        friend String operator+ (CharType c, const String& rhs);

        void Assign(const CharType* sz, size_t numChars);
        void Assign(const CharType* sz);
        void Append(const CharType* sz, size_t numChars);
        void Append(const CharType* sz);

        void Insert(size_t pos, const String& str);
        void InsertSubstring(size_t pos, const String& str, size_t substringPos, size_t substringLength);
        void Insert(size_t pos, const CharType* str, size_t numChars);
        void Insert(size_t pos, const CharType* str);

        void Erase(size_t pos, size_t length);

        CharType& At(size_t index);
        const CharType& At(size_t index) const;

        CharType* GetWriteBuffer();
        const CharType* GetBuffer() const;

        // This method also reserves the extra null character.
        void Resize(size_t newSize);

        void Reserve(size_t newCapacity);

        void Clear();

        size_t Size() const;
        size_t Capacity() const;
        bool IsEmpty() const;

        size_t FindIndexOfFirst(const String& strToFind, size_t startingPos) const;
        size_t FindIndexOfFirst(const CharType* strToFind, size_t numChars, size_t startingPos) const;
        size_t FindIndexOfFirst(const CharType* strToFind, size_t startingPos) const;
        size_t FindIndexOfFirst(CharType charToFind, size_t startingPos) const;

        size_t FindIndexOfFirstReverse(const String& strToFind, size_t startingPos) const;
        size_t FindIndexOfFirstReverse(const CharType* strToFind, size_t numChars, size_t startingPos) const;
        size_t FindIndexOfFirstReverse(const CharType* strToFind, size_t startingPos) const;
        size_t FindIndexOfFirstReverse(CharType charToFind, size_t startingPos) const;

        size_t FindIndexOfFirstCaseInsensitive(const String& strToFind, size_t startingPos) const;
        size_t FindIndexOfFirstCaseInsensitive(const CharType* strToFind, size_t numChars, size_t startingPos) const;
        size_t FindIndexOfFirstCaseInsensitive(const CharType* strToFind, size_t startingPos) const;
        size_t FindIndexOfFirstCaseInsensitive(CharType charToFind, size_t startingPos) const;

        size_t FindIndexOfFirstCaseInsensitiveReverse(const String& strToFind, size_t startingPos) const;
        size_t FindIndexOfFirstCaseInsensitiveReverse(const CharType* strToFind, size_t numChars, size_t startingPos) const;
        size_t FindIndexOfFirstCaseInsensitiveReverse(const CharType* strToFind, size_t startingPos) const;
        size_t FindIndexOfFirstCaseInsensitiveReverse(CharType charToFind, size_t startingPos) const;

        String Substring(size_t pos, size_t lengthOfSubstring) const;

        int Compare(const String& str) const;
        int Compare(const CharType* str) const;

        int CompareCaseInsensitive(const String& str) const;
        int CompareCaseInsensitive(const CharType* str) const;

        bool EqualCaseInsensitive(const String& str) const;
        bool EqualCaseInsensitive(const CharType* str, size_t numChars) const;
        bool EqualCaseInsensitive(const CharType* str) const;

        void Format(const char* format, ...);

        bool operator== (const String& rhs) const;
        bool operator== (const CharType* rhs) const;
        bool operator!= (const String& rhs) const;
        bool operator!= (const CharType* rhs) const;

    protected:
        CharType* m_Buffer;
        size_t m_NumChars;
        size_t m_Capacity;
    };

    using StringA = String<char>;
    using StringT = StringA;
}

#include <system/string.inl>