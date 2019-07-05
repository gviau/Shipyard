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
    bool IsUsingConstantBuffer(ShaderInputProviderUsage shaderInputProviderUsage)
    {
        SHIP_STATIC_ASSERT_MSG(uint32_t(ShaderInputProviderUsage::Count) == 2, "ShaderInputProviderUsage size changed, need to update this method!");

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

bool ShaderInputProviderManager::Initialize(BaseRenderDevice& gfxRenderDevice)
{
    m_GfxRenderDevice = &gfxRenderDevice;

    uint32_t shaderInputProviderIdx = 0;
    ShaderInputProviderDeclaration* pCurrent = m_pHead;
    while (pCurrent != nullptr)
    {
        pCurrent->m_ShaderInputProviderDeclarationIndex = shaderInputProviderIdx;

        pCurrent = pCurrent->m_pNextRegisteredShaderInputProviderDeclaration;

        shaderInputProviderIdx += 1;
    }

    return WriteEveryShaderInputProviderFile();
}

void ShaderInputProviderManager::Destroy()
{

}

uint32_t ShaderInputProviderManager::GetRequiredSizeForProvider(const ShaderInputProvider& shaderInputProvider) const
{
    return shaderInputProvider.GetRequiredSizeForProvider();
}

uint32_t ShaderInputProviderManager::GetShaderInputProviderDeclarationIndex(const ShaderInputProvider& shaderInputProvider) const
{
    return shaderInputProvider.GetShaderInputProviderDeclarationIndex();
}

bool ShaderInputProviderManager::WriteEveryShaderInputProviderFile()
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

bool ShaderInputProviderManager::WriteShaderInputProviderUtilsFile()
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

    content += "ByteAddressBuffer g_UnifiedConstantBuffer;\n";

    // content += "#define UINT_TO_FLOAT(val) asfloat(val)\n";
    // content += "#define UINT2_TO_FLOAT2(val) float2(UINT_TO_FLOAT(val.x), UINT_TO_FLOAT(val.y))\n";
    // content += "#define UINT3_TO_FLOAT3(val) float3(UINT_TO_FLOAT(val.x), UINT_TO_FLOAT(val.y))\n";

    content += "#endif";

    shaderInputProviderUtilsFile.WriteChars(0, content.GetBuffer(), content.Size());

    return true;
}

