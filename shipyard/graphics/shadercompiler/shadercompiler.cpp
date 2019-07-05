#include <graphics/shadercompiler/shadercompiler.h>

#include <graphics/shader/shaderfamilies.h>
#include <graphics/shader/shaderoptions.h>

#include <graphics/shadercompiler/renderstateblockcompiler.h>

#pragma warning( disable : 4005 )

#include <d3dcommon.h>
#include <d3dcompiler.h>

#pragma warning( default : 4005 )

namespace Shipyard
{;

const shipChar* ShaderCompiler::RenderStateBlockName = "RenderState";

volatile shipBool ShaderCompiler::m_RunShaderCompilerThread = true;

extern const shipChar* g_ShaderFamilyFilenames[shipUint8(ShaderFamily::Count)];
extern shipUint8 g_NumBitsForShaderOption[shipUint32(ShaderOption::Count)];
extern const shipChar* g_ShaderOptionString[shipUint32(ShaderOption::Count)];

class ShaderCompilerIncludeHandler : public ID3DInclude
{
public:
    STDMETHOD(Open)(D3D_INCLUDE_TYPE includeType, const shipChar* includeFilename, const void* parentData, const void** outData, UINT* outByteLength)
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

    // Make sure the ShaderFamily error is compiled initialy
    CompileShaderFamily(ShaderFamily::Error);

