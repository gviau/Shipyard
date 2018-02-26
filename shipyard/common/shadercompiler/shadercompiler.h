#pragma once

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

        // Returns false if the ShaderKey isn't compiled yet, in which case the blob returned are from the error ShaderKey that corresponds to the
        // ShaderKey passed
        bool GetShaderBlobsForShaderKey(ShaderKey shaderKey, ID3D10Blob*& vertexShaderBlob, ID3D10Blob*& pixelShaderBlob, ID3D10Blob*& computeShaderBlob, bool& gotRecompiledSinceLastAccess);

        void RequestCompilationFromShaderFiles(const String& directoryName, const Array<String>& shaderFilenames);

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

        void CompileShaderKey(ShaderKey::RawShaderKeyType rawShaderKey, const Array<ShaderOption>& everyPossibleShaderOptionForShaderKey, const String& source);
        ID3D10Blob* CompileVertexShaderForShaderKey(const String& source, _D3D_SHADER_MACRO* shaderOptionDefines);
        ID3D10Blob* CompilePixelShaderForShaderKey(const String& source, _D3D_SHADER_MACRO* shaderOptionDefines);
        ID3D10Blob* CompileComputeShaderForShaderKey(const String& source, _D3D_SHADER_MACRO* shaderOptionDefines);
        ID3D10Blob* CompileShader(const String& shaderSource, const String& version, const String& mainName, _D3D_SHADER_MACRO* shaderOptionDefines);

        CompiledShaderKeyEntry& GetCompiledShaderKeyEntry(ShaderKey::RawShaderKeyType rawShaderKey);

        thread m_ShaderCompilerThread;
        mutable mutex m_ShaderCompilationRequestLock;

        static volatile bool m_RunShaderCompilerThread;

        String m_ShaderDirectoryName;

        Array<ShaderFamily> m_ShaderFamiliesToCompile;
        ShaderFamily m_CurrentShaderFamilyBeingCompiled;
        bool m_RecompileCurrentRequest;

        Array<CompiledShaderKeyEntry> m_CompiledShaderKeyEntries;
    };
}