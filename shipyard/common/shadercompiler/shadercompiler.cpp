#include <common/shadercompiler/shadercompiler.h>

#include <common/shaderfamilies.h>
#include <common/shaderoptions.h>

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
extern uint8_t g_NumBitsForShaderOption[uint32_t(ShaderOption::Count)];
extern String g_ShaderOptionString[uint32_t(ShaderOption::Count)];

ShaderCompiler::ShaderCompiler()
    : m_ShaderDirectoryName(".\\shaders\\")
{
    m_CurrentShaderFamilyBeingCompiled = ShaderFamily::Count;
    m_RecompileCurrentRequest = false;

    ShaderKey::InitializeShaderKeyGroups();

    // Make sure the ShaderFamily error is compiled initialy
    CompileShaderFamily(ShaderFamily::Error);

    m_ShaderCompilerThread = thread(&ShaderCompiler::ShaderCompilerThreadFunction, this);
}

ShaderCompiler::~ShaderCompiler()
{
    m_RunShaderCompilerThread = false;
    m_ShaderCompilerThread.join();
}

bool ShaderCompiler::GetShaderBlobsForShaderKey(ShaderKey shaderKey, ID3D10Blob*& vertexShaderBlob, ID3D10Blob*& pixelShaderBlob, ID3D10Blob*& computeShaderBlob, bool& gotRecompiledSinceLastAccess)
{
    ShaderFamily shaderFamily = shaderKey.GetShaderFamily();

    ShaderKey errorShaderKey;
    errorShaderKey.SetShaderFamily(ShaderFamily::Error);

    vertexShaderBlob = nullptr;
    pixelShaderBlob = nullptr;
    computeShaderBlob = nullptr;
    gotRecompiledSinceLastAccess = false;

    bool isShaderCompiled = true;

    m_ShaderCompilationRequestLock.lock();

    if (shaderFamily == m_CurrentShaderFamilyBeingCompiled || find(m_ShaderFamiliesToCompile.begin(), m_ShaderFamiliesToCompile.end(), shaderFamily) != m_ShaderFamiliesToCompile.end())
    {
        shaderKey = errorShaderKey;
        isShaderCompiled = false;
    }

    CompiledShaderKeyEntry& shaderKeyEntry = GetCompiledShaderKeyEntry(shaderKey.GetRawShaderKey());
    if (shaderKeyEntry.m_GotCompilationError)
    {
        isShaderCompiled = false;
    }

    vertexShaderBlob = shaderKeyEntry.m_CompiledVertexShaderBlob;
    pixelShaderBlob = shaderKeyEntry.m_CompiledPixelShaderBlob;
    computeShaderBlob = shaderKeyEntry.m_CompiledComputeShaderBlob;
    gotRecompiledSinceLastAccess = shaderKeyEntry.m_GotRecompiledSinceLastAccess;

    if (isShaderCompiled)
    {
        shaderKeyEntry.m_GotRecompiledSinceLastAccess = false;
    }

    m_ShaderCompilationRequestLock.unlock();

    return isShaderCompiled;
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
        // The error shader family will never be recompiled at runtime, since it's supposed to be always available as a fallback
        if (shaderFamilyToCompile == ShaderFamily::Count || shaderFamilyToCompile == ShaderFamily::Error)
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

    Array<ShaderOption> shaderOptions;
    ShaderKey::GetShaderKeyOptionsForShaderFamily(shaderFamily, shaderOptions);

    uint32_t shaderOptionAsInt = 0;
    for (uint32_t i = shaderOptions.size(); i > 0; i--)
    {
        uint32_t idx = i - 1;

        uint32_t bitShift = uint32_t(g_NumBitsForShaderOption[uint32_t(shaderOptions[idx])]);

        shaderOptionAsInt <<= bitShift;

        shaderOptionAsInt |= ((1 << bitShift) - 1);
    }

    uint32_t possibleNumberOfPermutations = shaderOptionAsInt + 1;

    ShaderKey shaderKey;
    shaderKey.SetShaderFamily(shaderFamily);

    ShaderKey::RawShaderKeyType baseRawShaderKey = shaderKey.GetRawShaderKey();

    for (uint32_t i = 0; i < possibleNumberOfPermutations; i++)
    {
        ShaderKey::RawShaderKeyType rawShaderKey = baseRawShaderKey | (shaderOptionAsInt << ShaderKey::ms_ShaderOptionShift);

        CompileShaderKey(rawShaderKey, shaderOptions, source);

        shaderOptionAsInt -= 1;
    }
}

