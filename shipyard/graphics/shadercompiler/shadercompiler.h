#pragma once

#include <graphics/shader/shaderdatabase.h>
#include <graphics/shader/shaderkey.h>
#include <graphics/shader/shaderresourcebinder.h>

#include <system/array.h>
#include <system/platform.h>
#include <system/singleton.h>
#include <system/string.h>

#include <mutex>
#include <thread>

struct _D3D_SHADER_MACRO;
struct ID3D10Blob;

namespace Shipyard
{
    enum class ShaderFamily : shipUint8;
    enum class ShaderOption : shipUint32;

    class SHIPYARD_API ShaderCompiler : public Singleton<ShaderCompiler>
    {
        friend class Singleton<ShaderCompiler>;

    public:
        static const shipChar* RenderStateBlockName;
        static const shipChar* SamplerStateBlockName;

    public:
        ShaderCompiler();
        virtual ~ShaderCompiler();

        void StopThread();

        void AddCompilationRequestForShaderKey(ShaderKey shaderKey);

        // Returns false if the ShaderKey isn't compiled yet, in which case the blob returned are from the error ShaderKey that corresponds to the
        // ShaderKey passed
        shipBool GetRawShadersForShaderKey(ShaderKey shaderKey, ShaderDatabase::ShaderEntrySet& compiledShaderEntrySet, shipBool& gotRecompiledSinceLastAccess);

        void SetShaderDirectoryName(const StringT& shaderDirectoryName);
        const StringT& GetShaderDirectoryName() const { return m_ShaderDirectoryName; }

    public:
        struct SamplerStateToBeCompiled
        {
            StringA Name;
            StringA SamplerStateSource;
        };

    private:
        struct CompiledShaderKeyEntry
        {
            void Reset();

            ShaderKey::RawShaderKeyType m_RawShaderKey = 0;
            shipBool m_GotRecompiledSinceLastAccess = false;
            shipBool m_GotCompilationError = false;

            ID3D10Blob* m_CompiledVertexShaderBlob = nullptr;
            ID3D10Blob* m_CompiledPixelShaderBlob = nullptr;
            ID3D10Blob* m_CompiledComputeShaderBlob = nullptr;

            RenderStateBlock m_CompiledRenderStateBlock;

            InplaceArray<RootSignatureParameterEntry, 8> m_RootSignatureParameters;

            ShaderResourceBinder m_ShaderResourceBinder;

            InplaceArray<DescriptorSetEntryDeclaration, 8> m_DescriptorSetEntryDeclarations;

            InplaceArray<SamplerState, 4> m_SamplerStates;
        };

        struct ShaderInputReflectionData
        {
            SmallInplaceStringA Name;
            shipUint16 BindPoint;
            ShaderVisibility shaderVisibility;
        };

        struct ShaderReflectionData
        {
            InplaceArray<ShaderInputReflectionData, GfxConstants_MaxConstantBufferViewsBoundPerShaderStage> ConstantBufferReflectionDatas;
            InplaceArray<ShaderInputReflectionData, GfxConstants_MaxShaderResourceViewsBoundPerShaderStage> ShaderResourceViewReflectionDatas;
            InplaceArray<ShaderInputReflectionData, GfxConstants_MaxUnorderedAccessViewsBoundPerShaderStage> UnorderedAccessViewReflectionDatas;
            InplaceArray<ShaderInputReflectionData, GfxConstants_MaxSamplersBoundPerShaderStage> SamplerReflctionDatas;

            ShaderVisibility ConstantBuffersShaderVisibility = ShaderVisibility::ShaderVisibility_None;
            ShaderVisibility ShaderResourceViewsShaderVisibility = ShaderVisibility::ShaderVisibility_None;
            ShaderVisibility UnorderedAccessViewsShaderVisibility = ShaderVisibility::ShaderVisibility_None;
            ShaderVisibility SamplersShaderVisibility = ShaderVisibility::ShaderVisibility_None;
        };

        struct CompiledSamplerState
        {
            StringA Name;
            SamplerState State;
        };

    private:
        void ShaderCompilerThreadFunction();

        void CompileShaderFamily(ShaderFamily shaderFamily);

