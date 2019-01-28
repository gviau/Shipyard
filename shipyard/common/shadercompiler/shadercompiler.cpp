#include <common/shadercompiler/shadercompiler.h>

#include <common/shaderfamilies.h>
#include <common/shaderoptions.h>

#pragma warning( disable : 4005 )

#include <d3dcommon.h>
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

class ShaderCompilerIncludeHandler : public ID3DInclude
{
public:
    STDMETHOD(Open)(D3D_INCLUDE_TYPE includeType, const char* includeFilename, const void* parentData, const void** outData, UINT* outByteLength)
    {
        String filename = ((includeType == D3D_INCLUDE_LOCAL) ? (String(ShaderCompiler::GetInstance().GetShaderDirectoryName()) + includeFilename) : includeFilename);

        ifstream includeFile(filename);
        if (!includeFile.is_open())
        {
            return E_FAIL;
        }

        includeFile.ignore((std::numeric_limits<std::streamsize>::max)());
        size_t includeFileContentLength = size_t(includeFile.gcount());
        includeFile.clear();
        includeFile.seekg(0, std::ios::beg);

        char* data = new char[includeFileContentLength];

        includeFile.read(data, includeFileContentLength);

        *outData = data;
        *outByteLength = UINT(includeFileContentLength);

        return S_OK;
    }

    STDMETHOD(Close)(const void* data)
    {
        char* charData = (char*)data;
        delete[] charData;

        return S_OK;
    }
};

ShaderCompiler::ShaderCompiler()
    : m_ShaderDirectoryName(".\\shaders\\")
{
    m_CurrentShaderKeyBeingCompiled.SetShaderFamily(ShaderFamily::Count);
    
    ShaderKey::InitializeShaderKeyGroups();

    // Make sure the ShaderFamily error is compiled initialy
    CompileShaderFamily(ShaderFamily::Error);

    m_ShaderCompilerThread = thread(&ShaderCompiler::ShaderCompilerThreadFunction, this);
}

ShaderCompiler::~ShaderCompiler()
{
    m_RunShaderCompilerThread = false;
    m_ShaderCompilerThread.join();

    for (CompiledShaderKeyEntry& compiledShaderKeyEntry : m_CompiledShaderKeyEntries)
    {
        compiledShaderKeyEntry.Reset();
    }
}

void ShaderCompiler::AddCompilationRequestForShaderKey(ShaderKey shaderKey)
{
    m_ShaderCompilationRequestLock.lock();

    if (shaderKey != m_CurrentShaderKeyBeingCompiled && !m_ShaderKeysToCompile.Exists(shaderKey))
    {
        m_ShaderKeysToCompile.Add(shaderKey);
    }

    m_ShaderCompilationRequestLock.unlock();
}

bool ShaderCompiler::GetRawShadersForShaderKey(ShaderKey shaderKey, ShaderDatabase::ShaderEntrySet& compiledShaderEntrySet, bool& gotRecompiledSinceLastAccess)
{
    ShaderKey errorShaderKey;
    errorShaderKey.SetShaderFamily(ShaderFamily::Error);

    gotRecompiledSinceLastAccess = false;

    bool isShaderCompiled = true;

    m_ShaderCompilationRequestLock.lock();

    if (shaderKey == m_CurrentShaderKeyBeingCompiled || m_ShaderKeysToCompile.Exists(shaderKey))
    {
        shaderKey = errorShaderKey;
        isShaderCompiled = false;
    }

    if (isShaderCompiled)
    {
        CompiledShaderKeyEntry& shaderKeyEntry = GetCompiledShaderKeyEntry(shaderKey.GetRawShaderKey());
        if (shaderKeyEntry.m_GotCompilationError)
        {
            isShaderCompiled = false;
        }
        else
        {
            if (shaderKeyEntry.m_CompiledVertexShaderBlob != nullptr)
            {
                compiledShaderEntrySet.rawVertexShader = (uint8_t*)shaderKeyEntry.m_CompiledVertexShaderBlob->GetBufferPointer();
                compiledShaderEntrySet.rawVertexShaderSize = shaderKeyEntry.m_CompiledVertexShaderBlob->GetBufferSize();
            }

            if (shaderKeyEntry.m_CompiledPixelShaderBlob != nullptr)
            {
                compiledShaderEntrySet.rawPixelShader = (uint8_t*)shaderKeyEntry.m_CompiledPixelShaderBlob->GetBufferPointer();
                compiledShaderEntrySet.rawPixelShaderSize = shaderKeyEntry.m_CompiledPixelShaderBlob->GetBufferSize();
            }

            if (shaderKeyEntry.m_CompiledComputeShaderBlob != nullptr)
            {
                compiledShaderEntrySet.rawComputeShader = (uint8_t*)shaderKeyEntry.m_CompiledComputeShaderBlob->GetBufferPointer();
                compiledShaderEntrySet.rawComputeShaderSize = shaderKeyEntry.m_CompiledComputeShaderBlob->GetBufferSize();
            }

            gotRecompiledSinceLastAccess = shaderKeyEntry.m_GotRecompiledSinceLastAccess;

            shaderKeyEntry.m_GotRecompiledSinceLastAccess = false;
        }
    }
    m_ShaderCompilationRequestLock.unlock();

    return isShaderCompiled;
}

