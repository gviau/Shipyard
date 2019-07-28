#include <graphics/shadercompiler/shadercompiler.h>

#include <graphics/shader/shaderfamilies.h>
#include <graphics/shader/shaderinputprovider.h>
#include <graphics/shader/shaderoptions.h>

#include <graphics/shadercompiler/renderstateblockcompiler.h>

#include <math/mathutilities.h>

#pragma warning( disable : 4005 )

#include <d3dcommon.h>
#include <d3dcompiler.h>
#include <d3d11shader.h>

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
            compiledShaderEntrySet.rootSignatureParameters = shaderKeyEntry.m_RootSignatureParameters;
            compiledShaderEntrySet.shaderResourceBinder = shaderKeyEntry.m_ShaderResourceBinder;
            compiledShaderEntrySet.descriptorSetEntryDeclarations = shaderKeyEntry.m_DescriptorSetEntryDeclarations;

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
    // Small hack: we need to compile the error shader family at the start, but if we include any files, we'll rely
    // on the include handler which will call ShaderCompiler::GetInstance().GetShaderDirectory, so we can't compile
    // the shader family in the constructor, since we wouldn't have had a change yet to set the instance.
    while (ms_Instance != nullptr);

    // Make sure the ShaderFamily error is compiled initially
    CompileShaderFamily(ShaderFamily::Error);

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
// Also returns the included shader input providers.
shipBool ReadShaderFile(
        const StringT& sourceFilename,
        StringA& shaderSource,
        StringA& renderStateBlockSource,
        Array<ShaderInputProviderDeclaration*>& includedShaderInputProviders)
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

    ShaderInputProviderManager& shaderInputProviderManager = GetShaderInputProviderManager();

    size_t currentPos = 0;
    const char* includeDirective = "#include \"shaderinputproviders";
    size_t includeDirectiveLength = strlen(includeDirective);

    do
    {
        currentPos = shaderSource.FindIndexOfFirstCaseInsensitive(includeDirective, currentPos);
        if (currentPos == shaderSource.InvalidIndex)
        {
            break;
        }

        size_t shaderInputProviderDeclarationIdx = currentPos + includeDirectiveLength;
        while (shaderSource[shaderInputProviderDeclarationIdx] == '\\' || shaderSource[shaderInputProviderDeclarationIdx] == '/')
        {
            shaderInputProviderDeclarationIdx += 1;
        }

        size_t endOfIncludeIdx = shaderSource.FindIndexOfFirst('.', shaderInputProviderDeclarationIdx);
        if (endOfIncludeIdx == shaderSource.InvalidIndex)
        {
            break;
        }

        StringA shaderInputProviderName = shaderSource.Substring(shaderInputProviderDeclarationIdx, (endOfIncludeIdx - shaderInputProviderDeclarationIdx));

        ShaderInputProviderDeclaration* shaderInputProvider = shaderInputProviderManager.FindShaderInputProviderDeclarationFromName(shaderInputProviderName);
        if (shaderInputProvider != nullptr)
        {
            includedShaderInputProviders.Add(shaderInputProvider);
        }

        currentPos = endOfIncludeIdx;
    } while (true);

    SplitShaderSourceAndRenderStateBlock(shaderSource, renderStateBlockSource);

    return true;
}

void ShaderCompiler::CompileShaderFamily(ShaderFamily shaderFamily)
{
    SmallInplaceStringT sourceFilename = m_ShaderDirectoryName;
    sourceFilename += g_ShaderFamilyFilenames[shipUint32(shaderFamily)];

    StringA shaderSource;
    LargeInplaceStringA renderStateBlockSource;
    InplaceArray<ShaderInputProviderDeclaration*, 8> includedShaderInputProviders;

    shipBool couldReadShaderFile = ReadShaderFile(sourceFilename, shaderSource, renderStateBlockSource, includedShaderInputProviders);
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

        CompileShaderKey(
                currentShaderKeyToCompile,
                everyPossibleShaderOption,
                sourceFilename,
                shaderSource,
                renderStateBlockSource,
                includedShaderInputProviders);

        shaderOptionAsInt -= 1;
    }
}

