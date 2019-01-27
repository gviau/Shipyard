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

    for (CompiledShaderKeyEntry& compiledShaderKeyEntry : m_CompiledShaderKeyEntries)
    {
        compiledShaderKeyEntry.Reset();
    }
}

void ShaderCompiler::AddCompilationRequestForShaderKey(ShaderKey shaderKey)
{
    ShaderFamily shaderFamily = shaderKey.GetShaderFamily();

    m_ShaderCompilationRequestLock.lock();

    if (shaderFamily != m_CurrentShaderFamilyBeingCompiled && !m_ShaderFamiliesToCompile.Exists(shaderFamily))
    {
        m_ShaderFamiliesToCompile.Add(shaderFamily);
    }

    m_ShaderCompilationRequestLock.unlock();
}

bool ShaderCompiler::GetRawShadersForShaderKey(ShaderKey shaderKey, ShaderDatabase::ShaderEntrySet& compiledShaderEntrySet, bool& gotRecompiledSinceLastAccess)
{
    ShaderFamily shaderFamily = shaderKey.GetShaderFamily();

    ShaderKey errorShaderKey;
    errorShaderKey.SetShaderFamily(ShaderFamily::Error);

    gotRecompiledSinceLastAccess = false;

    bool isShaderCompiled = true;

    m_ShaderCompilationRequestLock.lock();

    if (shaderFamily == m_CurrentShaderFamilyBeingCompiled || m_ShaderFamiliesToCompile.Exists(shaderFamily))
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
            for (const ShaderFamilyModificationEntry& shaderFamilyModificationEntry : m_ShaderFamilyModificationEntries)
            {
                if (shaderFamilyModificationEntry.shaderFamily == shaderFamily)
                {
                    compiledShaderEntrySet.lastModifiedTimestamp = shaderFamilyModificationEntry.lastModifiedTimestamp;
                    break;
                }
            }

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

uint64_t ShaderCompiler::GetTimestampForShaderKey(const ShaderKey& shaderKey) const
{
    ShaderFamily shaderFamily = shaderKey.GetShaderFamily();

    uint64_t lastModifiedTimestamp = 0;

    m_ShaderCompilationRequestLock.lock();

    for (const ShaderFamilyModificationEntry& shaderFamilyModificationEntry : m_ShaderFamilyModificationEntries)
    {
        if (shaderFamilyModificationEntry.shaderFamily == shaderFamily)
        {
            lastModifiedTimestamp = shaderFamilyModificationEntry.lastModifiedTimestamp;
            break;
        }
    }

    m_ShaderCompilationRequestLock.unlock();

    return lastModifiedTimestamp;
}

void ShaderCompiler::RequestCompilationFromShaderFiles(const Array<ShaderFileCompilationRequest>& shaderFileCompilationRequests)
{
    m_ShaderCompilationRequestLock.lock();

    for (const ShaderFileCompilationRequest& shaderFileCompilationRequest : shaderFileCompilationRequests)
    {
        const String& shaderFilename = shaderFileCompilationRequest.shaderFilename;

        if (shaderFilename.substr(shaderFilename.size() - 3) == ".fx")
        {
            AddCompilationRequestForFxFile(shaderFileCompilationRequest);
        }
        else if (shaderFilename.substr(shaderFilename.size() - 5) == ".hlsl")
        {
            AddCompilationRequestForHlslFile(shaderFileCompilationRequest);
        }
    }

    m_ShaderCompilationRequestLock.unlock();
}

void ShaderCompiler::SetShaderDirectoryName(const String& shaderDirectoryName)
{
    m_ShaderDirectoryName = shaderDirectoryName;

    // Make sure the ShaderFamily error is compiled initialy
    CompileShaderFamily(ShaderFamily::Error);
}

void ShaderCompiler::AddCompilationRequestForFxFile(const ShaderFileCompilationRequest& shaderFileCompilationRequest)
{
    const String& fxFilename = shaderFileCompilationRequest.shaderFilename;

    ShaderFamily shaderFamilyToCompile = ShaderFamily::Count;

    for (uint32_t i = 0; i < uint32_t(ShaderFamily::Count); i++)
    {
        if (fxFilename == g_ShaderFamilyFilenames[i])
        {
            shaderFamilyToCompile = ShaderFamily(i);
            break;
        }
    }

    if (shaderFamilyToCompile == ShaderFamily::Count)
    {
        return;
    }

    AddShaderFamilyCompilationRequest(shaderFamilyToCompile, shaderFileCompilationRequest.modificationTimestamp);
}

void ShaderCompiler::AddCompilationRequestForHlslFile(const ShaderFileCompilationRequest& shaderFileCompilationRequest)
{
    // For Hlsl files, we need to inspect every FX file and check whether they reference that Hlsl file. Moreoever, we also have to inspect
    // every Hlsl files to see if they reference the touched Hlsl file.
    const String& hlslFilename = shaderFileCompilationRequest.shaderFilename;
    
    for (uint32_t i = 0; i < uint32_t(ShaderFamily::Count); i++)
    {
        ShaderFamily shaderFamily = ShaderFamily(i);

        // The error shader family will never be recompiled at runtime, since it's supposed to be always available as a fallback
        if (shaderFamily == ShaderFamily::Error)
        {
            continue;
        }

        String shaderFamilyFilename = g_ShaderFamilyFilenames[i];

        if (CheckFileForHlslReference(shaderFamilyFilename, hlslFilename))
        {
            AddShaderFamilyCompilationRequest(shaderFamily, shaderFileCompilationRequest.modificationTimestamp);
        }
    }
}

bool ShaderCompiler::CheckFileForHlslReference(const String& filenameToCheck, const String& touchedHlslFilename) const
{
    ifstream file(m_ShaderDirectoryName + filenameToCheck);
    if (!file.is_open())
    {
        return false;
    }

    String filenameToCheckContent;
    file.ignore((std::numeric_limits<std::streamsize>::max)());
    filenameToCheckContent.resize((size_t)file.gcount());
    file.clear();
    file.seekg(0, std::ios::beg);

    file.read(&filenameToCheckContent[0], filenameToCheckContent.length());

    Array<String> includeDirectives;
    GetIncludeDirectives(filenameToCheckContent, includeDirectives);

    bool immediateReference = includeDirectives.Exists(touchedHlslFilename);
    
    if (immediateReference)
    {
        return true;
    }

    for (const String& includeDirective : includeDirectives)
    {
        if (CheckFileForHlslReference(includeDirective, touchedHlslFilename))
        {
            return true;
        }
    }

    return false;
}

void ShaderCompiler::GetIncludeDirectives(const String& fileContent, Array<String>& includeDirectives) const
{
    size_t currentOffset = 0;
    size_t findPosition = 0;

    // For now, it is assumed that all #include directives are one per line, never in a comment, and with a single whitespace between the #include and the file name
    static const String searchString = "#include ";

    while (true)
    {
        findPosition = fileContent.find(searchString, currentOffset);
        if (findPosition == String::npos)
        {
            break;
        }

        String includeFilename = "";
        char currentChar = '\0';

        currentOffset = findPosition + searchString.length();

        bool startCollectingFilename = false;

        do 
        {
            currentChar = fileContent[currentOffset++];
            if (currentChar == '\"')
            {
                if (startCollectingFilename)
                {
                    startCollectingFilename = false;
                    break;
                }
                else
                {
                    startCollectingFilename = true;
                }
            }
            else if (startCollectingFilename)
            {
                includeFilename += currentChar;
            }

        } while (currentChar != '\n');

        if (!startCollectingFilename)
        {
            includeDirectives.Add(includeFilename);
        }
    }
}

void ShaderCompiler::AddShaderFamilyCompilationRequest(ShaderFamily shaderFamilyToCompile, uint64_t modificationTimestamp)
{
    // The error shader family will never be recompiled at runtime, since it's supposed to be always available as a fallback
    if (shaderFamilyToCompile == ShaderFamily::Error)
    {
        return;
    }

    for (ShaderFamilyModificationEntry& shaderFamiliyModificationEntry : m_ShaderFamilyModificationEntries)
    {
        if (shaderFamiliyModificationEntry.shaderFamily == shaderFamilyToCompile)
        {
            shaderFamiliyModificationEntry.lastModifiedTimestamp = modificationTimestamp;
            return;
        }
    }

    ShaderFamilyModificationEntry& newShaderFamilyModificationEntry = m_ShaderFamilyModificationEntries.Grow();
    newShaderFamilyModificationEntry.shaderFamily = shaderFamilyToCompile;
    newShaderFamilyModificationEntry.lastModifiedTimestamp = modificationTimestamp;
}

void ShaderCompiler::ShaderCompilerThreadFunction()
{
    while (m_RunShaderCompilerThread)
    {
        uint32_t shaderFamilyToCompileIndex = 0;

        if (m_ShaderFamiliesToCompile.Size() > 0 && m_CurrentShaderFamilyBeingCompiled == ShaderFamily::Count)
        {
            m_ShaderCompilationRequestLock.lock();

            m_CurrentShaderFamilyBeingCompiled = m_ShaderFamiliesToCompile.Back();
            shaderFamilyToCompileIndex = (m_ShaderFamiliesToCompile.Size() - 1);

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

            m_ShaderFamiliesToCompile.RemoveAt(shaderFamilyToCompileIndex);
        }

        m_RecompileCurrentRequest = false;

        m_ShaderCompilationRequestLock.unlock();
    }
}

void ShaderCompiler::CompileShaderFamily(ShaderFamily shaderFamily)
{
    const String& shaderFamilyFilename = g_ShaderFamilyFilenames[uint32_t(shaderFamily)];

    String sourceFilename = m_ShaderDirectoryName + shaderFamilyFilename;

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
            return;
        }
    }

    String source;
    shaderFile.ignore((std::numeric_limits<std::streamsize>::max)());
    source.resize((size_t)shaderFile.gcount());
    shaderFile.clear();
    shaderFile.seekg(0, std::ios::beg);

    shaderFile.read(&source[0], source.length());

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
        ShaderKey::RawShaderKeyType rawShaderKey = baseRawShaderKey | (shaderOptionAsInt << ShaderKey::ms_ShaderOptionShift);

        CompileShaderKey(rawShaderKey, shaderOptions, sourceFilename, source);

        shaderOptionAsInt -= 1;
    }
}

void ShaderCompiler::CompileShaderKey(ShaderKey::RawShaderKeyType rawShaderKey, const Array<ShaderOption>& everyPossibleShaderOptionForShaderKey, const String& sourceFilename, const String& source)
{
    m_ShaderCompilationRequestLock.lock();

    CompiledShaderKeyEntry& compiledShaderKeyEntry = GetCompiledShaderKeyEntry(rawShaderKey);
    compiledShaderKeyEntry.Reset();

    ShaderKey shaderKey;
    shaderKey.m_RawShaderKey = rawShaderKey;

    Array<D3D_SHADER_MACRO> shaderOptionDefines;
    shaderOptionDefines.Reserve(everyPossibleShaderOptionForShaderKey.Size());

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