#include <common/shadercompiler/shadercompiler.h>

#include <common/shaderfamilies.h>
#include <common/shaderoptions.h>

#pragma warning( disable : 4005 )

#include <d3dcommon.h>
#include <d3dcompiler.h>

#pragma warning( default : 4005 )

namespace Shipyard
{;

volatile bool ShaderCompiler::m_RunShaderCompilerThread = true;

extern StringA g_ShaderFamilyFilenames[uint8_t(ShaderFamily::Count)];
extern uint8_t g_NumBitsForShaderOption[uint32_t(ShaderOption::Count)];
extern StringA g_ShaderOptionString[uint32_t(ShaderOption::Count)];

class ShaderCompilerIncludeHandler : public ID3DInclude
{
public:
    STDMETHOD(Open)(D3D_INCLUDE_TYPE includeType, const char* includeFilename, const void* parentData, const void** outData, UINT* outByteLength)
    {
        StringT filename = ((includeType == D3D_INCLUDE_LOCAL) ? (StringT(ShaderCompiler::GetInstance().GetShaderDirectoryName()) + includeFilename) : includeFilename);

        FileHandler includeFile(filename, FileHandlerOpenFlag::FileHandlerOpenFlag_Read);
        if (!includeFile.IsOpen())
        {
            return E_FAIL;
        }

        includeFile.ReadWholeFile(m_Data);

        *outByteLength = UINT(m_Data.Size());
        *outData = m_Data.GetBuffer();

        return S_OK;
    }

    STDMETHOD(Close)(const void* data)
    {
        m_Data.Clear();

        return S_OK;
    }

