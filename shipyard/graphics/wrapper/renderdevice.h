#pragma once

#include <graphics/wrapper/wrapper_common.h>

#include <graphics/wrapper/buffer.h>
#include <graphics/wrapper/descriptorset.h>
#include <graphics/wrapper/pipelinestateobject.h>
#include <graphics/wrapper/rendertarget.h>
#include <graphics/wrapper/rootsignature.h>
#include <graphics/wrapper/shader.h>
#include <graphics/wrapper/texture.h>

#include <graphics/vertexformat.h>

namespace Shipyard
{
    class SHIPYARD_GRAPHICS_API BaseRenderDevice
    {
    public:
#ifdef DEBUG_WRAPPER_INTERFACE_COMPILATION
        virtual GFXVertexBufferHandle CreateVertexBuffer(shipUint32 numVertices, VertexFormatType vertexFormatType, shipBool dynamic, void* initialData) = 0;
        virtual void DestroyVertexBuffer(GFXVertexBufferHandle& gfxVertexBufferHandle) = 0;
        virtual GFXVertexBuffer& GetVertexBuffer(GFXVertexBufferHandle gfxVertexBufferHandle) = 0;
        virtual const GFXVertexBuffer& GetVertexBuffer(GFXVertexBufferHandle gfxVertexBufferHandle) const = 0;

        virtual GFXIndexBufferHandle CreateIndexBuffer(shipUint32 numIndices, shipBool uses2BytesPerIndex, shipBool dynamic, void* initialData) = 0;
        virtual void DestroyIndexBuffer(GFXIndexBufferHandle& gfxIndexBufferHandle) = 0;
        virtual GFXIndexBuffer& GetIndexBuffer(GFXIndexBufferHandle gfxIndexBufferHandle) = 0;
        virtual const GFXIndexBuffer& GetIndexBuffer(GFXIndexBufferHandle gfxIndexBufferHandle) const = 0;

        virtual GFXConstantBufferHandle CreateConstantBuffer(shipUint32 dataSizeInBytes, shipBool dynamic, void* initialData) = 0;
        virtual void DestroyConstantBuffer(GFXConstantBufferHandle& gfxConstantBufferHandle) = 0;
        virtual GFXConstantBuffer& GetConstantBuffer(GFXConstantBufferHandle gfxConstantBufferHandle) = 0;
        virtual const GFXConstantBuffer& GetConstantBuffer(GFXConstantBufferHandle gfxConstantBufferHandle) const = 0;

        virtual GFXByteBufferHandle CreateByteBuffer(ByteBuffer::ByteBufferCreationFlags byteBufferCreationFlags, shipUint32 dataSizeInBytes, shipBool dynamic, void* initialData) = 0;
        virtual void DestroyByteBuffer(GFXByteBufferHandle& gfxByteBufferHandle) = 0;
        virtual GFXByteBuffer& GetByteBuffer(GFXByteBufferHandle gfxByteBufferHandle) = 0;
        virtual const GFXByteBuffer& GetByteBuffer(GFXByteBufferHandle gfxByteBufferHandle) const = 0;
        virtual GFXByteBuffer* GetByteBufferPtr(GFXByteBufferHandle gfxByteBufferHandle) = 0;
        virtual const GFXByteBuffer* GetByteBufferPtr(GFXByteBufferHandle gfxByteBufferHandle) const = 0;

        virtual GFXTexture2DHandle CreateTexture2D(
                shipUint32 width,
                shipUint32 height,
                GfxFormat pixelFormat,
                shipBool dynamic,
                void* initialData,
                shipBool generateMips,
                TextureUsage textureUsage = TextureUsage::TextureUsage_Default) = 0;
        virtual void DestroyTexture2D(GFXTexture2DHandle& gfxTextureHandle) = 0;
        virtual GFXTexture2D& GetTexture2D(GFXTexture2DHandle gfxTexture2dHandle) = 0;
        virtual const GFXTexture2D& GetTexture2D(GFXTexture2DHandle gfxTexture2dHandle) const = 0;
        virtual GFXTexture2D* GetTexture2DPtr(GFXTexture2DHandle gfxTexture2dHandle) = 0;
        virtual const GFXTexture2D* GetTexture2DPtr(GFXTexture2DHandle gfxTexture2dHandle) const = 0;

