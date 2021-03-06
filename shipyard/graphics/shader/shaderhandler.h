#pragma once

#include <graphics/graphicstypes.h>

#include <graphics/shader/shaderkey.h>
#include <graphics/shader/shaderresourcebinder.h>

#include <graphics/wrapper/wrapper_common.h>

#include <graphics/wrapper/shader.h>

#include <system/platform.h>

namespace Shipyard
{
    class ShaderDatabase;
    class ShaderHandlerManager;
    class ShaderInputProvider;

    class SHIPYARD_GRAPHICS_API ShaderHandler
    {
        friend class ShaderDatabase;
        friend class ShaderHandlerManager;

    public:
        struct RenderState
        {
            GFXRenderTargetHandle GfxRenderTargetHandle = { InvalidGfxHandle };
            GFXDepthStencilRenderTargetHandle GfxDepthStencilRenderTargetHandle = { InvalidGfxHandle };

            PrimitiveTopology PrimitiveTopologyToRender = PrimitiveTopology::TriangleList;
            VertexFormatType VertexFormatTypeToRender = VertexFormatType::VertexFormatType_Count;

            RenderStateBlockStateOverride* OptionalRenderStateBlockStateOverride = nullptr;
        };

        struct ShaderRenderElementsForGraphics
        {
            GFXGraphicsPipelineStateObjectHandle GfxGraphicsPipelineStateObjectHandle;
            GFXRootSignatureHandle GfxRootSignatureHandle;
            GFXDescriptorSetHandle GfxDescriptorSetHandle;
        };

        struct ShaderRenderElementsForCompute
        {
            GFXComputePipelineStateObjectHandle GfxComputePipelineStateObjectHandle;
            GFXRootSignatureHandle GfxRootSignatureHandle;
            GFXDescriptorSetHandle GfxDescriptorSetHandle;
        };

    public:
        ShaderHandler(ShaderKey shaderKey);
        ~ShaderHandler();

        void ApplyShaderInputProvidersForGraphics(
                GFXRenderDevice& gfxRenderDevice,
                GFXDirectRenderCommandList& gfxDirectRenderCommandList,
                const Array<const ShaderInputProvider*>& shaderInputProviders);
        void ApplyShaderInputProvidersForCompute(
                GFXRenderDevice& gfxRenderDevice,
                GFXDirectRenderCommandList& gfxDirectRenderCommandList,
                const Array<const ShaderInputProvider*>& shaderInputProviders);

        const ShaderKey& GetShaderKey() const { return m_ShaderKey; }
        ShaderRenderElementsForGraphics GetShaderRenderElementsForGraphicsCommands(GFXRenderDevice& gfxRenderDevice, const RenderState& renderState);
        ShaderRenderElementsForCompute GetShaderRenderElementsForCompute(GFXRenderDevice& gfxRenderDevice);

    private:
        ShaderKey m_ShaderKey;

        GFXVertexShaderHandle m_GfxVertexShaderHandle;
        GFXPixelShaderHandle m_GfxPixelShaderHandle;
        GFXComputeShaderHandle m_GfxComputeShaderHander;

        GFXGraphicsPipelineStateObjectHandle m_GfxEffectivePipelineStateObjectHandle;

        ShaderRenderElementsForGraphics m_ShaderRenderElementsForGraphics;
        ShaderRenderElementsForCompute m_ShaderRenderElementsForCompute;
        ShaderResourceBinder m_ShaderResourceBinder;

        Array<GFXSamplerHandle> m_SamplerStateHandles;
    };
}