    m_ShaderCompilerThread = std::thread(&ShaderCompiler::ShaderCompilerThreadFunction, this);
}

ShaderCompiler::~ShaderCompiler()
{
    StopThread();
}

void ShaderCompiler::StopThread()
{
    m_RunShaderCompilerThread = false;

    if (m_ShaderCompilerThread.native_handle() != nullptr)
    {
        m_ShaderCompilerThread.join();
    }

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

shipBool ShaderCompiler::GetRawShadersForShaderKey(ShaderKey shaderKey, ShaderDatabase::ShaderEntrySet& compiledShaderEntrySet, shipBool& gotRecompiledSinceLastAccess)
{
    ShaderKey errorShaderKey;
    errorShaderKey.SetShaderFamily(ShaderFamily::Error);

    gotRecompiledSinceLastAccess = false;

    shipBool isShaderCompiled = true;

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
                compiledShaderEntrySet.rawVertexShader = (shipUint8*)shaderKeyEntry.m_CompiledVertexShaderBlob->GetBufferPointer();
                compiledShaderEntrySet.rawVertexShaderSize = shaderKeyEntry.m_CompiledVertexShaderBlob->GetBufferSize();
            }

            if (shaderKeyEntry.m_CompiledPixelShaderBlob != nullptr)
            {
                compiledShaderEntrySet.rawPixelShader = (shipUint8*)shaderKeyEntry.m_CompiledPixelShaderBlob->GetBufferPointer();
                compiledShaderEntrySet.rawPixelShaderSize = shaderKeyEntry.m_CompiledPixelShaderBlob->GetBufferSize();
            }

            if (shaderKeyEntry.m_CompiledComputeShaderBlob != nullptr)
            {
                compiledShaderEntrySet.rawComputeShader = (shipUint8*)shaderKeyEntry.m_CompiledComputeShaderBlob->GetBufferPointer();
                compiledShaderEntrySet.rawComputeShaderSize = shaderKeyEntry.m_CompiledComputeShaderBlob->GetBufferSize();
            }

            compiledShaderEntrySet.renderStateBlock = shaderKeyEntry.m_CompiledRenderStateBlock;

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
        Sleep(100);

        shipUint32 shaderFamilyToCompileIndex = 0;

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

shipBool SplitShaderSourceAndRenderStateBlock(StringA& shaderSource, StringA& renderStateBlockSource)
{
    size_t renderStateBlockStartIndex = shaderSource.FindIndexOfFirstCaseInsensitive(ShaderCompiler::RenderStateBlockName, 0);
    if (renderStateBlockStartIndex != shaderSource.InvalidIndex)
    {
        // Make sure it's not in comments
        size_t inlineCommentBeforeRenderStateBlockStartIndex = shaderSource.FindIndexOfFirstReverse("//", renderStateBlockStartIndex);
        shipBool isRenderStateBlockInComment = (inlineCommentBeforeRenderStateBlockStartIndex != shaderSource.InvalidIndex);

        if (isRenderStateBlockInComment)
        {
            size_t newlineAfterInlineCommentIndex = shaderSource.FindIndexOfFirst('\n', inlineCommentBeforeRenderStateBlockStartIndex);
            if (newlineAfterInlineCommentIndex == shaderSource.InvalidIndex || newlineAfterInlineCommentIndex < renderStateBlockStartIndex)
            {
                isRenderStateBlockInComment = false;
            }
        }
        else
        {
            size_t blockCommentBeforeRenderStateBlockStartIndex = shaderSource.FindIndexOfFirstReverse("/*", renderStateBlockStartIndex);
            if (blockCommentBeforeRenderStateBlockStartIndex != shaderSource.InvalidIndex)
            {
                size_t blockCommentEndIndex = shaderSource.FindIndexOfFirst("*/", blockCommentBeforeRenderStateBlockStartIndex);
                if (blockCommentEndIndex != shaderSource.InvalidIndex && blockCommentEndIndex > renderStateBlockStartIndex)
                {
                    isRenderStateBlockInComment = true;
                }
            }
        }

        if (isRenderStateBlockInComment)
        {
            return false;
        }

        size_t openingBracketIndex = shaderSource.FindIndexOfFirst('{', renderStateBlockStartIndex);
        if (openingBracketIndex == shaderSource.InvalidIndex)
        {
            return false;
        }

        shipUint32 bracketCount = 1;
        size_t endingBracketIndex = 0;

        for (endingBracketIndex = openingBracketIndex + 1; (bracketCount > 0 || endingBracketIndex < shaderSource.Size()); endingBracketIndex++)
        {
            if (shaderSource[endingBracketIndex] == '{')
            {
                bracketCount += 1;
            }
            else if (shaderSource[endingBracketIndex] == '}')
            {
                bracketCount -= 1;
            }
        }

        renderStateBlockSource = shaderSource.Substring(openingBracketIndex + 1, (endingBracketIndex - openingBracketIndex - 1));
        shaderSource.Erase(renderStateBlockStartIndex, (endingBracketIndex - renderStateBlockStartIndex));

        return true;
    }

    return true;
}

// Reads a shader file and separates it into the shader source and, if any entry available, the render state pipeline source.
shipBool ReadShaderFile(const StringT& sourceFilename, StringA& shaderSource, StringA& renderStateBlockSource)
{
    // This is kind of ugly: if a file is saved inside of Visual Studio with the AutoRecover feature enabled, it will
    // first save the file's content in a temporary file, and then copy the content to the real file before quickly deleting the
    // temporary file. This means that, sometimes, when saving a file through Visual Studio, we won't be able to open it as
    // the file descriptor is already opened by Visual Studio. To counter that, we try a few times to open the same file.
    FileHandler shaderFile(sourceFilename, FileHandlerOpenFlag::FileHandlerOpenFlag_Read);
    if (!shaderFile.IsOpen())
    {
        constexpr shipUint32 timesToTryOpeningAgain = 5;

        shipUint32 i = 0;
        for (; i < timesToTryOpeningAgain; i++)
        {
            Sleep(100);

            shaderFile.Open(sourceFilename.GetBuffer(), FileHandlerOpenFlag::FileHandlerOpenFlag_Read);
            if (shaderFile.IsOpen())
            {
                break;
            }
        }

        shipBool couldntOpenFile = (i == timesToTryOpeningAgain);
        if (couldntOpenFile)
        {
            return false;
        }
    }

    shaderFile.ReadWholeFile(shaderSource);

    if (shaderSource.IsEmpty())
    {
        return false;
    }

    SplitShaderSourceAndRenderStateBlock(shaderSource, renderStateBlockSource);

    return true;
}

void ShaderCompiler::CompileShaderFamily(ShaderFamily shaderFamily)
{
    SmallInplaceStringT sourceFilename = m_ShaderDirectoryName;
    sourceFilename += g_ShaderFamilyFilenames[shipUint32(shaderFamily)];

    StringA shaderSource;
    LargeInplaceStringA renderStateBlockSource;

    shipBool couldReadShaderFile = ReadShaderFile(sourceFilename, shaderSource, renderStateBlockSource);
    if (!couldReadShaderFile)
    {
        return;
    }

    Array<ShaderOption> everyPossibleShaderOption;
    ShaderKey::GetShaderKeyOptionsForShaderFamily(shaderFamily, everyPossibleShaderOption);

    shipUint32 numBitsInShaderKey = 0;
    for (ShaderOption shaderOption : everyPossibleShaderOption)
    {
        numBitsInShaderKey += shipUint32(g_NumBitsForShaderOption[shipUint32(shaderOption)]);
    }

    shipUint32 everyShaderOptionSet = ((1 << numBitsInShaderKey) - 1);

    shipUint32 possibleNumberOfPermutations = everyShaderOptionSet + 1;

    ShaderKey shaderKey;
    shaderKey.SetShaderFamily(shaderFamily);

    ShaderKey::RawShaderKeyType baseRawShaderKey = shaderKey.GetRawShaderKey();

    // Go through every permutation of shader options
    shipUint32 shaderOptionAsInt = everyShaderOptionSet;

    for (shipUint32 i = 0; i < possibleNumberOfPermutations; i++)
    {
        ShaderKey currentShaderKeyToCompile;
        currentShaderKeyToCompile.m_RawShaderKey = (baseRawShaderKey | (shaderOptionAsInt << ShaderKey::ms_ShaderOptionShift));

        CompileShaderKey(currentShaderKeyToCompile, everyPossibleShaderOption, sourceFilename, shaderSource, renderStateBlockSource);

        shaderOptionAsInt -= 1;
    }
}

void ShaderCompiler::CompileShaderKey(const ShaderKey& shaderKeyToCompile)
{
    SmallInplaceStringT sourceFilename = m_ShaderDirectoryName;
    sourceFilename += g_ShaderFamilyFilenames[shipUint32(shaderKeyToCompile.GetShaderFamily())];

    StringA shaderSource;
    LargeInplaceStringA renderStateBlockSource;

    shipBool couldReadShaderFile = ReadShaderFile(sourceFilename, shaderSource, renderStateBlockSource);
    if (!couldReadShaderFile)
    {
        return;
    }

    Array<ShaderOption> everyPossibleShaderOptionForShaderKey;
    ShaderKey::GetShaderKeyOptionsForShaderFamily(shaderKeyToCompile.GetShaderFamily(), everyPossibleShaderOptionForShaderKey);

    CompileShaderKey(shaderKeyToCompile, everyPossibleShaderOptionForShaderKey, sourceFilename, shaderSource, renderStateBlockSource);
}

void ShaderCompiler::CompileShaderKey(
        const ShaderKey& shaderKeyToCompile,
        const Array<ShaderOption>& everyPossibleShaderOptionForShaderKey,
        const StringT& sourceFilename,
        const StringA& shaderSource,
        const StringA& renderStateBlockSource)
{
    m_ShaderCompilationRequestLock.lock();

    CompiledShaderKeyEntry& compiledShaderKeyEntry = GetCompiledShaderKeyEntry(shaderKeyToCompile.GetRawShaderKey());
    compiledShaderKeyEntry.Reset();

    Array<D3D_SHADER_MACRO> shaderOptionDefines;
    shaderOptionDefines.Reserve(everyPossibleShaderOptionForShaderKey.Size());

    for (ShaderOption shaderOption : everyPossibleShaderOptionForShaderKey)
    {
        shipUint32 valueForShaderOption = shaderKeyToCompile.GetShaderOptionValue(shaderOption);

        D3D_SHADER_MACRO shaderDefine;
        shaderDefine.Name = g_ShaderOptionString[shipUint32(shaderOption)];

        shipChar* buf = reinterpret_cast<shipChar*>(SHIP_ALLOC(8, 1));
        sprintf_s(buf, 8, "%u", valueForShaderOption);

        shaderDefine.Definition = buf;

        shaderOptionDefines.Add(shaderDefine);
    }

    D3D_SHADER_MACRO nullShaderDefine = { nullptr, nullptr };
    shaderOptionDefines.Add(nullShaderDefine);

    ID3D10Blob* vertexShaderBlob = CompileVertexShaderForShaderKey(sourceFilename, shaderSource, &shaderOptionDefines[0]);
    ID3D10Blob* pixelShaderBlob = CompilePixelShaderForShaderKey(sourceFilename, shaderSource, &shaderOptionDefines[0]);
    ID3D10Blob* computeShaderBlob = CompileComputeShaderForShaderKey(sourceFilename, shaderSource, &shaderOptionDefines[0]);

    for (D3D_SHADER_MACRO& shaderMacro : shaderOptionDefines)
    {
        SHIP_FREE(shaderMacro.Definition);
    }

    compiledShaderKeyEntry.m_GotCompilationError = ((vertexShaderBlob != nullptr && pixelShaderBlob == nullptr) || (pixelShaderBlob != nullptr && vertexShaderBlob == nullptr) || (vertexShaderBlob == nullptr && pixelShaderBlob == nullptr && computeShaderBlob == nullptr));

    if (!compiledShaderKeyEntry.m_GotCompilationError)
    {
        compiledShaderKeyEntry.m_CompiledVertexShaderBlob = vertexShaderBlob;
        compiledShaderKeyEntry.m_CompiledPixelShaderBlob = pixelShaderBlob;
        compiledShaderKeyEntry.m_CompiledComputeShaderBlob = computeShaderBlob;
        compiledShaderKeyEntry.m_GotRecompiledSinceLastAccess = true;
    }

    RenderStateBlock renderStateBlock;
    RenderStateBlockCompilationError renderStateBlockCompilationError = CompileRenderStateBlock(
            shaderKeyToCompile,
            everyPossibleShaderOptionForShaderKey,
            renderStateBlockSource,
            renderStateBlock);

    if (renderStateBlockCompilationError == RenderStateBlockCompilationError::NoError)
    {
        compiledShaderKeyEntry.m_CompiledRenderStateBlock = renderStateBlock;
    }

    m_ShaderCompilationRequestLock.unlock();
}

ID3D10Blob* ShaderCompiler::CompileVertexShaderForShaderKey(const StringT& sourceFilename, const StringA& source, D3D_SHADER_MACRO* shaderOptionDefines)
{
    static const shipChar* vertexShaderEntryPoint = "VS_Main";

    if (source.FindIndexOfFirst(vertexShaderEntryPoint, 0) == StringA::InvalidIndex)
    {
        return nullptr;
    }

    return CompileShader(sourceFilename, source, "vs_5_0", vertexShaderEntryPoint, shaderOptionDefines);
}

ID3D10Blob* ShaderCompiler::CompilePixelShaderForShaderKey(const StringT& sourceFilename, const StringA& source, D3D_SHADER_MACRO* shaderOptionDefines)
{
    static const shipChar* pixelShaderEntryPoint = "PS_Main";

    if (source.FindIndexOfFirst(pixelShaderEntryPoint, 0) == StringA::InvalidIndex)
    {
        return nullptr;
    }

    return CompileShader(sourceFilename, source, "ps_5_0", pixelShaderEntryPoint, shaderOptionDefines);
}

ID3D10Blob* ShaderCompiler::CompileComputeShaderForShaderKey(const StringT& sourceFilename, const StringA& source, D3D_SHADER_MACRO* shaderOptionDefines)
{
    static const shipChar* computeShaderEntryPoint = "CS_Main";

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
            shipChar* errorMsg = (shipChar*)preprocessError->GetBufferPointer();
            OutputDebugString(errorMsg);
        }
    }

    HRESULT hr = D3DCompile(shaderSource.GetBuffer(), shaderSource.Size(), shaderSourceFilename.GetBuffer(), shaderOptionDefines, &shaderCompilerIncludeHandler, mainName.GetBuffer(), version.GetBuffer(), flags, 0, &shaderBlob, &error);
    if (FAILED(hr))
    {
        if (error != nullptr)
        {
            shipChar* errorMsg = (shipChar*)error->GetBufferPointer();
            OutputDebugString(errorMsg);

            shipChar* data = (shipChar*)preprocessedBlob->GetBufferPointer();
            OutputDebugString(data);
        }

        return nullptr;
    }

    return shaderBlob;
}

ShaderCompiler::CompiledShaderKeyEntry& ShaderCompiler::GetCompiledShaderKeyEntry(ShaderKey::RawShaderKeyType rawShaderKey)
{
    shipUint32 idx = 0;
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

    m_CompiledRenderStateBlock = RenderStateBlock();
}

}