void ShaderCompiler::SetShaderDirectoryName(const String& shaderDirectoryName)
{
    m_ShaderDirectoryName = shaderDirectoryName;

    // Make sure the ShaderFamily error is compiled initialy
    CompileShaderFamily(ShaderFamily::Error);
}

void ShaderCompiler::ShaderCompilerThreadFunction()
{
    while (m_RunShaderCompilerThread)
    {
        uint32_t shaderFamilyToCompileIndex = 0;

        if (m_ShaderKeysToCompile.Size() > 0 && m_CurrentShaderKeyBeingCompiled.GetShaderFamily() == ShaderFamily::Count)
        {
            m_ShaderCompilationRequestLock.lock();

            m_CurrentShaderKeyBeingCompiled = m_ShaderKeysToCompile.Back();
            shaderFamilyToCompileIndex = (m_ShaderKeysToCompile.Size() - 1);

            m_ShaderCompilationRequestLock.unlock();
        }

        if (m_CurrentShaderKeyBeingCompiled.GetShaderFamily() == ShaderFamily::Count)
        {
            continue;
        }

        CompileShaderKey(m_CurrentShaderKeyBeingCompiled);

        m_ShaderCompilationRequestLock.lock();

        m_CurrentShaderKeyBeingCompiled.SetShaderFamily(ShaderFamily::Count);

        m_ShaderKeysToCompile.RemoveAt(shaderFamilyToCompileIndex);

        m_ShaderCompilationRequestLock.unlock();
    }
}

bool ShaderCompiler::ReadShaderFile(const String& sourceFilename, String& source) const
{
    // This is kind of ugly: if a file is saved inside of Visual Studio with the AutoRecover feature enabled, it will
    // first save the file's content in a temporary file, and then copy the content to the real file before quickly deleting the
    // temporary file. This means that, sometimes, when saving a file through Visual Studio, we won't be able to open it as
    // the file descriptor is already opened by Visual Studio. To counter that, we try a few times to open the same file.
    ifstream shaderFile(sourceFilename);
    if (!shaderFile.is_open())
    {
        constexpr uint32_t timesToTryOpeningAgain = 5;

        uint32_t i = 0;
        for (; i < timesToTryOpeningAgain; i++)
        {
            Sleep(100);

            shaderFile.open(sourceFilename, ios_base::in);
            if (shaderFile.is_open())
            {
                break;
            }
        }

        bool couldntOpenFile = (i == timesToTryOpeningAgain);
        if (couldntOpenFile)
        {
            return false;
        }
    }

    shaderFile.ignore((std::numeric_limits<std::streamsize>::max)());
    source.resize((size_t)shaderFile.gcount());
    shaderFile.clear();
    shaderFile.seekg(0, std::ios::beg);

    if (source.size() == 0)
    {
        return false;
    }

    shaderFile.read(&source[0], source.length());

    return true;
}

void ShaderCompiler::CompileShaderFamily(ShaderFamily shaderFamily)
{
    const String& shaderFamilyFilename = g_ShaderFamilyFilenames[uint32_t(shaderFamily)];

    String sourceFilename = m_ShaderDirectoryName + shaderFamilyFilename;
    String source;

    bool couldReadShaderFile = ReadShaderFile(sourceFilename, source);
    if (!couldReadShaderFile)
    {
        return;
    }

    Array<ShaderOption> shaderOptions;
    ShaderKey::GetShaderKeyOptionsForShaderFamily(shaderFamily, shaderOptions);

    uint32_t numBitsInShaderKey = 0;
    for (ShaderOption shaderOption : shaderOptions)
    {
        numBitsInShaderKey += uint32_t(g_NumBitsForShaderOption[uint32_t(shaderOption)]);
    }

    uint32_t everyShaderOptionSet = ((1 << numBitsInShaderKey) - 1);

    uint32_t possibleNumberOfPermutations = everyShaderOptionSet + 1;

    ShaderKey shaderKey;
    shaderKey.SetShaderFamily(shaderFamily);

    ShaderKey::RawShaderKeyType baseRawShaderKey = shaderKey.GetRawShaderKey();

    // Go through every permutation of shader options
    uint32_t shaderOptionAsInt = everyShaderOptionSet;

    for (uint32_t i = 0; i < possibleNumberOfPermutations; i++)
    {
        ShaderKey currentShaderKeyToCompile;
        currentShaderKeyToCompile.m_RawShaderKey = (baseRawShaderKey | (shaderOptionAsInt << ShaderKey::ms_ShaderOptionShift));

        CompileShaderKey(currentShaderKeyToCompile, shaderOptions, sourceFilename, source);

        shaderOptionAsInt -= 1;
    }
}

