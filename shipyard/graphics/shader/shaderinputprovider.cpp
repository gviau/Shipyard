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
    ShaderInputProviderDeclaration* pCurrent = m_pHead;
    while (pCurrent != nullptr)
    {
        WriteSingleShaderInputProviderFile(pCurrent);

        pCurrent = pCurrent->m_pNextRegisteredShaderInputProviderDeclaration;
    }

    return true;
}

bool ShaderInputProviderManager::WriteSingleShaderInputProviderFile(ShaderInputProviderDeclaration* pShaderInputProviderDeclaration)
{
    StringT shaderInputProviderFileName;
    shaderInputProviderFileName = "shaders\\shaderinputproviders\\";
    shaderInputProviderFileName += pShaderInputProviderDeclaration->m_ShaderInputProviderName;
    shaderInputProviderFileName += ".hlsl";

    FileHandler shaderInputProviderFile;
    shaderInputProviderFile.Open(shaderInputProviderFileName.GetBuffer(), FileHandlerOpenFlag(FileHandlerOpenFlag::FileHandlerOpenFlag_Write | FileHandlerOpenFlag::FileHandlerOpenFlag_Create));

    if (!shaderInputProviderFile.IsOpen())
    {
        return false;
    }

    StringA content;

    content += "/**\n* AUTO-GENERATED FILE\n**/\n";

    if (ShaderInputProviderUtils::IsUsingConstantBuffer(pShaderInputProviderDeclaration->m_ShaderInputProviderUsage))
    {
        content += "cbuffer ";
    }
    else
    {
        content += "struct ";
    }

    content += pShaderInputProviderDeclaration->m_ShaderInputProviderName;

    // Register is temporary, until ShaderCompiler deduces binding points by reflection
    content += "Data : register(b0)\n{\n";

    for (uint32_t i = 0; i < pShaderInputProviderDeclaration->m_NumShaderInputDeclarations; i++)
    {
        ShaderInputProviderDeclaration::ShaderInputDeclaration& shaderInputDeclaration = pShaderInputProviderDeclaration->m_ShaderInputDeclarations[i];

        if (shaderInputDeclaration.Type == ShaderInputType::Scalar)
        {
            WriteSingleShaderScalarInput(shaderInputDeclaration, content);
        }
    }

    content += "};\n";

    WriteShaderInputsOutsideOfStruct(pShaderInputProviderDeclaration, content);

    shaderInputProviderFile.WriteChars(0, content.GetBuffer(), content.Size());
    shaderInputProviderFile.Close();

    return true;
}

bool ShaderInputProviderManager::WriteSingleShaderScalarInput(const ShaderInputProviderDeclaration::ShaderInputDeclaration& shaderInputDeclaration, StringA& content)
{
    // Indented with respect to struct root, for easier readability.
    content += "    ";

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
    content += " : register(t0);\n";

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

}