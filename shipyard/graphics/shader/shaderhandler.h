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

    struct RenderStateBlock;

    struct PipelineStateObjectCreationParameters;

    class SHIPYARD_API ShaderHandler
    {
        friend class ShaderDatabase;
        friend class ShaderHandlerManager;

    public:
        ShaderHandler(ShaderKey shaderKey);
        ~ShaderHandler();

        void ApplyShader(PipelineStateObjectCreationParameters& pipelineStateObjectCreationParameters) const;
        void GetRootSignature(GFXRootSignature& gfxRootSignature) const;

        void ApplyShaderInputProviders(
                GFXRenderDevice& gfxRenderDevice,
                GFXDirectRenderCommandList& gfxDirectRenderCommandList,
                const Array<ShaderInputProvider*>& shaderInputProviders);

        const ShaderKey& GetShaderKey() const { return m_ShaderKey; }
        GFXDescriptorSetHandle GetDescriptorSetHandle() const { return m_GfxDescriptorSetHandle; }

    private:
        ShaderKey m_ShaderKey;

        GFXVertexShaderHandle m_VertexShaderHandle;
        GFXPixelShaderHandle m_PixelShaderHandle;

        RenderStateBlock m_RenderStateBlock;
        Array<RootSignatureParameterEntry> m_RootSignatureParameters;
        ShaderResourceBinder m_ShaderResourceBinder;
        GFXDescriptorSetHandle m_GfxDescriptorSetHandle;
    };
}