void ShaderCompiler::CompileShaderKey(const ShaderKey& shaderKeyToCompile)
{
    SmallInplaceStringT sourceFilename = m_ShaderDirectoryName;
    sourceFilename += g_ShaderFamilyFilenames[shipUint32(shaderKeyToCompile.GetShaderFamily())];

    StringA shaderSource;
    LargeInplaceStringA renderStateBlockSource;
    InplaceArray<ShaderInputProviderDeclaration*, 8> includedShaderInputProviders;

    shipBool couldReadShaderFile = ReadShaderFile(sourceFilename, shaderSource, renderStateBlockSource, includedShaderInputProviders);
    if (!couldReadShaderFile)
    {
        return;
    }

    Array<ShaderOption> everyPossibleShaderOptionForShaderKey;
    ShaderKey::GetShaderKeyOptionsForShaderFamily(shaderKeyToCompile.GetShaderFamily(), everyPossibleShaderOptionForShaderKey);

    CompileShaderKey(
            shaderKeyToCompile,
            everyPossibleShaderOptionForShaderKey,
            sourceFilename,
            shaderSource,
            renderStateBlockSource,
            includedShaderInputProviders);
}

void ShaderCompiler::CompileShaderKey(
        const ShaderKey& shaderKeyToCompile,
        const Array<ShaderOption>& everyPossibleShaderOptionForShaderKey,
        const StringT& sourceFilename,
        const StringA& shaderSource,
        const StringA& renderStateBlockSource,
        const Array<ShaderInputProviderDeclaration*>& includedShaderInputProviders)
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

        ShaderReflectionData shaderReflectionData;

        GetReflectionDataForShader(vertexShaderBlob, shaderReflectionData, ShaderVisibility::ShaderVisibility_Vertex);
        GetReflectionDataForShader(pixelShaderBlob, shaderReflectionData, ShaderVisibility::ShaderVisibility_Pixel);

        constexpr shipUint32 expectedNumRootEntries = 8;
        InplaceArray<RootSignatureParameterEntry, expectedNumRootEntries> rootSignatureParameters;
        shipUint16 rootIndexPerDescriptorRangeTypePerShaderStage[expectedNumRootEntries][shipUint32(ShaderStage::ShaderStage_Count)];

        for (shipUint32 i = 0; i < expectedNumRootEntries; i++)
        {
            for (shipUint32 j = 0; j < shipUint32(ShaderStage::ShaderStage_Count); j++)
            {
                rootIndexPerDescriptorRangeTypePerShaderStage[i][j] = shipUint16(-1);
            }
        }

        FillRootSignatureEntriesForDescriptorRangeType(
                shaderReflectionData.ConstantBufferReflectionDatas,
                DescriptorRangeType::ConstantBufferView,
                rootSignatureParameters,
                rootIndexPerDescriptorRangeTypePerShaderStage);

        FillRootSignatureEntriesForDescriptorRangeType(
                shaderReflectionData.ShaderResourceViewReflectionDatas,
                DescriptorRangeType::ShaderResourceView,
                rootSignatureParameters,
                rootIndexPerDescriptorRangeTypePerShaderStage);

        FillRootSignatureEntriesForDescriptorRangeType(
                shaderReflectionData.UnorderedAccessViewReflectionDatas,
                DescriptorRangeType::UnorderedAccessView,
                rootSignatureParameters,
                rootIndexPerDescriptorRangeTypePerShaderStage);

        FillRootSignatureEntriesForDescriptorRangeType(
                shaderReflectionData.SamplerReflctionDatas,
                DescriptorRangeType::Sampler,
                rootSignatureParameters,
                rootIndexPerDescriptorRangeTypePerShaderStage);

        compiledShaderKeyEntry.m_RootSignatureParameters = rootSignatureParameters;

        CreateShaderResourceBinder(
                shaderReflectionData,
                rootSignatureParameters,
                includedShaderInputProviders,
                rootIndexPerDescriptorRangeTypePerShaderStage,
                compiledShaderKeyEntry.m_ShaderResourceBinder);

        for (shipUint32 rootIndex = 0; rootIndex < rootSignatureParameters.Size(); rootIndex++)
        {
            const RootSignatureParameterEntry& rootSignatureParameterEntry = rootSignatureParameters[rootIndex];

            if (rootSignatureParameterEntry.parameterType == RootSignatureParameterType::DescriptorTable)
            {
                for (shipUint32 descriptorRangeIndex = 0; descriptorRangeIndex < rootSignatureParameterEntry.descriptorTable.descriptorRanges.Size(); descriptorRangeIndex++)
                {
                    const DescriptorRange& descriptorRange = rootSignatureParameterEntry.descriptorTable.descriptorRanges[descriptorRangeIndex];
                    if (descriptorRange.descriptorRangeType == DescriptorRangeType::Sampler)
                    {
                        continue;
                    }

                    DescriptorSetEntryDeclaration& descriptorSetEntryDeclaration = compiledShaderKeyEntry.m_DescriptorSetEntryDeclarations.Grow();
                    descriptorSetEntryDeclaration.rootIndex = shipUint16(rootIndex);
                    descriptorSetEntryDeclaration.descriptorRangeIndex = shipUint16(descriptorRangeIndex);
                    descriptorSetEntryDeclaration.numResources = shipUint16(descriptorRange.numDescriptors);
                }
            }
            else
            {
                SHIP_ASSERT(rootSignatureParameterEntry.parameterType != RootSignatureParameterType::Unknown);

                DescriptorSetEntryDeclaration& descriptorSetEntryDeclaration = compiledShaderKeyEntry.m_DescriptorSetEntryDeclarations.Grow();
                descriptorSetEntryDeclaration.rootIndex = shipUint16(rootIndex);
                descriptorSetEntryDeclaration.numResources = 1;
            }
        }
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

