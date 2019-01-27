#pragma once

#include <common/common.h>

#include <system/common.h>

#include <system/array.h>
#include <system/wrapper/wrapper.h>

#include <common/shaderhandler.h>

namespace Shipyard
{
    class SHIPYARD_API ShaderDatabase
    {
    public:
        enum : uint64_t
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
            uint64_t lowMagic;
            uint64_t highMagic;

            uint32_t platform;

            // Used to determine if database's content is compatible.
            uint32_t databaseVersionNumber;
        };

        struct ShaderEntriesHeader
        {
            uint32_t numShaderEntries = 0;
        };

        struct ShaderEntryHeader
        {
            ShaderKey shaderKey;
            uint64_t lastModifiedTimestamp = 0;

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
            uint64_t lastModifiedTimestamp = 0;

            size_t rawVertexShaderSize = 0;
            size_t rawPixelShaderSize = 0;
            size_t rawHullShaderSize = 0;
            size_t rawDomainShaderSize = 0;
            size_t rawGeometryShaderSize = 0;
            size_t rawComputeShaderSize = 0;

            uint8_t* rawVertexShader = nullptr;
            uint8_t* rawPixelShader = nullptr;
            uint8_t* rawHullShader = nullptr;
            uint8_t* rawDomainShader = nullptr;
            uint8_t* rawGeometryShader = nullptr;
            uint8_t* rawComputeShader = nullptr;
        };

    public:
        ShaderDatabase();
        ~ShaderDatabase();

        bool Load(const String& filename);
        void Close();

        bool Invalidate();

        bool RetrieveShadersForShaderKey(const ShaderKey& shaderKey, uint64_t lastModifiedTimestamp, ShaderEntrySet& shaderEntrySet) const;

        void RemoveShadersForShaderKey(const ShaderKey& shaderKey);
        void AppendShadersForShaderKey(const ShaderKey& shaderKey, ShaderEntrySet& shaderEntrySet);

    private:
        struct ShaderEntryKey
        {
            ShaderKey shaderKey;
        };

    private:
        void LoadNextShaderEntry(uint8_t* databaseBuffer, BigArray<ShaderEntryKey>& shaderEntryKeys, BigArray<ShaderEntrySet>& shaderEntrySets) const;

        String m_Filename;
        FileHandler m_FileHandler;

        BigArray<ShaderEntryKey> m_ShaderEntryKeys;
        BigArray<ShaderEntrySet> m_ShaderEntrySets;
    };
}