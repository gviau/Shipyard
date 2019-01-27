#pragma once

#include <common/shaderdatabase.h>
#include <common/shaderkey.h>

#include <system/array.h>
#include <system/platform.h>
#include <system/singleton.h>
#include <system/string.h>

#include <mutex>
#include <thread>
using namespace std;

struct _D3D_SHADER_MACRO;
struct ID3D10Blob;

namespace Shipyard
{
    enum class ShaderFamily : uint8_t;
    enum class ShaderOption : uint32_t;

    class SHIPYARD_API ShaderCompiler : public Singleton<ShaderCompiler>
    {
        friend class Singleton<ShaderCompiler>;

    public:
        struct ShaderFileCompilationRequest
        {
            String shaderFilename;
            uint64_t modificationTimestamp;
        };

    public:
        ShaderCompiler();
        virtual ~ShaderCompiler();

        void AddCompilationRequestForShaderKey(ShaderKey shaderKey);

        // Returns false if the ShaderKey isn't compiled yet, in which case the blob returned are from the error ShaderKey that corresponds to the
        // ShaderKey passed
        bool GetRawShadersForShaderKey(ShaderKey shaderKey, ShaderDatabase::ShaderEntrySet& compiledShaderEntrySet, bool& gotRecompiledSinceLastAccess);

        uint64_t GetTimestampForShaderKey(const ShaderKey& shaderKey) const;

        void RequestCompilationFromShaderFiles(const Array<ShaderFileCompilationRequest>& shaderFileCompilationRequests);

        void SetShaderDirectoryName(const String& shaderDirectoryName);
        const String& GetShaderDirectoryName() const { return m_ShaderDirectoryName; }

    private:
        struct CompiledShaderKeyEntry
        {
            CompiledShaderKeyEntry()
                : m_RawShaderKey(0)
                , m_GotRecompiledSinceLastAccess(false)
                , m_GotCompilationError(false)
                , m_CompiledVertexShaderBlob(nullptr)
                , m_CompiledPixelShaderBlob(nullptr)
                , m_CompiledComputeShaderBlob(nullptr)
            {}

            void Reset();

            ShaderKey::RawShaderKeyType m_RawShaderKey;
            bool m_GotRecompiledSinceLastAccess;
            bool m_GotCompilationError;

            ID3D10Blob* m_CompiledVertexShaderBlob;
            ID3D10Blob* m_CompiledPixelShaderBlob;
            ID3D10Blob* m_CompiledComputeShaderBlob;
        };

        struct ShaderFamilyModificationEntry
        {
            ShaderFamily shaderFamily;
            uint64_t lastModifiedTimestamp;
        };

    private:
        void AddCompilationRequestForFxFile(const ShaderFileCompilationRequest& shaderFileCompilationRequest);
        void AddCompilationRequestForHlslFile(const ShaderFileCompilationRequest& shaderFileCompilationRequest);
        bool CheckFileForHlslReference(const String& filenameToCheck, const String& touchedHlslFilename) const;
        void GetIncludeDirectives(const String& fileContent, Array<String>& includeDirectives) const;
        void AddShaderFamilyCompilationRequest(ShaderFamily shaderFamilyToCompile, uint64_t modificationTimestamp);

        void ShaderCompilerThreadFunction();

        void CompileShaderFamily(ShaderFamily shaderFamily);

        void CompileShaderKey(ShaderKey::RawShaderKeyType rawShaderKey, const Array<ShaderOption>& everyPossibleShaderOptionForShaderKey, const String& sourceFilename, const String& source);
        ID3D10Blob* CompileVertexShaderForShaderKey(const String& sourceFilename, const String& source, _D3D_SHADER_MACRO* shaderOptionDefines);
        ID3D10Blob* CompilePixelShaderForShaderKey(const String& sourceFilename, const String& source, _D3D_SHADER_MACRO* shaderOptionDefines);
        ID3D10Blob* CompileComputeShaderForShaderKey(const String& sourceFilename, const String& source, _D3D_SHADER_MACRO* shaderOptionDefines);
        ID3D10Blob* CompileShader(const String& shaderSourceFilename, const String& shaderSource, const String& version, const String& mainName, _D3D_SHADER_MACRO* shaderOptionDefines);

        CompiledShaderKeyEntry& GetCompiledShaderKeyEntry(ShaderKey::RawShaderKeyType rawShaderKey);

        thread m_ShaderCompilerThread;
        mutable mutex m_ShaderCompilationRequestLock;

        static volatile bool m_RunShaderCompilerThread;

        String m_ShaderDirectoryName;

        Array<ShaderFamily> m_ShaderFamiliesToCompile;
        ShaderFamily m_CurrentShaderFamilyBeingCompiled;
        bool m_RecompileCurrentRequest;

        Array<ShaderFamilyModificationEntry> m_ShaderFamilyModificationEntries;
        Array<CompiledShaderKeyEntry> m_CompiledShaderKeyEntries;
    };
}