void ShaderCompiler::CompileShaderKey(ShaderKey::RawShaderKeyType rawShaderKey, const Array<ShaderOption>& everyPossibleShaderOptionForShaderKey, const String& source)
{
    CompiledShaderKeyEntry& compiledShaderKeyEntry = GetCompiledShaderKeyEntry(rawShaderKey);

    ShaderKey shaderKey;
    shaderKey.m_RawShaderKey = rawShaderKey;

    Array<D3D_SHADER_MACRO> shaderOptionDefines;
    shaderOptionDefines.reserve(everyPossibleShaderOptionForShaderKey.size());

    for (ShaderOption shaderOption : everyPossibleShaderOptionForShaderKey)
    {
        uint32_t valueForShaderOption = shaderKey.GetShaderOptionValue(shaderOption);
        if (valueForShaderOption == 0)
        {
            continue;
        }

        D3D_SHADER_MACRO shaderDefine;
        shaderDefine.Name = g_ShaderOptionString[uint32_t(shaderOption)].c_str();

        char* buf = new char[8];
        sprintf_s(buf, 8, "%u", valueForShaderOption);

        shaderDefine.Definition = buf;

        shaderOptionDefines.push_back(shaderDefine);
    }

    D3D_SHADER_MACRO nullShaderDefine = { nullptr, nullptr };
    shaderOptionDefines.push_back(nullShaderDefine);

    ID3D10Blob* vertexShaderBlob = CompileVertexShaderForShaderKey(source, &shaderOptionDefines[0]);
    ID3D10Blob* pixelShaderBlob = CompilePixelShaderForShaderKey(source, &shaderOptionDefines[0]);
    ID3D10Blob* computeShaderBlob = CompileComputeShaderForShaderKey( source, &shaderOptionDefines[0]);

    for (D3D_SHADER_MACRO& shaderMacro : shaderOptionDefines)
    {
        delete[] shaderMacro.Definition;
    }

    if (!m_RecompileCurrentRequest)
    {
        compiledShaderKeyEntry.m_GotCompilationError = ((vertexShaderBlob != nullptr && pixelShaderBlob == nullptr) || (pixelShaderBlob != nullptr && vertexShaderBlob == nullptr) || (vertexShaderBlob == nullptr && pixelShaderBlob == nullptr && computeShaderBlob == nullptr));

        if (!compiledShaderKeyEntry.m_GotCompilationError)
        {
            compiledShaderKeyEntry.m_CompiledVertexShaderBlob = vertexShaderBlob;
            compiledShaderKeyEntry.m_CompiledPixelShaderBlob = pixelShaderBlob;
            compiledShaderKeyEntry.m_CompiledComputeShaderBlob = computeShaderBlob;
            compiledShaderKeyEntry.m_GotRecompiledSinceLastAccess = true;
        }
    }
}

ID3D10Blob* ShaderCompiler::CompileVertexShaderForShaderKey(const String& source, D3D_SHADER_MACRO* shaderOptionDefines)
{
    return CompileShader(source, "vs_5_0", "VS_Main", shaderOptionDefines);
}

ID3D10Blob* ShaderCompiler::CompilePixelShaderForShaderKey(const String& source, D3D_SHADER_MACRO* shaderOptionDefines)
{
    return CompileShader(source, "ps_5_0", "PS_Main", shaderOptionDefines);
}

ID3D10Blob* ShaderCompiler::CompileComputeShaderForShaderKey(const String& source, D3D_SHADER_MACRO* shaderOptionDefines)
{
    return CompileShader(source, "cs_5_0", "CS_Main", shaderOptionDefines);
}

ID3D10Blob* ShaderCompiler::CompileShader(const String& shaderSource, const String& version, const String& mainName, D3D_SHADER_MACRO* shaderOptionDefines)
{
    ID3D10Blob* shaderBlob = nullptr;
    ID3D10Blob* error = nullptr;

    unsigned int flags = 0;

#ifdef _DEBUG
    flags = D3DCOMPILE_DEBUG;
#endif

    HRESULT hr = D3DCompile(shaderSource.c_str(), shaderSource.size(), nullptr, shaderOptionDefines, nullptr, mainName.c_str(), version.c_str(), flags, 0, &shaderBlob, &error);
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

ShaderCompiler::CompiledShaderKeyEntry& ShaderCompiler::GetCompiledShaderKeyEntry(ShaderKey::RawShaderKeyType rawShaderKey)
{
    uint32_t idx = 0;
    for (; idx < m_CompiledShaderKeyEntries.size(); idx++)
    {
        if (m_CompiledShaderKeyEntries[idx].m_RawShaderKey == rawShaderKey)
        {
            break;
        }
    }

    if (idx == m_CompiledShaderKeyEntries.size())
    {
        CompiledShaderKeyEntry newCompiledShaderKeyEntry;
        newCompiledShaderKeyEntry.m_RawShaderKey = rawShaderKey;

        m_CompiledShaderKeyEntries.push_back(newCompiledShaderKeyEntry);
    }

    return m_CompiledShaderKeyEntries[idx];
}

}