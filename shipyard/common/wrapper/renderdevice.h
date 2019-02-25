#pragma once

#include <common/wrapper/wrapper_common.h>

#include <common/vertexformat.h>

namespace Shipyard
{
    class SHIPYARD_API BaseRenderDevice
    {
    public:
#ifdef DEBUG_WRAPPER_INTERFACE_COMPILATION
        virtual GFXVertexBuffer* CreateVertexBuffer(uint32_t numVertices, VertexFormatType vertexFormatType, bool dynamic, void* initialData) = 0;
        virtual GFXIndexBuffer* CreateIndexBuffer(uint32_t numIndices, bool uses2BytesPerIndex, bool dynamic, void* initialData) = 0;
        virtual GFXConstantBuffer* CreateConstantBuffer(uint32_t dataSizeInBytes, bool dynamic, void* initialData) = 0;

        virtual GFXTexture2D* CreateTexture2D(
                uint32_t width,
                uint32_t height,
                GfxFormat pixelFormat,
                bool dynamic,
                void* initialData,
                bool generateMips,
                TextureUsage textureUsage = TextureUsage::TextureUsage_Default) = 0;

        virtual GFXRenderTarget* CreateRenderTarget(GFXTexture2D** texturesToAttach, uint32_t numTexturesToAttach) = 0;
        virtual GFXDepthStencilRenderTarget* CreateDepthStencilRenderTarget(GFXTexture2D& depthStencilTexture) = 0;

        virtual GFXVertexShader* CreateVertexShader(void* shaderData, uint64_t shaderDataSize) = 0;
        virtual GFXPixelShader* CreatePixelShader(void* shaderData, uint64_t shaderDataSize) = 0;

        virtual GFXRootSignature* CreateRootSignature(const Array<RootSignatureParameterEntry>& rootSignatureParameters) = 0;
        virtual GFXPipelineStateObject* CreatePipelineStateObject(const PipelineStateObjectCreationParameters& pipelineStateObjectCreationParameters) = 0;
        virtual GFXDescriptorSet* CreateDescriptorSet(DescriptorSetType descriptorSetType, const RootSignature& rootSignature) = 0;
#endif // #ifdef DEBUG_WRAPPER_INTERFACE_COMPILATION
    };
}