void ShaderCompiler::GetReflectionDataForShader(
        ID3D10Blob* shaderBlob,
        ShaderReflectionData& shaderReflectionData,
        ShaderVisibility shaderVisibility) const
{
    ID3D11ShaderReflection* pReflection = nullptr;
    HRESULT hResult = D3DReflect(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&pReflection);
    if (FAILED(hResult))
    {
        return;
    }

    SHIP_ASSERT(pReflection != nullptr);

    D3D11_SHADER_DESC shaderDesc;
    pReflection->GetDesc(&shaderDesc);

    for (UINT i = 0; i < shaderDesc.BoundResources; i++)
    {
        D3D11_SHADER_INPUT_BIND_DESC resourceDesc;
        pReflection->GetResourceBindingDesc(i, &resourceDesc);

        Array<ShaderInputReflectionData>* pShaderInputReflectionDatas = nullptr;

        switch (resourceDesc.Type)
        {
        case D3D_SHADER_INPUT_TYPE::D3D_SIT_CBUFFER:
            pShaderInputReflectionDatas = &shaderReflectionData.ConstantBufferReflectionDatas;
            shaderReflectionData.ConstantBuffersShaderVisibility = ShaderVisibility(shaderReflectionData.ConstantBuffersShaderVisibility | shaderVisibility);
            break;

        case D3D_SHADER_INPUT_TYPE::D3D_SIT_BYTEADDRESS:
        case D3D_SHADER_INPUT_TYPE::D3D_SIT_STRUCTURED:
        case D3D_SHADER_INPUT_TYPE::D3D_SIT_TBUFFER:
        case D3D_SHADER_INPUT_TYPE::D3D_SIT_TEXTURE:
            pShaderInputReflectionDatas = &shaderReflectionData.ShaderResourceViewReflectionDatas;
            shaderReflectionData.ShaderResourceViewsShaderVisibility = ShaderVisibility(shaderReflectionData.ShaderResourceViewsShaderVisibility | shaderVisibility);
            break;

        case D3D_SHADER_INPUT_TYPE::D3D_SIT_UAV_APPEND_STRUCTURED:
        case D3D_SHADER_INPUT_TYPE::D3D_SIT_UAV_CONSUME_STRUCTURED:
        case D3D_SHADER_INPUT_TYPE::D3D_SIT_UAV_RWBYTEADDRESS:
        case D3D_SHADER_INPUT_TYPE::D3D_SIT_UAV_RWSTRUCTURED:
        case D3D_SHADER_INPUT_TYPE::D3D_SIT_UAV_RWTYPED:
            pShaderInputReflectionDatas = &shaderReflectionData.UnorderedAccessViewReflectionDatas;
            shaderReflectionData.UnorderedAccessViewsShaderVisibility = ShaderVisibility(shaderReflectionData.UnorderedAccessViewsShaderVisibility | shaderVisibility);
            break;

        case D3D_SHADER_INPUT_TYPE::D3D_SIT_SAMPLER:
            pShaderInputReflectionDatas = &shaderReflectionData.SamplerReflctionDatas;
            shaderReflectionData.SamplersShaderVisibility = ShaderVisibility(shaderReflectionData.SamplersShaderVisibility | shaderVisibility);
            break;

        default:
            SHIP_ASSERT(!"Unsupported type");
            break;
        }

        if (pShaderInputReflectionDatas != nullptr)
        {
            Array<ShaderInputReflectionData>& shaderInputReflectionDatas = *pShaderInputReflectionDatas;

            shipUint32 idx = 0;
            for (; idx < shaderInputReflectionDatas.Size(); idx++)
            {
                if (shaderInputReflectionDatas[idx].Name == resourceDesc.Name)
                {
                    SHIP_ASSERT(shaderInputReflectionDatas[idx].BindPoint == resourceDesc.BindPoint);

                    shaderInputReflectionDatas[idx].shaderVisibility = ShaderVisibility(shaderInputReflectionDatas[idx].shaderVisibility | shaderVisibility);
                    break;
                }
            }

            if (idx == shaderInputReflectionDatas.Size())
            {
                ShaderInputReflectionData& newEntry = shaderInputReflectionDatas.Grow();
                newEntry.Name = resourceDesc.Name;
                newEntry.BindPoint = shipUint16(resourceDesc.BindPoint);
                newEntry.shaderVisibility = shaderVisibility;
            }
        }
    }

    pReflection->Release();
}

