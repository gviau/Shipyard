#pragma once

#include <system/wrapper/filehandler.h>

#include <fstream>

namespace Shipyard
{
    class SHIPYARD_API MswinFileHandlerStream : public BaseFileHandler
    {
    public:
        MswinFileHandlerStream();
        MswinFileHandlerStream(const StringT& filename, FileHandlerOpenFlag openFlag);
        MswinFileHandlerStream(const shipChar* filename, FileHandlerOpenFlag openFlag);
        ~MswinFileHandlerStream();

        shipBool Open(const StringT& filename, FileHandlerOpenFlag openFlag);
        shipBool Open(const shipChar* filename, FileHandlerOpenFlag openFlag);
        shipBool IsOpen() const;
        void Close();

        // Returns the number of characters read
        size_t ReadChars(size_t startingPosition, shipChar* content, size_t numChars);
        size_t ReadChars(size_t startingPosition, StringA& content, size_t numChars);
        size_t ReadWholeFile(StringA& content);

        void WriteChars(size_t startingPosition, const shipChar* chars, size_t numChars, shipBool flush = false);
        void InsertChars(size_t startingPosition, const shipChar* chars, size_t numChars, shipBool flush = false);
        void AppendChars(const shipChar* chars, size_t numChars, shipBool flush = false);
        void RemoveChars(size_t startingPosition, size_t numCharsToRemove);

        void Flush();

        size_t Size();

    private:
        int GetFileMode(FileHandlerOpenFlag openFlag) const;
        void FlushInternalBuffer();

        std::fstream m_File;
        SmallInplaceStringT m_Filename;
        FileHandlerOpenFlag m_OpenFlag;

        static const size_t ms_FileHandlerStreamBufferSize = 32768;
        shipChar m_FileHandlerStreamBuffer[ms_FileHandlerStreamBufferSize];
        size_t m_FileHandlerStreamBufferPos = 0;
    };
}