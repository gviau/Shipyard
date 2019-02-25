#pragma once

#include <common/wrapper/wrapper_common.h>

#include <system/bitfield.h>

struct ID3D11Buffer;
struct ID3D11DepthStencilState;
struct ID3D11DepthStencilView;
struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11RasterizerState;
struct ID3D11RenderTargetView;
struct ID3D11SamplerState;
struct ID3D11ShaderResourceView;
struct ID3D11UnorderedAccessView;

namespace Shipyard
{
    class GfxResource;

    class DX11RenderStateCache
    {
    public:
        DX11RenderStateCache(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
        ~DX11RenderStateCache();

        void Reset();

        void BindRootSignature(const GFXRootSignature& rootSignature);
        void BindPipelineStateObject(const GFXPipelineStateObject& pipelineStateObject);
        void BindDescriptorSet(const GFXDescriptorSet& descriptorSet, const GFXRootSignature& rootSignature);

        void BindRenderTarget(const GFXRenderTarget& renderTarget);
        void BindDepthStencilRenderTarget(const GFXDepthStencilRenderTarget& depthStencilRenderTarget);

        void CommitStateChangesForGraphics();

    private:
        void BindDescriptorTableFromDescriptorSet(const Array<GfxResource*>& descriptorTableResources, const RootSignatureParameterEntry& rootSignatureParameter);
        void BindDescriptorFromDescriptorSet(GfxResource* descriptorResource, const RootSignatureParameterEntry& rootSignatureParameter);

        template <uint32_t NumBits>
        void MarkBindingSlotAsDirty(Bitfield<NumBits>* dirtyBindingSlots, ShaderVisibility shaderVisibility, uint32_t bindingSlot)
        {
            if ((shaderVisibility & ShaderVisibility::ShaderVisibility_Vertex) > 0)
            {
                dirtyBindingSlots[ShaderStage::ShaderStage_Vertex].SetBit(bindingSlot);
            }

            if ((shaderVisibility & ShaderVisibility::ShaderVisibility_Pixel) > 0)
            {
                dirtyBindingSlots[ShaderStage::ShaderStage_Pixel].SetBit(bindingSlot);
            }

            if ((shaderVisibility & ShaderVisibility::ShaderVisibility_Hull) > 0)
            {
                dirtyBindingSlots[ShaderStage::ShaderStage_Hull].SetBit(bindingSlot);
            }

            if ((shaderVisibility & ShaderVisibility::ShaderVisibility_Domain) > 0)
            {
                dirtyBindingSlots[ShaderStage::ShaderStage_Domain].SetBit(bindingSlot);
            }

            if ((shaderVisibility & ShaderVisibility::ShaderVisibility_Geometry) > 0)
            {
                dirtyBindingSlots[ShaderStage::ShaderStage_Geometry].SetBit(bindingSlot);
            }

            if ((shaderVisibility & ShaderVisibility::ShaderVisibility_Compute) > 0)
            {
                dirtyBindingSlots[ShaderStage::ShaderStage_Compute].SetBit(bindingSlot);
            }
        }

        ID3D11RasterizerState* CreateRasterizerState(const RasterizerState& rasterizerState) const;
        ID3D11DepthStencilState* CreateDepthStencilState(const DepthStencilState& depthStencilState) const;

    private:
        enum RenderStateCacheDirtyFlag : uint8_t
        {
            RenderStateCacheDirtyFlag_RasterizerState,
            RenderStateCacheDirtyFlag_DepthStencilState,
            RenderStateCacheDirtyFlag_Viewport,

            RenderStateCacheDirtyFlag_VertexShader,
            RenderStateCacheDirtyFlag_PixelShader,
            RenderStateCacheDirtyFlag_Hull,
            RenderStateCacheDirtyFlag_Domain,
            RenderStateCacheDirtyFlag_Geometry,
            RenderStateCacheDirtyFlag_Compute,

            RenderStateCacheDirtyFlag_ConstantBufferViews,
            RenderStateCacheDirtyFlag_ShaderResourceViews,
            RenderStateCacheDirtyFlag_UnorderedAccessViews,
            RenderStateCacheDirtyFlag_Samplers,

            RenderStateCacheDirtyFlag_PrimitiveTopology,
            RenderStateCacheDirtyFlag_VertexFormatType,

            RenderStateCacheDirtyFlag_RenderTargets,
            RenderStateCacheDirtyFlag_DepthStencilRenderTarget,

            RenderStateCacheDirtyFlag_Count
        };

    private:
        ID3D11Device* m_Device;
        ID3D11DeviceContext* m_DeviceContext;

        // Redundant render state cache
        Bitfield<RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_Count> m_RenderStateCacheDirtyFlags;

        GFXVertexShader* m_VertexShader;
        GFXPixelShader* m_PixelShader;

        RasterizerState m_RasterizerState;
        DepthStencilState m_DepthStencilState;

        VertexFormatType m_VertexFormatType;
        PrimitiveTopology m_PrimitiveTopology;

        uint32_t m_NumRenderTargets;
        GfxFormat m_RenderTargetsFormat[GfxConstants::GfxConstants_MaxRenderTargetsBound];
        GfxFormat m_DepthStencilFormat;

        GfxViewport m_Viewport;

        ShaderVisibility m_ConstantBufferViewsShaderVisibility[GfxConstants::GfxConstants_MaxConstantBufferViewsBoundPerShaderStage];
        ShaderVisibility m_ShaderResourceViewsShaderVisibility[GfxConstants::GfxConstants_MaxShaderResourceViewsBoundPerShaderStage];
        ShaderVisibility m_UnorderedAccessViewsShaderVisibility[GfxConstants::GfxConstants_MaxUnorderedAccessViewsBoundPerShaderStage];
        ShaderVisibility m_SamplersShaderVisibility[GfxConstants::GfxConstants_MaxSamplersBoundPerShaderStage];

        Bitfield<GfxConstants::GfxConstants_MaxShaderResourceViewsBoundPerShaderStage> m_DirtySlotShaderResourceViewsPerShaderStage[ShaderStage::ShaderStage_Count];
        Bitfield<GfxConstants::GfxConstants_MaxConstantBufferViewsBoundPerShaderStage> m_DirtySlotConstantBufferViewsPerShaderStage[ShaderStage::ShaderStage_Count];
        Bitfield<GfxConstants::GfxConstants_MaxUnorderedAccessViewsBoundPerShaderStage> m_DirtySlotUnorderedAccessViewsPerShaderStage[ShaderStage::ShaderStage_Count];
        Bitfield<GfxConstants::GfxConstants_MaxSamplersBoundPerShaderStage> m_DirtySlotSamplersPerShaderStage[ShaderStage::ShaderStage_Count];

        // Native interface for actual binding
        ID3D11RasterizerState* m_NativeRasterizerState;
        ID3D11DepthStencilState* m_NativeDepthStencilState;

        ID3D11RenderTargetView* m_NativeRenderTargets[GfxConstants::GfxConstants_MaxRenderTargetsBound];
        ID3D11DepthStencilView* m_NativeDepthStencilView;

        ID3D11ShaderResourceView* m_NativeShaderResourceViews[GfxConstants::GfxConstants_MaxShaderResourceViewsBoundPerShaderStage];
        ID3D11Buffer* m_NativeConstantBufferViews[GfxConstants::GfxConstants_MaxConstantBufferViewsBoundPerShaderStage];
        ID3D11UnorderedAccessView* m_NativeUnorderedAccessViews[GfxConstants::GfxConstants_MaxUnorderedAccessViewsBoundPerShaderStage];
        ID3D11SamplerState* m_NativeSamplers[GfxConstants::GfxConstants_MaxSamplersBoundPerShaderStage];
    };
}