#include <graphics/material/materialunifiedconstantbuffer.h>

#include <graphics/shader/shaderinputprovider.h>

#include <graphics/wrapper/wrapper.h>

#include <system/array.h>
#include <system/atomicoperations.h>
#include <system/systemdebug.h>

namespace Shipyard
{;

GFXMaterialUnifiedConstantBuffer::GFXMaterialUnifiedConstantBuffer()
    : m_pGfxRenderDevice(nullptr)
    , m_pAllocator(nullptr)
    , m_pConstantBufferHeap(nullptr)
    , m_ConstantBufferHeapSize(0)
    , m_IsDirty(false)
{
}

GFXMaterialUnifiedConstantBuffer::~GFXMaterialUnifiedConstantBuffer()
{
    Destroy();
}

shipBool GFXMaterialUnifiedConstantBuffer::Create(BaseRenderDevice& renderDevice, BaseAllocator* pAllocator, size_t maxConstantBufferPoolHeapSize)
{
    SHIP_ASSERT(maxConstantBufferPoolHeapSize > 0);

    if (pAllocator == nullptr)
    {
        pAllocator = &GetGlobalAllocator();
    }
    m_pAllocator = pAllocator;

    Array<ShaderInputProviderDeclaration*> shaderInputProviderDeclarations;
    ShaderInputProviderManager::GetInstance().GetShaderInputProviderDeclarations(shaderInputProviderDeclarations);

    shipUint32 numShaderInputProviders = shaderInputProviderDeclarations.Size();
    m_ShaderInputProviderRequiredSizes.Reserve(numShaderInputProviders);

    size_t totalMemoryNeeded = 0;
    for (ShaderInputProviderDeclaration* shaderInputProviderDeclaration : shaderInputProviderDeclarations)
    {
        shipUint32 requiredSizeForProvider = shaderInputProviderDeclaration->GetRequiredSizeForProvider();
        m_ShaderInputProviderRequiredSizes.Add(requiredSizeForProvider);

        totalMemoryNeeded += requiredSizeForProvider;
    }
    
    totalMemoryNeeded *= SHIP_MAX_BATCHED_DRAW_CALLS_PER_MATERIAL;

    if (totalMemoryNeeded > maxConstantBufferPoolHeapSize)
    {
        return false;
    }

    m_pConstantBufferHeap = SHIP_ALLOC_EX(m_pAllocator, totalMemoryNeeded, SHIP_CACHE_LINE_SIZE);
    if (m_pConstantBufferHeap == nullptr)
    {
        return false;
    }

    m_ConstantBufferHeapSize = totalMemoryNeeded;
    
    m_WriteOffsetsPerShaderInputProvider.Reserve(numShaderInputProviders);
    for (shipUint32 i = 0; i < numShaderInputProviders; i++)
    {
        m_WriteOffsetsPerShaderInputProvider.Add(0);
    }

    m_pGfxRenderDevice = &renderDevice;
    GFXRenderDevice& gfxRenderDevice = *static_cast<GFXRenderDevice*>(m_pGfxRenderDevice);

    constexpr shipBool dynamic = true;
    void* initialData = nullptr;
    m_MaterialUnifiedConstantBuffer = gfxRenderDevice.CreateByteBuffer(
            ByteBuffer::ByteBufferCreationFlags::ByteBufferCreationFlags_ShaderResourceView,
            shipUint32(m_ConstantBufferHeapSize),
            dynamic,
            initialData);

    return true;
}

void GFXMaterialUnifiedConstantBuffer::Destroy()
{
    GFXRenderDevice& gfxRenderDevice = *static_cast<GFXRenderDevice*>(m_pGfxRenderDevice);
    gfxRenderDevice.DestroyByteBuffer(m_MaterialUnifiedConstantBuffer);

    SHIP_FREE_EX(m_pAllocator, m_pConstantBufferHeap);

    m_pGfxRenderDevice = nullptr;
    m_pAllocator = nullptr;
    m_pConstantBufferHeap = nullptr;
    m_ConstantBufferHeapSize = 0;

    m_ShaderInputProviderRequiredSizes.Clear();
    m_WriteOffsetsPerShaderInputProvider.Clear();
}

void GFXMaterialUnifiedConstantBuffer::PrepareForNextDrawCall()
{
    for (size_t& writeOffsetForShaderInputProvider : m_WriteOffsetsPerShaderInputProvider)
    {
        writeOffsetForShaderInputProvider = 0;
    }
}

void GFXMaterialUnifiedConstantBuffer::ApplyShaderInputProviders(const Array<const ShaderInputProvider*>& shaderInputProviders)
{
    for (const ShaderInputProvider* shaderInputProvider : shaderInputProviders)
    {
        SHIP_ASSERT(shaderInputProvider != nullptr);

        ApplyShaderInputProvider(*shaderInputProvider);
    }
}

void GFXMaterialUnifiedConstantBuffer::ApplyShaderInputProvider(const ShaderInputProvider& shaderInputProvider)
{
    void* pMappedBuffer = MapBufferForShaderInputProvider(shaderInputProvider);
    SHIP_ASSERT(pMappedBuffer != nullptr);

    ShaderInputProviderManager::GetInstance().CopyShaderInputsToBuffer(shaderInputProvider, pMappedBuffer);
}

GFXByteBufferHandle GFXMaterialUnifiedConstantBuffer::BindMaterialUnfiedConstantBuffer(DirectRenderCommandList& gfxDirectRenderCommandList)
{
    return GetUpdatedMaterialUnifiedConstantBuffer(gfxDirectRenderCommandList);
}

void* GFXMaterialUnifiedConstantBuffer::MapBufferForShaderInputProvider(const ShaderInputProvider& shaderInputProvider)
{
    shipUint32 shaderInputProviderIndex = GetShaderInputProviderManager().GetShaderInputProviderDeclarationIndex(shaderInputProvider);

    shipUint32 requiredSizeForProvider = m_ShaderInputProviderRequiredSizes[shaderInputProviderIndex];
    size_t& writeOffset = m_WriteOffsetsPerShaderInputProvider[shaderInputProviderIndex];

    size_t writeOffsetToUse = AtomicOperations::Add(writeOffset, size_t(requiredSizeForProvider));

#ifdef SHIP_ENABLE_ASSERTS
    size_t maximumOffset = SHIP_MAX_BATCHED_DRAW_CALLS_PER_MATERIAL * requiredSizeForProvider;
    SHIP_ASSERT_MSG(writeOffset <= maximumOffset, "MapBufferForShaderInputProvider got called too many times for a single draw call!");
#endif // #ifdef SHIP_ENABLE_ASSERTS

    m_IsDirty = true;

    return reinterpret_cast<void*>(size_t(m_pConstantBufferHeap) + writeOffsetToUse);
}

GFXByteBufferHandle GFXMaterialUnifiedConstantBuffer::GetUpdatedMaterialUnifiedConstantBuffer(DirectRenderCommandList& gfxDirectRenderCommandList)
{
    if (m_IsDirty)
    {
        // For now, Write_Discard will do. Will revisit later.
        constexpr shipUint32 bufferOffset = 0;
        void* pMappedBuffer = gfxDirectRenderCommandList.MapByteBuffer(m_MaterialUnifiedConstantBuffer, MapFlag::Write_Discard, bufferOffset);

        memcpy(pMappedBuffer, m_pConstantBufferHeap, m_ConstantBufferHeapSize);

        m_IsDirty = false;
    }

    return m_MaterialUnifiedConstantBuffer;
}

SHIPYARD_API GFXMaterialUnifiedConstantBuffer& GetGFXMaterialUnifiedConstantBuffer()
{
    return GFXMaterialUnifiedConstantBuffer::GetInstance();
}

}