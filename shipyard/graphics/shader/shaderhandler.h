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

    class SHIPYARD_API ShaderHandler
    {
        friend class ShaderDatabase;
        friend class ShaderHandlerManager;

    public:
        struct ShaderRenderElements
        {
            GFXPipelineStateObjectHandle GfxPipelineStateObjectHandle;
            GFXRootSignatureHandle GfxRootSignatureHandle;
            GFXDescriptorSetHandle GfxDescriptorSetHandle;
        };

    public:
        ShaderHandler(ShaderKey shaderKey);
        ~ShaderHandler();

        void ApplyShaderInputProviders(
                GFXRenderDevice& gfxRenderDevice,
                GFXDirectRenderCommandList& gfxDirectRenderCommandList,
                const Array<ShaderInputProvider*>& shaderInputProviders);

        const ShaderKey& GetShaderKey() const { return m_ShaderKey; }
        const ShaderRenderElements& GetShaderRenderElements() const { return m_ShaderRenderElements; }

    private:
        ShaderKey m_ShaderKey;

        GFXVertexShaderHandle m_GfxVertexShaderHandle;
        GFXPixelShaderHandle m_GfxPixelShaderHandle;

        ShaderRenderElements m_ShaderRenderElements;
        ShaderResourceBinder m_ShaderResourceBinder;

        Array<GFXSamplerHandle> m_SamplerStateHandles;
    };
}