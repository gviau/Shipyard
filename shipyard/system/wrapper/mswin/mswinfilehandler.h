#pragma once

#include <system/wrapper/filehandler.h>

#include <fstream>

namespace Shipyard
{
    class MswinFileHandler : public BaseFileHandler
    {
    public:
        MswinFileHandler();
        MswinFileHandler(const String& filename, FileHandlerOpenFlag openFlag);

        bool Open(const String& filename, FileHandlerOpenFlag openFlag);
        bool IsOpen() const;
        void Close();

        // Returns the number of characters read
        size_t ReadChars(size_t startingPosition, char* content, size_t numChars);
        size_t ReadChars(size_t startingPosition, String& content, size_t numChars);
        size_t ReadWholeFile(String& content);

        void WriteChars(size_t startingPosition, const char* chars, size_t numChars, bool flush = false);
        void InsertChars(size_t startingPosition, const char* chars, size_t numChars, bool flush = false);
        void AppendChars(const char* chars, size_t numChars, bool flush = false);
        void RemoveChars(size_t startingPosition, size_t numCharsToRemove);

        size_t Size();

    private:
        int GetFileMode(FileHandlerOpenFlag openFlag) const;

        std::fstream m_File;
        String m_Filename;
        FileHandlerOpenFlag m_OpenFlag;
    };
}