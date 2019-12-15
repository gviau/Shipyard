#include <graphics/graphicsprecomp.h>

#include <graphics/shader/shaderresourcebinder.h>

#include <graphics/material/gfxmaterialunifiedconstantbuffer.h>

#include <graphics/shader/shaderinputprovider.h>

#include <system/systemdebug.h>

namespace Shipyard
{;

void ShaderResourceBinder::AddShaderResourceBinderEntryForProviderToDescriptor(
        const ShaderInputProviderDeclaration* shaderInputProviderDeclaration,
        shipUint16 rootIndexToBindTo,
        ShaderVisibility shaderStageToBindTo)
{
    ShaderResourceBinderEntry& shaderResourceBinderEntry = m_ShaderResourceBinderEntries.Grow();
    shaderResourceBinderEntry.Declaration = shaderInputProviderDeclaration;
    shaderResourceBinderEntry.RootIndexToBindTo = rootIndexToBindTo;
    shaderResourceBinderEntry.BindDescriptorTable = false;

    shaderResourceBinderEntry.BindConstantBuffer = ShaderInputProviderUtils::IsUsingConstantBuffer(shaderInputProviderDeclaration->GetShaderInputProviderUsage());
    shaderResourceBinderEntry.BindDescriptor = false;
    shaderResourceBinderEntry.BindGlobalBuffer = false;

    shaderResourceBinderEntry.ShaderStageToBindTo = shaderStageToBindTo;
}

void ShaderResourceBinder::AddShaderResourceBinderEntryForProviderToDescriptorTable(
        const ShaderInputProviderDeclaration* shaderInputProviderDeclaration,
        shipUint16 rootIndexToBindTo,
        shipUint16 descriptorRangeIndexToBind,
        shipUint16 descriptorRangeEntryIndexToBind,
        ShaderVisibility shaderStageToBindTo)
{
    ShaderResourceBinderEntry& shaderResourceBinderEntry = m_ShaderResourceBinderEntries.Grow();
    shaderResourceBinderEntry.Declaration = shaderInputProviderDeclaration;
    shaderResourceBinderEntry.RootIndexToBindTo = rootIndexToBindTo;
    shaderResourceBinderEntry.DescriptorRangeIndexToBindTo = descriptorRangeIndexToBind;
    shaderResourceBinderEntry.DescriptorRangeEntryIndexToBind = descriptorRangeEntryIndexToBind;
    shaderResourceBinderEntry.BindDescriptorTable = true;

    shaderResourceBinderEntry.BindConstantBuffer = ShaderInputProviderUtils::IsUsingConstantBuffer(shaderInputProviderDeclaration->GetShaderInputProviderUsage());
    shaderResourceBinderEntry.BindDescriptor = false;
    shaderResourceBinderEntry.BindGlobalBuffer = false;

    shaderResourceBinderEntry.ShaderStageToBindTo = shaderStageToBindTo;
}

void ShaderResourceBinder::AddShaderResourceBinderEntryForDescriptorToDescriptor(
        const ShaderInputProviderDeclaration* shaderInputProviderDeclaration,
        shipUint16 rootIndexToBindTo,
        shipInt32 offsetInProvider,
        ShaderInputType shaderInputType,
        ShaderVisibility shaderStageToBindTo)
{
    ShaderResourceBinderEntry& shaderResourceBinderEntry = m_ShaderResourceBinderEntries.Grow();
    shaderResourceBinderEntry.Declaration = shaderInputProviderDeclaration;
    shaderResourceBinderEntry.RootIndexToBindTo = rootIndexToBindTo;
    shaderResourceBinderEntry.BindDescriptorTable = false;

    shaderResourceBinderEntry.DataOffsetInProvider = offsetInProvider;
    shaderResourceBinderEntry.DescriptorType = shaderInputType;

    shaderResourceBinderEntry.BindConstantBuffer = false;
    shaderResourceBinderEntry.BindDescriptor = true;
    shaderResourceBinderEntry.BindGlobalBuffer = false;

    shaderResourceBinderEntry.ShaderStageToBindTo = shaderStageToBindTo;
}

void ShaderResourceBinder::AddShaderResourceBinderEntryForDescriptorToDescriptorTable(
        const ShaderInputProviderDeclaration* shaderInputProviderDeclaration,
        shipUint16 rootIndexToBindTo,
        shipUint16 descriptorRangeIndexToBind,
        shipUint16 descriptorRangeEntryIndexToBind,
        shipInt32 offsetInProvider,
        ShaderInputType shaderInputType,
        ShaderVisibility shaderStageToBindTo)
{
    ShaderResourceBinderEntry& shaderResourceBinderEntry = m_ShaderResourceBinderEntries.Grow();
    shaderResourceBinderEntry.Declaration = shaderInputProviderDeclaration;
    shaderResourceBinderEntry.RootIndexToBindTo = rootIndexToBindTo;
    shaderResourceBinderEntry.DescriptorRangeIndexToBindTo = descriptorRangeIndexToBind;
    shaderResourceBinderEntry.DescriptorRangeEntryIndexToBind = descriptorRangeEntryIndexToBind;
    shaderResourceBinderEntry.BindDescriptorTable = true;

    shaderResourceBinderEntry.DataOffsetInProvider = offsetInProvider;
    shaderResourceBinderEntry.DescriptorType = shaderInputType;

    shaderResourceBinderEntry.BindConstantBuffer = false;
    shaderResourceBinderEntry.BindDescriptor = true;
    shaderResourceBinderEntry.BindGlobalBuffer = false;

    shaderResourceBinderEntry.ShaderStageToBindTo = shaderStageToBindTo;
}

void ShaderResourceBinder::AddShaderResourceBinderEntryForSamplerToDescriptorTable(
        const SamplerState& samplerState,
        shipUint16 rootIndexToBindTo,
        shipUint16 descriptorRangeIndexToBind,
        shipUint16 descriptorRangeEntryIndexToBind,
        ShaderVisibility shaderStageToBindTo)
{
    ShaderResourceBinderEntry& shaderResourceBinderEntry = m_ShaderResourceBinderEntries.Grow();
    shaderResourceBinderEntry.RootIndexToBindTo = rootIndexToBindTo;
    shaderResourceBinderEntry.DescriptorRangeIndexToBindTo = descriptorRangeIndexToBind;
    shaderResourceBinderEntry.DescriptorRangeEntryIndexToBind = descriptorRangeEntryIndexToBind;

    shaderResourceBinderEntry.ShaderStageToBindTo = shaderStageToBindTo;

    shaderResourceBinderEntry.Sampler = samplerState;
    shaderResourceBinderEntry.BindSamplerState = true;
}

void ShaderResourceBinder::AddShaderResourceBinderEntryForGlobalBufferToDescriptor(
        ShaderInputProviderUsage shaderInputProviderUsage,
        shipUint16 rootIndexToBindTo,
        ShaderVisibility shaderStageToBindTo)
{
    ShaderResourceBinderEntry& shaderResourceBinderEntry = m_ShaderResourceBinderEntries.Grow();
    shaderResourceBinderEntry.GlobalBufferUsage = shaderInputProviderUsage;
    shaderResourceBinderEntry.RootIndexToBindTo = rootIndexToBindTo;
    shaderResourceBinderEntry.BindDescriptorTable = false;

    shaderResourceBinderEntry.BindConstantBuffer = false;
    shaderResourceBinderEntry.BindDescriptor = false;
    shaderResourceBinderEntry.BindGlobalBuffer = true;

    shaderResourceBinderEntry.ShaderStageToBindTo = shaderStageToBindTo;
}

void ShaderResourceBinder::AddShaderResourceBinderEntryForGlobalBufferToDescriptorTable(
        ShaderInputProviderUsage shaderInputProviderUsage,
        shipUint16 rootIndexToBindTo,
        shipUint16 descriptorRangeIndexToBind,
        shipUint16 descriptorRangeEntryIndexToBind,
        ShaderVisibility shaderStageToBindTo)
{
    ShaderResourceBinderEntry& shaderResourceBinderEntry = m_ShaderResourceBinderEntries.Grow();
    shaderResourceBinderEntry.GlobalBufferUsage = shaderInputProviderUsage;
    shaderResourceBinderEntry.RootIndexToBindTo = rootIndexToBindTo;
    shaderResourceBinderEntry.DescriptorRangeIndexToBindTo = descriptorRangeIndexToBind;
    shaderResourceBinderEntry.DescriptorRangeEntryIndexToBind = descriptorRangeEntryIndexToBind;
    shaderResourceBinderEntry.BindDescriptorTable = true;

    shaderResourceBinderEntry.BindConstantBuffer = false;
    shaderResourceBinderEntry.BindDescriptor = false;
    shaderResourceBinderEntry.BindGlobalBuffer = true;

    shaderResourceBinderEntry.ShaderStageToBindTo = shaderStageToBindTo;
}

void ShaderResourceBinder::BindShaderInputProvders(
        GFXRenderDevice& gfxRenderDevice,
        GFXDirectRenderCommandList& gfxDirectRenderCommandList,
        const Array<const ShaderInputProvider*>& shaderInputProviders,
        GFXDescriptorSetHandle gfxDescriptorSetHandle) const
{
    ShaderInputProviderManager& shaderInputProviderManager = GetShaderInputProviderManager();

    GFXDescriptorSet& gfxDescriptorSet = gfxRenderDevice.GetDescriptorSet(gfxDescriptorSetHandle);

    for (const ShaderResourceBinderEntry& shaderResourceBinderEntry : m_ShaderResourceBinderEntries)
    {
        if (shaderResourceBinderEntry.BindSamplerState)
        {
            continue;
        }

        if (shaderResourceBinderEntry.BindGlobalBuffer)
        {
            BindShaderGlobalBuffer(shaderResourceBinderEntry, gfxRenderDevice, gfxDirectRenderCommandList, gfxDescriptorSet);
        }
        else
        {
            const ShaderInputProvider* shaderInputProvider = shaderInputProviderManager.GetShaderInputProviderForDeclaration(shaderInputProviders, shaderResourceBinderEntry.Declaration);
            SHIP_ASSERT_MSG(
                    shaderInputProvider != nullptr,
                    "ShaderInputProvider %s is not present, things will break, most likely result in a GPU hang!",
                    shaderResourceBinderEntry.Declaration->GetShaderInputProviderName());

            if (shaderResourceBinderEntry.BindDescriptor)
            {
                BindShaderInputProviderDescriptor(shaderResourceBinderEntry, shaderInputProvider, gfxRenderDevice, gfxDescriptorSet);
            }
            else if (shaderResourceBinderEntry.BindConstantBuffer)
            {
                BindShaderInputProviderConstantBuffer(shaderResourceBinderEntry, shaderInputProvider, gfxRenderDevice, gfxDescriptorSet);
            }
            else
            {
                SHIP_ASSERT(!"Should not happen.");
            }
        }
    }
}

void ShaderResourceBinder::BindSamplerStates(
        GFXRenderDevice& gfxRenderDevice,
        const Array<GFXSamplerHandle>& gfxSamplerHandles,
        GFXDescriptorSetHandle gfxDescriptorSetHandle) const
{
    GFXDescriptorSet& gfxDescriptorSet = gfxRenderDevice.GetDescriptorSet(gfxDescriptorSetHandle);

    for (const ShaderResourceBinderEntry& shaderResourceBinderEntry : m_ShaderResourceBinderEntries)
    {
        if (!shaderResourceBinderEntry.BindSamplerState)
        {
            continue;
        }

        for (const GFXSamplerHandle& gfxSamplerHandle : gfxSamplerHandles)
        {
            GFXSampler* gfxSamplerToBind = gfxRenderDevice.GetSamplerPtr(gfxSamplerHandle);
            if (gfxSamplerToBind->GetSamplerState() == shaderResourceBinderEntry.Sampler)
            {
                gfxDescriptorSet.SetDescriptorTableEntryForRootIndex(
                        shaderResourceBinderEntry.RootIndexToBindTo,
                        shaderResourceBinderEntry.DescriptorRangeIndexToBindTo,
                        shaderResourceBinderEntry.DescriptorRangeEntryIndexToBind,
                        *gfxSamplerToBind);
            }
        }
    }
}

void ShaderResourceBinder::BindShaderInputProviderDescriptor(
        const ShaderResourceBinderEntry& shaderResourceBinderEntry,
        const ShaderInputProvider* shaderInputProvider,
        GFXRenderDevice& gfxRenderDevice,
        GFXDescriptorSet& gfxDescriptorSet) const
{
    GfxResource* gfxResourceToBind = nullptr;

    switch (shaderResourceBinderEntry.DescriptorType)
    {
    case ShaderInputType::Texture2D:
        {
            size_t shaderInputProviderAddress = reinterpret_cast<size_t>(shaderInputProvider);
            size_t textureHandleAddress = shaderInputProviderAddress + shaderResourceBinderEntry.DataOffsetInProvider;
            GFXTexture2DHandle gfxTexture2DHandle = *reinterpret_cast<GFXTexture2DHandle*>(textureHandleAddress);

#ifdef VALIDATE_SHADER_INPUT_PROVIDER_BINDING
            ShaderInputProviderDeclaration* shaderInputProviderDeclaration = shaderInputProvider->GetShaderInputProviderDeclaration();
            SHIP_ASSERT_MSG(
                    gfxTexture2DHandle.IsValid(),
                    "Texture2D %s in ShaderInputProvider %s is not valid!",
                    GetShaderInputProviderManager().GetShaderInputNameFromProvider(shaderInputProviderDeclaration, shaderResourceBinderEntry.DataOffsetInProvider),
                    shaderInputProviderDeclaration->GetShaderInputProviderName());
#endif // #ifdef VALIDATE_SHADER_INPUT_PROVIDER_BINDING

            gfxResourceToBind = gfxRenderDevice.GetTexture2DPtr(gfxTexture2DHandle);
        }
        break;

    case ShaderInputType::Texture2DArray:
        {
            size_t shaderInputProviderAddress = reinterpret_cast<size_t>(shaderInputProvider);
            size_t textureHandleAddress = shaderInputProviderAddress + shaderResourceBinderEntry.DataOffsetInProvider;
            GFXTexture2DArrayHandle gfxTexture2DArrayHandle = *reinterpret_cast<GFXTexture2DArrayHandle*>(textureHandleAddress);

#ifdef VALIDATE_SHADER_INPUT_PROVIDER_BINDING
            ShaderInputProviderDeclaration* shaderInputProviderDeclaration = shaderInputProvider->GetShaderInputProviderDeclaration();
            SHIP_ASSERT_MSG(
                    gfxTexture2DArrayHandle.IsValid(),
                    "Texture2DArray %s in ShaderInputProvider %s is not valid!",
                    GetShaderInputProviderManager().GetShaderInputNameFromProvider(shaderInputProviderDeclaration, shaderResourceBinderEntry.DataOffsetInProvider),
                    shaderInputProviderDeclaration->GetShaderInputProviderName());
#endif // #ifdef VALIDATE_SHADER_INPUT_PROVIDER_BINDING

            gfxResourceToBind = gfxRenderDevice.GetTexture2DArrayPtr(gfxTexture2DArrayHandle);
        }
        break;

    case ShaderInputType::Texture3D:
        {
            size_t shaderInputProviderAddress = reinterpret_cast<size_t>(shaderInputProvider);
            size_t textureHandleAddress = shaderInputProviderAddress + shaderResourceBinderEntry.DataOffsetInProvider;
            GFXTexture3DHandle gfxTexture3DHandle = *reinterpret_cast<GFXTexture3DHandle*>(textureHandleAddress);

#ifdef VALIDATE_SHADER_INPUT_PROVIDER_BINDING
            ShaderInputProviderDeclaration* shaderInputProviderDeclaration = shaderInputProvider->GetShaderInputProviderDeclaration();
            SHIP_ASSERT_MSG(
                    gfxTexture3DHandle.IsValid(),
                    "Texture3D %s in ShaderInputProvider %s is not valid!",
                    GetShaderInputProviderManager().GetShaderInputNameFromProvider(shaderInputProviderDeclaration, shaderResourceBinderEntry.DataOffsetInProvider),
                    shaderInputProviderDeclaration->GetShaderInputProviderName());
#endif // #ifdef VALIDATE_SHADER_INPUT_PROVIDER_BINDING

            gfxResourceToBind = gfxRenderDevice.GetTexture3DPtr(gfxTexture3DHandle);
        }
        break;

    case ShaderInputType::TextureCube:
        {
            size_t shaderInputProviderAddress = reinterpret_cast<size_t>(shaderInputProvider);
            size_t textureHandleAddress = shaderInputProviderAddress + shaderResourceBinderEntry.DataOffsetInProvider;
            GFXTextureCubeHandle gfxTextureCubeHandle = *reinterpret_cast<GFXTextureCubeHandle*>(textureHandleAddress);

#ifdef VALIDATE_SHADER_INPUT_PROVIDER_BINDING
            ShaderInputProviderDeclaration* shaderInputProviderDeclaration = shaderInputProvider->GetShaderInputProviderDeclaration();
            SHIP_ASSERT_MSG(
                    gfxTextureCubeHandle.IsValid(),
                    "TextureCube %s in ShaderInputProvider %s is not valid!",
                    GetShaderInputProviderManager().GetShaderInputNameFromProvider(shaderInputProviderDeclaration, shaderResourceBinderEntry.DataOffsetInProvider),
                    shaderInputProviderDeclaration->GetShaderInputProviderName());
#endif // #ifdef VALIDATE_SHADER_INPUT_PROVIDER_BINDING

            gfxResourceToBind = gfxRenderDevice.GetTextureCubePtr(gfxTextureCubeHandle);
        }
        break;

    case ShaderInputType::ByteBuffer:
        {
            size_t shaderInputProviderAddress = reinterpret_cast<size_t>(shaderInputProvider);
            size_t byteBufferHandleAddress = shaderInputProviderAddress + shaderResourceBinderEntry.DataOffsetInProvider;
            GFXByteBufferHandle gfxByteBufferHandle = *reinterpret_cast<GFXByteBufferHandle*>(byteBufferHandleAddress);

#ifdef VALIDATE_SHADER_INPUT_PROVIDER_BINDING
            ShaderInputProviderDeclaration* shaderInputProviderDeclaration = shaderInputProvider->GetShaderInputProviderDeclaration();
            SHIP_ASSERT_MSG(
                    gfxByteBufferHandle.IsValid(),
                    "ByteBuffer %s in ShaderInputProvider %s is not valid!",
                    GetShaderInputProviderManager().GetShaderInputNameFromProvider(shaderInputProviderDeclaration, shaderResourceBinderEntry.DataOffsetInProvider),
                    shaderInputProviderDeclaration->GetShaderInputProviderName());
#endif // #ifdef VALIDATE_SHADER_INPUT_PROVIDER_BINDING

            gfxResourceToBind = gfxRenderDevice.GetByteBufferPtr(gfxByteBufferHandle);
        }
        break;

    default:
        SHIP_ASSERT(!"Unsupported shader input type.");
        break;
    }

    SHIP_ASSERT(gfxResourceToBind != nullptr);

    if (shaderResourceBinderEntry.BindDescriptorTable)
    {
        gfxDescriptorSet.SetDescriptorTableEntryForRootIndex(
                shaderResourceBinderEntry.RootIndexToBindTo,
                shaderResourceBinderEntry.DescriptorRangeIndexToBindTo,
                shaderResourceBinderEntry.DescriptorRangeEntryIndexToBind,
                *gfxResourceToBind);
    }
    else
    {
        gfxDescriptorSet.SetDescriptorForRootIndex(shaderResourceBinderEntry.RootIndexToBindTo, *gfxResourceToBind);
    }
}

void ShaderResourceBinder::BindShaderInputProviderConstantBuffer(
        const ShaderResourceBinderEntry& shaderResourceBinderEntry,
        const ShaderInputProvider* shaderInputProvider,
        GFXRenderDevice& gfxRenderDevice,
        GFXDescriptorSet& gfxDescriptorSet) const
{
    SHIP_ASSERT_MSG(
            shaderInputProvider->m_GfxConstantBufferHandle.IsValid(),
            "ShaderInputProvider %s ConstantBuffer is invalid, this shouldn't happen.",
            shaderInputProvider->GetShaderInputProviderDeclaration()->GetShaderInputProviderName());

    GFXConstantBuffer& gfxConstantBuffer = gfxRenderDevice.GetConstantBuffer(shaderInputProvider->m_GfxConstantBufferHandle);
    
    if (shaderResourceBinderEntry.BindDescriptorTable)
    {
        gfxDescriptorSet.SetDescriptorTableEntryForRootIndex(
                shaderResourceBinderEntry.RootIndexToBindTo,
                shaderResourceBinderEntry.DescriptorRangeIndexToBindTo,
                shaderResourceBinderEntry.DescriptorRangeEntryIndexToBind,
                gfxConstantBuffer);
    }
    else
    {
        gfxDescriptorSet.SetDescriptorForRootIndex(shaderResourceBinderEntry.RootIndexToBindTo, gfxConstantBuffer);
    }
}

void ShaderResourceBinder::BindShaderGlobalBuffer(
        const ShaderResourceBinderEntry& shaderResourceBinderEntry,
        GFXRenderDevice& gfxRenderDevice,
        GFXDirectRenderCommandList& gfxDirectRenderCommandList,
        GFXDescriptorSet& gfxDescriptorSet) const
{
    SHIP_STATIC_ASSERT_MSG(shipUint32(ShaderInputProviderUsage::Count) == 2, "Need to update code below");

    switch (shaderResourceBinderEntry.GlobalBufferUsage)
    {
    case ShaderInputProviderUsage::PerInstance:
        {
            GFXMaterialUnifiedConstantBuffer& gfxMaterialUnifiedConstantBuffer = GetGFXMaterialUnifiedConstantBuffer();
            GFXByteBufferHandle gfxByteBufferHandle = gfxMaterialUnifiedConstantBuffer.BindMaterialUnfiedConstantBuffer(gfxDirectRenderCommandList);

            SHIP_ASSERT(gfxByteBufferHandle.IsValid());

            GFXByteBuffer& gfxByteBuffer = gfxRenderDevice.GetByteBuffer(gfxByteBufferHandle);

            if (shaderResourceBinderEntry.BindDescriptorTable)
            {
                gfxDescriptorSet.SetDescriptorTableEntryForRootIndex(
                        shaderResourceBinderEntry.RootIndexToBindTo,
                        shaderResourceBinderEntry.DescriptorRangeIndexToBindTo,
                        shaderResourceBinderEntry.DescriptorRangeEntryIndexToBind,
                        gfxByteBuffer);
            }
            else
            {
                gfxDescriptorSet.SetDescriptorForRootIndex(
                        shaderResourceBinderEntry.RootIndexToBindTo,
                        gfxByteBuffer);
            }
        }
        break;

    default:
        SHIP_ASSERT(!"Unimplemented shader input provider usage");
        break;
    }
}

}