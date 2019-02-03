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
        ShaderCompiler();
        virtual ~ShaderCompiler();

        void AddCompilationRequestForShaderKey(ShaderKey shaderKey);

        // Returns false if the ShaderKey isn't compiled yet, in which case the blob returned are from the error ShaderKey that corresponds to the
        // ShaderKey passed
        bool GetRawShadersForShaderKey(ShaderKey shaderKey, ShaderDatabase::ShaderEntrySet& compiledShaderEntrySet, bool& gotRecompiledSinceLastAccess);

        void SetShaderDirectoryName(const StringT& shaderDirectoryName);
        const StringT& GetShaderDirectoryName() const { return m_ShaderDirectoryName; }

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

    private:
        void ShaderCompilerThreadFunction();

        void CompileShaderFamily(ShaderFamily shaderFamily);

        void CompileShaderKey(const ShaderKey& shaderKeyToCompile);
        void CompileShaderKey(
                const ShaderKey& shaderKeyToCompile,
                const Array<ShaderOption>& everyPossibleShaderOptionForShaderKey,
                const StringT& sourceFilename,
                const StringA& source);

        ID3D10Blob* CompileVertexShaderForShaderKey(const StringT& sourceFilename, const StringA& source, _D3D_SHADER_MACRO* shaderOptionDefines);
        ID3D10Blob* CompilePixelShaderForShaderKey(const StringT& sourceFilename, const StringA& source, _D3D_SHADER_MACRO* shaderOptionDefines);
        ID3D10Blob* CompileComputeShaderForShaderKey(const StringT& sourceFilename, const StringA& source, _D3D_SHADER_MACRO* shaderOptionDefines);
        ID3D10Blob* CompileShader(const StringT& shaderSourceFilename, const StringA& shaderSource, const StringA& version, const StringA& mainName, _D3D_SHADER_MACRO* shaderOptionDefines);

        CompiledShaderKeyEntry& GetCompiledShaderKeyEntry(ShaderKey::RawShaderKeyType rawShaderKey);

        thread m_ShaderCompilerThread;
        mutable mutex m_ShaderCompilationRequestLock;

        static volatile bool m_RunShaderCompilerThread;

        StringT m_ShaderDirectoryName;

        Array<ShaderKey> m_ShaderKeysToCompile;
        ShaderKey m_CurrentShaderKeyBeingCompiled;

        Array<CompiledShaderKeyEntry> m_CompiledShaderKeyEntries;
    };
}