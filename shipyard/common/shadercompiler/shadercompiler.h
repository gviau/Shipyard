#pragma once

#include <common/shaderkey.h>

#include <system/array.h>
#include <system/platform.h>
#include <system/singleton.h>
#include <system/string.h>

#include <mutex>
#include <thread>
using namespace std;

struct ID3D10Blob;

namespace Shipyard
{
    enum class ShaderFamily : uint8_t;

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
                : m_GotRecompiledSinceLastAccess(false)
                , m_GotCompilationError(false)
                , m_CompiledVertexShaderBlob(nullptr)
                , m_CompiledPixelShaderBlob(nullptr)
                , m_CompiledComputeShaderBlob(nullptr)
            {}

            ShaderKey m_ShaderKey;
            bool m_GotRecompiledSinceLastAccess;
            bool m_GotCompilationError;

            ID3D10Blob* m_CompiledVertexShaderBlob;
            ID3D10Blob* m_CompiledPixelShaderBlob;
            ID3D10Blob* m_CompiledComputeShaderBlob;
        };

    private:
        void ShaderCompilerThreadFunction();

        void CompileShaderFamily(ShaderFamily shaderFamily);

        void CompileShaderKey(ShaderKey shaderKey, const String& source);
        ID3D10Blob* CompileVertexShaderForShaderKey(ShaderKey shaderKey, const String& source);
        ID3D10Blob* CompilePixelShaderForShaderKey(ShaderKey shaderKey, const String& source);
        ID3D10Blob* CompileComputeShaderForShaderKey(ShaderKey shaderKey, const String& source);
        ID3D10Blob* CompileShaderForShaderKey(ShaderKey shaderKey, const String& shaderSource, const String& version, const String& mainName);

        CompiledShaderKeyEntry& GetCompiledShaderKeyEntry(ShaderKey shaderKey);

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