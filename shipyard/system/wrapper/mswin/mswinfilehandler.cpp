#include <system/wrapper/mswin/mswinfilehandler.h>

#include <cassert>

namespace Shipyard
{;

MswinFileHandler::MswinFileHandler()
{

}

MswinFileHandler::MswinFileHandler(const StringT& filename, FileHandlerOpenFlag openFlag)
{
    Open(filename, openFlag);
}

MswinFileHandler::MswinFileHandler(const char* filename, FileHandlerOpenFlag openFlag)
{
    Open(filename, openFlag);
}

bool MswinFileHandler::Open(const StringT& filename, FileHandlerOpenFlag openFlag)
{
    return Open(filename.GetBuffer(), openFlag);
}

bool MswinFileHandler::Open(const char* filename, FileHandlerOpenFlag openFlag)
{
    m_Filename = filename;
    m_OpenFlag = openFlag;

    int fileMode = GetFileMode(m_OpenFlag);

    m_OpenFlag = FileHandlerOpenFlag(m_OpenFlag & ~FileHandlerOpenFlag_Create);

    m_File.open(m_Filename.GetBuffer(), fileMode);

    return m_File.is_open();
}

bool MswinFileHandler::IsOpen() const
{
    return m_File.is_open();
}

void MswinFileHandler::Close()
{
    m_File.close();
}

size_t MswinFileHandler::ReadChars(size_t startingPosition, char* content, size_t numChars)
{
    if (startingPosition > 0)
    {
        m_File.seekg(startingPosition, std::ios::beg);
    }

    m_File.read(content, numChars);

    m_File.clear();
    m_File.seekg(0, std::ios::beg);

    return numChars;
}

size_t MswinFileHandler::ReadChars(size_t startingPosition, StringA& content, size_t numChars)
{
    if (numChars == 0)
    {
        return 0;
    }

    content.Resize(numChars);

    return ReadChars(startingPosition, &content[0], numChars);
}

size_t MswinFileHandler::ReadWholeFile(StringA& content)
{
    size_t fileSize = Size();

    constexpr size_t startingPosition = 0;
    return ReadChars(startingPosition, content, fileSize);
}

void MswinFileHandler::WriteChars(size_t startingPosition, const char* chars, size_t numChars, bool flush)
{
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

void MswinFileHandler::InsertChars(size_t startingPosition, const char* chars, size_t numChars, bool flush)
{
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

void MswinFileHandler::AppendChars(const char* chars, size_t numChars, bool flush)
{
    m_File.seekg(Size(), std::ios::beg);

    m_File.write(chars, numChars);

    m_File.clear();
    m_File.seekg(0, std::ios::beg);

    if (flush)
    {
        m_File.flush();
    }
}

void MswinFileHandler::RemoveChars(size_t startingPosition, size_t numCharsToRemove)
{
    assert(numCharsToRemove > 0);

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

    assert(m_File.is_open());
    
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

    assert(m_File.is_open());
}

size_t MswinFileHandler::Size()
{
    m_File.ignore((std::numeric_limits<std::streamsize>::max)());

    size_t fileSize = size_t(m_File.gcount());

    m_File.clear();
    m_File.seekg(0, std::ios::beg);

    return fileSize;
}

int MswinFileHandler::GetFileMode(FileHandlerOpenFlag openFlag) const
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

}