void ShaderCompiler::CompileShaderKey(const ShaderKey& shaderKeyToCompile)
{
    const String& shaderFamilyFilename = g_ShaderFamilyFilenames[uint32_t(shaderKeyToCompile.GetShaderFamily())];

    String sourceFilename = m_ShaderDirectoryName + shaderFamilyFilename;
    String source;

    bool couldReadShaderFile = ReadShaderFile(sourceFilename, source);
    if (!couldReadShaderFile)
    {
        return;
    }

    Array<ShaderOption> everyPossibleShaderOptionForShaderKey;
    ShaderKey::GetShaderKeyOptionsForShaderFamily(shaderKeyToCompile.GetShaderFamily(), everyPossibleShaderOptionForShaderKey);

    CompileShaderKey(shaderKeyToCompile, everyPossibleShaderOptionForShaderKey, sourceFilename, source);
}

void ShaderCompiler::CompileShaderKey(
        const ShaderKey& shaderKeyToCompile,
        const Array<ShaderOption>& everyPossibleShaderOptionForShaderKey,
        const string& sourceFilename,
        const string& source)
{
    m_ShaderCompilationRequestLock.lock();

    CompiledShaderKeyEntry& compiledShaderKeyEntry = GetCompiledShaderKeyEntry(shaderKeyToCompile.GetRawShaderKey());
    compiledShaderKeyEntry.Reset();

    Array<D3D_SHADER_MACRO> shaderOptionDefines;
    shaderOptionDefines.Reserve(everyPossibleShaderOptionForShaderKey.Size());

    for (ShaderOption shaderOption : everyPossibleShaderOptionForShaderKey)
    {
        uint32_t valueForShaderOption = shaderKeyToCompile.GetShaderOptionValue(shaderOption);

        D3D_SHADER_MACRO shaderDefine;
        shaderDefine.Name = g_ShaderOptionString[uint32_t(shaderOption)].c_str();

        char* buf = new char[8];
        sprintf_s(buf, 8, "%u", valueForShaderOption);

        shaderDefine.Definition = buf;

        shaderOptionDefines.Add(shaderDefine);
    }

    D3D_SHADER_MACRO nullShaderDefine = { nullptr, nullptr };
    shaderOptionDefines.Add(nullShaderDefine);

    ID3D10Blob* vertexShaderBlob = CompileVertexShaderForShaderKey(sourceFilename, source, &shaderOptionDefines[0]);
    ID3D10Blob* pixelShaderBlob = CompilePixelShaderForShaderKey(sourceFilename, source, &shaderOptionDefines[0]);
    ID3D10Blob* computeShaderBlob = CompileComputeShaderForShaderKey(sourceFilename, source, &shaderOptionDefines[0]);

    for (D3D_SHADER_MACRO& shaderMacro : shaderOptionDefines)
    {
        delete[] shaderMacro.Definition;
    }

    compiledShaderKeyEntry.m_GotCompilationError = ((vertexShaderBlob != nullptr && pixelShaderBlob == nullptr) || (pixelShaderBlob != nullptr && vertexShaderBlob == nullptr) || (vertexShaderBlob == nullptr && pixelShaderBlob == nullptr && computeShaderBlob == nullptr));

    if (!compiledShaderKeyEntry.m_GotCompilationError)
    {
        compiledShaderKeyEntry.m_CompiledVertexShaderBlob = vertexShaderBlob;
        compiledShaderKeyEntry.m_CompiledPixelShaderBlob = pixelShaderBlob;
        compiledShaderKeyEntry.m_CompiledComputeShaderBlob = computeShaderBlob;
        compiledShaderKeyEntry.m_GotRecompiledSinceLastAccess = true;
    }

    m_ShaderCompilationRequestLock.unlock();
}