    StringA m_Data;
};

ShaderCompiler::ShaderCompiler()
    : m_ShaderDirectoryName(".\\shaders\\")
{
    m_CurrentShaderKeyBeingCompiled.SetShaderFamily(ShaderFamily::Count);
    
    ShaderKey::InitializeShaderKeyGroups();

    // Make sure the ShaderFamily error is compiled initialy
    CompileShaderFamily(ShaderFamily::Error);

    m_ShaderCompilerThread = std::thread(&ShaderCompiler::ShaderCompilerThreadFunction, this);
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

void ShaderCompiler::SetShaderDirectoryName(const StringT& shaderDirectoryName)
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

// Reads a shader file and separates it into the shader source and, if any entry available, the render state pipeline source.
bool ReadShaderFile(const StringT& sourceFilename, StringA& shaderSource, StringA& renderStatePipelineSource)
{
    // This is kind of ugly: if a file is saved inside of Visual Studio with the AutoRecover feature enabled, it will
    // first save the file's content in a temporary file, and then copy the content to the real file before quickly deleting the
    // temporary file. This means that, sometimes, when saving a file through Visual Studio, we won't be able to open it as
    // the file descriptor is already opened by Visual Studio. To counter that, we try a few times to open the same file.
    FileHandler shaderFile(sourceFilename, FileHandlerOpenFlag::FileHandlerOpenFlag_Read);
    if (!shaderFile.IsOpen())
    {
        constexpr uint32_t timesToTryOpeningAgain = 5;

        uint32_t i = 0;
        for (; i < timesToTryOpeningAgain; i++)
        {
            Sleep(100);

            shaderFile.Open(sourceFilename.GetBuffer(), FileHandlerOpenFlag::FileHandlerOpenFlag_Read);
            if (shaderFile.IsOpen())
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

    shaderFile.ReadWholeFile(shaderSource);

    return (shaderSource.Size() > 0);
}

void ShaderCompiler::CompileShaderFamily(ShaderFamily shaderFamily)
{
    const StringA& shaderFamilyFilename = g_ShaderFamilyFilenames[uint32_t(shaderFamily)];

    StringA sourceFilename = m_ShaderDirectoryName + shaderFamilyFilename;
    StringA shaderSource;
    StringA renderStatePipelineSource;

    bool couldReadShaderFile = ReadShaderFile(sourceFilename, shaderSource, renderStatePipelineSource);
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

        CompileShaderKey(currentShaderKeyToCompile, shaderOptions, sourceFilename, shaderSource);

        shaderOptionAsInt -= 1;
    }
}

void ShaderCompiler::CompileShaderKey(const ShaderKey& shaderKeyToCompile)
{
    const StringA& shaderFamilyFilename = g_ShaderFamilyFilenames[uint32_t(shaderKeyToCompile.GetShaderFamily())];

    StringA sourceFilename = m_ShaderDirectoryName + shaderFamilyFilename;
    StringA shaderSource;
    StringA renderStatePipelineSource;

    bool couldReadShaderFile = ReadShaderFile(sourceFilename, shaderSource, renderStatePipelineSource);
    if (!couldReadShaderFile)
    {
        return;
    }

    Array<ShaderOption> everyPossibleShaderOptionForShaderKey;
    ShaderKey::GetShaderKeyOptionsForShaderFamily(shaderKeyToCompile.GetShaderFamily(), everyPossibleShaderOptionForShaderKey);

    CompileShaderKey(shaderKeyToCompile, everyPossibleShaderOptionForShaderKey, sourceFilename, shaderSource);
}

void ShaderCompiler::CompileShaderKey(
        const ShaderKey& shaderKeyToCompile,
        const Array<ShaderOption>& everyPossibleShaderOptionForShaderKey,
        const StringT& sourceFilename,
        const StringA& source)
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
        shaderDefine.Name = g_ShaderOptionString[uint32_t(shaderOption)].GetBuffer();

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

ID3D10Blob* ShaderCompiler::CompileVertexShaderForShaderKey(const StringT& sourceFilename, const StringA& source, D3D_SHADER_MACRO* shaderOptionDefines)
{
    static const StringA vertexShaderEntryPoint = "VS_Main";

    if (source.FindIndexOfFirst(vertexShaderEntryPoint, 0) == StringA::InvalidIndex)
    {
        return nullptr;
    }

    return CompileShader(sourceFilename, source, "vs_5_0", vertexShaderEntryPoint, shaderOptionDefines);
}

ID3D10Blob* ShaderCompiler::CompilePixelShaderForShaderKey(const StringT& sourceFilename, const StringA& source, D3D_SHADER_MACRO* shaderOptionDefines)
{
    static const StringA pixelShaderEntryPoint = "PS_Main";

    if (source.FindIndexOfFirst(pixelShaderEntryPoint, 0) == StringA::InvalidIndex)
    {
        return nullptr;
    }

    return CompileShader(sourceFilename, source, "ps_5_0", pixelShaderEntryPoint, shaderOptionDefines);
}

ID3D10Blob* ShaderCompiler::CompileComputeShaderForShaderKey(const StringT& sourceFilename, const StringA& source, D3D_SHADER_MACRO* shaderOptionDefines)
{
    static const StringA computeShaderEntryPoint = "CS_Main";

    if (source.FindIndexOfFirst(computeShaderEntryPoint, 0) == StringA::InvalidIndex)
    {
        return nullptr;
    }

    return CompileShader(sourceFilename, source, "cs_5_0", computeShaderEntryPoint, shaderOptionDefines);
}

ID3D10Blob* ShaderCompiler::CompileShader(const StringT& shaderSourceFilename, const StringA& shaderSource, const StringA& version, const StringA& mainName, D3D_SHADER_MACRO* shaderOptionDefines)
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

    HRESULT tmp = D3DPreprocess(shaderSource.GetBuffer(), shaderSource.Size(), shaderSourceFilename.GetBuffer(), shaderOptionDefines, &shaderCompilerIncludeHandler, &preprocessedBlob, &preprocessError);
    if (FAILED(tmp))
    {
        if (preprocessError != nullptr)
        {
            char* errorMsg = (char*)preprocessError->GetBufferPointer();
            OutputDebugString(errorMsg);
        }
    }

    HRESULT hr = D3DCompile(shaderSource.GetBuffer(), shaderSource.Size(), shaderSourceFilename.GetBuffer(), shaderOptionDefines, &shaderCompilerIncludeHandler, mainName.GetBuffer(), version.GetBuffer(), flags, 0, &shaderBlob, &error);
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