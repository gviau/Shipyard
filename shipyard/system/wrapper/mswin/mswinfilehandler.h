#pragma once

#include <system/wrapper/filehandler.h>

#include <fstream>

namespace Shipyard
{
    class SHIPYARD_API MswinFileHandler : public BaseFileHandler
    {
    public:
        MswinFileHandler();
        MswinFileHandler(const StringT& filename, FileHandlerOpenFlag openFlag);
        MswinFileHandler(const char* filename, FileHandlerOpenFlag openFlag);

        bool Open(const StringT& filename, FileHandlerOpenFlag openFlag);
        bool Open(const char* filename, FileHandlerOpenFlag openFlag);
        bool IsOpen() const;
        void Close();

        // Returns the number of characters read
        size_t ReadChars(size_t startingPosition, char* content, size_t numChars);
        size_t ReadChars(size_t startingPosition, StringA& content, size_t numChars);
        size_t ReadWholeFile(StringA& content);

        void WriteChars(size_t startingPosition, const char* chars, size_t numChars, bool flush = false);
        void InsertChars(size_t startingPosition, const char* chars, size_t numChars, bool flush = false);
        void AppendChars(const char* chars, size_t numChars, bool flush = false);
        void RemoveChars(size_t startingPosition, size_t numCharsToRemove);

        size_t Size();

    private:
        int GetFileMode(FileHandlerOpenFlag openFlag) const;

        std::fstream m_File;
        SmallInplaceStringT m_Filename;
        FileHandlerOpenFlag m_OpenFlag;
    };
}