        virtual GFXTexture2DArrayHandle CreateTexture2DArray(
                shipUint32 width,
                shipUint32 height,
                shipUint32 numSlices,
                GfxFormat pixelFormat,
                shipBool dynamic,
                void* initialData,
                shipBool generateMips,
                TextureUsage textureUsage = TextureUsage::TextureUsage_Default) = 0;
        virtual void DestroyTexture2DArray(GFXTexture2DArrayHandle& gfxTexture2DArrayHandle) = 0;
        virtual GFXTexture2DArray& GetTexture2DArray(GFXTexture2DArrayHandle gfxTexture2DArrayHandle) = 0;
        virtual const GFXTexture2DArray& GetTexture2DArray(GFXTexture2DArrayHandle gfxTexture2DArrayHandle) const = 0;
        virtual GFXTexture2DArray* GetTexture2DArrayPtr(GFXTexture2DArrayHandle gfxTexture2DArrayHandle) = 0;
        virtual const GFXTexture2DArray* GetTexture2DArrayPtr(GFXTexture2DArrayHandle gfxTexture2DArrayHandle) const = 0;

        virtual GFXTexture3DHandle CreateTexture3D(
                shipUint32 width,
                shipUint32 height,
                shipUint32 depth,
                GfxFormat pixelFormat,
                shipBool dynamic,
                void* initialData,
                shipBool generateMips,
                TextureUsage textureUsage = TextureUsage::TextureUsage_Default) = 0;
        virtual void DestroyTexture3D(GFXTexture3DHandle& gfxTexture3DHandle) = 0;
        virtual GFXTexture3D& GetTexture3D(GFXTexture3DHandle gfxTexture3DHandle) = 0;
        virtual const GFXTexture3D& GetTexture3D(GFXTexture3DHandle gfxTexture3DHandle) const = 0;
        virtual GFXTexture3D* GetTexture3DPtr(GFXTexture3DHandle gfxTexture3DHandle) = 0;
        virtual const GFXTexture3D* GetTexture3DPtr(GFXTexture3DHandle gfxTexture3DHandle) const = 0;

        virtual GFXTextureCubeHandle CreateTextureCube(
                shipUint32 width,
                shipUint32 height,
                GfxFormat pixelFormat,
                shipBool dynamic,
                void* initialData,
                shipBool generateMips,
                TextureUsage textureUsage = TextureUsage::TextureUsage_Default) = 0;
        virtual void DestroyTextureCube(GFXTextureCubeHandle& gfxTextureCubeHandle) = 0;
        virtual GFXTextureCube& GetTextureCube(GFXTextureCubeHandle gfxTextureCubeHandle) = 0;
        virtual const GFXTextureCube& GetTextureCube(GFXTextureCubeHandle gfxTextureCubeHandle) const = 0;
        virtual GFXTextureCube* GetTextureCubePtr(GFXTextureCubeHandle gfxTextureCubeHandle) = 0;
        virtual const GFXTextureCube* GetTextureCubePtr(GFXTextureCubeHandle gfxTextureCubeHandle) const = 0;

        virtual GFXSamplerHandle CreateSampler(const SamplerState& samplerState) = 0;
        virtual void DestroySampler(GFXSamplerHandle& gfxSamplerHandle) = 0;
        virtual GFXSampler& GetSampler(GFXSamplerHandle gfxSamplerHandle) = 0;
        virtual const GFXSampler& GetSampler(GFXSamplerHandle gfxSamplerHandle) const = 0;
        virtual GFXSampler* GetSamplerPtr(GFXSamplerHandle gfxSamplerHandle) = 0;
        virtual const GFXSampler* GetSamplerPtr(GFXSamplerHandle gfxSamplerHandle) const = 0;

        virtual GFXRenderTargetHandle CreateRenderTarget(GFXTexture2DHandle* texturesToAttach, shipUint32 numTexturesToAttach) = 0;
        virtual void DestroyRenderTarget(GFXRenderTargetHandle& gfxRenderTargetHandle) = 0;
        virtual GFXRenderTarget& GetRenderTarget(GFXRenderTargetHandle gfxRenderTargetHandle) = 0;
        virtual const GFXRenderTarget& GetRenderTarget(GFXRenderTargetHandle gfxRenderTargetHandle) const = 0;

        virtual GFXDepthStencilRenderTargetHandle CreateDepthStencilRenderTarget(GFXTexture2DHandle depthStencilTexture) = 0;
        virtual void DestroyDepthStencilRenderTarget(GFXDepthStencilRenderTargetHandle& gfxDepthStencilRenderTargetHandle) = 0;
        virtual GFXDepthStencilRenderTarget& GetDepthStencilRenderTarget(GFXDepthStencilRenderTargetHandle gfxDepthStencilRenderTargetHandle) = 0;
        virtual const GFXDepthStencilRenderTarget& GetDepthStencilRenderTarget(GFXDepthStencilRenderTargetHandle gfxDepthStencilRenderTargetHandle) const = 0;

