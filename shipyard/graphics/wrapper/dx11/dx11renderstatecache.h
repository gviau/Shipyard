#pragma once

#include <graphics/wrapper/wrapper_common.h>

#include <system/bitfield.h>

struct ID3D11BlendState;
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
    class GfxRenderDevice;
    class GfxResource;

    class DX11RenderStateCache
    {
    public:
        DX11RenderStateCache(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
        ~DX11RenderStateCache();

        void Reset();

        void BindRootSignature(const GFXRootSignature& rootSignature);
        void BindGraphicsPipelineStateObject(const GFXGraphicsPipelineStateObject& pipelineStateObject);
        void BindComputePipelineStateObject(const GFXComputePipelineStateObject& pipelineStateObject);
        void BindDescriptorSet(const GFXDescriptorSet& descriptorSet, const GFXRootSignature& rootSignature);

        void BindRenderTarget(const GFXRenderTarget& renderTarget);
        void BindDepthStencilRenderTarget(const GFXDepthStencilRenderTarget& depthStencilRenderTarget);

        void SetViewport(const GfxViewport& gfxViewport);
        void SetScissor(const GfxRect& gfxScissor);

        void SetVertexBuffers(GFXVertexBuffer* const * vertexBuffers, shipUint32 startSlot, shipUint32 numVertexBuffers, shipUint32* vertexBufferOffsets);
        void SetIndexBuffer(const GFXIndexBuffer& indexBuffer, shipUint32 indexBufferOffset);

        void CommitStateChangesForGraphics(GFXRenderDevice& gfxRenderDevice);

    private:
        void BindRootSignatureDescriptorTableEntry(const RootSignatureParameterEntry& rootSignatureParameter, ShaderVisibility shaderVisibilityForParameter);

        void BindDescriptorTableFromDescriptorSet(const Array<GfxResource*>& descriptorTableResources, shipUint32 descriptorRangeIndex, const RootSignatureParameterEntry& rootSignatureParameter);
        void BindDescriptorFromDescriptorSet(GfxResource* descriptorResource, const RootSignatureParameterEntry& rootSignatureParameter);

        void BindResourceAsConstantBuffer(GfxResource* descriptorResource, ShaderVisibility shaderVisibility, shipUint32 shaderBindingSlot);
        void BindResourceAsShaderResourceView(GfxResource* descriptorResource, ShaderVisibility shaderVisibility, shipUint32 shaderBindingSlot);
        void BindResourceAsUnorderedAccessView(GfxResource* descriptorResource, ShaderVisibility shaderVisibility, shipUint32 shaderBindingSlot);
        void BindResourceAsSampler(GfxResource* descriptorResource, ShaderVisibility shaderVisibility, shipUint32 shaderBindingSlot);

        template <shipUint32 NumBits>
        void MarkBindingSlotAsDirty(InplaceBitfield<NumBits>* dirtyBindingSlots, ShaderVisibility shaderVisibility, shipUint32 bindingSlot)
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
        ID3D11BlendState* CreateBlendState(const BlendState& blendState) const;

    private:
        enum RenderStateCacheDirtyFlag : shipUint8
        {
            RenderStateCacheDirtyFlag_RasterizerState,
            RenderStateCacheDirtyFlag_DepthStencilState,
            RenderStateCacheDirtyFlag_BlendState,
            RenderStateCacheDirtyFlag_Viewport,
            RenderStateCacheDirtyFlag_Scissor,

            RenderStateCacheDirtyFlag_VertexShader,
            RenderStateCacheDirtyFlag_PixelShader,
            RenderStateCacheDirtyFlag_HullShader,
            RenderStateCacheDirtyFlag_DomainShader,
            RenderStateCacheDirtyFlag_GeometryShader,
            RenderStateCacheDirtyFlag_ComputeShader,

            RenderStateCacheDirtyFlag_ConstantBufferViews,
            RenderStateCacheDirtyFlag_ShaderResourceViews,
            RenderStateCacheDirtyFlag_UnorderedAccessViews,
            RenderStateCacheDirtyFlag_Samplers,

            RenderStateCacheDirtyFlag_PrimitiveTopology,
            RenderStateCacheDirtyFlag_VertexFormatType,

            RenderStateCacheDirtyFlag_RenderTargets,
            RenderStateCacheDirtyFlag_DepthStencilRenderTarget,

            RenderStateCacheDirtyFlag_VertexBuffers,
            RenderStateCacheDirtyFlag_IndexBuffer,

            RenderStateCacheDirtyFlag_Count
        };

    private:
        ID3D11Device* m_Device;
        ID3D11DeviceContext* m_DeviceContext;

        // Redundant render state cache
        InplaceBitfield<RenderStateCacheDirtyFlag::RenderStateCacheDirtyFlag_Count> m_RenderStateCacheDirtyFlags;

        GFXVertexShaderHandle m_VertexShaderHandle;
        GFXPixelShaderHandle m_PixelShaderHandle;
        GFXComputeShaderHandle m_ComputeShaderHandle;

        RasterizerState m_RasterizerState;
        DepthStencilState m_DepthStencilState;
        BlendState m_BlendState;

        VertexFormatType m_VertexFormatType;
        PrimitiveTopology m_PrimitiveTopology;

        shipUint32 m_NumRenderTargets;
        GfxFormat m_RenderTargetsFormat[GfxConstants::GfxConstants_MaxRenderTargetsBound];
        GfxFormat m_DepthStencilFormat;

        GfxViewport m_Viewport;
        GfxRect m_Scissor;

        ID3D11Buffer* m_NativeVertexBuffers[GfxConstants::GfxConstants_MaxVertexBuffers];
        shipUint32 m_VertexBufferStartSlot;
        shipUint32 m_NumVertexBuffers;
        shipUint32 m_VertexBufferOffsets[GfxConstants::GfxConstants_MaxVertexBuffers];

        ID3D11Buffer* m_NativeIndexBuffer;
        shipUint32 m_IndexBufferFormat;
        shipUint32 m_IndexBufferOffset;

        ShaderVisibility m_ConstantBufferViewsShaderVisibility[GfxConstants::GfxConstants_MaxConstantBufferViewsBoundPerShaderStage];
        ShaderVisibility m_ShaderResourceViewsShaderVisibility[GfxConstants::GfxConstants_MaxShaderResourceViewsBoundPerShaderStage];
        ShaderVisibility m_UnorderedAccessViewsShaderVisibility[GfxConstants::GfxConstants_MaxUnorderedAccessViewsBoundPerShaderStage];
        ShaderVisibility m_SamplersShaderVisibility[GfxConstants::GfxConstants_MaxSamplersBoundPerShaderStage];

        InplaceBitfield<GfxConstants::GfxConstants_MaxShaderResourceViewsBoundPerShaderStage> m_DirtySlotShaderResourceViewsPerShaderStage[ShaderStage::ShaderStage_Count];
        InplaceBitfield<GfxConstants::GfxConstants_MaxConstantBufferViewsBoundPerShaderStage> m_DirtySlotConstantBufferViewsPerShaderStage[ShaderStage::ShaderStage_Count];
        InplaceBitfield<GfxConstants::GfxConstants_MaxUnorderedAccessViewsBoundPerShaderStage> m_DirtySlotUnorderedAccessViewsPerShaderStage[ShaderStage::ShaderStage_Count];
        InplaceBitfield<GfxConstants::GfxConstants_MaxSamplersBoundPerShaderStage> m_DirtySlotSamplersPerShaderStage[ShaderStage::ShaderStage_Count];

        // Native interface for actual binding
        ID3D11RasterizerState* m_NativeRasterizerState;
        ID3D11DepthStencilState* m_NativeDepthStencilState;
        ID3D11BlendState* m_NativeBlendState;

        ID3D11RenderTargetView* m_NativeRenderTargets[GfxConstants::GfxConstants_MaxRenderTargetsBound];
        ID3D11DepthStencilView* m_NativeDepthStencilView;

        ID3D11ShaderResourceView* m_NativeShaderResourceViews[ShaderStage::ShaderStage_Count][GfxConstants::GfxConstants_MaxShaderResourceViewsBoundPerShaderStage];
        ID3D11Buffer* m_NativeConstantBufferViews[ShaderStage::ShaderStage_Count][GfxConstants::GfxConstants_MaxConstantBufferViewsBoundPerShaderStage];
        ID3D11UnorderedAccessView* m_NativeUnorderedAccessViews[ShaderStage::ShaderStage_Count][GfxConstants::GfxConstants_MaxUnorderedAccessViewsBoundPerShaderStage];
        ID3D11SamplerState* m_NativeSamplers[ShaderStage::ShaderStage_Count][GfxConstants::GfxConstants_MaxSamplersBoundPerShaderStage];
    };
}