        void CompileShaderKey(const ShaderKey& shaderKeyToCompile);
        void CompileShaderKey(
                const ShaderKey& shaderKeyToCompile,
                const Array<ShaderOption>& everyPossibleShaderOptionForShaderKey,
                const StringT& sourceFilename,
                const StringA& shaderSource,
                const StringA& renderStateBlockSource,
                const Array<SamplerStateToBeCompiled>& samplerStatesToBeCompiled,
                const Array<ShaderInputProviderDeclaration*>& includedShaderInputProviders);

        ID3D10Blob* CompileVertexShaderForShaderKey(const StringT& sourceFilename, const StringA& source, _D3D_SHADER_MACRO* shaderOptionDefines);
        ID3D10Blob* CompilePixelShaderForShaderKey(const StringT& sourceFilename, const StringA& source, _D3D_SHADER_MACRO* shaderOptionDefines);
        ID3D10Blob* CompileComputeShaderForShaderKey(const StringT& sourceFilename, const StringA& source, _D3D_SHADER_MACRO* shaderOptionDefines);
        ID3D10Blob* CompileShader(const StringT& shaderSourceFilename, const StringA& shaderSource, const StringA& version, const StringA& mainName, _D3D_SHADER_MACRO* shaderOptionDefines);

        void GetReflectionDataForShader(ID3D10Blob* shaderBlob, ShaderReflectionData& shaderReflectionData, ShaderVisibility shaderVisibility) const;

        void FillRootSignatureEntriesForDescriptorRangeType(
                const Array<ShaderInputReflectionData>& shaderInputReflectionDatas,
                DescriptorRangeType descriptorRangeType,
                Array<RootSignatureParameterEntry>& rootSignatureParameters,
                shipUint16 rootIndexPerDescriptorRangeTypePerShaderStage[][shipUint32(ShaderStage::ShaderStage_Count)]) const;

        shipBool FillRootSignatureEntryForDescriptorRangeTypeForShaderStage(
                const Array<ShaderInputReflectionData>& shaderInputReflectionDatas,
                DescriptorRangeType descriptorRangeType,
                ShaderStage shaderStage,
                Array<RootSignatureParameterEntry>& rootSignatureParameters) const;

        void CreateShaderResourceBinder(
                const ShaderReflectionData& shaderReflectionData,
                const Array<RootSignatureParameterEntry>& rootSignatureParameters,
                const Array<ShaderInputProviderDeclaration*>& includedShaderInputProviders,
                const Array<CompiledSamplerState>& compiledSamplerStates,
                shipUint16 rootIndexPerDescriptorRangeTypePerShaderStage[][shipUint32(ShaderStage::ShaderStage_Count)],
                ShaderResourceBinder& shaderResourceBinder);

        void CreateShaderResourceBinderForDescriptorRangeType(
                const Array<ShaderInputReflectionData>& shaderInputReflectionDatas,
                const Array<RootSignatureParameterEntry>& rootSignatureParameters,
                const Array<ShaderInputProviderDeclaration*>& includedShaderInputProviders,
                shipUint16 rootIndexPerDescriptorRangeTypePerShaderStage[][shipUint32(ShaderStage::ShaderStage_Count)],
                DescriptorRangeType descriptorRangeType,
                ShaderResourceBinder& shaderResourceBinder);

        shipUint16 GetDescriptorRangeEntryIndex(
                const ShaderInputReflectionData& shaderInputReflectionData,
                const Array<RootSignatureParameterEntry>& rootSignatureParameters,
                DescriptorRangeType descriptorRangeType) const;

        CompiledShaderKeyEntry& GetCompiledShaderKeyEntry(ShaderKey::RawShaderKeyType rawShaderKey);

        std::thread m_ShaderCompilerThread;
        mutable std::mutex m_ShaderCompilationRequestLock;

        static volatile shipBool m_RunShaderCompilerThread;

        SmallInplaceStringT m_ShaderDirectoryName;

        Array<ShaderKey> m_ShaderKeysToCompile;
        ShaderKey m_CurrentShaderKeyBeingCompiled;

        Array<CompiledShaderKeyEntry> m_CompiledShaderKeyEntries;
    };
}