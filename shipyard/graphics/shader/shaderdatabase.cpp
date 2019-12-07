#include <graphics/shader/shaderdatabase.h>

#include <graphics/shader/shaderresourcebinder.h>

#include <system/memory.h>

namespace Shipyard
{;

enum : shipUint64
{
    // Increment version if changes were made to shaders or database that would render already existing databases
    // incompatible.
    Version = 1,

    LowMagicConstant = 0x2b8e8a3b5f02ce78,
    HighMagicConstant = 0xba927e7f8abc09d
};

struct DatabaseHeader
{
    // Used to identify a shader database, regardless of file extension.
    shipUint64 lowMagic;
    shipUint64 highMagic;

    shipUint32 platform;

    // Used to determine if database's content is compatible.
    shipUint32 databaseVersionNumber;
};

struct ShaderInputProviderDeclarationEntriesHeader
{
    shipUint32 numShaderInputProviderEntries = 0;
};

struct ShaderEntriesHeader
{
    shipUint32 numShaderEntries = 0;
};

struct ShaderEntryHeader
{
    ShaderKey shaderKey;
    shipUint64 lastModifiedTimestamp = 0;

    size_t rawVertexShaderSize = 0;
    size_t rawPixelShaderSize = 0;
    size_t rawHullShaderSize = 0;
    size_t rawDomainShaderSize = 0;
    size_t rawGeometryShaderSize = 0;
    size_t rawComputeShaderSize = 0;
};

ShaderDatabase::ShaderDatabase()
    : m_ShaderEntrySets(shipUint32(0))
    , m_ShaderEntryKeys(shipUint32(0))
{

}

ShaderDatabase::~ShaderDatabase()
{
    Close();
}

shipBool ShaderDatabase::Load(const StringT& filename)
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

    shipUint8* databaseBuffer = (shipUint8*)&databaseContent[0];

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

    if (!ValidateShaderInputProviderDeclarations(databaseBuffer, m_ShaderInputProviderDeclarationEntries))
    {
        Invalidate();
        return false;
    }

    if (shaderEntriesHeader.numShaderEntries > 0)
    {
        m_ShaderEntryKeys.Reserve(shaderEntriesHeader.numShaderEntries);
        m_ShaderEntrySets.Reserve(shaderEntriesHeader.numShaderEntries);
    }

    for (shipUint32 i = 0; i < shaderEntriesHeader.numShaderEntries; i++)
    {
        shipBool continueLoadingShaderEntries = LoadNextShaderEntry(databaseBuffer, m_ShaderEntryKeys, m_ShaderEntrySets);

        if (!continueLoadingShaderEntries)
        {
            Invalidate();
            return false;
        }
    }

    return true;
}