void ShaderCompiler::FillRootSignatureEntriesForDescriptorRangeType(
        const Array<ShaderInputReflectionData>& shaderInputReflectionDatas,
        DescriptorRangeType descriptorRangeType,
        Array<RootSignatureParameterEntry>& rootSignatureParameters,
        shipUint16 rootIndexPerDescriptorRangeTypePerShaderStage[][shipUint32(ShaderStage::ShaderStage_Count)]) const
{
    if (shaderInputReflectionDatas.Empty())
    {
        return;
    }

    for (shipUint32 i = 0; i < shipUint32(ShaderStage::ShaderStage_Count); i++)
    {
        ShaderStage shaderStage = ShaderStage(i);

        shipBool hadAnyShaderInputForShaderStage = FillRootSignatureEntryForDescriptorRangeTypeForShaderStage(
                shaderInputReflectionDatas,
                descriptorRangeType,
                shaderStage,
                rootSignatureParameters);

        if (hadAnyShaderInputForShaderStage)
        {
            rootIndexPerDescriptorRangeTypePerShaderStage[shipUint32(descriptorRangeType)][i] = shipUint16(rootSignatureParameters.Size() - 1);
        }
    }
}

shipBool ShaderCompiler::FillRootSignatureEntryForDescriptorRangeTypeForShaderStage(
        const Array<ShaderInputReflectionData>& shaderInputReflectionDatas,
        DescriptorRangeType descriptorRangeType,
        ShaderStage shaderStage,
        Array<RootSignatureParameterEntry>& rootSignatureParameters) const
{
    shipBool hasAnyShaderInputForShaderStage = false;

    shipUint32 minBindingPoint = 255;
    shipUint32 maxBindingPoint = 0;

    ShaderVisibility shaderVisibility = GetShaderVisibilityForShaderStage(shaderStage);

    for (const ShaderInputReflectionData& shaderInputReflectionData : shaderInputReflectionDatas)
    {
        if ((shaderInputReflectionData.shaderVisibility & shaderVisibility) > 0)
        {
            minBindingPoint = MIN(shaderInputReflectionData.BindPoint, minBindingPoint);
            maxBindingPoint = MAX(shaderInputReflectionData.BindPoint, maxBindingPoint);

            hasAnyShaderInputForShaderStage = true;
        }
    }

    if (!hasAnyShaderInputForShaderStage)
    {
        return false;
    }

    RootSignatureParameterEntry& rootSignatureParameterEntry = rootSignatureParameters.Grow();

    rootSignatureParameterEntry.parameterType = RootSignatureParameterType::DescriptorTable;
    rootSignatureParameterEntry.shaderVisibility = shaderVisibility;

    shipUint32 numDescriptors = (maxBindingPoint - minBindingPoint) + 1;
    SHIP_ASSERT(numDescriptors > 0);

    DescriptorRange& descriptorRange = rootSignatureParameterEntry.descriptorTable.descriptorRanges.Grow();
    descriptorRange.numDescriptors = numDescriptors;
    descriptorRange.baseShaderRegister = minBindingPoint;
    descriptorRange.descriptorRangeType = descriptorRangeType;

    return true;
}

