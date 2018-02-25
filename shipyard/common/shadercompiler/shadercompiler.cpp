#include <common/shadercompiler/shadercompiler.h>

#include <common/shaderfamilies.h>

#pragma warning( disable : 4005 )

#include <d3dcompiler.h>

#pragma warning( default : 4005 )

#include <iostream>
#include <fstream>
using namespace std;

namespace Shipyard
{;

volatile bool ShaderCompiler::m_RunShaderCompilerThread = true;

extern String g_ShaderFamilyFilenames[uint8_t(ShaderFamily::Count)];

ShaderCompiler::ShaderCompiler()
{
    m_CurrentShaderFamilyBeingCompiled = ShaderFamily::Count;
    m_RecompileCurrentRequest = false;

    ShaderKey::InitializeShaderKeyGroups();

    m_ShaderCompilerThread = thread(&ShaderCompiler::ShaderCompilerThreadFunction, this);
}

ShaderCompiler::~ShaderCompiler()
{
    m_RunShaderCompilerThread = false;
    m_ShaderCompilerThread.join();
}

void ShaderCompiler::RequestCompilationFromShaderFiles(const String& directoryName, const Array<String>& shaderFilenames)
{
    m_ShaderCompilationRequestLock.lock();

    for (const String& shaderFilename : shaderFilenames)
    {
        ShaderFamily shaderFamilyToCompile = ShaderFamily::Count;

        for (uint32_t i = 0; i < uint32_t(ShaderFamily::Count); i++)
        {
            if (shaderFilename == g_ShaderFamilyFilenames[i])
            {
                shaderFamilyToCompile = ShaderFamily(i);
                break;
            }
        }

        if (shaderFamilyToCompile == ShaderFamily::Count)
        {
            continue;
        }

        if (shaderFamilyToCompile == m_CurrentShaderFamilyBeingCompiled)
        {
            m_RecompileCurrentRequest = true;
        }

        // Ensure we don't have duplicates
        if (find(m_ShaderFamiliesToCompile.begin(), m_ShaderFamiliesToCompile.end(), shaderFamilyToCompile) != m_ShaderFamiliesToCompile.end())
        {
            continue;
        }

        m_ShaderFamiliesToCompile.push_back(shaderFamilyToCompile);
    }

    m_ShaderDirectoryName = directoryName;

    m_ShaderCompilationRequestLock.unlock();
}

void ShaderCompiler::ShaderCompilerThreadFunction()
{
    while (m_RunShaderCompilerThread)
    {
        uint32_t shaderFamilyToCompileIndex = 0;

        if (m_ShaderFamiliesToCompile.size() > 0 && m_CurrentShaderFamilyBeingCompiled == ShaderFamily::Count)
        {
            m_ShaderCompilationRequestLock.lock();

            m_CurrentShaderFamilyBeingCompiled = m_ShaderFamiliesToCompile.back();
            shaderFamilyToCompileIndex = (m_ShaderFamiliesToCompile.size() - 1);

            m_ShaderCompilationRequestLock.unlock();
        }

        if (m_CurrentShaderFamilyBeingCompiled == ShaderFamily::Count)
        {
            continue;
        }

        CompileShaderFamily(m_CurrentShaderFamilyBeingCompiled);

        m_ShaderCompilationRequestLock.lock();

        if (!m_RecompileCurrentRequest)
        {
            m_CurrentShaderFamilyBeingCompiled = ShaderFamily::Count;

            m_ShaderFamiliesToCompile.erase(m_ShaderFamiliesToCompile.begin() + shaderFamilyToCompileIndex);
        }

        m_RecompileCurrentRequest = false;

        m_ShaderCompilationRequestLock.unlock();
    }
}

void ShaderCompiler::CompileShaderFamily(ShaderFamily shaderFamily)
{
    Array<ShaderOption> shaderOptions;
    ShaderKey::GetShaderKeyOptionsForShaderFamily(shaderFamily, shaderOptions);

    ShaderKey shaderKey;
    shaderKey.SetShaderFamily(shaderFamily);

    const String& sourceFilename = g_ShaderFamilyFilenames[uint32_t(shaderFamily)];

    ifstream shaderFile(m_ShaderDirectoryName + sourceFilename);
    if (!shaderFile.is_open())
    {
        return;
    }

    String source;
    shaderFile.seekg(0, std::ios::end);
    source.resize((size_t)shaderFile.tellg());
    shaderFile.seekg(0, std::ios::beg);

    shaderFile.read(&source[0], source.length());

    CompileShaderKey(shaderKey, source);
}

void ShaderCompiler::CompileShaderKey(ShaderKey shaderKey, const String& source)
{
    CompiledShaderKeyEntry& compiledShaderKeyEntry = GetCompiledShaderKeyEntry(shaderKey);

    ID3D10Blob* vertexShaderBlob = CompileVertexShaderForShaderKey(shaderKey, source);
    ID3D10Blob* pixelShaderBlob = CompilePixelShaderForShaderKey(shaderKey, source);
    ID3D10Blob* computeShaderBlob = CompileComputeShaderForShaderKey(shaderKey, source);

    if (!m_RecompileCurrentRequest)
    {
        compiledShaderKeyEntry.m_CompiledVertexShaderBlob = vertexShaderBlob;
        compiledShaderKeyEntry.m_CompiledPixelShaderBlob = pixelShaderBlob;
        compiledShaderKeyEntry.m_CompiledComputeShaderBlob = computeShaderBlob;
    }
}

ID3D10Blob* ShaderCompiler::CompileVertexShaderForShaderKey(ShaderKey shaderKey, const String& source)
{
    return CompileShaderForShaderKey(shaderKey, source, "vs_5_0", "VS_Main");
}

ID3D10Blob* ShaderCompiler::CompilePixelShaderForShaderKey(ShaderKey shaderKey, const String& source)
{
    return CompileShaderForShaderKey(shaderKey, source, "ps_5_0", "PS_Main");
}

ID3D10Blob* ShaderCompiler::CompileComputeShaderForShaderKey(ShaderKey shaderKey, const String& source)
{
    return CompileShaderForShaderKey(shaderKey, source, "cs_5_0", "CS_Main");
}

ID3D10Blob* ShaderCompiler::CompileShaderForShaderKey(ShaderKey shaderKey, const String& shaderSource, const String& version, const String& mainName)
{
    ID3D10Blob* shaderBlob = nullptr;
    ID3D10Blob* error = nullptr;

    unsigned int flags = 0;

#ifdef _DEBUG
    flags = D3DCOMPILE_DEBUG;
#endif

    HRESULT hr = D3DCompile(shaderSource.c_str(), shaderSource.size(), nullptr, nullptr, nullptr, mainName.c_str(), version.c_str(), flags, 0, &shaderBlob, &error);
    if (FAILED(hr))
    {
        if (error != nullptr)
        {
            char* errorMsg = (char*)error->GetBufferPointer();
            cout << errorMsg << endl;
            // MessageBox(NULL, errorMsg, "DX11 error", MB_OK);
        }

        return nullptr;
    }

    return shaderBlob;
}

ShaderCompiler::CompiledShaderKeyEntry& ShaderCompiler::GetCompiledShaderKeyEntry(ShaderKey shaderKey)
{
    uint32_t idx = 0;
    for (; idx < m_CompiledShaderKeyEntries.size(); idx++)
    {
        if (m_CompiledShaderKeyEntries[idx].m_ShaderKey.GetRawShaderKey() == shaderKey.GetRawShaderKey())
        {
            break;
        }
    }

    if (idx == m_CompiledShaderKeyEntries.size())
    {
        CompiledShaderKeyEntry newCompiledShaderKeyEntry;
        newCompiledShaderKeyEntry.m_ShaderKey = shaderKey;

        m_CompiledShaderKeyEntries.push_back(newCompiledShaderKeyEntry);
    }

    return m_CompiledShaderKeyEntries[idx];
}

}