        virtual GFXVertexShaderHandle CreateVertexShader(void* shaderData, shipUint64 shaderDataSize) = 0;
        virtual void DestroyVertexShader(GFXVertexShaderHandle& gfxVertexShaderHandle) = 0;
        virtual GFXVertexShader& GetVertexShader(GFXVertexShaderHandle gfxVertexShaderHandle) = 0;
        virtual const GFXVertexShader& GetVertexShader(GFXVertexShaderHandle gfxVertexShaderHandle) const = 0;

        virtual GFXPixelShaderHandle CreatePixelShader(void* shaderData, shipUint64 shaderDataSize) = 0;
        virtual void DestroyPixelShader(GFXPixelShaderHandle& gfxPixelShaderHandle) = 0;
        virtual GFXPixelShader& GetPixelShader(GFXPixelShaderHandle gfxPixelShaderHandle) = 0;
        virtual const GFXPixelShader& GetPixelShader(GFXPixelShaderHandle gfxPixelShaderHandle) const = 0;

        virtual GFXComputeShaderHandle CreateComputeShader(void* shaderData, shipUint64 shaderDataSize) = 0;
        virtual void DestroyComputeShader(GFXComputeShaderHandle& gfxComputeShaderHandle) = 0;
        virtual GFXComputeShader& GetComputeShader(GFXComputeShaderHandle gfxComputeShaderHandle) = 0;
        virtual const GFXComputeShader& GetComputeShader(GFXComputeShaderHandle gfxComputeShaderHandle) const = 0;

        virtual GFXRootSignatureHandle CreateRootSignature(const Array<RootSignatureParameterEntry>& rootSignatureParameters) = 0;
        virtual void DestroyRootSignature(GFXRootSignatureHandle& gfxRootSignatureHandle) = 0;
        virtual GFXRootSignature& GetRootSignature(GFXRootSignatureHandle gfxRootSignatureHandle) = 0;
        virtual const GFXRootSignature& GetRootSignature(GFXRootSignatureHandle gfxRootSignatureHandle) const = 0;
        virtual GFXRootSignature* GetRootSignaturePtr(GFXRootSignatureHandle gfxRootSignatureHandle) = 0;
        virtual const GFXRootSignature* GetRootSignaturePtr(GFXRootSignatureHandle gfxRootSignatureHandle) const = 0;

        virtual GFXGraphicsPipelineStateObjectHandle CreateGraphicsPipelineStateObject(const GraphicsPipelineStateObjectCreationParameters& pipelineStateObjectCreationParameters) = 0;
        virtual void DestroyGraphicsPipelineStateObject(GFXGraphicsPipelineStateObjectHandle& gfxPipelineStateObjectHandle) = 0;
        virtual GFXGraphicsPipelineStateObject& GetGraphicsPipelineStateObject(GFXGraphicsPipelineStateObjectHandle gfxPipelineStateObjectHandle) = 0;
        virtual const GFXGraphicsPipelineStateObject& GetGraphicsPipelineStateObject(GFXGraphicsPipelineStateObjectHandle gfxPipelineStateObjectHandle) const = 0;

        virtual GFXComputePipelineStateObjectHandle CreateComputePipelineStateObject(const ComputePipelineStateObjectCreationParameters& pipelineStateObjectCreationParameters) = 0;
        virtual void DestroyComputePipelineStateObject(GFXComputePipelineStateObjectHandle& gfxPipelineStateObjectHandle) = 0;
        virtual GFXComputePipelineStateObject& GetComputePipelineStateObject(GFXComputePipelineStateObjectHandle gfxPipelineStateObjectHandle) = 0;
        virtual const GFXComputePipelineStateObject& GetComputePipelineStateObject(GFXComputePipelineStateObjectHandle gfxPipelineStateObjectHandle) const = 0;

        virtual GFXDescriptorSetHandle CreateDescriptorSet(DescriptorSetType descriptorSetType, const Array<DescriptorSetEntryDeclaration>& descriptorSetEntryDeclarations) = 0;
        virtual void DestroyDescriptorSet(GFXDescriptorSetHandle& gfxDescriptorSetHandle) = 0;
        virtual GFXDescriptorSet& GetDescriptorSet(GFXDescriptorSetHandle gfxDescriptorSetHandle) = 0;
        virtual const GFXDescriptorSet& GetDescriptorSet(GFXDescriptorSetHandle gfxDescriptorSetHandle) const = 0;
#endif // #ifdef DEBUG_WRAPPER_INTERFACE_COMPILATION
    };
}