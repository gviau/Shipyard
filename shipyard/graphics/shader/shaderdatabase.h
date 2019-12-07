#pragma once

#include <graphics/graphicscommon.h>

#include <graphics/shader/shaderhandler.h>
#include <graphics/shader/shaderinputprovider.h>
#include <graphics/shader/shaderresourcebinder.h>

#include <system/systemcommon.h>

#include <system/array.h>
#include <system/wrapper/wrapper.h>

namespace Shipyard
{
    class SHIPYARD_API ShaderDatabase
    {
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

            InplaceArray<SamplerState, 4> samplerStates;
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
        struct ShaderInputProviderDeclarationEntry
        {
            shipUint32 shaderInputProviderDeclarationNameLength = 0;
            shipChar shaderInputProviderDeclarationName[ShaderInputProviderDeclaration::MaxShaderInputProviderNameLength];
        };

        struct ShaderEntryKey
        {
            ShaderKey shaderKey;
        };

    private:
        shipBool ValidateShaderInputProviderDeclarations(shipUint8*& databaseBuffer, Array<ShaderInputProviderDeclarationEntry>& shaderInputProviderDeclarationEntries) const;

        shipBool LoadNextShaderEntry(shipUint8*& databaseBuffer, BigArray<ShaderEntryKey>& shaderEntryKeys, BigArray<ShaderEntrySet>& shaderEntrySets) const;

        void WriteRootSignatureParameters(const Array<RootSignatureParameterEntry>& rootSignatureParameters);
        void WriteShaderResourceBinderEntries(const Array<ShaderResourceBinder::ShaderResourceBinderEntry>& shaderResourceBinderEntries);

        void ReadRootSignatureParameters(shipUint8*& databaseBuffer, Array<RootSignatureParameterEntry>& rootSignatureParameters) const;
        shipBool ReadShaderResourceBinderEntries(shipUint8*& databaseBuffer, Array<ShaderResourceBinder::ShaderResourceBinderEntry>& shaderResourceBinderEntries) const;

        size_t GetShaderEntrySetStartPosition() const;
        size_t GetShaderEntrySetSize(const ShaderEntrySet& shaderEntrySet) const;

        StringT m_Filename;
        FileHandlerStream m_FileHandler;

        Array<ShaderInputProviderDeclarationEntry> m_ShaderInputProviderDeclarationEntries;
        BigArray<ShaderEntryKey> m_ShaderEntryKeys;
        BigArray<ShaderEntrySet> m_ShaderEntrySets;
    };
}