bool ShaderInputProviderManager::WriteSingleShaderInputProviderFile(ShaderInputProviderDeclaration* pShaderInputProviderDeclaration, size_t numBytesBeforeThisProviderInUnifiedBuffer)
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

    bool isUsingConstantBuffer = ShaderInputProviderUtils::IsUsingConstantBuffer(pShaderInputProviderDeclaration->m_ShaderInputProviderUsage);

    if (isUsingConstantBuffer)
    {
        content += "cbuffer ";

        content += pShaderInputProviderDeclaration->m_ShaderInputProviderName;

        content += "Data";

        // Register is temporary, until ShaderCompiler deduces binding points by reflection
        content += " : register(b0)";
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

    for (uint32_t i = 0; i < pShaderInputProviderDeclaration->m_NumShaderInputDeclarations; i++)
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

bool ShaderInputProviderManager::WriteSingleShaderScalarInput(const ShaderInputProviderDeclaration::ShaderInputDeclaration& shaderInputDeclaration, StringA& content, bool isUsingConstantBuffer)
{
    // Indented with respect to struct root, for easier readability.
    content += "    ";

    if (!isUsingConstantBuffer)
    {
        content += "static ";
    }

    SHIP_STATIC_ASSERT_MSG(uint32_t(ShaderInputScalarType::Count) == 33, "ShaderInputScalarType size changed, need to update this switch case!");

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

bool ShaderInputProviderManager::WriteShaderInputsOutsideOfStruct(ShaderInputProviderDeclaration* pShaderInputProviderDeclaration, StringA& content)
{
    for (uint32_t i = 0; i < pShaderInputProviderDeclaration->m_NumShaderInputDeclarations; i++)
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

bool ShaderInputProviderManager::WriteSingleShaderInputOutsideOfStruct(const ShaderInputProviderDeclaration::ShaderInputDeclaration& shaderInputDeclaration, StringA& content)
{
    SHIP_STATIC_ASSERT_MSG(uint32_t(ShaderInputScalarType::Count) == 33, "ShaderInputScalarType size changed, need to update this switch case!");

    switch (shaderInputDeclaration.Type)
    {
    case ShaderInputType::Texture2D:
        content += "Texture2D<";
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

    // Register is temporary, until ShaderCompiler deduces binding points by reflection
    content += " : register(t1);\n";

    return true;
}

bool ShaderInputProviderManager::WriteShaderInputProviderUnifiedBufferAccessMethod(ShaderInputProviderDeclaration* pShaderInputProviderDeclaration, StringA& content)
{
    content += "void Load";
    content += pShaderInputProviderDeclaration->m_ShaderInputProviderName;
    content += "ConstantsForInstance(uint instanceIndex)\n{\n";

    content += "    uint offsetInUnifiedBuffer = instanceIndex * SHADER_INPUT_PROVIDER_STRIDE + SHADER_INPUT_PROVIDER_OFFSET_IN_GLOBAL_BUFFER;\n";

    for (uint32_t i = 0; i < pShaderInputProviderDeclaration->m_NumShaderInputDeclarations; i++)
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

    void GetBaseHighLevelScalarType(ScalarHighLevelType scalarHighLevelType, uint32_t numComponentsX, StringA& castType)
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

    void AddLoadingCodeFromUnifiedBufferForShaderInput(ScalarHighLevelType scalarHighLevelType, uint32_t numComponentsX, uint32_t numComponentsY, uint32_t offsetInBuffer, StringA& content)
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

        for (uint32_t y = 0; y < numComponentsY; y++)
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

            switch (numComponentsX)
            {
            case 1:
                content += StringFormat("g_UnifiedConstantBuffer.Load(offsetInUnifiedBuffer + 0x%x)", offsetInBuffer);
                break;

            case 2:
                content += StringFormat("g_UnifiedConstantBuffer.Load2(offsetInUnifiedBuffer + 0x%x)", offsetInBuffer);
                break;

            case 3:
                content += StringFormat("g_UnifiedConstantBuffer.Load3(offsetInUnifiedBuffer + 0x%x)", offsetInBuffer);
                break;

            case 4:
                content += StringFormat("g_UnifiedConstantBuffer.Load4(offsetInUnifiedBuffer + 0x%x)", offsetInBuffer);
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

            bool isNotLastYComponent = (y < numComponentsY - 1);
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

bool ShaderInputProviderManager::WriteShaderInputScalarLoadFromBuffer(const ShaderInputProviderDeclaration::ShaderInputDeclaration& shaderInputDeclaration, StringA& content)
{
    // Indented with respect to method root, for easier readability.
    content += "    ";
    content += shaderInputDeclaration.Name;
    content += " = ";

    SHIP_STATIC_ASSERT_MSG(uint32_t(ShaderInputScalarType::Count) == 33, "ShaderInputScalarType size changed, need to update this switch case!");

    ShaderInputProviderUtils::ScalarHighLevelType scalarHighLevelType = ShaderInputProviderUtils::ScalarHighLevelType::Float;
    uint32_t numComponentsX = 0;
    uint32_t numComponentsY = 0;

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
    uint32_t numShaderInputProviderDeclarations = 0;
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

    for (uint32_t shaderInputIdx = 0; shaderInputIdx < shaderInputProviderDeclaration->m_NumShaderInputDeclarations; shaderInputIdx++)
    {
        const ShaderInputProviderDeclaration::ShaderInputDeclaration& shaderInputDeclaration = shaderInputProviderDeclaration->m_ShaderInputDeclarations[shaderInputIdx];
        if (shaderInputDeclaration.Type != ShaderInputType::Scalar)
        {
            continue;
        }

        // Might need to change this to not always use a memcpy, overhead might be too big for small data sizes?
        void* pDest = reinterpret_cast<void*>(int64_t(pBuffer) + int64_t(shaderInputDeclaration.DataOffsetInBuffer));
        void* pSrc = reinterpret_cast<void*>(int64_t(pShaderInputProvider) + int64_t(shaderInputDeclaration.DataOffsetInProvider));

        memcpy(pDest, pSrc, shaderInputDeclaration.DataSize);
    }
}

void ShaderInputProviderManager::CreateShaderInputProviderConstantBuffer(ShaderInputProvider* shaderInputProvider)
{
    SHIP_ASSERT(m_GfxRenderDevice != nullptr);
    GFXRenderDevice* gfxRenderDevice = static_cast<GFXRenderDevice*>(m_GfxRenderDevice);

    uint32_t constantBufferSize = shaderInputProvider->GetRequiredSizeForProvider();
    constexpr bool dynamic = true;
    constexpr void* initialData = nullptr;

    shaderInputProvider->m_GfxConstantBufferHandle = gfxRenderDevice->CreateConstantBuffer(constantBufferSize, dynamic, initialData);
}

void ShaderInputProviderManager::DestroyShaderInputProviderConstantBuffer(ShaderInputProvider* shaderInputProvider)
{
    SHIP_ASSERT(m_GfxRenderDevice != nullptr);
    GFXRenderDevice* gfxRenderDevice = static_cast<GFXRenderDevice*>(m_GfxRenderDevice);

    gfxRenderDevice->DestroyConstantBuffer(shaderInputProvider->m_GfxConstantBufferHandle);
}

uint32_t ShaderInputProviderManager::GetTexture2DHandlesFromProvider(const ShaderInputProvider& shaderInputProvider, GFXTexture2DHandle* pGfxTextureHandles) const
{
    uint32_t numTexture2DHandles = 0;
    size_t providerAddress = reinterpret_cast<size_t>(&shaderInputProvider);

    ShaderInputProviderDeclaration* shaderInputProviderDeclaration = shaderInputProvider.GetShaderInputProviderDeclaration();

    uint32_t numShaderInputDeclarations = 0;
    const ShaderInputProviderDeclaration::ShaderInputDeclaration* shaderInputDeclarations = shaderInputProviderDeclaration->GetShaderInputDeclarations(numShaderInputDeclarations);

    for (uint32_t i = 0; i < numShaderInputDeclarations; i++)
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

ShaderInputProviderManager& GetShaderInputProviderManager()
{
    return ShaderInputProviderManager::GetInstance();
}

ShaderInputProviderDeclaration::ShaderInputProviderDeclaration()
    : m_NumShaderInputDeclarations(0)
    , m_RequiredSizeForProvider(0)
    , m_ShaderInputProviderDeclarationIndex(0)
    , m_pNextRegisteredShaderInputProviderDeclaration(nullptr)
{
    GetShaderInputProviderManager().RegisterShaderInputProviderDeclaration(*this);
}

const ShaderInputProviderDeclaration::ShaderInputDeclaration* ShaderInputProviderDeclaration::GetShaderInputDeclarations(uint32_t& numShaderInputDeclarations) const
{
    numShaderInputDeclarations = m_NumShaderInputDeclarations;

    return m_ShaderInputDeclarations;
}

uint32_t ShaderInputProviderDeclaration::GetRequiredSizeForProvider() const
{
    return m_RequiredSizeForProvider;
}

uint32_t ShaderInputProviderDeclaration::GetShaderInputProviderDeclarationIndex() const
{
    return m_ShaderInputProviderDeclarationIndex;
}

ShaderInputProviderUsage ShaderInputProviderDeclaration::GetShaderInputProviderUsage() const
{
    return m_ShaderInputProviderUsage;
}

// template <typename T> ShaderInputScalarType GetShaderInputScalarType(const T& data) { return ShaderInputScalarType::Unknown; }
template <> ShaderInputScalarType GetShaderInputScalarType(const float& data) { return ShaderInputScalarType::Float; }
template <> ShaderInputScalarType GetShaderInputScalarType(const glm::vec2& data) { return ShaderInputScalarType::Float2; }
template <> ShaderInputScalarType GetShaderInputScalarType(const glm::vec3& data) { return ShaderInputScalarType::Float3; }
template <> ShaderInputScalarType GetShaderInputScalarType(const glm::vec4& data) { return ShaderInputScalarType::Float4; }

template <> ShaderInputScalarType GetShaderInputScalarType(const double& data) { return ShaderInputScalarType::Double; }
template <> ShaderInputScalarType GetShaderInputScalarType(const glm::dvec2& data) { return ShaderInputScalarType::Double2; }
template <> ShaderInputScalarType GetShaderInputScalarType(const glm::dvec3& data) { return ShaderInputScalarType::Double3; }
template <> ShaderInputScalarType GetShaderInputScalarType(const glm::dvec4& data) { return ShaderInputScalarType::Double4; }

template <> ShaderInputScalarType GetShaderInputScalarType(const int32_t& data) { return ShaderInputScalarType::Int; }
template <> ShaderInputScalarType GetShaderInputScalarType(const glm::ivec2& data) { return ShaderInputScalarType::Int2; }
template <> ShaderInputScalarType GetShaderInputScalarType(const glm::ivec3& data) { return ShaderInputScalarType::Int3; }
template <> ShaderInputScalarType GetShaderInputScalarType(const glm::ivec4& data) { return ShaderInputScalarType::Int4; }

template <> ShaderInputScalarType GetShaderInputScalarType(const uint32_t& data) { return ShaderInputScalarType::Uint; }
template <> ShaderInputScalarType GetShaderInputScalarType(const glm::uvec2& data) { return ShaderInputScalarType::Uint2; }
template <> ShaderInputScalarType GetShaderInputScalarType(const glm::uvec3& data) { return ShaderInputScalarType::Uint3; }
template <> ShaderInputScalarType GetShaderInputScalarType(const glm::uvec4& data) { return ShaderInputScalarType::Uint4; }

template <> ShaderInputScalarType GetShaderInputScalarType(const bool& data) { return ShaderInputScalarType::Bool; }
template <> ShaderInputScalarType GetShaderInputScalarType(const glm::bvec2& data) { return ShaderInputScalarType::Bool2; }
template <> ShaderInputScalarType GetShaderInputScalarType(const glm::bvec3& data) { return ShaderInputScalarType::Bool3; }
template <> ShaderInputScalarType GetShaderInputScalarType(const glm::bvec4& data) { return ShaderInputScalarType::Bool4; }

template <> ShaderInputScalarType GetShaderInputScalarType(const glm::mat2x2& data) { return ShaderInputScalarType::Float2x2; }
template <> ShaderInputScalarType GetShaderInputScalarType(const glm::mat3x3& data) { return ShaderInputScalarType::Float3x3; }
template <> ShaderInputScalarType GetShaderInputScalarType(const glm::mat4x4& data) { return ShaderInputScalarType::Float4x4; }

template <> ShaderInputScalarType GetShaderInputScalarType(const glm::dmat2x2& data) { return ShaderInputScalarType::Double2x2; }
template <> ShaderInputScalarType GetShaderInputScalarType(const glm::dmat3x3& data) { return ShaderInputScalarType::Double3x3; }
template <> ShaderInputScalarType GetShaderInputScalarType(const glm::dmat4x4& data) { return ShaderInputScalarType::Double4x4; }

}