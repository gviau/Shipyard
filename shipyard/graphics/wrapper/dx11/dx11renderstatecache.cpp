#include <graphics/wrapper/dx11/dx11renderstatecache.h>

#include <graphics/vertexformat.h>

#include <graphics/wrapper/dx11/dx11_common.h>
#include <graphics/wrapper/dx11/dx11buffer.h>
#include <graphics/wrapper/dx11/dx11descriptorset.h>
#include <graphics/wrapper/dx11/dx11pipelinestateobject.h>
#include <graphics/wrapper/dx11/dx11renderdevice.h>
#include <graphics/wrapper/dx11/dx11rendertarget.h>
#include <graphics/wrapper/dx11/dx11rootsignature.h>
#include <graphics/wrapper/dx11/dx11shader.h>
#include <graphics/wrapper/dx11/dx11texture.h>

#include <system/logger.h>

#pragma warning( disable : 4005 )

#include <d3d11.h>

#pragma warning( default : 4005 )

namespace Shipyard
{;

extern ID3D11InputLayout* g_RegisteredInputLayouts[uint32_t(VertexFormatType::VertexFormatType_Count)];

DX11RenderStateCache::DX11RenderStateCache(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
    : m_Device(device)
    , m_DeviceContext(deviceContext)
    , m_NativeRasterizerState(nullptr)
    , m_NativeDepthStencilState(nullptr)
    , m_NativeBlendState(nullptr)
    , m_NativeDepthStencilView(nullptr)
{
    Reset();

    // Make sure all states are set the first time: otherwise, state with the same value will not bet set the
    // first time.
    m_RenderStateCacheDirtyFlags.SetAllBits();
}

DX11RenderStateCache::~DX11RenderStateCache()
{
    Reset();
}

void DX11RenderStateCache::Reset()
{
    m_RasterizerState = RasterizerState();
    m_DepthStencilState = DepthStencilState();

    m_VertexFormatType = VertexFormatType::VertexFormatType_Count;
    m_PrimitiveTopology = PrimitiveTopology::TriangleList;

    m_NumRenderTargets = 0;
    for (uint32_t i = 0; i < GfxConstants::GfxConstants_MaxRenderTargetsBound; i++)
    {
        m_RenderTargetsFormat[i] = GfxFormat::Unknown;
    }

    m_DepthStencilFormat = GfxFormat::R32_UINT;

    m_Viewport = GfxViewport();

    memset(m_NativeVertexBuffers, 0, sizeof(m_NativeVertexBuffers));
    m_VertexBufferStartSlot = 0;
    m_NumVertexBuffers = 0;
    memset(m_VertexBufferOffsets, 0, sizeof(m_VertexBufferOffsets));

    m_NativeIndexBuffer = nullptr;
    m_IndexBufferFormat = DXGI_FORMAT_UNKNOWN;
    m_IndexBufferOffset = 0;

    if (m_NativeRasterizerState != nullptr)
    {
        m_NativeRasterizerState->Release();
        m_NativeRasterizerState = nullptr;
    }

    if (m_NativeDepthStencilState != nullptr)
    {
        m_NativeDepthStencilState->Release();
        m_NativeDepthStencilState = nullptr;
    }

    if (m_NativeBlendState != nullptr)
    {
        m_NativeBlendState->Release();
        m_NativeBlendState = nullptr;
    }

    m_VertexShaderHandle.handle = InvalidGfxHandle;
    m_PixelShaderHandle.handle = InvalidGfxHandle;

    memset(m_NativeRenderTargets, 0, sizeof(m_NativeRenderTargets));
    m_NativeDepthStencilView = nullptr;

    for (uint32_t i = 0; i < GfxConstants::GfxConstants_MaxConstantBufferViewsBoundPerShaderStage; i++)
    {
        m_ConstantBufferViewsShaderVisibility[i] = ShaderVisibility::ShaderVisibility_None;
    }

    for (uint32_t i = 0; i < GfxConstants::GfxConstants_MaxShaderResourceViewsBoundPerShaderStage; i++)
    {
        m_ShaderResourceViewsShaderVisibility[i] = ShaderVisibility::ShaderVisibility_None;
    }

    for (uint32_t i = 0; i < GfxConstants::GfxConstants_MaxUnorderedAccessViewsBoundPerShaderStage; i++)
    {
        m_UnorderedAccessViewsShaderVisibility[i] = ShaderVisibility::ShaderVisibility_None;
    }

    for (uint32_t i = 0; i < GfxConstants::GfxConstants_MaxSamplersBoundPerShaderStage; i++)
    {
        m_SamplersShaderVisibility[i] = ShaderVisibility::ShaderVisibility_None;
    }

    for (uint32_t shaderStage = 0; shaderStage < ShaderStage::ShaderStage_Count; shaderStage++)
    {
        m_DirtySlotShaderResourceViewsPerShaderStage[shaderStage].Clear();
        m_DirtySlotConstantBufferViewsPerShaderStage[shaderStage].Clear();
        m_DirtySlotUnorderedAccessViewsPerShaderStage[shaderStage].Clear();
        m_DirtySlotSamplersPerShaderStage[shaderStage].Clear();
    }

    memset(m_NativeShaderResourceViews, 0, sizeof(m_NativeShaderResourceViews));
    memset(m_NativeConstantBufferViews, 0, sizeof(m_NativeConstantBufferViews));
    memset(m_NativeUnorderedAccessViews, 0, sizeof(m_NativeUnorderedAccessViews));
    memset(m_NativeSamplers, 0, sizeof(m_NativeSamplers));

    m_RenderStateCacheDirtyFlags.Clear();
}

void DX11RenderStateCache::BindRootSignature(const GFXRootSignature& rootSignature)
{
    const Array<RootSignatureParameterEntry>& rootSignatureParameters = rootSignature.GetRootSignatureParameters();

    for (const RootSignatureParameterEntry& rootSignatureParameter : rootSignatureParameters)
    {
        ShaderVisibility shaderVisibilityForParameter = rootSignatureParameter.shaderVisibility;

        switch (rootSignatureParameter.parameterType)
        {
        case RootSignatureParameterType::ConstantBufferView:
            {
                uint32_t startingBindingSlot = rootSignatureParameter.descriptor.shaderBindingSlot;
                uint32_t endingBindingSlot = startingBindingSlot + MAX(rootSignatureParameter.descriptor.registerSpace, 1);

                for (uint32_t bindingSlot = startingBindingSlot; bindingSlot < endingBindingSlot; bindingSlot++)
                {
                    m_ConstantBufferViewsShaderVisibility[bindingSlot] = shaderVisibilityForParameter;
                }

                break;
            }

        case RootSignatureParameterType::ShaderResourceView:
            {
                uint32_t startingBindingSlot = rootSignatureParameter.descriptor.shaderBindingSlot;
                uint32_t endingBindingSlot = startingBindingSlot + MAX(rootSignatureParameter.descriptor.registerSpace, 1);

                for (uint32_t bindingSlot = startingBindingSlot; bindingSlot < endingBindingSlot; bindingSlot++)
                {
                    m_ShaderResourceViewsShaderVisibility[bindingSlot] = shaderVisibilityForParameter;
                }

                break;
            }

        case RootSignatureParameterType::UnorderedAccessView:
            {
                uint32_t startingBindingSlot = rootSignatureParameter.descriptor.shaderBindingSlot;
                uint32_t endingBindingSlot = startingBindingSlot + MAX(rootSignatureParameter.descriptor.registerSpace, 1);

                for (uint32_t bindingSlot = startingBindingSlot; bindingSlot < endingBindingSlot; bindingSlot++)
                {
                    m_UnorderedAccessViewsShaderVisibility[bindingSlot] = shaderVisibilityForParameter;
                }

                break;
            }

        default:
            // Not yet implemented
            SHIP_ASSERT(false);
        }
    }
}

void DX11RenderStateCache::BindPipelineStateObject(const GFXPipelineStateObject& pipelineStateObject)
{
    const PipelineStateObjectCreationParameters& pipelineStateObjectParameters = pipelineStateObject.GetCreationParameters();

    if (pipelineStateObjectParameters.renderStateBlock.rasterizerState != m_RasterizerState)
    {
        m_RasterizerState = pipelineStateObjectParameters.renderStateBlock.rasterizerState;
        m_RenderStateCacheDirtyFlags.SetBit(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_RasterizerState);
    }

    if (pipelineStateObjectParameters.renderStateBlock.depthStencilState != m_DepthStencilState)
    {
        m_DepthStencilState = pipelineStateObjectParameters.renderStateBlock.depthStencilState;
        m_RenderStateCacheDirtyFlags.SetBit(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_DepthStencilState);
    }

    if (pipelineStateObjectParameters.renderStateBlock.blendState != m_BlendState)
    {
        m_BlendState = pipelineStateObjectParameters.renderStateBlock.blendState;
        m_RenderStateCacheDirtyFlags.SetBit(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_BlendState);
    }

    if (pipelineStateObjectParameters.vertexShaderHandle.handle != m_VertexShaderHandle.handle)
    {
        SHIP_ASSERT(pipelineStateObjectParameters.vertexShaderHandle.handle != InvalidGfxHandle);

        m_VertexShaderHandle = pipelineStateObjectParameters.vertexShaderHandle;
        m_RenderStateCacheDirtyFlags.SetBit(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_VertexShader);
    }

    if (pipelineStateObjectParameters.pixelShaderHandle.handle != m_PixelShaderHandle.handle)
    {
        // Null pixel shader is actually okay (for example, depth rendering only).
        if (pipelineStateObjectParameters.pixelShaderHandle.handle == InvalidGfxHandle)
        {
            m_PixelShaderHandle.handle = InvalidGfxHandle;
        }
        else
        {
            m_PixelShaderHandle = pipelineStateObjectParameters.pixelShaderHandle;
        }

        m_RenderStateCacheDirtyFlags.SetBit(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_PixelShader);
    }

    if (pipelineStateObjectParameters.primitiveTopology != m_PrimitiveTopology)
    {
        m_PrimitiveTopology = pipelineStateObjectParameters.primitiveTopology;
        m_RenderStateCacheDirtyFlags.SetBit(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_PrimitiveTopology);
    }

    if (pipelineStateObjectParameters.vertexFormatType != m_VertexFormatType)
    {
        m_VertexFormatType = pipelineStateObjectParameters.vertexFormatType;
        m_RenderStateCacheDirtyFlags.SetBit(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_VertexFormatType);
    }
}

void DX11RenderStateCache::BindDescriptorSet(const GFXDescriptorSet& descriptorSet, const GFXRootSignature& rootSignature)
{
    const Array<GFXDescriptorSet::DescriptorSetEntry>& resourcesToBind = descriptorSet.GetDescriptorSetEntries();
    const Array<RootSignatureParameterEntry>& rootSignatureParameters = rootSignature.GetRootSignatureParameters();

    for (const GFXDescriptorSet::DescriptorSetEntry& descriptorSetEntry : resourcesToBind)
    {
        const RootSignatureParameterEntry& rootSignatureParameter = rootSignatureParameters[descriptorSetEntry.rootIndex];

        if (descriptorSetEntry.isDescriptorTable)
        {
            BindDescriptorTableFromDescriptorSet(descriptorSetEntry.descriptorResources, rootSignatureParameter);
        }
        else
        {
            BindDescriptorFromDescriptorSet(descriptorSetEntry.descriptorResources[0], rootSignatureParameter);
        }
    }
}

void DX11RenderStateCache::BindRenderTarget(const GFXRenderTarget& renderTarget)
{
    ID3D11RenderTargetView* const * renderTargetViews = renderTarget.GetRenderTargetViews();

    bool changeRenderTargets = false;
    for (uint32_t i = 0; i < GfxConstants::GfxConstants_MaxRenderTargetsBound; i++)
    {
        if (m_NativeRenderTargets[i] != renderTargetViews[i])
        {
            changeRenderTargets = true;
        }

        m_NativeRenderTargets[i] = renderTargetViews[i];
    }
    
    if (changeRenderTargets)
    {
        m_RenderStateCacheDirtyFlags.SetBit(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_RenderTargets);
    }
}

void DX11RenderStateCache::BindDepthStencilRenderTarget(const GFXDepthStencilRenderTarget& depthStencilRenderTarget)
{
    ID3D11DepthStencilView* const depthStencilView = depthStencilRenderTarget.GetDepthStencilView();

    if (m_NativeDepthStencilView != depthStencilView)
    {
        m_NativeDepthStencilView = depthStencilView;

        m_RenderStateCacheDirtyFlags.SetBit(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_DepthStencilRenderTarget);
    }
}

void DX11RenderStateCache::SetViewport(const GfxViewport& gfxViewport)
{
    if (m_Viewport != gfxViewport)
    {
        m_Viewport = gfxViewport;

        m_RenderStateCacheDirtyFlags.SetBit(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_Viewport);
    }
}

void DX11RenderStateCache::SetVertexBuffers(GFXVertexBuffer* const * vertexBuffers, uint32_t startSlot, uint32_t numVertexBuffers, uint32_t* vertexBufferOffsets)
{
    if (m_VertexBufferStartSlot != startSlot ||
        m_NumVertexBuffers != numVertexBuffers)
    {
        m_VertexBufferStartSlot = startSlot;
        m_NumVertexBuffers = numVertexBuffers;

        memset(m_NativeVertexBuffers, 0, sizeof(m_NativeVertexBuffers));

        for (uint32_t i = 0; i < numVertexBuffers; i++)
        {
            if (vertexBuffers[i] != nullptr)
            {
                m_NativeVertexBuffers[i + startSlot] = vertexBuffers[i]->GetBuffer();
            }

            m_VertexBufferOffsets[i + startSlot] = vertexBufferOffsets[i];
        }

        m_RenderStateCacheDirtyFlags.SetBit(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_VertexBuffers);
    }
    else
    {
        bool changeVertexBuffers = false;

        for (uint32_t i = 0; i < numVertexBuffers; i++)
        {
            if (vertexBufferOffsets[i] != m_VertexBufferOffsets[i + startSlot])
            {
                changeVertexBuffers = true;
            }

            m_VertexBufferOffsets[i + startSlot] = vertexBufferOffsets[i];
        }

        for (uint32_t i = 0; i < numVertexBuffers; i++)
        {
            if (vertexBuffers[i] == nullptr && m_NativeVertexBuffers[i + startSlot] != nullptr ||
                vertexBuffers[i]->GetBuffer() != m_NativeVertexBuffers[i + startSlot])
            {
                changeVertexBuffers = true;
            }

            m_NativeVertexBuffers[i + startSlot] = vertexBuffers[i]->GetBuffer();
        }

        if (changeVertexBuffers)
        {
            m_RenderStateCacheDirtyFlags.SetBit(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_VertexBuffers);
        }
    }
}

void DX11RenderStateCache::SetIndexBuffer(const GFXIndexBuffer& indexBuffer, uint32_t indexBufferOffset)
{
    DXGI_FORMAT indexFormat = (indexBuffer.Uses2BytesPerIndex() ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT);
    if (DXGI_FORMAT(m_IndexBufferFormat) != indexFormat ||
        m_IndexBufferOffset != indexBufferOffset ||
        m_NativeIndexBuffer != indexBuffer.GetBuffer())
    {
        m_RenderStateCacheDirtyFlags.SetBit(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_IndexBuffer);
    }

    m_IndexBufferFormat = indexFormat;
    m_IndexBufferOffset = indexBufferOffset;
    m_NativeIndexBuffer = indexBuffer.GetBuffer();
}

void DX11RenderStateCache::BindDescriptorTableFromDescriptorSet(
        const Array<GfxResource*>& descriptorTableResources,
        const RootSignatureParameterEntry& rootSignatureParameter)
{
    // Not yet implemented
    SHIP_ASSERT(false);
}

void DX11RenderStateCache::BindDescriptorFromDescriptorSet(GfxResource* descriptorResource, const RootSignatureParameterEntry& rootSignatureParameter)
{
    uint32_t startingBindingSlot = rootSignatureParameter.descriptor.shaderBindingSlot;
    uint32_t endingBindingSlot = startingBindingSlot + MAX(rootSignatureParameter.descriptor.registerSpace, 1);
    
    for (uint32_t bindingSlot = startingBindingSlot; bindingSlot < endingBindingSlot; bindingSlot++)
    {
        switch (rootSignatureParameter.parameterType)
        {
        case RootSignatureParameterType::ConstantBufferView:
            {
                GFXConstantBuffer* constantBuffer = static_cast<GFXConstantBuffer*>(descriptorResource);
                ID3D11Buffer* nativeBuffer = constantBuffer->GetBuffer();

                if (m_NativeConstantBufferViews[bindingSlot] != nativeBuffer)
                {
                    m_NativeConstantBufferViews[bindingSlot] = nativeBuffer;

                    MarkBindingSlotAsDirty(m_DirtySlotConstantBufferViewsPerShaderStage, rootSignatureParameter.shaderVisibility, bindingSlot);

                    m_RenderStateCacheDirtyFlags.SetBit(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_ConstantBufferViews);
                }

                break;
            }

        case RootSignatureParameterType::ShaderResourceView:
            {
                GfxResourceType resourceType = descriptorResource->GetResourceType();
                
                ID3D11ShaderResourceView* nativeShaderResourceView = nullptr;

                if (resourceType == GfxResourceType::Texture)
                {
                    DX11BaseTexture* texture = static_cast<DX11BaseTexture*>(descriptorResource);
                    nativeShaderResourceView = texture->GetShaderResourceView();
                }
                else
                {
                    DX11BaseBuffer* buffer = static_cast<DX11BaseBuffer*>(descriptorResource);
                    nativeShaderResourceView = buffer->GetShaderResourceView();
                }

                if (m_NativeShaderResourceViews[bindingSlot] != nativeShaderResourceView)
                {
                    m_NativeShaderResourceViews[bindingSlot] = nativeShaderResourceView;

                    MarkBindingSlotAsDirty(m_DirtySlotShaderResourceViewsPerShaderStage, rootSignatureParameter.shaderVisibility, bindingSlot);

                    m_RenderStateCacheDirtyFlags.SetBit(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_ShaderResourceViews);
                }

                break;
            }

        default:
            // Not yet implemented
            SHIP_ASSERT(false);
            break;
        }
    }
}

namespace
{
    // Create lookup tables for shader resource setting method, to avoid super larger if blocks
    typedef void (STDMETHODCALLTYPE ID3D11DeviceContext::*ConstantBufferViewsSetterFunctionPtr)
            (UINT firstBindingSlots, UINT numBindingSlotsToSet, ID3D11Buffer* const* constantBufferViewsToBind);
    typedef void (STDMETHODCALLTYPE ID3D11DeviceContext::*ShaderResourceViewsSetterFunctionPtr)
            (UINT firstBindingSlots, UINT numBindingSlotsToSet, ID3D11ShaderResourceView* const* shaderResourceViewsToBind);

    ConstantBufferViewsSetterFunctionPtr g_ConstantBufferViewsSetterTable[] =
    {
        &ID3D11DeviceContext::VSSetConstantBuffers,
        &ID3D11DeviceContext::PSSetConstantBuffers,
        &ID3D11DeviceContext::HSSetConstantBuffers,
        &ID3D11DeviceContext::DSSetConstantBuffers,
        &ID3D11DeviceContext::GSSetConstantBuffers,
        &ID3D11DeviceContext::CSSetConstantBuffers
    };

    ShaderResourceViewsSetterFunctionPtr g_ShaderResourceViewsSetterTable[] =
    {
        &ID3D11DeviceContext::VSSetShaderResources,
        &ID3D11DeviceContext::PSSetShaderResources,
        &ID3D11DeviceContext::HSSetShaderResources,
        &ID3D11DeviceContext::DSSetShaderResources,
        &ID3D11DeviceContext::GSSetShaderResources,
        &ID3D11DeviceContext::CSSetShaderResources
    };
}

void DX11RenderStateCache::CommitStateChangesForGraphics(GFXRenderDevice& gfxRenderDevice)
{
    if (m_RenderStateCacheDirtyFlags.IsClear())
    {
        return;
    }

    // Apply cached changes
    if (m_RenderStateCacheDirtyFlags.IsBitSet(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_RasterizerState))
    {
        if (m_NativeRasterizerState != nullptr)
        {
            m_NativeRasterizerState->Release();
        }

        m_NativeRasterizerState = CreateRasterizerState(m_RasterizerState);

        m_DeviceContext->RSSetState(m_NativeRasterizerState);

        m_RenderStateCacheDirtyFlags.UnsetBit(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_RasterizerState);
    }

    if (m_RenderStateCacheDirtyFlags.IsBitSet(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_DepthStencilState))
    {
        if (m_NativeDepthStencilState != nullptr)
        {
            m_NativeDepthStencilState->Release();
        }

        m_NativeDepthStencilState = CreateDepthStencilState(m_DepthStencilState);

        m_DeviceContext->OMSetDepthStencilState(m_NativeDepthStencilState, 0);

        m_RenderStateCacheDirtyFlags.UnsetBit(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_DepthStencilState);
    }

    if (m_RenderStateCacheDirtyFlags.IsBitSet(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_BlendState))
    {
        if (m_NativeBlendState != nullptr)
        {
            m_NativeBlendState->Release();
        }

        m_NativeBlendState = CreateBlendState(m_BlendState);

        m_DeviceContext->OMSetBlendState(m_NativeBlendState, &m_BlendState.m_RedBlendUserFactor, 0xffffffff);

        m_RenderStateCacheDirtyFlags.UnsetBit(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_BlendState);
    }

    if (m_RenderStateCacheDirtyFlags.IsBitSet(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_Viewport))
    {
        D3D11_VIEWPORT nativeViewport;
        nativeViewport.TopLeftX = m_Viewport.topLeftX;
        nativeViewport.TopLeftY = m_Viewport.topLeftY;
        nativeViewport.Width = m_Viewport.width;
        nativeViewport.Height = m_Viewport.height;
        nativeViewport.MinDepth = m_Viewport.minDepth;
        nativeViewport.MaxDepth = m_Viewport.maxDepth;

        m_DeviceContext->RSSetViewports(1, &nativeViewport);

        m_RenderStateCacheDirtyFlags.UnsetBit(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_Viewport);
    }

    if (m_RenderStateCacheDirtyFlags.IsBitSet(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_RenderTargets) ||
        m_RenderStateCacheDirtyFlags.IsBitSet(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_DepthStencilRenderTarget))
    {
        m_DeviceContext->OMSetRenderTargets(GfxConstants::GfxConstants_MaxRenderTargetsBound, m_NativeRenderTargets, m_NativeDepthStencilView);

        m_RenderStateCacheDirtyFlags.UnsetBit(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_RenderTargets);
        m_RenderStateCacheDirtyFlags.UnsetBit(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_DepthStencilRenderTarget);
    }

    // In Direct3D 11, no need to rebind resources when the shader changes.
    if (m_RenderStateCacheDirtyFlags.IsBitSet(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_VertexShader))
    {
        GFXVertexShader& gfxVertexShader = gfxRenderDevice.GetVertexShader(m_VertexShaderHandle);
        m_DeviceContext->VSSetShader(gfxVertexShader.GetShader(), nullptr, 0);

        m_RenderStateCacheDirtyFlags.UnsetBit(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_VertexShader);
    }

    if (m_RenderStateCacheDirtyFlags.IsBitSet(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_PixelShader))
    {
        if (m_PixelShaderHandle.handle == InvalidGfxHandle)
        {
            m_DeviceContext->PSSetShader(nullptr, nullptr, 0);
        }
        else
        {
            GFXPixelShader& gfxPixelShader = gfxRenderDevice.GetPixelShader(m_PixelShaderHandle);
            m_DeviceContext->PSSetShader(gfxPixelShader.GetShader(), nullptr, 0);
        }

        m_RenderStateCacheDirtyFlags.UnsetBit(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_PixelShader);
    }

    if (m_RenderStateCacheDirtyFlags.IsBitSet(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_Hull))
    {
        m_RenderStateCacheDirtyFlags.UnsetBit(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_Hull);
    }

    if (m_RenderStateCacheDirtyFlags.IsBitSet(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_Domain))
    {
        m_RenderStateCacheDirtyFlags.UnsetBit(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_Domain);
    }

    if (m_RenderStateCacheDirtyFlags.IsBitSet(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_Geometry))
    {
        m_RenderStateCacheDirtyFlags.UnsetBit(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_Geometry);
    }

    if (m_RenderStateCacheDirtyFlags.IsBitSet(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_Compute))
    {
        m_RenderStateCacheDirtyFlags.UnsetBit(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_Compute);
    }

    if (m_RenderStateCacheDirtyFlags.IsBitSet(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_ConstantBufferViews))
    {
        for (uint32_t shaderStage = 0; shaderStage < ShaderStage::ShaderStage_Count; shaderStage++)
        {
            if (m_DirtySlotConstantBufferViewsPerShaderStage[shaderStage].IsClear())
            {
                continue;
            }

            uint32_t startingBindingSlot = 0;
            uint32_t firstBindingSlot = 0;

            ConstantBufferViewsSetterFunctionPtr constantBufferViewsSetterForShaderStage = g_ConstantBufferViewsSetterTable[shaderStage];

            while (m_DirtySlotConstantBufferViewsPerShaderStage[shaderStage].GetFirstBitSet(startingBindingSlot, firstBindingSlot))
            {
                uint32_t numBindingSlotsToSet = m_DirtySlotConstantBufferViewsPerShaderStage[shaderStage].GetLongestRangeWithBitsSet(firstBindingSlot);

                (m_DeviceContext->*constantBufferViewsSetterForShaderStage)(firstBindingSlot, numBindingSlotsToSet, &m_NativeConstantBufferViews[firstBindingSlot]);

                startingBindingSlot = firstBindingSlot + numBindingSlotsToSet;
            }

            m_DirtySlotConstantBufferViewsPerShaderStage[shaderStage].Clear();
        }

        m_RenderStateCacheDirtyFlags.UnsetBit(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_ConstantBufferViews);
    }

    if (m_RenderStateCacheDirtyFlags.IsBitSet(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_ShaderResourceViews))
    {
        for (uint32_t shaderStage = 0; shaderStage < ShaderStage::ShaderStage_Count; shaderStage++)
        {
            if (m_DirtySlotShaderResourceViewsPerShaderStage[shaderStage].IsClear())
            {
                continue;
            }

            uint32_t startingBindingSlot = 0;
            uint32_t firstBindingSlot = 0;

            ShaderResourceViewsSetterFunctionPtr shaderResourceViewsSetterForShaderStage = g_ShaderResourceViewsSetterTable[shaderStage];

            while (m_DirtySlotShaderResourceViewsPerShaderStage[shaderStage].GetFirstBitSet(startingBindingSlot, firstBindingSlot))
            {
                uint32_t numBindingSlotsToSet = m_DirtySlotShaderResourceViewsPerShaderStage[shaderStage].GetLongestRangeWithBitsSet(firstBindingSlot);

                (m_DeviceContext->*shaderResourceViewsSetterForShaderStage)(firstBindingSlot, numBindingSlotsToSet, &m_NativeShaderResourceViews[firstBindingSlot]);

                startingBindingSlot = firstBindingSlot + numBindingSlotsToSet;
            }

            m_DirtySlotShaderResourceViewsPerShaderStage[shaderStage].Clear();
        }

        m_RenderStateCacheDirtyFlags.UnsetBit(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_ShaderResourceViews);
    }

    if (m_RenderStateCacheDirtyFlags.IsBitSet(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_UnorderedAccessViews))
    {
        m_RenderStateCacheDirtyFlags.UnsetBit(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_UnorderedAccessViews);
    }

    if (m_RenderStateCacheDirtyFlags.IsBitSet(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_Samplers))
    {
        m_RenderStateCacheDirtyFlags.UnsetBit(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_Samplers);
    }

    if (m_RenderStateCacheDirtyFlags.IsBitSet(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_PrimitiveTopology))
    {
        D3D11_PRIMITIVE_TOPOLOGY topology = ConvertShipyardPrimitiveTopologyToDX11(m_PrimitiveTopology);
        m_DeviceContext->IASetPrimitiveTopology(topology);

        m_RenderStateCacheDirtyFlags.UnsetBit(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_PrimitiveTopology);
    }

    if (m_RenderStateCacheDirtyFlags.IsBitSet(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_VertexFormatType))
    {
        VertexFormatType vertexFormatType = m_VertexFormatType;
        VertexFormat* vertexFormat = nullptr;

        GetVertexFormat(vertexFormatType, vertexFormat);

        m_DeviceContext->IASetInputLayout(g_RegisteredInputLayouts[uint32_t(vertexFormatType)]);

        m_RenderStateCacheDirtyFlags.UnsetBit(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_VertexFormatType);
    }

    if (m_RenderStateCacheDirtyFlags.IsBitSet(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_VertexBuffers))
    {
        uint32_t vertexBufferStrides[GfxConstants::GfxConstants_MaxVertexBuffers];

        if (m_NumVertexBuffers != 0)
        {
            VertexFormat* vertexFormat = nullptr;

            GetVertexFormat(m_VertexFormatType, vertexFormat);

            uint32_t stride = vertexFormat->GetSize();
            for (uint32_t i = 0; i < GfxConstants::GfxConstants_MaxVertexBuffers; i++)
            {
                vertexBufferStrides[i] = stride;
            }
        }

        m_DeviceContext->IASetVertexBuffers(m_VertexBufferStartSlot, m_NumVertexBuffers, m_NativeVertexBuffers, vertexBufferStrides, m_VertexBufferOffsets);

        m_RenderStateCacheDirtyFlags.UnsetBit(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_VertexBuffers);
    }

    if (m_RenderStateCacheDirtyFlags.IsBitSet(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_IndexBuffer))
    {
        m_DeviceContext->IASetIndexBuffer(m_NativeIndexBuffer, DXGI_FORMAT(m_IndexBufferFormat), m_IndexBufferOffset);

        m_RenderStateCacheDirtyFlags.UnsetBit(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_IndexBuffer);
    }

    // Everything should be accounted for.
    SHIP_ASSERT(m_RenderStateCacheDirtyFlags.IsClear());
}

ID3D11RasterizerState* DX11RenderStateCache::CreateRasterizerState(const RasterizerState& rasterizerState) const
{
    D3D11_RASTERIZER_DESC rasterizerDesc;
    rasterizerDesc.FillMode = ConvertShipyardFillModeToDX11(rasterizerState.m_FillMode);
    rasterizerDesc.CullMode = ConvertShipyardCullModeToDX11(rasterizerState.m_CullMode);
    rasterizerDesc.FrontCounterClockwise = rasterizerState.m_IsFrontCounterClockwise;
    rasterizerDesc.DepthBias = rasterizerState.m_DepthBias;
    rasterizerDesc.DepthBiasClamp = rasterizerState.m_DepthBiasClamp;
    rasterizerDesc.SlopeScaledDepthBias = rasterizerState.m_SlopeScaledDepthBias;
    rasterizerDesc.DepthClipEnable = rasterizerState.m_DepthClipEnable;
    rasterizerDesc.ScissorEnable = rasterizerState.m_ScissorEnable;
    rasterizerDesc.MultisampleEnable = rasterizerState.m_MultisampleEnable;
    rasterizerDesc.AntialiasedLineEnable = rasterizerState.m_AntialiasedLineEnable;

    ID3D11RasterizerState* nativeRasterizerState = nullptr;

    HRESULT hr = m_Device->CreateRasterizerState(&rasterizerDesc, &nativeRasterizerState);
    if (FAILED(hr))
    {
        SHIP_LOG_ERROR("DX11RenderStateCache::CreateRasterizerState() --> Couldn't create rasterizer state.");
        return nullptr;
    }

    return nativeRasterizerState;
}

ID3D11DepthStencilState* DX11RenderStateCache::CreateDepthStencilState(const DepthStencilState& depthStencilState) const
{
    D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
    depthStencilDesc.DepthEnable = depthStencilState.m_DepthEnable;
    depthStencilDesc.DepthWriteMask = (depthStencilState.m_EnableDepthWrite ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO);
    depthStencilDesc.DepthFunc = ConvertShipyardComparisonFuncToDX11(depthStencilState.m_DepthComparisonFunc);
    depthStencilDesc.StencilEnable = depthStencilState.m_StencilEnable;
    depthStencilDesc.StencilReadMask = depthStencilState.m_StencilReadMask;
    depthStencilDesc.StencilWriteMask = depthStencilState.m_StencilWriteMask;
    depthStencilDesc.FrontFace.StencilFailOp = ConvertShipyardStencilOperationToDX11(depthStencilState.m_FrontFaceStencilFailOp);
    depthStencilDesc.FrontFace.StencilDepthFailOp = ConvertShipyardStencilOperationToDX11(depthStencilState.m_FrontFaceStencilDepthFailOp);
    depthStencilDesc.FrontFace.StencilPassOp = ConvertShipyardStencilOperationToDX11(depthStencilState.m_FrontFaceStencilPassOp);
    depthStencilDesc.FrontFace.StencilFunc = ConvertShipyardComparisonFuncToDX11(depthStencilState.m_FrontFaceStencilComparisonFunc);
    depthStencilDesc.BackFace.StencilFailOp = ConvertShipyardStencilOperationToDX11(depthStencilState.m_BackFaceStencilFailOp);
    depthStencilDesc.BackFace.StencilDepthFailOp = ConvertShipyardStencilOperationToDX11(depthStencilState.m_BackFaceStencilDepthFailOp);
    depthStencilDesc.BackFace.StencilPassOp = ConvertShipyardStencilOperationToDX11(depthStencilState.m_BackFaceStencilPassOp);
    depthStencilDesc.BackFace.StencilFunc = ConvertShipyardComparisonFuncToDX11(depthStencilState.m_BackFaceStencilComparisonFunc);

    ID3D11DepthStencilState* nativeDepthStencilState = nullptr;

    HRESULT hr = m_Device->CreateDepthStencilState(&depthStencilDesc, &nativeDepthStencilState);
    if (FAILED(hr))
    {
        SHIP_LOG_ERROR("DX11RenderStateCache::CreateDepthStencilState() --> Couldn't create depth stencil state.");
        return nullptr;
    }

    return nativeDepthStencilState;
}

ID3D11BlendState* DX11RenderStateCache::CreateBlendState(const BlendState& blendState) const
{
    D3D11_BLEND_DESC blendDesc;
    blendDesc.AlphaToCoverageEnable = blendState.m_AlphaToCoverageEnable;
    blendDesc.IndependentBlendEnable = blendState.m_IndependentBlendEnable;

    for (uint32_t i = 0; i < GfxConstants::GfxConstants_MaxRenderTargetsBound; i++)
    {
        D3D11_RENDER_TARGET_BLEND_DESC& renderTargetBlendDesc = blendDesc.RenderTarget[i];
        const RenderTargetBlendState& renderTargetBlendState = blendState.renderTargetBlendStates[i];

        renderTargetBlendDesc.BlendEnable = renderTargetBlendState.m_BlendEnable;
        renderTargetBlendDesc.SrcBlend = ConvertShipyardBlendFactorToDX11(renderTargetBlendState.m_SourceBlend);
        renderTargetBlendDesc.DestBlend = ConvertShipyardBlendFactorToDX11(renderTargetBlendState.m_DestBlend);
        renderTargetBlendDesc.BlendOp = ConvertShipyardBlendOperatorToDX11(renderTargetBlendState.m_BlendOperator);
        renderTargetBlendDesc.SrcBlendAlpha = ConvertShipyardBlendFactorToDX11(renderTargetBlendState.m_SourceAlphaBlend);
        renderTargetBlendDesc.DestBlendAlpha = ConvertShipyardBlendFactorToDX11(renderTargetBlendState.m_DestAlphaBlend);
        renderTargetBlendDesc.BlendOpAlpha = ConvertShipyardBlendOperatorToDX11(renderTargetBlendState.m_AlphaBlendOperator);
        renderTargetBlendDesc.RenderTargetWriteMask = ConvertShipyardRenderTargetWriteMaskToDX11(renderTargetBlendState.m_RenderTargetWriteMask);
    }

    ID3D11BlendState* nativeBlendState = nullptr;

    HRESULT hr = m_Device->CreateBlendState(&blendDesc, &nativeBlendState);
    if (FAILED(hr))
    {
        SHIP_LOG_ERROR("DX11RenderStateCache::CreateBlendState() --> Couldn't create blend state.");
        return nullptr;
    }

    return nativeBlendState;
}

}