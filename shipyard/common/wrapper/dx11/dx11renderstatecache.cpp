#include <common/wrapper/dx11/dx11renderstatecache.h>

#include <common/vertexformat.h>

#include <common/wrapper/dx11/dx11_common.h>
#include <common/wrapper/dx11/dx11buffer.h>
#include <common/wrapper/dx11/dx11descriptorset.h>
#include <common/wrapper/dx11/dx11pipelinestateobject.h>
#include <common/wrapper/dx11/dx11rootsignature.h>
#include <common/wrapper/dx11/dx11shader.h>
#include <common/wrapper/dx11/dx11texture.h>

#pragma warning( disable : 4005 )

#include <d3d11.h>

#pragma warning( default : 4005 )

namespace Shipyard
{;

DX11RenderStateCache::DX11RenderStateCache(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
    : m_Device(device)
    , m_DeviceContext(deviceContext)
    , m_NativeRasterizerState(nullptr)
    , m_NativeDepthStencilState(nullptr)
    , m_NativeDepthStencilView(nullptr)
{
    Reset();

    // Make sure these states are set the first time: otherwise, a rasterizer state that has the same value as the default
    // constructor will not detect it as changed
    m_RenderStateCacheDirtyFlags.SetBit(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_RasterizerState);
    m_RenderStateCacheDirtyFlags.SetBit(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_DepthStencilState);
}

DX11RenderStateCache::~DX11RenderStateCache()
{
    Reset();
}

void DX11RenderStateCache::Reset()
{
    m_VertexShader = nullptr;
    m_PixelShader = nullptr;

    m_RasterizerState = RasterizerState();
    m_DepthStencilState = DepthStencilState();

    m_VertexFormatType = VertexFormatType::VertexFormatType_Count;
    m_PrimitiveTopology = PrimitiveTopology::TriangleList;

    m_NumRenderTargets = 0;
    for (uint32_t i = 0; i < 8; i++)
    {
        m_RenderTargetsFormat[i] = GfxFormat::R8G8B8A8_UNORM;
    }

    m_DepthStencilFormat = GfxFormat::R32_UINT;

    m_Viewport = GfxViewport();

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

    // The render state cache is not the owner of those interfaces, therefore, we don't release them.
    m_VertexShader = nullptr;
    m_PixelShader = nullptr;

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

    for (uint32_t i = 0; i < GfxConstants::GfxConstatns_MaxSamplersBoundPerShaderStage; i++)
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
            assert(false);
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

    if (pipelineStateObjectParameters.vertexShader != m_VertexShader)
    {
        assert(pipelineStateObjectParameters.vertexShader != nullptr);

        m_VertexShader = static_cast<GFXVertexShader*>(pipelineStateObjectParameters.vertexShader);
        m_RenderStateCacheDirtyFlags.SetBit(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_VertexShader);
    }

    if (pipelineStateObjectParameters.pixelShader != m_PixelShader)
    {
        // Null pixel shader is actually okay (for example, depth rendering only).
        if (pipelineStateObjectParameters.pixelShader == nullptr)
        {
            m_PixelShader = nullptr;
        }
        else
        {
            m_PixelShader = static_cast<GFXPixelShader*>(pipelineStateObjectParameters.pixelShader);
        }

        m_RenderStateCacheDirtyFlags.SetBit(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_PixelShader);
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

void DX11RenderStateCache::BindDescriptorTableFromDescriptorSet(
        const Array<GfxResource*>& descriptorTableResources,
        const RootSignatureParameterEntry& rootSignatureParameter)
{
    // Not yet implemented
    assert(false);
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
                    // Not yet implemented
                    assert(false);
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
            assert(false);
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

void DX11RenderStateCache::CommitStateChangesForGraphics()
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

    // In Direct3D 11, no need to rebind resources when the shader changes.
    if (m_RenderStateCacheDirtyFlags.IsBitSet(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_VertexShader))
    {
        m_DeviceContext->VSSetShader(m_VertexShader->GetShader(), nullptr, 0);

        m_RenderStateCacheDirtyFlags.UnsetBit(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_VertexShader);
    }

    if (m_RenderStateCacheDirtyFlags.IsBitSet(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_PixelShader))
    {
        m_DeviceContext->PSSetShader(m_PixelShader->GetShader(), nullptr, 0);

        m_RenderStateCacheDirtyFlags.UnsetBit(RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_PixelShader);
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

    // Everything should be accounted for.
    assert(m_RenderStateCacheDirtyFlags.IsClear());
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
        MessageBox(NULL, "CreateRasterizerState failed", "DX11 error", MB_OK);
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
        MessageBox(NULL, "CreateDepthStencilState failed", "DX11 error", MB_OK);
        return nullptr;
    }

    return nativeDepthStencilState;
}

}