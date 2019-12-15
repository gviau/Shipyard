#include <graphics/graphicsprecomp.h>

#include <graphics/shader/shaderinputprovider.h>

#include <graphics/wrapper/wrapper.h>

#include <system/wrapper/wrapper.h>

namespace Shipyard
{;

ShaderInputProvider::ShaderInputProvider()
{
}

ShaderInputProvider::~ShaderInputProvider()
{

}

namespace ShaderInputProviderUtils
{
    shipBool IsUsingConstantBuffer(ShaderInputProviderUsage shaderInputProviderUsage)
    {
        SHIP_STATIC_ASSERT_MSG(shipUint32(ShaderInputProviderUsage::Count) == 2, "ShaderInputProviderUsage size changed, need to update this method!");

        switch (shaderInputProviderUsage)
        {
        case ShaderInputProviderUsage::Default:
            return true;

        case ShaderInputProviderUsage::PerInstance:
            return false;

        default:
            SHIP_ASSERT(false);
            break;
        }

        return false;
    }

    const shipChar* GetGlobalBufferNameFromProviderUsage(ShaderInputProviderUsage shaderInputProviderUsage)
    {
        SHIP_STATIC_ASSERT_MSG(shipUint32(ShaderInputProviderUsage::Count) == 2, "Need to update code below");

        switch (shaderInputProviderUsage)
        {
        case ShaderInputProviderUsage::PerInstance:
            return "g_UnifiedConstantBuffer";

        default:
            SHIP_ASSERT(!"Unimplemented shader input provider usage");
            break;
        }

        return nullptr;
    }
}

ShaderInputProviderManager::ShaderInputProviderManager()
    : m_pHead(nullptr)
    , m_GfxRenderDevice(nullptr)
{

}

ShaderInputProviderManager::~ShaderInputProviderManager()
{

}

void ShaderInputProviderManager::RegisterShaderInputProviderDeclaration(ShaderInputProviderDeclaration& shaderInputProviderDeclaration)
{
    shaderInputProviderDeclaration.m_pNextRegisteredShaderInputProviderDeclaration = m_pHead;
    m_pHead = &shaderInputProviderDeclaration;
}

shipBool ShaderInputProviderManager::Initialize(BaseRenderDevice& gfxRenderDevice)
{
    m_GfxRenderDevice = &gfxRenderDevice;

    shipUint32 shaderInputProviderIdx = 0;
    ShaderInputProviderDeclaration* pCurrent = m_pHead;
    while (pCurrent != nullptr)
    {
        pCurrent->m_ShaderInputProviderDeclarationIndex = shaderInputProviderIdx;

        InitializeShaderInputProviderDeclarationCopyRanges(pCurrent);

        pCurrent = pCurrent->m_pNextRegisteredShaderInputProviderDeclaration;

        shaderInputProviderIdx += 1;
    }

    return WriteEveryShaderInputProviderFile();
}

void ShaderInputProviderManager::Destroy()
{

}

shipUint32 ShaderInputProviderManager::GetRequiredSizeForProvider(const ShaderInputProvider& shaderInputProvider) const
{
    return shaderInputProvider.GetRequiredSizeForProvider();
}

shipUint32 ShaderInputProviderManager::GetShaderInputProviderDeclarationIndex(const ShaderInputProvider& shaderInputProvider) const
{
    return shaderInputProvider.GetShaderInputProviderDeclarationIndex();
}

void ShaderInputProviderManager::InitializeShaderInputProviderDeclarationCopyRanges(ShaderInputProviderDeclaration* pShaderInputProviderDeclaration) const
{
    shipUint32 numShaderInputDeclarations = pShaderInputProviderDeclaration->m_NumShaderInputDeclarations;

    InplaceArray<ShaderInputProviderDeclaration::ShaderInputDeclaration, ShaderInputProviderDeclaration::MaxShaderInputsPerProvider> sortedShaderInputDeclarations;

    for (shipUint32 i = 0; i < numShaderInputDeclarations; i++)
    {
        const ShaderInputProviderDeclaration::ShaderInputDeclaration& shaderInputDeclaration = pShaderInputProviderDeclaration->m_ShaderInputDeclarations[i];
        if (shaderInputDeclaration.Type != ShaderInputType::Scalar)
        {
            continue;
        }

        shipInt32 dataOffsetInProvider = shaderInputDeclaration.DataOffsetInProvider;

        shipUint32 insertIdx = 0;
        for (; insertIdx < sortedShaderInputDeclarations.Size(); insertIdx++)
        {
            if (sortedShaderInputDeclarations[insertIdx].DataOffsetInProvider > dataOffsetInProvider)
            {
                break;
            }
        }

        sortedShaderInputDeclarations.InsertAt(insertIdx, shaderInputDeclaration);
    }

    if (sortedShaderInputDeclarations.Empty())
    {
        return;
    }

    // Group shader inputs that are continuous both in the provider and in the buffer. Worst-case scenario, we get the same
    // number of memcopies when uploading the provider, best case, we get a single memcopy for the whole provider.
    shipInt32 startOffsetInBuffer = 0x7fffffff;
    shipInt32 endOffsetInBuffer = 0x7fffffff;
    shipInt32 startOffsetInProvider = 0xffffffff;
    shipInt32 endOffsetInProvider = 0xffffffff;
    shipBool firstEntry = true;

    for (const ShaderInputProviderDeclaration::ShaderInputDeclaration& shaderInputDeclaration : sortedShaderInputDeclarations)
    {
        if (shaderInputDeclaration.DataOffsetInProvider == endOffsetInProvider && shaderInputDeclaration.DataOffsetInBuffer == endOffsetInBuffer)
        {
            endOffsetInBuffer += shipInt32(shaderInputDeclaration.DataSize);
            endOffsetInProvider += shipInt32(shaderInputDeclaration.DataSize);
        }
        else
        {
            if (!firstEntry)
            {
                shipInt32 dataSizeToCopy = (endOffsetInProvider - startOffsetInProvider);
                SHIP_ASSERT(dataSizeToCopy > 0);
                SHIP_ASSERT(dataSizeToCopy == (endOffsetInBuffer - startOffsetInBuffer));

                ShaderInputProviderDeclaration::ShaderInputCopyRange& newEntry = pShaderInputProviderDeclaration->m_ShaderInputCopyRanges[pShaderInputProviderDeclaration->m_NumShaderInputCopyRanges];
                newEntry.DataOffsetInBuffer = startOffsetInBuffer;
                newEntry.DataOffsetInProvider = startOffsetInProvider;
                newEntry.DataSizeToCopy = dataSizeToCopy;

                pShaderInputProviderDeclaration->m_NumShaderInputCopyRanges += 1;
            }

            startOffsetInBuffer = shaderInputDeclaration.DataOffsetInBuffer;
            startOffsetInProvider = shaderInputDeclaration.DataOffsetInProvider;
            endOffsetInBuffer = startOffsetInBuffer + shipInt32(shaderInputDeclaration.DataSize);
            endOffsetInProvider = startOffsetInProvider + shipInt32(shaderInputDeclaration.DataSize);

            firstEntry = false;
        }
    }

    // Write left-over
    shipBool leftOver = (pShaderInputProviderDeclaration->m_NumShaderInputCopyRanges == 0);

    if (!leftOver)
    {
        shipInt32 lastDataOffsetInBufferAdded = pShaderInputProviderDeclaration->m_ShaderInputCopyRanges[pShaderInputProviderDeclaration->m_NumShaderInputCopyRanges - 1].DataOffsetInBuffer;
        leftOver = (lastDataOffsetInBufferAdded != startOffsetInBuffer);
    }

    if (leftOver)
    {
        shipInt32 dataSizeToCopy = (endOffsetInProvider - startOffsetInProvider);
        SHIP_ASSERT(dataSizeToCopy > 0);
        SHIP_ASSERT(dataSizeToCopy == (endOffsetInBuffer - startOffsetInBuffer));

        ShaderInputProviderDeclaration::ShaderInputCopyRange& newEntry = pShaderInputProviderDeclaration->m_ShaderInputCopyRanges[pShaderInputProviderDeclaration->m_NumShaderInputCopyRanges];
        newEntry.DataOffsetInBuffer = startOffsetInBuffer;
        newEntry.DataOffsetInProvider = startOffsetInProvider;
        newEntry.DataSizeToCopy = dataSizeToCopy;

        pShaderInputProviderDeclaration->m_NumShaderInputCopyRanges += 1;
    }
}

shipBool ShaderInputProviderManager::WriteEveryShaderInputProviderFile()
{
    WriteShaderInputProviderUtilsFile();

    size_t numBytesBeforeThisProviderInUnifiedBuffer = 0;

    ShaderInputProviderDeclaration* pCurrent = m_pHead;
    while (pCurrent != nullptr)
    {
        WriteSingleShaderInputProviderFile(pCurrent, numBytesBeforeThisProviderInUnifiedBuffer);

        if (pCurrent->m_ShaderInputProviderUsage == ShaderInputProviderUsage::PerInstance)
        {
            numBytesBeforeThisProviderInUnifiedBuffer += pCurrent->m_RequiredSizeForProvider * SHIP_MAX_BATCHED_DRAW_CALLS_PER_MATERIAL;
        }

        pCurrent = pCurrent->m_pNextRegisteredShaderInputProviderDeclaration;
    }

    return true;
}

shipBool ShaderInputProviderManager::WriteShaderInputProviderUtilsFile()
{
    FileHandler shaderInputProviderUtilsFile;
    shaderInputProviderUtilsFile.Open("shaders\\shaderinputproviders\\ShaderInputProviderUtils.hlsl", FileHandlerOpenFlag(FileHandlerOpenFlag::FileHandlerOpenFlag_Write | FileHandlerOpenFlag::FileHandlerOpenFlag_Create));

    if (!shaderInputProviderUtilsFile.IsOpen())
    {
        return false;
    }

    StringA content;
    content += "/**\n* AUTO-GENERATED FILE\n**/\n";
    content += "#ifndef SHADER_INPUT_PROVIDER_UTILS_HLSL\n";
    content += "#define SHADER_INPUT_PROVIDER_UTILS_HLSL\n\n";

    content += "ByteAddressBuffer ";
    
    content += ShaderInputProviderUtils::GetGlobalBufferNameFromProviderUsage(ShaderInputProviderUsage::PerInstance);

    content += ";\n";

    content += "#endif";

    shaderInputProviderUtilsFile.WriteChars(0, content.GetBuffer(), content.Size());

    return true;
}

shipBool ShaderInputProviderManager::WriteSingleShaderInputProviderFile(ShaderInputProviderDeclaration* pShaderInputProviderDeclaration, size_t numBytesBeforeThisProviderInUnifiedBuffer)
{
    StringT shaderInputProviderFileName;
    shaderInputProviderFileName = "shaders\\shaderinputproviders\\";
    shaderInputProviderFileName += pShaderInputProviderDeclaration->m_ShaderInputProviderName;
    shaderInputProviderFileName += ".hlsl";

    FileHandler shaderInputProviderFile;

    StringA content;

    content += "/**\n* AUTO-GENERATED FILE\n**/\n";
    content += "#ifndef SHADER_INPUT_PROVIDER_";
    content += pShaderInputProviderDeclaration->m_ShaderInputProviderName;
    content += "\n";
    content += "#define SHADER_INPUT_PROVIDER_";
    content += pShaderInputProviderDeclaration->m_ShaderInputProviderName;
    content += "\n\n";

    shipBool isUsingConstantBuffer = ShaderInputProviderUtils::IsUsingConstantBuffer(pShaderInputProviderDeclaration->m_ShaderInputProviderUsage);

    if (isUsingConstantBuffer)
    {
        content += "cbuffer ";

        content += GetShaderInputProviderConstantBufferName(pShaderInputProviderDeclaration);

        content += "\n{\n";
    }
    else
    {
        content += "#include \"shaderinputproviders\\ShaderInputProviderUtils.hlsl\"\n";

        content += "#define SHADER_INPUT_PROVIDER_STRIDE ";

        content += StringFormat("%llu", pShaderInputProviderDeclaration->m_RequiredSizeForProvider);
        content += "\n";

        content += "#define SHADER_INPUT_PROVIDER_OFFSET_IN_GLOBAL_BUFFER ";
        content += StringFormat("%llu", numBytesBeforeThisProviderInUnifiedBuffer);
        content += "\n\n";

        // content += "struct ";
    }

    for (shipUint32 i = 0; i < pShaderInputProviderDeclaration->m_NumShaderInputDeclarations; i++)
    {
        ShaderInputProviderDeclaration::ShaderInputDeclaration& shaderInputDeclaration = pShaderInputProviderDeclaration->m_ShaderInputDeclarations[i];

        if (shaderInputDeclaration.Type == ShaderInputType::Scalar)
        {
            WriteSingleShaderScalarInput(shaderInputDeclaration, content, isUsingConstantBuffer);
        }
    }

    if (isUsingConstantBuffer)
    {
        content += "};\n";
    }
    
    content += "\n";

    WriteShaderInputsOutsideOfStruct(pShaderInputProviderDeclaration, content);

    content += "\n";

    if (pShaderInputProviderDeclaration->m_ShaderInputProviderUsage == ShaderInputProviderUsage::PerInstance)
    {
        WriteShaderInputProviderUnifiedBufferAccessMethod(pShaderInputProviderDeclaration, content);

        content += "\n";
        content += "#undef SHADER_INPUT_PROVIDER_STRIDE\n";
        content += "#undef SHADER_INPUT_PROVIDER_OFFSET_IN_GLOBAL_BUFFER\n";
    }

    content += "\n#endif";

    // We first decide if we want to overwrite. If it's the same content, do nothing. Otherwise, overwriting the file will
    // cause a shader recompilation for every shader family including this provider, since the write timestamp would have changed.
    shaderInputProviderFile.Open(shaderInputProviderFileName.GetBuffer(), FileHandlerOpenFlag::FileHandlerOpenFlag_Read);
    if (shaderInputProviderFile.IsOpen())
    {
        StringA shaderInputProviderFileContent;
        shaderInputProviderFile.ReadWholeFile(shaderInputProviderFileContent);

        if (shaderInputProviderFileContent == content)
        {
            return true;
        }

        shaderInputProviderFile.Close();
    }

    shaderInputProviderFile.Open(shaderInputProviderFileName.GetBuffer(), FileHandlerOpenFlag(FileHandlerOpenFlag::FileHandlerOpenFlag_Write | FileHandlerOpenFlag::FileHandlerOpenFlag_Create));

    if (!shaderInputProviderFile.IsOpen())
    {
        return false;
    }

    shaderInputProviderFile.WriteChars(0, content.GetBuffer(), content.Size());
    shaderInputProviderFile.Close();

    return true;
}

shipBool ShaderInputProviderManager::WriteSingleShaderScalarInput(const ShaderInputProviderDeclaration::ShaderInputDeclaration& shaderInputDeclaration, StringA& content, shipBool isUsingConstantBuffer)
{
    // Indented with respect to struct root, for easier readability.
    content += "    ";

    if (!isUsingConstantBuffer)
    {
        content += "static ";
    }

    SHIP_STATIC_ASSERT_MSG(shipUint32(ShaderInputScalarType::Count) == 33, "ShaderInputScalarType size changed, need to update this switch case!");

    switch (shaderInputDeclaration.ScalarType)
    {
    case ShaderInputScalarType::Float:
        content += "float ";
        break;
    case ShaderInputScalarType::Float2:
        content += "float2 ";
        break;
    case ShaderInputScalarType::Float3:
        content += "float3 ";
        break;
    case ShaderInputScalarType::Float4:
        content += "float4 ";
        break;
    case ShaderInputScalarType::Half:
        content += "half ";
        break;
    case ShaderInputScalarType::Half2:
        content += "half2 ";
        break;
    case ShaderInputScalarType::Half3:
        content += "half3 ";
        break;
    case ShaderInputScalarType::Half4:
        content += "half4 ";
        break;
    case ShaderInputScalarType::Double:
        content += "double ";
        break;
    case ShaderInputScalarType::Double2:
        content += "double2";
        break;
    case ShaderInputScalarType::Double3:
        content += "double3 ";
        break;
    case ShaderInputScalarType::Double4:
        content += "double4 ";
        break;
    case ShaderInputScalarType::Int:
        content += "int ";
        break;
    case ShaderInputScalarType::Int2:
        content += "int2 ";
        break;
    case ShaderInputScalarType::Int3:
        content += "int3 ";
        break;
    case ShaderInputScalarType::Int4:
        content += "int4 ";
        break;
    case ShaderInputScalarType::Uint:
        content += "uint ";
        break;
    case ShaderInputScalarType::Uint2:
        content += "uint2 ";
        break;
    case ShaderInputScalarType::Uint3:
        content += "uint3 ";
        break;
    case ShaderInputScalarType::Uint4:
        content += "uint4 ";
        break;
    case ShaderInputScalarType::Bool:
        content += "bool ";
        break;
    case ShaderInputScalarType::Bool2:
        content += "bool2 ";
        break;
    case ShaderInputScalarType::Bool3:
        content += "bool3 ";
        break;
    case ShaderInputScalarType::Bool4:
        content += "bool4 ";
        break;
    case ShaderInputScalarType::Float2x2:
        content += "float2x2 ";
        break;
    case ShaderInputScalarType::Float3x3:
        content += "float3x3 ";
        break;
    case ShaderInputScalarType::Float4x4:
        content += "float4x4 ";
        break;
    case ShaderInputScalarType::Half2x2:
        content += "half2x2 ";
        break;
    case ShaderInputScalarType::Half3x3:
        content += "half3x3 ";
        break;
    case ShaderInputScalarType::Half4x4:
        content += "half4x4 ";
        break;
    case ShaderInputScalarType::Double2x2:
        content += "double2x2 ";
        break;
    case ShaderInputScalarType::Double3x3:
        content += "double3x3 ";
        break;
    case ShaderInputScalarType::Double4x4:
        content += "double4x4 ";
        break;
    }

    content += shaderInputDeclaration.Name;
    content += ";\n";

    return true;
}

shipBool ShaderInputProviderManager::WriteShaderInputsOutsideOfStruct(ShaderInputProviderDeclaration* pShaderInputProviderDeclaration, StringA& content)
{
    for (shipUint32 i = 0; i < pShaderInputProviderDeclaration->m_NumShaderInputDeclarations; i++)
    {
        ShaderInputProviderDeclaration::ShaderInputDeclaration& shaderInputDeclaration = pShaderInputProviderDeclaration->m_ShaderInputDeclarations[i];

        if (shaderInputDeclaration.Type == ShaderInputType::Scalar)
        {
            continue;
        }

        WriteSingleShaderInputOutsideOfStruct(shaderInputDeclaration, content);
    }

    return true;
}

shipBool ShaderInputProviderManager::WriteSingleShaderInputOutsideOfStruct(const ShaderInputProviderDeclaration::ShaderInputDeclaration& shaderInputDeclaration, StringA& content)
{
    SHIP_STATIC_ASSERT_MSG(shipUint32(ShaderInputScalarType::Count) == 33, "ShaderInputScalarType size changed, need to update this switch case!");

    if (shaderInputDeclaration.Usage == ShaderInputUsage::UnorderedAccessView)
    {
        content += "RW";
    }

    switch (shaderInputDeclaration.Type)
    {
    case ShaderInputType::Texture2D:
        content += "Texture2D<";
        break;

    case ShaderInputType::Texture2DArray:
        content += "Texture2DArray<";
        break;

    case ShaderInputType::Texture3D:
        content += "Texture3D<";
        break;

    case  ShaderInputType::TextureCube:
        content += "TextureCube<";
        break;

    default:
        SHIP_ASSERT(!"Unsupported shader input type!");
        break;
    }

    switch (shaderInputDeclaration.ScalarType)
    {
    case ShaderInputScalarType::Float:
        content += "float";
        break;
    case ShaderInputScalarType::Float2:
        content += "float2";
        break;
    case ShaderInputScalarType::Float3:
        content += "float3";
        break;
    case ShaderInputScalarType::Float4:
        content += "float4";
        break;
    case ShaderInputScalarType::Int:
        content += "int";
        break;
    case ShaderInputScalarType::Int2:
        content += "int2";
        break;
    case ShaderInputScalarType::Int3:
        content += "int3";
        break;
    case ShaderInputScalarType::Int4:
        content += "int4";
        break;
    case ShaderInputScalarType::Uint:
        content += "uint";
        break;
    case ShaderInputScalarType::Uint2:
        content += "uint2";
        break;
    case ShaderInputScalarType::Uint3:
        content += "uint3";
        break;
    case ShaderInputScalarType::Uint4:
        content += "uint4";
        break;

    default:
        SHIP_ASSERT(!"Unsupported shader input scalar type!");
        break;
    }

    content += "> ";
    content += shaderInputDeclaration.Name;

    content += ";\n";

    return true;
}

shipBool ShaderInputProviderManager::WriteShaderInputProviderUnifiedBufferAccessMethod(ShaderInputProviderDeclaration* pShaderInputProviderDeclaration, StringA& content)
{
    content += "void Load";
    content += pShaderInputProviderDeclaration->m_ShaderInputProviderName;
    content += "ConstantsForInstance(uint instanceIndex)\n{\n";

    content += "    uint offsetInUnifiedBuffer = instanceIndex * SHADER_INPUT_PROVIDER_STRIDE + SHADER_INPUT_PROVIDER_OFFSET_IN_GLOBAL_BUFFER;\n";

    for (shipUint32 i = 0; i < pShaderInputProviderDeclaration->m_NumShaderInputDeclarations; i++)
    {
        ShaderInputProviderDeclaration::ShaderInputDeclaration& shaderInputDeclaration = pShaderInputProviderDeclaration->m_ShaderInputDeclarations[i];

        if (shaderInputDeclaration.Type == ShaderInputType::Scalar)
        {
            WriteShaderInputScalarLoadFromBuffer(shaderInputDeclaration, content);
        }
    }

    content += "}\n";


    return true;
}

namespace ShaderInputProviderUtils
{
    enum class ScalarHighLevelType
    {
        Float,
        Double,
        Half,
        Int,
        Uint,
        Bool
    };

