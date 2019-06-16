#include <graphics/shader/shaderdatabase.h>

#include <system/memory.h>

namespace Shipyard
{;

ShaderDatabase::ShaderDatabase()
    : m_ShaderEntrySets(uint32_t(0))
    , m_ShaderEntryKeys(uint32_t(0))
{

}

ShaderDatabase::~ShaderDatabase()
{
    Close();
}

bool ShaderDatabase::Load(const StringT& filename)
{
    m_Filename = filename;

    if (!m_FileHandler.Open(m_Filename, FileHandlerOpenFlag(FileHandlerOpenFlag_ReadWrite | FileHandlerOpenFlag_Binary)))
    {
        Invalidate();
        return false;
    }

    StringA databaseContent;
    m_FileHandler.ReadWholeFile(databaseContent);

    if (databaseContent.Size() < (sizeof(DatabaseHeader) + sizeof(ShaderEntriesHeader)))
    {
        return false;
    }

    uint8_t* databaseBuffer = (uint8_t*)&databaseContent[0];

    DatabaseHeader databaseHeader = *(DatabaseHeader*)databaseBuffer;
    if (databaseHeader.lowMagic != LowMagicConstant || databaseHeader.highMagic != HighMagicConstant)
    {
        return false;
    }

    if (databaseHeader.platform != PLATFORM)
    {
        return false;
    }

    if (databaseHeader.databaseVersionNumber != Version)
    {
        Invalidate();
        return false;
    }

    databaseBuffer += sizeof(databaseHeader);

    ShaderEntriesHeader shaderEntriesHeader = *(ShaderEntriesHeader*)databaseBuffer;
    
    databaseBuffer += sizeof(shaderEntriesHeader);

    if (shaderEntriesHeader.numShaderEntries > 0)
    {
        m_ShaderEntryKeys.Reserve(shaderEntriesHeader.numShaderEntries);
        m_ShaderEntrySets.Reserve(shaderEntriesHeader.numShaderEntries);
    }

    for (uint32_t i = 0; i < shaderEntriesHeader.numShaderEntries; i++)
    {
        LoadNextShaderEntry(databaseBuffer, m_ShaderEntryKeys, m_ShaderEntrySets);
    }

    return true;
}

void ShaderDatabase::Close()
{
    m_Filename.Clear();

    m_FileHandler.Close();

    m_ShaderEntryKeys.Clear();

    for (ShaderEntrySet& shaderEntrySet : m_ShaderEntrySets)
    {
        SHIP_DELETE(shaderEntrySet.rawVertexShader);
        SHIP_DELETE(shaderEntrySet.rawPixelShader);
        SHIP_DELETE(shaderEntrySet.rawHullShader);
        SHIP_DELETE(shaderEntrySet.rawDomainShader);
        SHIP_DELETE(shaderEntrySet.rawGeometryShader);
        SHIP_DELETE(shaderEntrySet.rawComputeShader);
    }
    m_ShaderEntrySets.Clear();
}

bool ShaderDatabase::Invalidate()
{
    m_FileHandler.Close();

    if (!m_FileHandler.Open(m_Filename, FileHandlerOpenFlag(FileHandlerOpenFlag_ReadWrite | FileHandlerOpenFlag_Binary | FileHandlerOpenFlag_Create)))
    {
        return false;
    }

    DatabaseHeader databaseHeader;
    databaseHeader.lowMagic = LowMagicConstant;
    databaseHeader.highMagic = HighMagicConstant;
    databaseHeader.platform = PLATFORM;
    databaseHeader.databaseVersionNumber = Version;

    ShaderEntriesHeader shaderEntriesHeader;
    shaderEntriesHeader.numShaderEntries = 0;

    m_FileHandler.AppendChars((const char*)&databaseHeader, sizeof(databaseHeader));

    constexpr bool flush = true;
    m_FileHandler.AppendChars((const char*)&shaderEntriesHeader, sizeof(shaderEntriesHeader), flush);

    return true;
}

bool ShaderDatabase::RetrieveShadersForShaderKey(const ShaderKey& shaderKey, uint64_t lastModifiedTimestamp, ShaderEntrySet& shaderEntrySet) const
{
    uint32_t shaderSetIndex = 0;
    uint32_t numShaderEntries = m_ShaderEntryKeys.Size();

    for (; shaderSetIndex < numShaderEntries; shaderSetIndex++)
    {
        const ShaderEntryKey& shaderEntryKey = m_ShaderEntryKeys[shaderSetIndex];
        if (shaderEntryKey.shaderKey == shaderKey)
        {
            break;
        }
    }

    bool foundShaderKey = (shaderSetIndex < numShaderEntries);
    if (!foundShaderKey)
    {
        return false;
    }

    shaderEntrySet = m_ShaderEntrySets[shaderSetIndex];
    SHIP_ASSERT((shaderEntrySet.rawVertexShaderSize + shaderEntrySet.rawPixelShaderSize + shaderEntrySet.rawHullShaderSize +
            shaderEntrySet.rawDomainShaderSize + shaderEntrySet.rawGeometryShaderSize + shaderEntrySet.rawComputeShaderSize) > 0);

    return true;
}

void ShaderDatabase::RemoveShadersForShaderKey(const ShaderKey& shaderKey)
{
    uint32_t shaderSetIndexToRemove = 0;
    uint32_t numShaderEntries = m_ShaderEntryKeys.Size();

    for (; shaderSetIndexToRemove < numShaderEntries; shaderSetIndexToRemove++)
    {
        const ShaderEntryKey& shaderEntryKey = m_ShaderEntryKeys[shaderSetIndexToRemove];
        if (shaderEntryKey.shaderKey == shaderKey)
        {
            break;
        }
    }

    bool foundShaderKey = (shaderSetIndexToRemove < numShaderEntries);
    if (!foundShaderKey)
    {
        return;
    }

    size_t positionToRemoveInFile = sizeof(DatabaseHeader) + sizeof(ShaderEntriesHeader);
    for (uint32_t i = 0; i < shaderSetIndexToRemove; i++)
    {
        const ShaderEntrySet& shaderEntrySet = m_ShaderEntrySets[i];

        size_t shaderEntrySetSize =
                (shaderEntrySet.rawVertexShaderSize + shaderEntrySet.rawPixelShaderSize + shaderEntrySet.rawHullShaderSize +
                 shaderEntrySet.rawDomainShaderSize + shaderEntrySet.rawGeometryShaderSize + shaderEntrySet.rawComputeShaderSize +
                 sizeof(shaderEntrySet.renderStateBlock));

        positionToRemoveInFile += shaderEntrySetSize + sizeof(ShaderEntryHeader);
    }

    const ShaderEntrySet& shaderEntrySet = m_ShaderEntrySets[shaderSetIndexToRemove];

    size_t numCharsToRemove =
            (shaderEntrySet.rawVertexShaderSize + shaderEntrySet.rawPixelShaderSize + shaderEntrySet.rawHullShaderSize +
             shaderEntrySet.rawDomainShaderSize + shaderEntrySet.rawGeometryShaderSize + shaderEntrySet.rawComputeShaderSize +
             sizeof(shaderEntrySet.renderStateBlock));

    numCharsToRemove += sizeof(ShaderEntryHeader);

    SHIP_DELETE(shaderEntrySet.rawVertexShader);
    SHIP_DELETE(shaderEntrySet.rawPixelShader);
    SHIP_DELETE(shaderEntrySet.rawHullShader);
    SHIP_DELETE(shaderEntrySet.rawDomainShader);
    SHIP_DELETE(shaderEntrySet.rawGeometryShader);
    SHIP_DELETE(shaderEntrySet.rawComputeShader);

    m_FileHandler.RemoveChars(positionToRemoveInFile, numCharsToRemove);

    m_ShaderEntryKeys.RemoveAtPreserveOrder(shaderSetIndexToRemove);
    m_ShaderEntrySets.RemoveAtPreserveOrder(shaderSetIndexToRemove);

    ShaderEntriesHeader shaderEntriesHeader;

    size_t shaderEntriesHeaderPosition = sizeof(DatabaseHeader);
    m_FileHandler.ReadChars(shaderEntriesHeaderPosition, (char*)&shaderEntriesHeader, sizeof(shaderEntriesHeader));

    SHIP_ASSERT(shaderEntriesHeader.numShaderEntries - 1 == m_ShaderEntryKeys.Size());

    shaderEntriesHeader.numShaderEntries -= 1;

    constexpr bool flush = true;
    m_FileHandler.WriteChars(shaderEntriesHeaderPosition, (char*)&shaderEntriesHeader, sizeof(shaderEntriesHeader), flush);
}

namespace
{;

void StealShaderMemory(uint8_t* sourceRawShader, size_t sourceShaderSize, uint8_t*& destRawShader, size_t& destShaderSize)
{
    if (sourceShaderSize > 0)
    {
        destShaderSize = sourceShaderSize;
        destRawShader = reinterpret_cast<uint8_t*>(SHIP_ALLOC(sourceShaderSize, 1));

        memcpy(destRawShader, sourceRawShader, sourceShaderSize);
    }
}

void LoadShaderFromBuffer(uint8_t*& buffer, uint8_t*& rawShader, size_t shaderSize)
{
    if (shaderSize > 0)
    {
        rawShader = reinterpret_cast<uint8_t*>(SHIP_ALLOC(shaderSize, 1));

        memcpy(rawShader, buffer, shaderSize);

        buffer += shaderSize;
    }
}

}

void ShaderDatabase::AppendShadersForShaderKey(const ShaderKey& shaderKey, ShaderEntrySet& shaderEntrySet)
{
    // We copy the shaders so that the ShaderDatabase owns the memory
    ShaderEntrySet stolenShaderEntrySet;
    stolenShaderEntrySet.lastModifiedTimestamp = shaderEntrySet.lastModifiedTimestamp;
    stolenShaderEntrySet.renderStateBlock = shaderEntrySet.renderStateBlock;

    StealShaderMemory(shaderEntrySet.rawVertexShader, shaderEntrySet.rawVertexShaderSize, stolenShaderEntrySet.rawVertexShader, stolenShaderEntrySet.rawVertexShaderSize);
    StealShaderMemory(shaderEntrySet.rawPixelShader, shaderEntrySet.rawPixelShaderSize, stolenShaderEntrySet.rawPixelShader, stolenShaderEntrySet.rawPixelShaderSize);
    StealShaderMemory(shaderEntrySet.rawHullShader, shaderEntrySet.rawHullShaderSize, stolenShaderEntrySet.rawHullShader, stolenShaderEntrySet.rawHullShaderSize);
    StealShaderMemory(shaderEntrySet.rawDomainShader, shaderEntrySet.rawDomainShaderSize, stolenShaderEntrySet.rawDomainShader, stolenShaderEntrySet.rawDomainShaderSize);
    StealShaderMemory(shaderEntrySet.rawGeometryShader, shaderEntrySet.rawGeometryShaderSize, stolenShaderEntrySet.rawGeometryShader, stolenShaderEntrySet.rawGeometryShaderSize);
    StealShaderMemory(shaderEntrySet.rawComputeShader, shaderEntrySet.rawComputeShaderSize, stolenShaderEntrySet.rawComputeShader, stolenShaderEntrySet.rawComputeShaderSize);

    shaderEntrySet = stolenShaderEntrySet;

    ShaderEntryKey& newShaderEntryKey = m_ShaderEntryKeys.Grow();
    newShaderEntryKey.shaderKey = shaderKey;

    m_ShaderEntrySets.Add(shaderEntrySet);

    ShaderEntriesHeader shaderEntriesHeader;

    size_t shaderEntriesHeaderPosition = sizeof(DatabaseHeader);
    m_FileHandler.ReadChars(shaderEntriesHeaderPosition, (char*)&shaderEntriesHeader, sizeof(shaderEntriesHeader));

    SHIP_ASSERT(shaderEntriesHeader.numShaderEntries + 1 == m_ShaderEntryKeys.Size());

    shaderEntriesHeader.numShaderEntries += 1;
    m_FileHandler.WriteChars(shaderEntriesHeaderPosition, (char*)&shaderEntriesHeader, sizeof(shaderEntriesHeader));
    
    ShaderEntryHeader shaderEntryHeader;
    shaderEntryHeader.shaderKey = newShaderEntryKey.shaderKey;
    shaderEntryHeader.lastModifiedTimestamp = shaderEntrySet.lastModifiedTimestamp;
    shaderEntryHeader.rawVertexShaderSize = shaderEntrySet.rawVertexShaderSize;
    shaderEntryHeader.rawPixelShaderSize = shaderEntrySet.rawPixelShaderSize;
    shaderEntryHeader.rawHullShaderSize = shaderEntrySet.rawHullShaderSize;
    shaderEntryHeader.rawDomainShaderSize = shaderEntrySet.rawDomainShaderSize;
    shaderEntryHeader.rawGeometryShaderSize = shaderEntrySet.rawGeometryShaderSize;
    shaderEntryHeader.rawComputeShaderSize = shaderEntrySet.rawComputeShaderSize;

    m_FileHandler.AppendChars((const char*)&shaderEntryHeader, sizeof(shaderEntryHeader));

    constexpr bool flush = true;

    if (shaderEntrySet.rawVertexShaderSize > 0)
    {
        m_FileHandler.AppendChars((const char*)shaderEntrySet.rawVertexShader, shaderEntrySet.rawVertexShaderSize, flush);
    }

    if (shaderEntrySet.rawPixelShaderSize > 0)
    {
        m_FileHandler.AppendChars((const char*)shaderEntrySet.rawPixelShader, shaderEntrySet.rawPixelShaderSize, flush);
    }

    if (shaderEntrySet.rawHullShaderSize > 0)
    {
        m_FileHandler.AppendChars((const char*)shaderEntrySet.rawHullShader, shaderEntrySet.rawHullShaderSize, flush);
    }

    if (shaderEntrySet.rawDomainShaderSize > 0)
    {
        m_FileHandler.AppendChars((const char*)shaderEntrySet.rawDomainShader, shaderEntrySet.rawDomainShaderSize, flush);
    }

    if (shaderEntrySet.rawGeometryShaderSize > 0)
    {
        m_FileHandler.AppendChars((const char*)shaderEntrySet.rawGeometryShader, shaderEntrySet.rawGeometryShaderSize, flush);
    }

    if (shaderEntrySet.rawComputeShaderSize > 0)
    {
        m_FileHandler.AppendChars((const char*)shaderEntrySet.rawComputeShader, shaderEntrySet.rawComputeShaderSize, flush);
    }

    m_FileHandler.AppendChars((const char*)&shaderEntrySet.renderStateBlock, sizeof(shaderEntrySet.renderStateBlock), flush);
}

void ShaderDatabase::LoadNextShaderEntry(uint8_t*& databaseBuffer, BigArray<ShaderEntryKey>& shaderEntryKeys, BigArray<ShaderEntrySet>& shaderEntrySets) const
{
    ShaderEntryHeader shaderEntryHeader = *(ShaderEntryHeader*)databaseBuffer;

    databaseBuffer += sizeof(shaderEntryHeader);

    ShaderEntryKey& newShaderEntryKey = shaderEntryKeys.Grow();
    newShaderEntryKey.shaderKey = shaderEntryHeader.shaderKey;

    ShaderEntrySet& newShaderEntrySet = shaderEntrySets.Grow();
    memset(&newShaderEntrySet, 0, sizeof(newShaderEntrySet));

    newShaderEntrySet.lastModifiedTimestamp = shaderEntryHeader.lastModifiedTimestamp;

    newShaderEntrySet.rawVertexShaderSize = shaderEntryHeader.rawVertexShaderSize;
    newShaderEntrySet.rawPixelShaderSize = shaderEntryHeader.rawPixelShaderSize;
    newShaderEntrySet.rawHullShaderSize = shaderEntryHeader.rawHullShaderSize;
    newShaderEntrySet.rawDomainShaderSize = shaderEntryHeader.rawDomainShaderSize;
    newShaderEntrySet.rawGeometryShaderSize = shaderEntryHeader.rawGeometryShaderSize;
    newShaderEntrySet.rawComputeShaderSize = shaderEntryHeader.rawComputeShaderSize;

    LoadShaderFromBuffer(databaseBuffer, newShaderEntrySet.rawVertexShader, newShaderEntrySet.rawVertexShaderSize);
    LoadShaderFromBuffer(databaseBuffer, newShaderEntrySet.rawPixelShader, newShaderEntrySet.rawPixelShaderSize);
    LoadShaderFromBuffer(databaseBuffer, newShaderEntrySet.rawHullShader, newShaderEntrySet.rawHullShaderSize);
    LoadShaderFromBuffer(databaseBuffer, newShaderEntrySet.rawDomainShader, newShaderEntrySet.rawDomainShaderSize);
    LoadShaderFromBuffer(databaseBuffer, newShaderEntrySet.rawGeometryShader, newShaderEntrySet.rawGeometryShaderSize);
    LoadShaderFromBuffer(databaseBuffer, newShaderEntrySet.rawComputeShader, newShaderEntrySet.rawComputeShaderSize);

    memcpy((uint8_t*)&newShaderEntrySet.renderStateBlock, databaseBuffer, sizeof(newShaderEntrySet.renderStateBlock));
    databaseBuffer += sizeof(newShaderEntrySet.renderStateBlock);
}

}