ID3D10Blob* ShaderCompiler::CompileVertexShaderForShaderKey(const String& sourceFilename, const String& source, D3D_SHADER_MACRO* shaderOptionDefines)
{
    static const String vertexShaderEntryPoint = "VS_Main";

    if (source.find(vertexShaderEntryPoint) == String::npos)
    {
        return nullptr;
    }

    return CompileShader(sourceFilename, source, "vs_5_0", vertexShaderEntryPoint, shaderOptionDefines);
}

ID3D10Blob* ShaderCompiler::CompilePixelShaderForShaderKey(const String& sourceFilename, const String& source, D3D_SHADER_MACRO* shaderOptionDefines)
{
    static const String pixelShaderEntryPoint = "PS_Main";

    if (source.find(pixelShaderEntryPoint) == String::npos)
    {
        return nullptr;
    }

    return CompileShader(sourceFilename, source, "ps_5_0", pixelShaderEntryPoint, shaderOptionDefines);
}

ID3D10Blob* ShaderCompiler::CompileComputeShaderForShaderKey(const String& sourceFilename, const String& source, D3D_SHADER_MACRO* shaderOptionDefines)
{
    static const String computeShaderEntryPoint = "CS_Main";

    if (source.find(computeShaderEntryPoint) == String::npos)
    {
        return nullptr;
    }

    return CompileShader(sourceFilename, source, "cs_5_0", computeShaderEntryPoint, shaderOptionDefines);
}

ID3D10Blob* ShaderCompiler::CompileShader(const String& shaderSourceFilename, const String& shaderSource, const String& version, const String& mainName, D3D_SHADER_MACRO* shaderOptionDefines)
{
    ID3D10Blob* shaderBlob = nullptr;
    ID3D10Blob* error = nullptr;

    unsigned int flags = 0;

#ifdef _DEBUG
    flags = D3DCOMPILE_DEBUG;
#endif

    ShaderCompilerIncludeHandler shaderCompilerIncludeHandler;

    ID3D10Blob* preprocessedBlob = nullptr;
    ID3D10Blob* preprocessError = nullptr;

    HRESULT tmp = D3DPreprocess(shaderSource.c_str(), shaderSource.size(), shaderSourceFilename.c_str(), shaderOptionDefines, &shaderCompilerIncludeHandler, &preprocessedBlob, &preprocessError);
    if (FAILED(tmp))
    {
        if (preprocessError != nullptr)
        {
            char* errorMsg = (char*)preprocessError->GetBufferPointer();
            OutputDebugString(errorMsg);
        }
    }

    HRESULT hr = D3DCompile(shaderSource.c_str(), shaderSource.size(), shaderSourceFilename.c_str(), shaderOptionDefines, &shaderCompilerIncludeHandler, mainName.c_str(), version.c_str(), flags, 0, &shaderBlob, &error);
    if (FAILED(hr))
    {
        if (error != nullptr)
        {
            char* errorMsg = (char*)error->GetBufferPointer();
            OutputDebugString(errorMsg);

            char* data = (char*)preprocessedBlob->GetBufferPointer();
            OutputDebugString(data);
        }

        return nullptr;
    }

    return shaderBlob;
}

ShaderCompiler::CompiledShaderKeyEntry& ShaderCompiler::GetCompiledShaderKeyEntry(ShaderKey::RawShaderKeyType rawShaderKey)
{
    uint32_t idx = 0;
    for (; idx < m_CompiledShaderKeyEntries.Size(); idx++)
    {
        if (m_CompiledShaderKeyEntries[idx].m_RawShaderKey == rawShaderKey)
        {
            break;
        }
    }

    if (idx == m_CompiledShaderKeyEntries.Size())
    {
        CompiledShaderKeyEntry newCompiledShaderKeyEntry;
        newCompiledShaderKeyEntry.m_RawShaderKey = rawShaderKey;
        newCompiledShaderKeyEntry.m_GotCompilationError = true;

        m_CompiledShaderKeyEntries.Add(newCompiledShaderKeyEntry);
    }

    return m_CompiledShaderKeyEntries[idx];
}

void ShaderCompiler::CompiledShaderKeyEntry::Reset()
{
    if (m_CompiledVertexShaderBlob != nullptr)
    {
        m_CompiledVertexShaderBlob->Release();
        m_CompiledVertexShaderBlob = nullptr;
    }

    if (m_CompiledPixelShaderBlob != nullptr)
    {
        m_CompiledPixelShaderBlob->Release();
        m_CompiledPixelShaderBlob = nullptr;
    }

    if (m_CompiledComputeShaderBlob != nullptr)
    {
        m_CompiledComputeShaderBlob->Release();
        m_CompiledComputeShaderBlob = nullptr;
    }
}

}