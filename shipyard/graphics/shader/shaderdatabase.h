#pragma once

#include <graphics/graphicscommon.h>

#include <graphics/shader/shaderhandler.h>
#include <graphics/shader/shaderresourcebinder.h>

#include <system/systemcommon.h>

#include <system/array.h>
#include <system/wrapper/wrapper.h>

namespace Shipyard
{
    class SHIPYARD_API ShaderDatabase
    {
    public:
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

    public:
        struct ShaderEntrySet
        {
            shipUint64 lastModifiedTimestamp = 0;

            size_t rawVertexShaderSize = 0;
            size_t rawPixelShaderSize = 0;
            size_t rawHullShaderSize = 0;
            size_t rawDomainShaderSize = 0;
            size_t rawGeometryShaderSize = 0;
            size_t rawComputeShaderSize = 0;

            shipUint8* rawVertexShader = nullptr;
            shipUint8* rawPixelShader = nullptr;
            shipUint8* rawHullShader = nullptr;
            shipUint8* rawDomainShader = nullptr;
            shipUint8* rawGeometryShader = nullptr;
            shipUint8* rawComputeShader = nullptr;

            RenderStateBlock renderStateBlock;

            InplaceArray<RootSignatureParameterEntry, 8> rootSignatureParameters;

            ShaderResourceBinder shaderResourceBinder;

            InplaceArray<DescriptorSetEntryDeclaration, 8> descriptorSetEntryDeclarations;
        };

    public:
        ShaderDatabase();
        ~ShaderDatabase();

        shipBool Load(const StringT& filename);
        void Close();

        shipBool Invalidate();

        shipBool RetrieveShadersForShaderKey(const ShaderKey& shaderKey, ShaderEntrySet& shaderEntrySet) const;

        void RemoveShadersForShaderKey(const ShaderKey& shaderKey);
        void AppendShadersForShaderKey(const ShaderKey& shaderKey, ShaderEntrySet& shaderEntrySet);

    private:
        struct ShaderEntryKey
        {
            ShaderKey shaderKey;
        };

    private:
        void LoadNextShaderEntry(shipUint8*& databaseBuffer, BigArray<ShaderEntryKey>& shaderEntryKeys, BigArray<ShaderEntrySet>& shaderEntrySets) const;

        void WriteRootSignatureParameters(const Array<RootSignatureParameterEntry>& rootSignatureParameters);
        void ReadRootSignatureParameters(shipUint8*& databaseBuffer, Array<RootSignatureParameterEntry>& rootSignatureParameters) const;

        StringT m_Filename;
        FileHandler m_FileHandler;

        BigArray<ShaderEntryKey> m_ShaderEntryKeys;
        BigArray<ShaderEntrySet> m_ShaderEntrySets;
    };
}