void ShaderCompiler::CreateShaderResourceBinder(
        const ShaderReflectionData& shaderReflectionData,
        const Array<RootSignatureParameterEntry>& rootSignatureParameters,
        const Array<ShaderInputProviderDeclaration*>& includedShaderInputProviders,
        shipUint16 rootIndexPerDescriptorRangeTypePerShaderStage[][shipUint32(ShaderStage::ShaderStage_Count)],
        ShaderResourceBinder& shaderResourceBinder)
{
    ShaderInputProviderManager& shaderInputProviderManager = GetShaderInputProviderManager();

    for (const ShaderInputReflectionData& shaderInputReflectionData : shaderReflectionData.ConstantBufferReflectionDatas)
    {
        for (shipUint32 i = 0; i < shipUint32(ShaderStage::ShaderStage_Count); i++)
        {
            ShaderStage shaderStage = ShaderStage(i);

            ShaderVisibility shaderVisibility = GetShaderVisibilityForShaderStage(shaderStage);

            if ((shaderInputReflectionData.shaderVisibility & shaderVisibility) == 0)
            {
                continue;
            }

            for (ShaderInputProviderDeclaration* shaderInputProviderDeclaration : includedShaderInputProviders)
            {
                if (!ShaderInputProviderUtils::IsUsingConstantBuffer(shaderInputProviderDeclaration->GetShaderInputProviderUsage()))
                {
                    continue;
                }

                const shipChar* shaderInputProviderConstantBufferName = shaderInputProviderManager.GetShaderInputProviderConstantBufferName(shaderInputProviderDeclaration);

                if (shaderInputReflectionData.Name == shaderInputProviderConstantBufferName)
                {
                    shipUint16 descriptorRangeEntryIndex = GetDescriptorRangeEntryIndex(shaderInputReflectionData, rootSignatureParameters, DescriptorRangeType::ConstantBufferView);

                    constexpr shipUint32 descriptorRangeIndex = 0;
                    shaderResourceBinder.AddShaderResourceBinderEntryForProviderToDescriptorTable(
                        shaderInputProviderDeclaration,
                        rootIndexPerDescriptorRangeTypePerShaderStage[shipUint32(DescriptorRangeType::ConstantBufferView)][i],
                        descriptorRangeIndex,
                        descriptorRangeEntryIndex,
                        shaderInputReflectionData.shaderVisibility);

                    break;
                }
            }
        }
    }

    CreateShaderResourceBinderForDescriptorRangeType(
            shaderReflectionData.ShaderResourceViewReflectionDatas,
            rootSignatureParameters,
            includedShaderInputProviders,
            rootIndexPerDescriptorRangeTypePerShaderStage,
            DescriptorRangeType::ShaderResourceView,
            shaderResourceBinder);

    CreateShaderResourceBinderForDescriptorRangeType(
            shaderReflectionData.UnorderedAccessViewReflectionDatas,
            rootSignatureParameters,
            includedShaderInputProviders,
            rootIndexPerDescriptorRangeTypePerShaderStage,
            DescriptorRangeType::UnorderedAccessView,
            shaderResourceBinder);

    SHIP_STATIC_ASSERT_MSG(shipUint32(ShaderInputProviderUsage::Count) == 2, "Need to update code below this assert.");

    const shipChar* unifiedConstantBufferName = ShaderInputProviderUtils::GetGlobalBufferNameFromProviderUsage(ShaderInputProviderUsage::PerInstance);
    for (const ShaderInputReflectionData& shaderInputReflectionData : shaderReflectionData.ShaderResourceViewReflectionDatas)
    {
        for (shipUint32 i = 0; i < shipUint32(ShaderStage::ShaderStage_Count); i++)
        {
            ShaderStage shaderStage = ShaderStage(i);

            ShaderVisibility shaderVisibility = GetShaderVisibilityForShaderStage(shaderStage);

            if ((shaderInputReflectionData.shaderVisibility & shaderVisibility) == 0)
            {
                continue;
            }

            if (shaderInputReflectionData.Name == unifiedConstantBufferName)
            {
                shipUint16 descriptorRangeEntryIndex = GetDescriptorRangeEntryIndex(shaderInputReflectionData, rootSignatureParameters, DescriptorRangeType::ShaderResourceView);

                constexpr shipUint32 descriptorRangeIndex = 0;
                shaderResourceBinder.AddShaderResourceBinderEntryForGlobalBufferToDescriptorTable(
                    ShaderInputProviderUsage::PerInstance,
                    rootIndexPerDescriptorRangeTypePerShaderStage[shipUint32(DescriptorRangeType::ShaderResourceView)][i],
                    descriptorRangeIndex,
                    descriptorRangeEntryIndex,
                    shaderInputReflectionData.shaderVisibility);
                break;
            }
        }
    }
}

