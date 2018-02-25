#pragma once

#include <common/shaderkey.h>

#include <system/array.h>
#include <system/platform.h>
#include <system/string.h>

#include <mutex>
#include <thread>
using namespace std;

struct ID3D10Blob;

namespace Shipyard
{
    enum class ShaderFamily : uint8_t;

    class SHIPYARD_API ShaderCompiler
    {
    public:
        ShaderCompiler();
        ~ShaderCompiler();

        void RequestCompilationFromShaderFiles(const String& directoryName, const Array<String>& shaderFilenames);

    private:
        struct CompiledShaderKeyEntry
        {
            CompiledShaderKeyEntry()
                : m_CompiledVertexShaderBlob(nullptr)
                , m_CompiledPixelShaderBlob(nullptr)
                , m_CompiledComputeShaderBlob(nullptr)
            {}

            ShaderKey m_ShaderKey;
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
        mutex m_ShaderCompilationRequestLock;
        mutex m_CurrentShaderFamilyBeingCompiledLock;

        static volatile bool m_RunShaderCompilerThread;

        String m_ShaderDirectoryName;

        Array<ShaderFamily> m_ShaderFamiliesToCompile;
        ShaderFamily m_CurrentShaderFamilyBeingCompiled;
        bool m_RecompileCurrentRequest;

        Array<CompiledShaderKeyEntry> m_CompiledShaderKeyEntries;
    };
}