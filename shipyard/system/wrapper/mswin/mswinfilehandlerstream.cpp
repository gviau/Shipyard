#include <system/wrapper/mswin/mswinfilehandlerstream.h>

#include <system/pathutils.h>
#include <system/systemcommon.h>

namespace Shipyard
{;

MswinFileHandlerStream::MswinFileHandlerStream()
{

}

MswinFileHandlerStream::MswinFileHandlerStream(const StringT& filename, FileHandlerOpenFlag openFlag)
{
    Open(filename, openFlag);
}

MswinFileHandlerStream::MswinFileHandlerStream(const shipChar* filename, FileHandlerOpenFlag openFlag)
{
    Open(filename, openFlag);
}

MswinFileHandlerStream::~MswinFileHandlerStream()
{
    Close();
}

shipBool MswinFileHandlerStream::Open(const StringT& filename, FileHandlerOpenFlag openFlag)
{
    return Open(filename.GetBuffer(), openFlag);
}

shipBool MswinFileHandlerStream::Open(const shipChar* filename, FileHandlerOpenFlag openFlag)
{
    m_Filename = filename;
    m_OpenFlag = openFlag;

    if ((openFlag & FileHandlerOpenFlag_Create) > 0)
    {
        StringT fileDirectory;
        PathUtils::GetFileDirectory(filename, &fileDirectory);

        PathUtils::CreateDirectories(fileDirectory.GetBuffer());
    }

    int fileMode = GetFileMode(m_OpenFlag);

    m_OpenFlag = FileHandlerOpenFlag(m_OpenFlag & ~FileHandlerOpenFlag_Create);

    m_File.open(m_Filename.GetBuffer(), fileMode);

    return m_File.is_open();
}

shipBool MswinFileHandlerStream::IsOpen() const
{
    return m_File.is_open();
}

void MswinFileHandlerStream::Close()
{
    FlushInternalBuffer();

    m_File.close();
}

size_t MswinFileHandlerStream::ReadChars(size_t startingPosition, shipChar* content, size_t numChars)
{
    if (numChars == 0)
    {
        return 0;
    }

    FlushInternalBuffer();

    if (startingPosition > 0)
    {
        m_File.seekg(startingPosition, std::ios::beg);
    }

    m_File.read(content, numChars);

    m_File.clear();
    m_File.seekg(0, std::ios::beg);

    return numChars;
}

size_t MswinFileHandlerStream::ReadChars(size_t startingPosition, StringA& content, size_t numChars)
{
    if (numChars == 0)
    {
        return 0;
    }

    content.Resize(numChars);

    return ReadChars(startingPosition, &content[0], numChars);
}

size_t MswinFileHandlerStream::ReadWholeFile(StringA& content)
{
    size_t fileSize = Size();

    constexpr size_t startingPosition = 0;
    return ReadChars(startingPosition, content, fileSize);
}

void MswinFileHandlerStream::WriteChars(size_t startingPosition, const shipChar* chars, size_t numChars, shipBool flush)
{
    if (numChars == 0)
    {
        return;
    }

    FlushInternalBuffer();

    if (startingPosition > 0)
    {
        m_File.seekg(startingPosition, std::ios::beg);
    }

    m_File.write(chars, numChars);

    m_File.clear();
    m_File.seekg(0, std::ios::beg);

    if (flush)
    {
        m_File.flush();
    }
}

void MswinFileHandlerStream::InsertChars(size_t startingPosition, const shipChar* chars, size_t numChars, shipBool flush)
{
    if (numChars == 0)
    {
        return;
    }

    FlushInternalBuffer();

    size_t sizeOfFilePartToMove = (Size() - startingPosition);

    StringA filePartToMove;
    filePartToMove.Resize(sizeOfFilePartToMove);

    if (startingPosition > 0)
    {
        m_File.seekg(startingPosition, std::ios::beg);
    }

    m_File.read(&filePartToMove[0], sizeOfFilePartToMove);

    m_File.seekg(startingPosition, std::ios::beg);

    m_File.write(chars, numChars);
    m_File.write(&filePartToMove[0], sizeOfFilePartToMove);

    m_File.clear();
    m_File.seekg(0, std::ios::beg);

    if (flush)
    {
        m_File.flush();
    }
}

void MswinFileHandlerStream::AppendChars(const shipChar* chars, size_t numChars, shipBool flush)
{
    if (numChars == 0)
    {
        return;
    }

    shipBool enoughSpaceInBuffer = (numChars + m_FileHandlerStreamBufferPos < ms_FileHandlerStreamBufferSize);
    if (enoughSpaceInBuffer)
    {
        memcpy(&m_FileHandlerStreamBuffer[m_FileHandlerStreamBufferPos], chars, numChars);
        m_FileHandlerStreamBufferPos += numChars;
    }
    else
    {
        FlushInternalBuffer();

        m_File.seekg(Size(), std::ios::beg);

        size_t numIOWrites = numChars / ms_FileHandlerStreamBufferSize;
        size_t sizeToWrite = numIOWrites * ms_FileHandlerStreamBufferSize;
        size_t leftoverChars = numChars - sizeToWrite;

        const shipChar* ptr = chars;

        m_File.write(ptr, sizeToWrite);
        m_File.flush();

        ptr += sizeToWrite;

        if (leftoverChars > 0)
        {
            memcpy(&m_FileHandlerStreamBuffer[0], ptr, leftoverChars);
            m_FileHandlerStreamBufferPos += leftoverChars;
        }
    }

    if (flush)
    {
        FlushInternalBuffer();
    }

    m_File.clear();
    m_File.seekg(0, std::ios::beg);
}

void MswinFileHandlerStream::RemoveChars(size_t startingPosition, size_t numCharsToRemove)
{
    SHIP_ASSERT(numCharsToRemove > 0);

    FlushInternalBuffer();

    size_t fileSize = Size();
    size_t startingPositionAfterPartToRemove = (startingPosition + numCharsToRemove);
    size_t sizeAfterPartToRemove = (fileSize - startingPositionAfterPartToRemove);

    StringA before;
    StringA after;

    if (startingPosition > 0)
    {
        before.Resize(startingPosition);
        m_File.read(&before[0], startingPosition);
    }

    if (sizeAfterPartToRemove > 0)
    {
        after.Resize(sizeAfterPartToRemove);
        m_File.seekg(startingPositionAfterPartToRemove, std::ios::beg);
        m_File.read(&after[0], sizeAfterPartToRemove);
    }

    m_File.close();

    FileHandlerOpenFlag openFlag = FileHandlerOpenFlag(FileHandlerOpenFlag_Write | FileHandlerOpenFlag_Create);

    if ((m_OpenFlag & FileHandlerOpenFlag::FileHandlerOpenFlag_Binary) > 0)
    {
        openFlag = FileHandlerOpenFlag(openFlag | FileHandlerOpenFlag_Binary);
    }

    int fileMode = GetFileMode(openFlag);

    m_File.open(m_Filename.GetBuffer(), fileMode);

    SHIP_ASSERT(m_File.is_open());
    
    if (before.Size() > 0)
    {
        m_File.write(&before[0], before.Size());
    }

    if (after.Size() > 0)
    {
        m_File.write(&after[0], after.Size());
    }

    m_File.close();

    m_File.open(m_Filename.GetBuffer(), GetFileMode(m_OpenFlag));

    SHIP_ASSERT(m_File.is_open());
}

void MswinFileHandlerStream::Flush()
{
    FlushInternalBuffer();
}

size_t MswinFileHandlerStream::Size()
{
    m_File.ignore((std::numeric_limits<std::streamsize>::max)());

    size_t fileSize = size_t(m_File.gcount());

    m_File.clear();
    m_File.seekg(0, std::ios::beg);

    return fileSize + m_FileHandlerStreamBufferPos;
}

int MswinFileHandlerStream::GetFileMode(FileHandlerOpenFlag openFlag) const
{
    int fileMode = 0;

    if ((openFlag & FileHandlerOpenFlag::FileHandlerOpenFlag_Read) > 0)
    {
        fileMode |= std::ios_base::in;
    }

    if ((openFlag & FileHandlerOpenFlag::FileHandlerOpenFlag_Write) > 0)
    {
        fileMode |= std::ios_base::out;
    }

    if ((openFlag & FileHandlerOpenFlag::FileHandlerOpenFlag_Create) > 0)
    {
        fileMode |= std::ios_base::trunc;
    }

    if ((openFlag & FileHandlerOpenFlag::FileHandlerOpenFlag_Binary) > 0)
    {
        fileMode |= std::ios_base::binary;
    }

    return fileMode;
}

void MswinFileHandlerStream::FlushInternalBuffer()
{
    if (!m_File.is_open())
    {
        return;
    }

    m_File.flush();

    if (m_FileHandlerStreamBufferPos == 0)
    {
        return;
    }

    // Kind of idiot: I'd like Size() not to flush, so we have to remove the current position we're at to not
    // write at a position we're not at. Otherwise, I'd have to remove the m_FileHandlerStreamBufferPos inside of
    // Size and flush.

    m_File.seekg(Size() - m_FileHandlerStreamBufferPos, std::ios::beg);

    m_File.write(&m_FileHandlerStreamBuffer[0], m_FileHandlerStreamBufferPos);
    m_File.flush();

    m_File.clear();
    m_File.seekg(0, std::ios::beg);

    m_FileHandlerStreamBufferPos = 0;
}

}