void ShaderCompiler::CreateShaderResourceBinderForDescriptorRangeType(
        const Array<ShaderInputReflectionData>& shaderInputReflectionDatas,
        const Array<RootSignatureParameterEntry>& rootSignatureParameters,
        const Array<ShaderInputProviderDeclaration*>& includedShaderInputProviders,
        shipUint16 rootIndexPerDescriptorRangeTypePerShaderStage[][shipUint32(ShaderStage::ShaderStage_Count)],
        DescriptorRangeType descriptorRangeType,
        ShaderResourceBinder& shaderResourceBinder)
{
    for (const ShaderInputReflectionData& shaderInputReflectionData : shaderInputReflectionDatas)
    {
        for (shipUint32 i = 0; i < shipUint32(ShaderStage::ShaderStage_Count); i++)
        {
            ShaderStage shaderStage = ShaderStage(i);

            ShaderVisibility shaderVisibility = GetShaderVisibilityForShaderStage(shaderStage);

            if ((shaderInputReflectionData.shaderVisibility & shaderVisibility) == 0)
            {
                continue;
            }

            for (const ShaderInputProviderDeclaration* shaderInputProviderDeclaration : includedShaderInputProviders)
            {
                shipInt32 dataOffsetInProvider;
                if (shaderInputProviderDeclaration->HasShaderInput(shaderInputReflectionData.Name.GetBuffer(), dataOffsetInProvider))
                {
                    shipUint16 descriptorRangeEntryIndex = GetDescriptorRangeEntryIndex(shaderInputReflectionData, rootSignatureParameters, descriptorRangeType);

                    constexpr shipUint32 descriptorRangeIndex = 0;
                    shaderResourceBinder.AddShaderResourceBinderEntryForDescriptorToDescriptorTable(
                        shaderInputProviderDeclaration,
                        rootIndexPerDescriptorRangeTypePerShaderStage[shipUint32(descriptorRangeType)][i],
                        descriptorRangeIndex,
                        descriptorRangeEntryIndex,
                        dataOffsetInProvider,
                        ShaderInputType::Texture2D,
                        shaderInputReflectionData.shaderVisibility);

                    break;
                }
            }
        }
    }
}

shipUint16 ShaderCompiler::GetDescriptorRangeEntryIndex(
        const ShaderInputReflectionData& shaderInputReflectionData,
        const Array<RootSignatureParameterEntry>& rootSignatureParameters,
        DescriptorRangeType descriptorRangeType) const
{
    for (const RootSignatureParameterEntry& rootSignatureParameterEntry : rootSignatureParameters)
    {
        if ((rootSignatureParameterEntry.shaderVisibility & shaderInputReflectionData.shaderVisibility) == 0)
        {
            continue;
        }

        if (rootSignatureParameterEntry.parameterType != RootSignatureParameterType::DescriptorTable)
        {
            SHIP_ASSERT(!"ShaderCompiler only creates descriptor table entries. Please review ShaderCompiler");
            return shipUint16(-1);
        }

        for (const DescriptorRange& descriptorRange : rootSignatureParameterEntry.descriptorTable.descriptorRanges)
        {
            if (descriptorRange.descriptorRangeType != descriptorRangeType)
            {
                continue;
            }

            SHIP_ASSERT(
                    shaderInputReflectionData.BindPoint >= descriptorRange.baseShaderRegister &&
                    shaderInputReflectionData.BindPoint <= (descriptorRange.baseShaderRegister + descriptorRange.numDescriptors));

            return shipUint16(shaderInputReflectionData.BindPoint - descriptorRange.baseShaderRegister);
        }
    }

    SHIP_ASSERT(!"Couldn't find the shader input inside of the root signature.");

    return shipUint16(-1);
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
    m_RootSignatureParameters.Clear();
    m_ShaderResourceBinder = ShaderResourceBinder();
    m_DescriptorSetEntryDeclarations.Clear();
}

}