void ShaderDatabase::Close()
{
    m_Filename.Clear();

    m_FileHandler.Close();

    m_ShaderInputProviderDeclarationEntries.Clear();

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

shipBool ShaderDatabase::Invalidate()
{
    // Close clears the filename
    StringT filename = m_Filename;

    Close();

    m_Filename = filename;

    if (!m_FileHandler.Open(m_Filename, FileHandlerOpenFlag(FileHandlerOpenFlag_ReadWrite | FileHandlerOpenFlag_Binary | FileHandlerOpenFlag_Create)))
    {
        return false;
    }

    DatabaseHeader databaseHeader;
    databaseHeader.lowMagic = LowMagicConstant;
    databaseHeader.highMagic = HighMagicConstant;
    databaseHeader.platform = PLATFORM;
    databaseHeader.databaseVersionNumber = Version;

    m_FileHandler.AppendChars((const shipChar*)&databaseHeader, sizeof(databaseHeader));

    ShaderEntriesHeader shaderEntriesHeader;
    shaderEntriesHeader.numShaderEntries = 0;

    m_FileHandler.AppendChars((const shipChar*)&shaderEntriesHeader, sizeof(shaderEntriesHeader));

    Array<ShaderInputProviderDeclaration*> shaderInputProviderDeclarations;
    GetShaderInputProviderManager().GetShaderInputProviderDeclarations(shaderInputProviderDeclarations);

    ShaderInputProviderDeclarationEntriesHeader shaderInputProviderDeclarationEntriesHeader;
    shaderInputProviderDeclarationEntriesHeader.numShaderInputProviderEntries = shaderInputProviderDeclarations.Size();

    m_FileHandler.AppendChars((const shipChar*)&shaderInputProviderDeclarationEntriesHeader, sizeof(shaderInputProviderDeclarationEntriesHeader));

    for (ShaderInputProviderDeclaration* shaderInputProviderDeclaration : shaderInputProviderDeclarations)
    {
        const shipChar* shaderInputProviderDeclarationName = shaderInputProviderDeclaration->GetShaderInputProviderName();

        ShaderInputProviderDeclarationEntry& shaderInputProviderDeclarationEntry = m_ShaderInputProviderDeclarationEntries.Grow();
        shaderInputProviderDeclarationEntry.shaderInputProviderDeclarationNameLength = shipUint32(strlen(shaderInputProviderDeclarationName));
        memcpy(shaderInputProviderDeclarationEntry.shaderInputProviderDeclarationName, shaderInputProviderDeclarationName, shaderInputProviderDeclarationEntry.shaderInputProviderDeclarationNameLength);

        m_FileHandler.AppendChars((const shipChar*)&shaderInputProviderDeclarationEntry, sizeof(shaderInputProviderDeclarationEntry));
    }

    return true;
}

shipBool ShaderDatabase::RetrieveShadersForShaderKey(const ShaderKey& shaderKey, ShaderEntrySet& shaderEntrySet) const
{
    shipUint32 shaderSetIndex = 0;
    shipUint32 numShaderEntries = m_ShaderEntryKeys.Size();

    for (; shaderSetIndex < numShaderEntries; shaderSetIndex++)
    {
        const ShaderEntryKey& shaderEntryKey = m_ShaderEntryKeys[shaderSetIndex];
        if (shaderEntryKey.shaderKey == shaderKey)
        {
            break;
        }
    }

    shipBool foundShaderKey = (shaderSetIndex < numShaderEntries);
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
    shipUint32 shaderSetIndexToRemove = 0;
    shipUint32 numShaderEntries = m_ShaderEntryKeys.Size();

    for (; shaderSetIndexToRemove < numShaderEntries; shaderSetIndexToRemove++)
    {
        const ShaderEntryKey& shaderEntryKey = m_ShaderEntryKeys[shaderSetIndexToRemove];
        if (shaderEntryKey.shaderKey == shaderKey)
        {
            break;
        }
    }

    shipBool foundShaderKey = (shaderSetIndexToRemove < numShaderEntries);
    if (!foundShaderKey)
    {
        return;
    }

    size_t positionToRemoveInFile = GetShaderEntrySetStartPosition();

    for (shipUint32 i = 0; i < shaderSetIndexToRemove; i++)
    {
        const ShaderEntrySet& shaderEntrySet = m_ShaderEntrySets[i];

        size_t shaderEntrySetSize = GetShaderEntrySetSize(shaderEntrySet);

        positionToRemoveInFile += shaderEntrySetSize + sizeof(ShaderEntryHeader);
    }

    const ShaderEntrySet& shaderEntrySetToRemove = m_ShaderEntrySets[shaderSetIndexToRemove];

    size_t numCharsToRemove = GetShaderEntrySetSize(shaderEntrySetToRemove);

    numCharsToRemove += sizeof(ShaderEntryHeader);

    SHIP_DELETE(shaderEntrySetToRemove.rawVertexShader);
    SHIP_DELETE(shaderEntrySetToRemove.rawPixelShader);
    SHIP_DELETE(shaderEntrySetToRemove.rawHullShader);
    SHIP_DELETE(shaderEntrySetToRemove.rawDomainShader);
    SHIP_DELETE(shaderEntrySetToRemove.rawGeometryShader);
    SHIP_DELETE(shaderEntrySetToRemove.rawComputeShader);

    m_FileHandler.RemoveChars(positionToRemoveInFile, numCharsToRemove);

    m_ShaderEntryKeys.RemoveAtPreserveOrder(shaderSetIndexToRemove);
    m_ShaderEntrySets.RemoveAtPreserveOrder(shaderSetIndexToRemove);

    ShaderEntriesHeader shaderEntriesHeader;

    size_t shaderEntriesHeaderPosition = sizeof(DatabaseHeader);
    m_FileHandler.ReadChars(shaderEntriesHeaderPosition, (shipChar*)&shaderEntriesHeader, sizeof(shaderEntriesHeader));

    SHIP_ASSERT(shaderEntriesHeader.numShaderEntries - 1 == m_ShaderEntryKeys.Size());

    shaderEntriesHeader.numShaderEntries -= 1;

    m_FileHandler.WriteChars(shaderEntriesHeaderPosition, (shipChar*)&shaderEntriesHeader, sizeof(shaderEntriesHeader));

    m_FileHandler.Flush();
}

namespace
{;

void StealShaderMemory(shipUint8* sourceRawShader, size_t sourceShaderSize, shipUint8*& destRawShader, size_t& destShaderSize)
{
    if (sourceShaderSize > 0)
    {
        destShaderSize = sourceShaderSize;
        destRawShader = reinterpret_cast<shipUint8*>(SHIP_ALLOC(sourceShaderSize, 1));

        memcpy(destRawShader, sourceRawShader, sourceShaderSize);
    }
}

void LoadShaderFromBuffer(shipUint8*& buffer, shipUint8*& rawShader, size_t shaderSize)
{
    if (shaderSize > 0)
    {
        rawShader = reinterpret_cast<shipUint8*>(SHIP_ALLOC(shaderSize, 1));

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
    stolenShaderEntrySet.rootSignatureParameters = shaderEntrySet.rootSignatureParameters;
    stolenShaderEntrySet.shaderResourceBinder = shaderEntrySet.shaderResourceBinder;
    stolenShaderEntrySet.descriptorSetEntryDeclarations = shaderEntrySet.descriptorSetEntryDeclarations;
    stolenShaderEntrySet.samplerStates = shaderEntrySet.samplerStates;

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
    m_FileHandler.ReadChars(shaderEntriesHeaderPosition, (shipChar*)&shaderEntriesHeader, sizeof(shaderEntriesHeader));

    SHIP_ASSERT(shaderEntriesHeader.numShaderEntries + 1 == m_ShaderEntryKeys.Size());

    shaderEntriesHeader.numShaderEntries += 1;
    m_FileHandler.WriteChars(shaderEntriesHeaderPosition, (shipChar*)&shaderEntriesHeader, sizeof(shaderEntriesHeader));
    
    ShaderEntryHeader shaderEntryHeader;
    shaderEntryHeader.shaderKey = newShaderEntryKey.shaderKey;
    shaderEntryHeader.lastModifiedTimestamp = shaderEntrySet.lastModifiedTimestamp;
    shaderEntryHeader.rawVertexShaderSize = shaderEntrySet.rawVertexShaderSize;
    shaderEntryHeader.rawPixelShaderSize = shaderEntrySet.rawPixelShaderSize;
    shaderEntryHeader.rawHullShaderSize = shaderEntrySet.rawHullShaderSize;
    shaderEntryHeader.rawDomainShaderSize = shaderEntrySet.rawDomainShaderSize;
    shaderEntryHeader.rawGeometryShaderSize = shaderEntrySet.rawGeometryShaderSize;
    shaderEntryHeader.rawComputeShaderSize = shaderEntrySet.rawComputeShaderSize;

    m_FileHandler.AppendChars((const shipChar*)&shaderEntryHeader, sizeof(shaderEntryHeader));

    if (shaderEntrySet.rawVertexShaderSize > 0)
    {
        m_FileHandler.AppendChars((const shipChar*)shaderEntrySet.rawVertexShader, shaderEntrySet.rawVertexShaderSize);
    }

    if (shaderEntrySet.rawPixelShaderSize > 0)
    {
        m_FileHandler.AppendChars((const shipChar*)shaderEntrySet.rawPixelShader, shaderEntrySet.rawPixelShaderSize);
    }

    if (shaderEntrySet.rawHullShaderSize > 0)
    {
        m_FileHandler.AppendChars((const shipChar*)shaderEntrySet.rawHullShader, shaderEntrySet.rawHullShaderSize);
    }

    if (shaderEntrySet.rawDomainShaderSize > 0)
    {
        m_FileHandler.AppendChars((const shipChar*)shaderEntrySet.rawDomainShader, shaderEntrySet.rawDomainShaderSize);
    }

    if (shaderEntrySet.rawGeometryShaderSize > 0)
    {
        m_FileHandler.AppendChars((const shipChar*)shaderEntrySet.rawGeometryShader, shaderEntrySet.rawGeometryShaderSize);
    }

    if (shaderEntrySet.rawComputeShaderSize > 0)
    {
        m_FileHandler.AppendChars((const shipChar*)shaderEntrySet.rawComputeShader, shaderEntrySet.rawComputeShaderSize);
    }

    m_FileHandler.AppendChars((const shipChar*)&shaderEntrySet.renderStateBlock, sizeof(shaderEntrySet.renderStateBlock));

    WriteRootSignatureParameters(shaderEntrySet.rootSignatureParameters);

    const Array<ShaderResourceBinder::ShaderResourceBinderEntry>& shaderResourceBinderEntries = shaderEntrySet.shaderResourceBinder.GetShaderResourceBinderEntries();
    
    WriteShaderResourceBinderEntries(shaderResourceBinderEntries);

    shipUint32 numDescriptorSetEntryDeclarations = shaderEntrySet.descriptorSetEntryDeclarations.Size();

    m_FileHandler.AppendChars((const shipChar*)&numDescriptorSetEntryDeclarations, sizeof(numDescriptorSetEntryDeclarations));

    if (numDescriptorSetEntryDeclarations > 0)
    {
        m_FileHandler.AppendChars((const shipChar*)&shaderEntrySet.descriptorSetEntryDeclarations[0], sizeof(shaderEntrySet.descriptorSetEntryDeclarations[0]) * numDescriptorSetEntryDeclarations);
    }

    shipUint32 numSamplerStates = shaderEntrySet.samplerStates.Size();
    m_FileHandler.AppendChars((const shipChar*)&numSamplerStates, sizeof(numSamplerStates));

    if (numSamplerStates > 0)
    {
        m_FileHandler.AppendChars((const shipChar*)&shaderEntrySet.samplerStates[0], sizeof(shaderEntrySet.samplerStates[0]) * numSamplerStates);
    }

    m_FileHandler.Flush();
}

shipBool ShaderDatabase::ValidateShaderInputProviderDeclarations(shipUint8*& databaseBuffer, Array<ShaderInputProviderDeclarationEntry>& shaderInputProviderDeclarationEntries) const
{
    ShaderInputProviderDeclarationEntriesHeader shaderInputProviderDeclarationEntriesHeader = *(ShaderInputProviderDeclarationEntriesHeader*)databaseBuffer;

    databaseBuffer += sizeof(shaderInputProviderDeclarationEntriesHeader);

    ShaderInputProviderManager& shaderInputProviderManager = GetShaderInputProviderManager();

    Array<ShaderInputProviderDeclaration*> shaderInputProviderDeclarations;
    shaderInputProviderManager.GetShaderInputProviderDeclarations(shaderInputProviderDeclarations);

    if (shaderInputProviderDeclarations.Size() != shaderInputProviderDeclarationEntriesHeader.numShaderInputProviderEntries)
    {
        return false;
    }

    shaderInputProviderDeclarationEntries.Reserve(shaderInputProviderDeclarations.Size());

    for (shipUint32 i = 0; i < shaderInputProviderDeclarations.Size(); i++)
    {
        ShaderInputProviderDeclarationEntry& newEntry = shaderInputProviderDeclarationEntries.Grow();

        memcpy(&newEntry, databaseBuffer, sizeof(newEntry));
        databaseBuffer += sizeof(newEntry);

        if (shaderInputProviderManager.FindShaderInputProviderDeclarationFromName(newEntry.shaderInputProviderDeclarationName) == nullptr)
        {
            return false;
        }
    }

    return true;
}

shipBool ShaderDatabase::LoadNextShaderEntry(shipUint8*& databaseBuffer, BigArray<ShaderEntryKey>& shaderEntryKeys, BigArray<ShaderEntrySet>& shaderEntrySets) const
{
    ShaderEntryHeader shaderEntryHeader = *(ShaderEntryHeader*)databaseBuffer;

    databaseBuffer += sizeof(shaderEntryHeader);

    ShaderEntryKey& newShaderEntryKey = shaderEntryKeys.Grow();
    newShaderEntryKey.shaderKey = shaderEntryHeader.shaderKey;

    ShaderEntrySet& newShaderEntrySet = shaderEntrySets.Grow();
    
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

    memcpy((shipUint8*)&newShaderEntrySet.renderStateBlock, databaseBuffer, sizeof(newShaderEntrySet.renderStateBlock));
    databaseBuffer += sizeof(newShaderEntrySet.renderStateBlock);

    ReadRootSignatureParameters(databaseBuffer, newShaderEntrySet.rootSignatureParameters);

    shipBool noInvalidShaderInputProviderDeclaration = ReadShaderResourceBinderEntries(
            databaseBuffer,
            newShaderEntrySet.shaderResourceBinder.GetShaderResourceBinderEntries());

    if (!noInvalidShaderInputProviderDeclaration)
    {
        return false;
    }

    shipUint32 numDescriptorSetEntryDeclarations = *(const shipUint32*)databaseBuffer;
    databaseBuffer += sizeof(shipUint32);

    if (numDescriptorSetEntryDeclarations > 0)
    {
        newShaderEntrySet.descriptorSetEntryDeclarations.Resize(numDescriptorSetEntryDeclarations);

        memcpy(&newShaderEntrySet.descriptorSetEntryDeclarations[0], databaseBuffer, sizeof(newShaderEntrySet.descriptorSetEntryDeclarations[0]) * numDescriptorSetEntryDeclarations);
        databaseBuffer += sizeof(newShaderEntrySet.descriptorSetEntryDeclarations[0]) * numDescriptorSetEntryDeclarations;
    }

    shipUint32 numSamplerStates = *(const shipUint32*)databaseBuffer;
    databaseBuffer += sizeof(shipUint32);

    if (numSamplerStates > 0)
    {
        newShaderEntrySet.samplerStates.Resize(numSamplerStates);

        memcpy(&newShaderEntrySet.samplerStates[0], databaseBuffer, sizeof(newShaderEntrySet.samplerStates[0]) * numSamplerStates);
        databaseBuffer += sizeof(newShaderEntrySet.samplerStates[0]) * numSamplerStates;
    }

    return true;
}

void ShaderDatabase::WriteRootSignatureParameters(const Array<RootSignatureParameterEntry>& rootSignatureParameters)
{
    shipUint32 numRootSignatureParameters = rootSignatureParameters.Size();
    SHIP_ASSERT(numRootSignatureParameters < 8);
    m_FileHandler.AppendChars((const shipChar*)&numRootSignatureParameters, sizeof(numRootSignatureParameters));

    for (const RootSignatureParameterEntry& rootSignatureParameterEntry : rootSignatureParameters)
    {
        m_FileHandler.AppendChars((const shipChar*)&rootSignatureParameterEntry.shaderVisibility, sizeof(rootSignatureParameterEntry.shaderVisibility));
        m_FileHandler.AppendChars((const shipChar*)&rootSignatureParameterEntry.parameterType, sizeof(rootSignatureParameterEntry.parameterType));

        if (rootSignatureParameterEntry.parameterType == RootSignatureParameterType::DescriptorTable)
        {
            shipUint32 numDescriptorRanges = rootSignatureParameterEntry.descriptorTable.descriptorRanges.Size();
            m_FileHandler.AppendChars((const shipChar*)&numDescriptorRanges, sizeof(numDescriptorRanges));

            if (numDescriptorRanges > 0)
            {
                const DescriptorRange* descriptorRange = &rootSignatureParameterEntry.descriptorTable.descriptorRanges[0];
                m_FileHandler.AppendChars((const shipChar*)descriptorRange, sizeof(*descriptorRange) * numDescriptorRanges);
            }
        }
        else
        {
            m_FileHandler.AppendChars((const shipChar*)&rootSignatureParameterEntry.descriptor, sizeof(rootSignatureParameterEntry.descriptor));
        }
    }
}

void ShaderDatabase::WriteShaderResourceBinderEntries(const Array<ShaderResourceBinder::ShaderResourceBinderEntry>& shaderResourceBinderEntries)
{
    shipUint32 numShaderResourceBinderEntries = shaderResourceBinderEntries.Size();

    m_FileHandler.AppendChars((const shipChar*)&numShaderResourceBinderEntries, sizeof(numShaderResourceBinderEntries));

    for (shipUint32 i = 0; i < numShaderResourceBinderEntries; i++)
    {
        const ShaderResourceBinder::ShaderResourceBinderEntry& shaderResourceBinderEntry = shaderResourceBinderEntries[i];

        shipUint32 shaderInputProviderNameIndex = 0xffffffff;

        if (shaderResourceBinderEntry.Declaration != nullptr)
        {
            const shipChar* shaderInputProviderName = shaderResourceBinderEntry.Declaration->GetShaderInputProviderName();

            for (shaderInputProviderNameIndex = 0; shaderInputProviderNameIndex < m_ShaderInputProviderDeclarationEntries.Size(); shaderInputProviderNameIndex++)
            {
                const ShaderInputProviderDeclarationEntry& entry = m_ShaderInputProviderDeclarationEntries[shaderInputProviderNameIndex];
                if (AreStringsEqual(shaderInputProviderName, entry.shaderInputProviderDeclarationName))
                {
                    break;
                }
            }

            SHIP_ASSERT(shaderInputProviderNameIndex != m_ShaderInputProviderDeclarationEntries.Size());
        }

        m_FileHandler.AppendChars((const shipChar*)&shaderInputProviderNameIndex, sizeof(shaderInputProviderNameIndex));

        m_FileHandler.AppendChars((const shipChar*)&shaderResourceBinderEntry, sizeof(shaderResourceBinderEntry));
    }
}

void ShaderDatabase::ReadRootSignatureParameters(shipUint8*& databaseBuffer, Array<RootSignatureParameterEntry>& rootSignatureParameters) const
{
    shipUint32 numRootSignatureParameters = *(shipUint32*)databaseBuffer;
    databaseBuffer += sizeof(shipUint32);

    for (shipUint32 i = 0; i < numRootSignatureParameters; i++)
    {
        RootSignatureParameterEntry& newEntry = rootSignatureParameters.Grow();

        newEntry.shaderVisibility = *(ShaderVisibility*)databaseBuffer;
        databaseBuffer += sizeof(ShaderVisibility);

        newEntry.parameterType = *(RootSignatureParameterType*)databaseBuffer;
        databaseBuffer += sizeof(RootSignatureParameterType);

        if (newEntry.parameterType == RootSignatureParameterType::DescriptorTable)
        {
            shipUint32 numDescriptorRanges = *(shipUint32*)databaseBuffer;
            databaseBuffer += sizeof(shipUint32);

            if (numDescriptorRanges > 0)
            {
                newEntry.descriptorTable.descriptorRanges.Resize(numDescriptorRanges);
                void* pDest = &newEntry.descriptorTable.descriptorRanges[0];

                memcpy(pDest, databaseBuffer, sizeof(DescriptorRange) * numDescriptorRanges);
                databaseBuffer += sizeof(DescriptorRange) * numDescriptorRanges;
            }
        }
        else
        {
            newEntry.descriptor = *(RootDescriptor*)databaseBuffer;
            databaseBuffer += sizeof(RootDescriptor);
        }
    }
}

shipBool ShaderDatabase::ReadShaderResourceBinderEntries(shipUint8*& databaseBuffer, Array<ShaderResourceBinder::ShaderResourceBinderEntry>& shaderResourceBinderEntries) const
{
    ShaderInputProviderManager& shaderInputProviderManager = GetShaderInputProviderManager();

    shipUint32 numShaderResourceBinderEntries = *(const shipUint32*)databaseBuffer;
    databaseBuffer += sizeof(shipUint32);

    if (numShaderResourceBinderEntries > 0)
    {
        shaderResourceBinderEntries.Resize(numShaderResourceBinderEntries);

        for (shipUint32 i = 0; i < numShaderResourceBinderEntries; i++)
        {
            shipUint32 shaderInputProviderNameIndex = 0;
            memcpy(&shaderInputProviderNameIndex, databaseBuffer, sizeof(shaderInputProviderNameIndex));
            databaseBuffer += sizeof(shaderInputProviderNameIndex);

            ShaderInputProviderDeclaration* shaderInputProviderDeclaration = nullptr;

            if (shaderInputProviderNameIndex != 0xffffffff)
            {
                const shipChar* shaderInputProviderName = m_ShaderInputProviderDeclarationEntries[shaderInputProviderNameIndex].shaderInputProviderDeclarationName;

                shaderInputProviderDeclaration = shaderInputProviderManager.FindShaderInputProviderDeclarationFromName(shaderInputProviderName);
                if (shaderInputProviderDeclaration == nullptr)
                {
                    return false;
                }
            }

            memcpy(&shaderResourceBinderEntries[i], databaseBuffer, sizeof(shaderResourceBinderEntries[i]));
            databaseBuffer += sizeof(shaderResourceBinderEntries[i]);

            shaderResourceBinderEntries[i].Declaration = shaderInputProviderDeclaration;
        }
    }

    return true;
}

size_t ShaderDatabase::GetShaderEntrySetStartPosition() const
{
    return (sizeof(DatabaseHeader) + sizeof(ShaderEntriesHeader) + sizeof(ShaderInputProviderDeclarationEntriesHeader) +
            sizeof(ShaderInputProviderDeclarationEntry) * m_ShaderInputProviderDeclarationEntries.Size());
}

size_t ShaderDatabase::GetShaderEntrySetSize(const ShaderEntrySet& shaderEntrySet) const
{
    size_t shaderEntrySetSize =
            (shaderEntrySet.rawVertexShaderSize + shaderEntrySet.rawPixelShaderSize + shaderEntrySet.rawHullShaderSize +
            shaderEntrySet.rawDomainShaderSize + shaderEntrySet.rawGeometryShaderSize + shaderEntrySet.rawComputeShaderSize +
            sizeof(shaderEntrySet.renderStateBlock));

    shaderEntrySetSize += sizeof(shipUint32);

    for (const RootSignatureParameterEntry& rootSignatureParameterEntry : shaderEntrySet.rootSignatureParameters)
    {
        shaderEntrySetSize += sizeof(rootSignatureParameterEntry.shaderVisibility);
        shaderEntrySetSize += sizeof(rootSignatureParameterEntry.parameterType);

        if (rootSignatureParameterEntry.parameterType == RootSignatureParameterType::DescriptorTable)
        {
            shipUint32 numDescriptorRanges = rootSignatureParameterEntry.descriptorTable.descriptorRanges.Size();
            shaderEntrySetSize += sizeof(numDescriptorRanges);

            if (numDescriptorRanges > 0)
            {
                const DescriptorRange* descriptorRange = &rootSignatureParameterEntry.descriptorTable.descriptorRanges[0];
                shaderEntrySetSize += sizeof(*descriptorRange) * numDescriptorRanges;
            }
        }
        else
        {
            shaderEntrySetSize += sizeof(rootSignatureParameterEntry.descriptor);
        }
    }

    const Array<ShaderResourceBinder::ShaderResourceBinderEntry>& shaderResourceBinderEntries = shaderEntrySet.shaderResourceBinder.GetShaderResourceBinderEntries();
    shipUint32 numShaderResourceBinderEntries = shaderResourceBinderEntries.Size();

    shaderEntrySetSize += sizeof(numShaderResourceBinderEntries);

    for (const ShaderResourceBinder::ShaderResourceBinderEntry& shaderResourceBinderEntry : shaderResourceBinderEntries)
    {
        shaderEntrySetSize += sizeof(shipUint32);

        shaderEntrySetSize += sizeof(shaderResourceBinderEntry);
    }

    shipUint32 numDescriptorSetEntryDeclarations = shaderEntrySet.descriptorSetEntryDeclarations.Size();

    shaderEntrySetSize += sizeof(numDescriptorSetEntryDeclarations);

    if (numDescriptorSetEntryDeclarations > 0)
    {
        shaderEntrySetSize += sizeof(shaderEntrySet.descriptorSetEntryDeclarations[0]) * numDescriptorSetEntryDeclarations;
    }

    shipUint32 numSamplerStates = shaderEntrySet.samplerStates.Size();

    shaderEntrySetSize += sizeof(numSamplerStates);

    if (numSamplerStates > 0)
    {
        shaderEntrySetSize += sizeof(shaderEntrySet.samplerStates[0]) * numSamplerStates;
    }

    return shaderEntrySetSize;
}

}