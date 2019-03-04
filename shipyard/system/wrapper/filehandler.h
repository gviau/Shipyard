#pragma once

#include <system/systemcommon.h>

#include <system/string.h>

namespace Shipyard
{
    enum FileHandlerOpenFlag : uint8_t
    {
        FileHandlerOpenFlag_Read = 0x01,
        FileHandlerOpenFlag_Write = 0x02,
        FileHandlerOpenFlag_Create = 0x04,
        FileHandlerOpenFlag_Binary = 0x08,

        FileHandlerOpenFlag_ReadWrite = FileHandlerOpenFlag_Read | FileHandlerOpenFlag_Write
    };

    class BaseFileHandler
    {
    public:
        BaseFileHandler();
        BaseFileHandler(const StringT& filename, FileHandlerOpenFlag openFlag);
        BaseFileHandler(const char* filename, FileHandlerOpenFlag openFlag);

#ifdef DEBUG_WRAPPER_INTERFACE_COMPILATION
        virtual bool Open(const StringT& filename, FileHandlerOpenFlag openFlag) = 0;
        virtual bool Open(const char* filename, FileHandlerOpenFlag openFlag) = 0;
        virtual bool IsOpen() const = 0;
        virtual void Close() = 0;

        // Returns the number of characters read
        virtual size_t ReadChars(size_t startingPosition, char* content, size_t numChars) = 0;
        virtual size_t ReadChars(size_t startingPosition, StringA& content, size_t numChars) = 0;
        virtual size_t ReadWholeFile(StringA& content) = 0;

        virtual void WriteChars(size_t startingPosition, const char* chars, size_t numChars, bool flush = false) = 0;
        virtual void InsertChars(size_t startingPosition, const char* chars, size_t numChars, bool flush = false) = 0;
        virtual void AppendChars(const char* chars, size_t numChars, bool flush = false) = 0;
        virtual void RemoveChars(size_t startingPosition, size_t numCharsToRemove) = 0;

        virtual size_t Size() = 0;
#endif // #ifdef DEBUG_WRAPPER_INTERFACE_COMPILATION
    };
}