    void GetBaseHighLevelScalarType(ScalarHighLevelType scalarHighLevelType, shipUint32 numComponentsX, StringA& castType)
    {
        switch (scalarHighLevelType)
        {
        case ScalarHighLevelType::Float:    castType += "float"; break;
        case ScalarHighLevelType::Double:   castType += "double"; break;
        case ScalarHighLevelType::Half:     castType += "half"; break;
        case ScalarHighLevelType::Int:      castType += "int"; break;
        case ScalarHighLevelType::Uint:     castType += "uint"; break;
        case ScalarHighLevelType::Bool:     castType += "bool"; break;

        default:
            SHIP_ASSERT(!"Unimplemted");
            break;
        }

        switch (numComponentsX)
        {
        case 2: castType += "2"; break;
        case 3: castType += "3"; break;
        case 4: castType += "4"; break;
        default: break;
        }
    }

    void AddLoadingCodeFromUnifiedBufferForShaderInput(ScalarHighLevelType scalarHighLevelType, shipUint32 numComponentsX, shipUint32 numComponentsY, shipUint32 offsetInBuffer, StringA& content)
    {
        SHIP_ASSERT(numComponentsX > 0);
        SHIP_ASSERT(numComponentsY > 0);

        InplaceStringA<8> highLevelScalarType;
        GetBaseHighLevelScalarType(scalarHighLevelType, numComponentsX, highLevelScalarType);

        InplaceStringA<10> castType = highLevelScalarType;

        switch (numComponentsY)
        {
        case 2: castType += "x2"; break;
        case 3: castType += "x3"; break;
        case 4: castType += "x4"; break;
        default: break;
        }

        if (numComponentsY > 1)
        {
            content += "transpose(";
        }

        content += castType;
        content += "(";

        if (numComponentsY > 1)
        {
            content += "\n";
        }

        const shipChar* unifiedConstantBufferName = ShaderInputProviderUtils::GetGlobalBufferNameFromProviderUsage(ShaderInputProviderUsage::PerInstance);

        for (shipUint32 y = 0; y < numComponentsY; y++)
        {
            if (numComponentsY > 1)
            {
                content += "        ";
                content += highLevelScalarType;
                content += "(";
            }

            switch (scalarHighLevelType)
            {
            case ScalarHighLevelType::Double:
                content += "asdouble(";
                break;

            case ScalarHighLevelType::Float:
            case ScalarHighLevelType::Half:
                content += "asfloat(";
                break;

            case ScalarHighLevelType::Int:
                content += "asint(";
                break;

            default:
                break;
            }

            content += unifiedConstantBufferName;

            switch (numComponentsX)
            {
            case 1:
                content += StringFormat(".Load(offsetInUnifiedBuffer + 0x%x)", offsetInBuffer);
                break;

            case 2:
                content += StringFormat(".Load2(offsetInUnifiedBuffer + 0x%x)", offsetInBuffer);
                break;

            case 3:
                content += StringFormat(".Load3(offsetInUnifiedBuffer + 0x%x)", offsetInBuffer);
                break;

            case 4:
                content += StringFormat(".Load4(offsetInUnifiedBuffer + 0x%x)", offsetInBuffer);
                break;
            }

            switch (scalarHighLevelType)
            {
            case ScalarHighLevelType::Bool:
                offsetInBuffer += numComponentsX;
                break;

            case ScalarHighLevelType::Half:
                offsetInBuffer += numComponentsX * 2;
                break;

            case ScalarHighLevelType::Float:
            case ScalarHighLevelType::Int:
            case ScalarHighLevelType::Uint:
                offsetInBuffer += numComponentsX * 4;
                break;

            case ScalarHighLevelType::Double:
                offsetInBuffer += numComponentsX * 8;
                break;

            default:
                SHIP_ASSERT(!"Unsupported scalar type");
                break;
            }
            
            if (numComponentsY > 1)
            {
                content += ")";
            }

            switch (scalarHighLevelType)
            {
            case ScalarHighLevelType::Double:
            case ScalarHighLevelType::Float:
            case ScalarHighLevelType::Half:
            case ScalarHighLevelType::Int:
                content += ")";
                break;

            default:
                break;
            }

            shipBool isNotLastYComponent = (y < numComponentsY - 1);
            if (isNotLastYComponent)
            {
                content += ",\n";
            }
        }

        if (numComponentsY > 1)
        {
            content += ")";
        }

        content += ");\n";
    }
}

shipBool ShaderInputProviderManager::WriteShaderInputScalarLoadFromBuffer(const ShaderInputProviderDeclaration::ShaderInputDeclaration& shaderInputDeclaration, StringA& content)
{
    // Indented with respect to method root, for easier readability.
    content += "    ";
    content += shaderInputDeclaration.Name;
    content += " = ";

    SHIP_STATIC_ASSERT_MSG(shipUint32(ShaderInputScalarType::Count) == 33, "ShaderInputScalarType size changed, need to update this switch case!");

    ShaderInputProviderUtils::ScalarHighLevelType scalarHighLevelType = ShaderInputProviderUtils::ScalarHighLevelType::Float;
    shipUint32 numComponentsX = 0;
    shipUint32 numComponentsY = 0;

    switch (shaderInputDeclaration.ScalarType)
    {
    case ShaderInputScalarType::Float:
        scalarHighLevelType = ShaderInputProviderUtils::ScalarHighLevelType::Float;
        numComponentsX = 1;
        numComponentsY = 1;
        break;
    case ShaderInputScalarType::Float2:
        scalarHighLevelType = ShaderInputProviderUtils::ScalarHighLevelType::Float;
        numComponentsX = 2;
        numComponentsY = 1;
        break;
    case ShaderInputScalarType::Float3:
        scalarHighLevelType = ShaderInputProviderUtils::ScalarHighLevelType::Float;
        numComponentsX = 3;
        numComponentsY = 1;
        break;
    case ShaderInputScalarType::Float4:
        scalarHighLevelType = ShaderInputProviderUtils::ScalarHighLevelType::Float;
        numComponentsX = 4;
        numComponentsY = 1;
        break;
    case ShaderInputScalarType::Half:
        scalarHighLevelType = ShaderInputProviderUtils::ScalarHighLevelType::Half;
        numComponentsX = 1;
        numComponentsY = 1;
        break;
    case ShaderInputScalarType::Half2:
        scalarHighLevelType = ShaderInputProviderUtils::ScalarHighLevelType::Half;
        numComponentsX = 2;
        numComponentsY = 1;
        break;
    case ShaderInputScalarType::Half3:
        scalarHighLevelType = ShaderInputProviderUtils::ScalarHighLevelType::Half;
        numComponentsX = 3;
        numComponentsY = 1;
        break;
    case ShaderInputScalarType::Half4:
        scalarHighLevelType = ShaderInputProviderUtils::ScalarHighLevelType::Half;
        numComponentsX = 4;
        numComponentsY = 1;
        break;
    case ShaderInputScalarType::Double:
        scalarHighLevelType = ShaderInputProviderUtils::ScalarHighLevelType::Double;
        numComponentsX = 1;
        numComponentsY = 1;
        break;
    case ShaderInputScalarType::Double2:
        scalarHighLevelType = ShaderInputProviderUtils::ScalarHighLevelType::Double;
        numComponentsX = 2;
        numComponentsY = 1;
        break;
    case ShaderInputScalarType::Double3:
        scalarHighLevelType = ShaderInputProviderUtils::ScalarHighLevelType::Double;
        numComponentsX = 3;
        numComponentsY = 1;
        break;
    case ShaderInputScalarType::Double4:
        scalarHighLevelType = ShaderInputProviderUtils::ScalarHighLevelType::Double;
        numComponentsX = 4;
        numComponentsY = 1;
        break;
    case ShaderInputScalarType::Int:
        scalarHighLevelType = ShaderInputProviderUtils::ScalarHighLevelType::Int;
        numComponentsX = 1;
        numComponentsY = 1;
        break;
    case ShaderInputScalarType::Int2:
        scalarHighLevelType = ShaderInputProviderUtils::ScalarHighLevelType::Int;
        numComponentsX = 2;
        numComponentsY = 1;
        break;
    case ShaderInputScalarType::Int3:
        scalarHighLevelType = ShaderInputProviderUtils::ScalarHighLevelType::Int;
        numComponentsX = 3;
        numComponentsY = 1;
        break;
    case ShaderInputScalarType::Int4:
        scalarHighLevelType = ShaderInputProviderUtils::ScalarHighLevelType::Int;
        numComponentsX = 4;
        numComponentsY = 1;
        break;
    case ShaderInputScalarType::Uint:
        scalarHighLevelType = ShaderInputProviderUtils::ScalarHighLevelType::Uint;
        numComponentsX = 1;
        numComponentsY = 1;
        break;
    case ShaderInputScalarType::Uint2:
        scalarHighLevelType = ShaderInputProviderUtils::ScalarHighLevelType::Uint;
        numComponentsX = 2;
        numComponentsY = 1;
        break;
    case ShaderInputScalarType::Uint3:
        scalarHighLevelType = ShaderInputProviderUtils::ScalarHighLevelType::Uint;
        numComponentsX = 3;
        numComponentsY = 1;
        break;
    case ShaderInputScalarType::Uint4:
        scalarHighLevelType = ShaderInputProviderUtils::ScalarHighLevelType::Uint;
        numComponentsX = 4;
        numComponentsY = 1;
        break;
    case ShaderInputScalarType::Bool:
        scalarHighLevelType = ShaderInputProviderUtils::ScalarHighLevelType::Bool;
        numComponentsX = 1;
        numComponentsY = 1;
        break;
    case ShaderInputScalarType::Bool2:
        scalarHighLevelType = ShaderInputProviderUtils::ScalarHighLevelType::Bool;
        numComponentsX = 2;
        numComponentsY = 1;
        break;
    case ShaderInputScalarType::Bool3:
        scalarHighLevelType = ShaderInputProviderUtils::ScalarHighLevelType::Bool;
        numComponentsX = 3;
        numComponentsY = 1;
        break;
    case ShaderInputScalarType::Bool4:
        scalarHighLevelType = ShaderInputProviderUtils::ScalarHighLevelType::Bool;
        numComponentsX = 4;
        numComponentsY = 1;
        break;
    case ShaderInputScalarType::Float2x2:
        scalarHighLevelType = ShaderInputProviderUtils::ScalarHighLevelType::Float;
        numComponentsX = 2;
        numComponentsY = 2;
        break;
    case ShaderInputScalarType::Float3x3:
        scalarHighLevelType = ShaderInputProviderUtils::ScalarHighLevelType::Float;
        numComponentsX = 3;
        numComponentsY = 3;
        break;
    case ShaderInputScalarType::Float4x4:
        scalarHighLevelType = ShaderInputProviderUtils::ScalarHighLevelType::Float;
        numComponentsX = 4;
        numComponentsY = 4;
        break;
    case ShaderInputScalarType::Half2x2:
        scalarHighLevelType = ShaderInputProviderUtils::ScalarHighLevelType::Half;
        numComponentsX = 2;
        numComponentsY = 2;
        break;
    case ShaderInputScalarType::Half3x3:
        scalarHighLevelType = ShaderInputProviderUtils::ScalarHighLevelType::Half;
        numComponentsX = 3;
        numComponentsY = 3;
        break;
    case ShaderInputScalarType::Half4x4:
        scalarHighLevelType = ShaderInputProviderUtils::ScalarHighLevelType::Half;
        numComponentsX = 4;
        numComponentsY = 4;
        break;
    case ShaderInputScalarType::Double2x2:
        scalarHighLevelType = ShaderInputProviderUtils::ScalarHighLevelType::Double;
        numComponentsX = 2;
        numComponentsY = 2;
        break;
    case ShaderInputScalarType::Double3x3:
        scalarHighLevelType = ShaderInputProviderUtils::ScalarHighLevelType::Double;
        numComponentsX = 3;
        numComponentsY = 3;
        break;
    case ShaderInputScalarType::Double4x4:
        scalarHighLevelType = ShaderInputProviderUtils::ScalarHighLevelType::Double;
        numComponentsX = 4;
        numComponentsY = 4;
        break;
    }

    ShaderInputProviderUtils::AddLoadingCodeFromUnifiedBufferForShaderInput(scalarHighLevelType, numComponentsX, numComponentsY, shaderInputDeclaration.DataOffsetInBuffer, content);

    return true;
}

void ShaderInputProviderManager::GetShaderInputProviderDeclarations(Array<ShaderInputProviderDeclaration*>& shaderInputProviderDeclarations) const
{
    shipUint32 numShaderInputProviderDeclarations = 0;
    ShaderInputProviderDeclaration* pCurrent = m_pHead;
    while (pCurrent != nullptr)
    {
        numShaderInputProviderDeclarations += 1;

        pCurrent = pCurrent->m_pNextRegisteredShaderInputProviderDeclaration;
    }

    shaderInputProviderDeclarations.Reserve(numShaderInputProviderDeclarations);

    pCurrent = m_pHead;
    while (pCurrent != nullptr)
    {
        shaderInputProviderDeclarations.Add(pCurrent);

        pCurrent = pCurrent->m_pNextRegisteredShaderInputProviderDeclaration;
    }
}

void ShaderInputProviderManager::CopyShaderInputsToBuffer(const ShaderInputProvider& shaderInputProvider, void* pBuffer) const
{
    const ShaderInputProvider* pShaderInputProvider = &shaderInputProvider;

    ShaderInputProviderDeclaration* shaderInputProviderDeclaration = shaderInputProvider.GetShaderInputProviderDeclaration();

    for (shipUint32 i = 0; i < shaderInputProviderDeclaration->m_NumShaderInputCopyRanges; i++)
    {
        const ShaderInputProviderDeclaration::ShaderInputCopyRange& shaderInputCopyRange = shaderInputProviderDeclaration->m_ShaderInputCopyRanges[i];

        // Might need to change this to not always use a memcpy, overhead might be too big for small data sizes?
        void* pDest = reinterpret_cast<void*>(shipInt64(pBuffer) + shipInt64(shaderInputCopyRange.DataOffsetInBuffer));
        void* pSrc = reinterpret_cast<void*>(shipInt64(pShaderInputProvider) + shipInt64(shaderInputCopyRange.DataOffsetInProvider));

        memcpy(pDest, pSrc, shaderInputCopyRange.DataSizeToCopy);
    }
}

void ShaderInputProviderManager::CreateShaderInputProviderConstantBuffer(ShaderInputProvider* shaderInputProvider)
{
    SHIP_ASSERT(m_GfxRenderDevice != nullptr);
    GFXRenderDevice* gfxRenderDevice = static_cast<GFXRenderDevice*>(m_GfxRenderDevice);

    shipUint32 constantBufferSize = shaderInputProvider->GetRequiredSizeForProvider();
    constexpr shipBool dynamic = true;
    constexpr void* initialData = nullptr;

    shaderInputProvider->m_GfxConstantBufferHandle = gfxRenderDevice->CreateConstantBuffer(constantBufferSize, dynamic, initialData);
}

void ShaderInputProviderManager::DestroyShaderInputProviderConstantBuffer(ShaderInputProvider* shaderInputProvider)
{
    if (shaderInputProvider->m_GfxConstantBufferHandle.handle == InvalidGfxHandle)
    {
        return;
    }

    SHIP_ASSERT(m_GfxRenderDevice != nullptr);
    GFXRenderDevice* gfxRenderDevice = static_cast<GFXRenderDevice*>(m_GfxRenderDevice);

    gfxRenderDevice->DestroyConstantBuffer(shaderInputProvider->m_GfxConstantBufferHandle);
}

shipUint32 ShaderInputProviderManager::GetTexture2DHandlesFromProvider(const ShaderInputProvider& shaderInputProvider, GFXTexture2DHandle* pGfxTextureHandles) const
{
    shipUint32 numTexture2DHandles = 0;
    size_t providerAddress = reinterpret_cast<size_t>(&shaderInputProvider);

    ShaderInputProviderDeclaration* shaderInputProviderDeclaration = shaderInputProvider.GetShaderInputProviderDeclaration();

    shipUint32 numShaderInputDeclarations = 0;
    const ShaderInputProviderDeclaration::ShaderInputDeclaration* shaderInputDeclarations = shaderInputProviderDeclaration->GetShaderInputDeclarations(numShaderInputDeclarations);

    for (shipUint32 i = 0; i < numShaderInputDeclarations; i++)
    {
        const ShaderInputProviderDeclaration::ShaderInputDeclaration& shaderInputDeclaration = shaderInputDeclarations[i];

        if (shaderInputDeclaration.Type != ShaderInputType::Texture2D)
        {
            continue;
        }

        // Valid because we're guaranteeing at compile time that ShaderInputType::Texture2D are GFXTexture2DHandle
        void* pTexture2DHandleAddress = reinterpret_cast<void*>(providerAddress + size_t(shaderInputDeclaration.DataOffsetInProvider));
        pGfxTextureHandles[numTexture2DHandles] = *reinterpret_cast<GFXTexture2DHandle*>(pTexture2DHandleAddress);
        numTexture2DHandles += 1;
    }

    return numTexture2DHandles;
}

ShaderInputProviderDeclaration* ShaderInputProviderManager::FindShaderInputProviderDeclarationFromName(const StringA& shaderInputProviderDeclarationName) const
{
    ShaderInputProviderDeclaration* pCurrent = m_pHead;
    while (pCurrent != nullptr)
    {
        if (shaderInputProviderDeclarationName == pCurrent->m_ShaderInputProviderName)
        {
            return pCurrent;
        }

        pCurrent = pCurrent->m_pNextRegisteredShaderInputProviderDeclaration;
    }

    return nullptr;
}

const ShaderInputProvider* ShaderInputProviderManager::GetShaderInputProviderForDeclaration(
        const Array<const ShaderInputProvider*>& shaderInputProviders,
        const ShaderInputProviderDeclaration* shaderInputProviderDeclaration) const
{
    for (const ShaderInputProvider* shaderInputProvider : shaderInputProviders)
    {
        if (shaderInputProvider->GetShaderInputProviderDeclaration() == shaderInputProviderDeclaration)
        {
            return shaderInputProvider;
        }
    }

    return nullptr;
}

const shipChar* ShaderInputProviderManager::GetShaderInputNameFromProvider(
        const ShaderInputProviderDeclaration* shaderInputProviderDeclaration,
        shipInt32 dataOffsetInProvider) const
{
    shipUint32 numShaderInputDeclarations;
    const ShaderInputProviderDeclaration::ShaderInputDeclaration* shaderInputDeclarations = shaderInputProviderDeclaration->GetShaderInputDeclarations(numShaderInputDeclarations);

    for (shipUint32 i = 0; i < numShaderInputDeclarations; i++)
    {
        const ShaderInputProviderDeclaration::ShaderInputDeclaration& shaderInputDeclaration = shaderInputDeclarations[i];
        if (shaderInputDeclaration.DataOffsetInProvider == dataOffsetInProvider)
        {
            return shaderInputDeclaration.Name;
        }
    }

    SHIP_ASSERT_MSG(
            false,
            "Offset provided does not map to an input in the ShaderInputProvider %s",
            shaderInputProviderDeclaration->GetShaderInputProviderName());

    return nullptr;
}

const shipChar* ShaderInputProviderManager::GetShaderInputProviderConstantBufferName(const ShaderInputProviderDeclaration* shaderInputProviderDeclaration) const
{
    return StringFormat("%s%s", shaderInputProviderDeclaration->m_ShaderInputProviderName, "Data");
}

ShaderInputProviderManager& GetShaderInputProviderManager()
{
    return ShaderInputProviderManager::GetInstance();
}

ShaderInputProviderDeclaration::ShaderInputProviderDeclaration()
    : m_NumShaderInputDeclarations(0)
    , m_NumShaderInputCopyRanges(0)
    , m_RequiredSizeForProvider(0)
    , m_ShaderInputProviderDeclarationIndex(0)
    , m_pNextRegisteredShaderInputProviderDeclaration(nullptr)
{
    GetShaderInputProviderManager().RegisterShaderInputProviderDeclaration(*this);
}

const ShaderInputProviderDeclaration::ShaderInputDeclaration* ShaderInputProviderDeclaration::GetShaderInputDeclarations(shipUint32& numShaderInputDeclarations) const
{
    numShaderInputDeclarations = m_NumShaderInputDeclarations;

    return m_ShaderInputDeclarations;
}

shipUint32 ShaderInputProviderDeclaration::GetRequiredSizeForProvider() const
{
    return m_RequiredSizeForProvider;
}

shipUint32 ShaderInputProviderDeclaration::GetShaderInputProviderDeclarationIndex() const
{
    return m_ShaderInputProviderDeclarationIndex;
}

ShaderInputProviderUsage ShaderInputProviderDeclaration::GetShaderInputProviderUsage() const
{
    return m_ShaderInputProviderUsage;
}

const shipChar* ShaderInputProviderDeclaration::GetShaderInputProviderName() const
{
    return m_ShaderInputProviderName;
}

shipBool ShaderInputProviderDeclaration::HasShaderInput(const shipChar* shaderInputName, shipInt32& offsetInProvider) const
{
    for (shipUint32 i = 0; i < m_NumShaderInputDeclarations; i++)
    {
        if (StringCompare(m_ShaderInputDeclarations[i].Name, shaderInputName) == 0)
        {
            offsetInProvider = m_ShaderInputDeclarations[i].DataOffsetInProvider;
            return true;
        }
    }

    return false;
}

ShaderInputProviderDeclaration::ShaderInputDeclaration const * const ShaderInputProviderDeclaration::GetShaderInput(const shipChar* shaderInputName, shipInt32& offsetInProvider) const
{
    for (shipUint32 i = 0; i < m_NumShaderInputDeclarations; i++)
    {
        if (StringCompare(m_ShaderInputDeclarations[i].Name, shaderInputName) == 0)
        {
            offsetInProvider = m_ShaderInputDeclarations[i].DataOffsetInProvider;
            return &m_ShaderInputDeclarations[i];
        }
    }

    return nullptr;
}

// template <typename T> ShaderInputScalarType GetShaderInputScalarType(const T& data) { return ShaderInputScalarType::Unknown; }
template <> ShaderInputScalarType GetShaderInputScalarType(const shipFloat& data) { return ShaderInputScalarType::Float; }
template <> ShaderInputScalarType GetShaderInputScalarType(const shipVec2& data) { return ShaderInputScalarType::Float2; }
template <> ShaderInputScalarType GetShaderInputScalarType(const shipVec3& data) { return ShaderInputScalarType::Float3; }
template <> ShaderInputScalarType GetShaderInputScalarType(const shipVec4& data) { return ShaderInputScalarType::Float4; }

template <> ShaderInputScalarType GetShaderInputScalarType(const shipDouble& data) { return ShaderInputScalarType::Double; }
template <> ShaderInputScalarType GetShaderInputScalarType(const shipDVec2& data) { return ShaderInputScalarType::Double2; }
template <> ShaderInputScalarType GetShaderInputScalarType(const shipDVec3& data) { return ShaderInputScalarType::Double3; }
template <> ShaderInputScalarType GetShaderInputScalarType(const shipDVec4& data) { return ShaderInputScalarType::Double4; }

template <> ShaderInputScalarType GetShaderInputScalarType(const shipInt32& data) { return ShaderInputScalarType::Int; }
template <> ShaderInputScalarType GetShaderInputScalarType(const shipIVec2& data) { return ShaderInputScalarType::Int2; }
template <> ShaderInputScalarType GetShaderInputScalarType(const shipIVec3& data) { return ShaderInputScalarType::Int3; }
template <> ShaderInputScalarType GetShaderInputScalarType(const shipIVec4& data) { return ShaderInputScalarType::Int4; }

template <> ShaderInputScalarType GetShaderInputScalarType(const shipUint32& data) { return ShaderInputScalarType::Uint; }
template <> ShaderInputScalarType GetShaderInputScalarType(const shipUVec2& data) { return ShaderInputScalarType::Uint2; }
template <> ShaderInputScalarType GetShaderInputScalarType(const shipUVec3& data) { return ShaderInputScalarType::Uint3; }
template <> ShaderInputScalarType GetShaderInputScalarType(const shipUVec4& data) { return ShaderInputScalarType::Uint4; }

template <> ShaderInputScalarType GetShaderInputScalarType(const shipBool& data) { return ShaderInputScalarType::Bool; }
template <> ShaderInputScalarType GetShaderInputScalarType(const shipBVec2& data) { return ShaderInputScalarType::Bool2; }
template <> ShaderInputScalarType GetShaderInputScalarType(const shipBVec3& data) { return ShaderInputScalarType::Bool3; }
template <> ShaderInputScalarType GetShaderInputScalarType(const shipBVec4& data) { return ShaderInputScalarType::Bool4; }

template <> ShaderInputScalarType GetShaderInputScalarType(const shipMat2x2& data) { return ShaderInputScalarType::Float2x2; }
template <> ShaderInputScalarType GetShaderInputScalarType(const shipMat3x3& data) { return ShaderInputScalarType::Float3x3; }
template <> ShaderInputScalarType GetShaderInputScalarType(const shipMat4x4& data) { return ShaderInputScalarType::Float4x4; }

template <> ShaderInputScalarType GetShaderInputScalarType(const shipDMat2x2& data) { return ShaderInputScalarType::Double2x2; }
template <> ShaderInputScalarType GetShaderInputScalarType(const shipDMat3x3& data) { return ShaderInputScalarType::Double3x3; }
template <> ShaderInputScalarType GetShaderInputScalarType(const shipDMat4x4& data) { return ShaderInputScalarType::Double4x4; }

}