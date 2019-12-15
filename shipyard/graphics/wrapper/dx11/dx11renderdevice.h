#pragma once

#include <graphics/wrapper/renderdevice.h>

#include <graphics/wrapper/dx11/dx11_common.h>

#include <graphics/wrapper/dx11/dx11buffer.h>
#include <graphics/wrapper/dx11/dx11descriptorset.h>
#include <graphics/wrapper/dx11/dx11pipelinestateobject.h>
#include <graphics/wrapper/dx11/dx11rendertarget.h>
#include <graphics/wrapper/dx11/dx11rootsignature.h>
#include <graphics/wrapper/dx11/dx11sampler.h>
#include <graphics/wrapper/dx11/dx11shader.h>
#include <graphics/wrapper/dx11/dx11texture.h>

#include <system/datapool.h>

#include <windows.h>

struct ID3D11Device;
struct ID3D11DeviceContext;
struct IDXGISwapChain;

namespace Shipyard
{
    class SHIPYARD_GRAPHICS_API DX11RenderDevice : public BaseRenderDevice
    {
    public:
        DX11RenderDevice();
        ~DX11RenderDevice();

        shipBool Create();
        void Destroy();

        GFXVertexBufferHandle CreateVertexBuffer(shipUint32 numVertices, VertexFormatType vertexFormatType, shipBool dynamic, void* initialData);
        void DestroyVertexBuffer(GFXVertexBufferHandle& gfxVertexBufferHandle);
        GFXVertexBuffer& GetVertexBuffer(GFXVertexBufferHandle gfxVertexBufferHandle);
        const GFXVertexBuffer& GetVertexBuffer(GFXVertexBufferHandle gfxVertexBufferHandle) const;

        GFXIndexBufferHandle CreateIndexBuffer(shipUint32 numIndices, shipBool uses2BytesPerIndex, shipBool dynamic, void* initialData);
        void DestroyIndexBuffer(GFXIndexBufferHandle& gfxIndexBufferHandle);
        GFXIndexBuffer& GetIndexBuffer(GFXIndexBufferHandle gfxIndexBufferHandle);
        const GFXIndexBuffer& GetIndexBuffer(GFXIndexBufferHandle gfxIndexBufferHandle) const;

        GFXConstantBufferHandle CreateConstantBuffer(shipUint32 dataSizeInBytes, shipBool dynamic, void* initialData);
        void DestroyConstantBuffer(GFXConstantBufferHandle& gfxConstantBufferHandle);
        GFXConstantBuffer& GetConstantBuffer(GFXConstantBufferHandle gfxConstantBufferHandle);
        const GFXConstantBuffer& GetConstantBuffer(GFXConstantBufferHandle gfxConstantBufferHandle) const;

        GFXByteBufferHandle CreateByteBuffer(ByteBuffer::ByteBufferCreationFlags byteBufferCreationFlags, shipUint32 dataSizeInBytes, shipBool dynamic, void* initialData);
        void DestroyByteBuffer(GFXByteBufferHandle& gfxByteBufferHandle);
        GFXByteBuffer& GetByteBuffer(GFXByteBufferHandle gfxByteBufferHandle);
        const GFXByteBuffer& GetByteBuffer(GFXByteBufferHandle gfxByteBufferHandle) const;
        GFXByteBuffer* GetByteBufferPtr(GFXByteBufferHandle gfxByteBufferHandle);
        const GFXByteBuffer* GetByteBufferPtr(GFXByteBufferHandle gfxByteBufferHandle) const;

        GFXTexture2DHandle CreateTexture2D(
                shipUint32 width,
                shipUint32 height,
                GfxFormat pixelFormat,
                shipBool dynamic,
                void* initialData,
                shipBool generateMips,
                TextureUsage textureUsage = TextureUsage::TextureUsage_Default);
        void DestroyTexture2D(GFXTexture2DHandle& gfxTexture2dHandle);
        GFXTexture2D& GetTexture2D(GFXTexture2DHandle gfxTexture2dHandle);
        const GFXTexture2D& GetTexture2D(GFXTexture2DHandle gfxTexture2dHandle) const;
        GFXTexture2D* GetTexture2DPtr(GFXTexture2DHandle gfxTexture2dHandle);
        const GFXTexture2D* GetTexture2DPtr(GFXTexture2DHandle gfxTexture2dHandle) const;

        GFXTexture2DArrayHandle CreateTexture2DArray(
                shipUint32 width,
                shipUint32 height,
                shipUint32 numSlices,
                GfxFormat pixelFormat,
                shipBool dynamic,
                void* initialData,
                shipBool generateMips,
                TextureUsage textureUsage = TextureUsage::TextureUsage_Default);
        void DestroyTexture2DArray(GFXTexture2DArrayHandle& gfxTexture2DArrayHandle);
        GFXTexture2DArray& GetTexture2DArray(GFXTexture2DArrayHandle gfxTexture2DArrayHandle);
        const GFXTexture2DArray& GetTexture2DArray(GFXTexture2DArrayHandle gfxTexture2DArrayHandle) const;
        GFXTexture2DArray* GetTexture2DArrayPtr(GFXTexture2DArrayHandle gfxTexture2DArrayHandle);
        const GFXTexture2DArray* GetTexture2DArrayPtr(GFXTexture2DArrayHandle gfxTexture2DArrayHandle) const;

        GFXTexture3DHandle CreateTexture3D(
                shipUint32 width,
                shipUint32 height,
                shipUint32 depth,
                GfxFormat pixelFormat,
                shipBool dynamic,
                void* initialData,
                shipBool generateMips,
                TextureUsage textureUsage = TextureUsage::TextureUsage_Default);
        void DestroyTexture3D(GFXTexture3DHandle& gfxTexture3DHandle);
        GFXTexture3D& GetTexture3D(GFXTexture3DHandle gfxTexture3DHandle);
        const GFXTexture3D& GetTexture3D(GFXTexture3DHandle gfxTexture3DHandle) const;
        GFXTexture3D* GetTexture3DPtr(GFXTexture3DHandle gfxTexture3DHandle);
        const GFXTexture3D* GetTexture3DPtr(GFXTexture3DHandle gfxTexture3DHandle) const;

        GFXTextureCubeHandle CreateTextureCube(
                shipUint32 width,
                shipUint32 height,
                GfxFormat pixelFormat,
                shipBool dynamic,
                void* initialData,
                shipBool generateMips,
                TextureUsage textureUsage = TextureUsage::TextureUsage_Default);
        void DestroyTextureCube(GFXTextureCubeHandle& gfxTextureCubeHandle);
        GFXTextureCube& GetTextureCube(GFXTextureCubeHandle gfxTextureCubeHandle);
        const GFXTextureCube& GetTextureCube(GFXTextureCubeHandle gfxTextureCubeHandle) const;
        GFXTextureCube* GetTextureCubePtr(GFXTextureCubeHandle gfxTextureCubeHandle);
        const GFXTextureCube* GetTextureCubePtr(GFXTextureCubeHandle gfxTextureCubeHandle) const;

        GFXSamplerHandle CreateSampler(const SamplerState& samplerState);
        void DestroySampler(GFXSamplerHandle& gfxSamplerHandle);
        GFXSampler& GetSampler(GFXSamplerHandle gfxSamplerHandle);
        const GFXSampler& GetSampler(GFXSamplerHandle gfxSamplerHandle) const;
        GFXSampler* GetSamplerPtr(GFXSamplerHandle gfxSamplerHandle);
        const GFXSampler* GetSamplerPtr(GFXSamplerHandle gfxSamplerHandle) const;

        GFXRenderTargetHandle CreateRenderTarget(GFXTexture2DHandle* texturesToAttach, shipUint32 numTexturesToAttach);
        void DestroyRenderTarget(GFXRenderTargetHandle& gfxRenderTargetHandle);
        GFXRenderTarget& GetRenderTarget(GFXRenderTargetHandle gfxRenderTargetHandle);
        const GFXRenderTarget& GetRenderTarget(GFXRenderTargetHandle gfxRenderTargetHandle) const;

        GFXDepthStencilRenderTargetHandle CreateDepthStencilRenderTarget(GFXTexture2DHandle depthStencilTexture);
        void DestroyDepthStencilRenderTarget(GFXDepthStencilRenderTargetHandle& gfxDepthStencilRenderTargetHandle);
        GFXDepthStencilRenderTarget& GetDepthStencilRenderTarget(GFXDepthStencilRenderTargetHandle gfxDepthStencilRenderTargetHandle);
        const GFXDepthStencilRenderTarget& GetDepthStencilRenderTarget(GFXDepthStencilRenderTargetHandle gfxDepthStencilRenderTargetHandle) const;

        GFXVertexShaderHandle CreateVertexShader(void* shaderData, shipUint64 shaderDataSize);
        void DestroyVertexShader(GFXVertexShaderHandle& gfxVertexShaderHandle);
        GFXVertexShader& GetVertexShader(GFXVertexShaderHandle gfxVertexShaderHandle);
        const GFXVertexShader& GetVertexShader(GFXVertexShaderHandle gfxVertexShaderHandle) const;

        GFXPixelShaderHandle CreatePixelShader(void* shaderData, shipUint64 shaderDataSize);
        void DestroyPixelShader(GFXPixelShaderHandle& gfxPixelShaderHandle);
        GFXPixelShader& GetPixelShader(GFXPixelShaderHandle gfxPixelShaderHandle);
        const GFXPixelShader& GetPixelShader(GFXPixelShaderHandle gfxPixelShaderHandle) const;

        GFXComputeShaderHandle CreateComputeShader(void* shaderData, shipUint64 shaderDataSize);
        void DestroyComputeShader(GFXComputeShaderHandle& gfxComputeShaderHandle);
        GFXComputeShader& GetComputeShader(GFXComputeShaderHandle gfxComputeShaderHandle);
        const GFXComputeShader& GetComputeShader(GFXComputeShaderHandle gfxComputeShaderHandle) const;

        GFXRootSignatureHandle CreateRootSignature(const Array<RootSignatureParameterEntry>& rootSignatureParameters);
        void DestroyRootSignature(GFXRootSignatureHandle& gfxRootSignatureHandle);
        GFXRootSignature& GetRootSignature(GFXRootSignatureHandle gfxRootSignatureHandle);
        const GFXRootSignature& GetRootSignature(GFXRootSignatureHandle gfxRootSignatureHandle) const;
        GFXRootSignature* GetRootSignaturePtr(GFXRootSignatureHandle gfxRootSignatureHandle);
        const GFXRootSignature* GetRootSignaturePtr(GFXRootSignatureHandle gfxRootSignatureHandle) const;

        GFXGraphicsPipelineStateObjectHandle CreateGraphicsPipelineStateObject(const GraphicsPipelineStateObjectCreationParameters& pipelineStateObjectCreationParameters);
        void DestroyGraphicsPipelineStateObject(GFXGraphicsPipelineStateObjectHandle& gfxPipelineStateObjectHandle);
        GFXGraphicsPipelineStateObject& GetGraphicsPipelineStateObject(GFXGraphicsPipelineStateObjectHandle gfxPipelineStateObjectHandle);
        const GFXGraphicsPipelineStateObject& GetGraphicsPipelineStateObject(GFXGraphicsPipelineStateObjectHandle gfxPipelineStateObjectHandle) const;

        GFXComputePipelineStateObjectHandle CreateComputePipelineStateObject(const ComputePipelineStateObjectCreationParameters& pipelineStateObjectCreationParameters);
        void DestroyComputePipelineStateObject(GFXComputePipelineStateObjectHandle& gfxPipelineStateObjectHandle);
        GFXComputePipelineStateObject& GetComputePipelineStateObject(GFXComputePipelineStateObjectHandle gfxPipelineStateObjectHandle);
        const GFXComputePipelineStateObject& GetComputePipelineStateObject(GFXComputePipelineStateObjectHandle gfxPipelineStateObjectHandle) const;

        GFXDescriptorSetHandle CreateDescriptorSet(DescriptorSetType descriptorSetType, const Array<DescriptorSetEntryDeclaration>& descriptorSetEntryDeclarations);
        void DestroyDescriptorSet(GFXDescriptorSetHandle& gfxDescriptorSetHandle);
        GFXDescriptorSet& GetDescriptorSet(GFXDescriptorSetHandle gfxDescriptorSetHandle);
        const GFXDescriptorSet& GetDescriptorSet(GFXDescriptorSetHandle gfxDescriptorSetHandle) const;

        IDXGISwapChain* CreateSwapchain(shipUint32 width, shipUint32 height, GfxFormat format, HWND hWnd, GFXTexture2DHandle& swapChainTextureHandle);

        ID3D11Device* GetDevice() const { return m_Device; }
        ID3D11DeviceContext* GetImmediateDeviceContext() const { return m_ImmediateDeviceContext; }

    private:
        ID3D11Device* m_Device;
        ID3D11DeviceContext* m_ImmediateDeviceContext;

        // Render device pools.
        DataPool<GFXVertexBuffer, SHIP_MAX_VERTEX_BUFFERS> m_VertexBufferPool;
        DataPool<GFXIndexBuffer, SHIP_MAX_INDEX_BUFFERS> m_IndexBufferPool;
        DataPool<GFXConstantBuffer, SHIP_MAX_CONSTANT_BUFFERS> m_ConstantBufferPool;
        DataPool<GFXByteBuffer, SHIP_MAX_BYTE_BUFFERS> m_ByteBufferPool;
        DataPool<GFXTexture2D, SHIP_MAX_2D_TEXTURES> m_Texture2dPool;
        DataPool<GFXTexture2DArray, SHIP_MAX_2D_ARRAY_TEXTURES> m_Texture2dArrayPool;
        DataPool<GFXTexture3D, SHIP_MAX_3D_TEXTURES> m_Texture3dPool;
        DataPool<GFXTextureCube, SHIP_MAX_CUBE_TEXTURES> m_TextureCubePool;
        DataPool<GFXSampler, SHIP_MAX_SAMPLERS> m_SamplerPool;
        DataPool<GFXRenderTarget, SHIP_MAX_RENDER_TARGETS> m_RenderTargetPool;
        DataPool<GFXDepthStencilRenderTarget, SHIP_MAX_DEPTH_STENCIL_RENDER_TARGETS> m_DepthStencilRenderTargetPool;
        DataPool<GFXVertexShader, SHIP_MAX_VERTEX_SHADERS> m_VertexShaderPool;
        DataPool<GFXPixelShader, SHIP_MAX_PIXEL_SHADERS> m_PixelShaderPool;
        DataPool<GFXComputeShader, SHIP_MAX_COMPUTE_SHADERS> m_ComputeShaderPool;
        DataPool<GFXRootSignature, SHIP_MAX_ROOT_SIGNATURES> m_RootSignaturePool;
        DataPool<GFXGraphicsPipelineStateObject, SHIP_MAX_GRAPHICS_PIPELINE_STATE_OBJECTS> m_GraphicsPipelineStateObjectPool;
        DataPool<GFXComputePipelineStateObject, SHIP_MAX_COMPUTE_PIPELINE_STATE_OBJECTS> m_ComputePipelineStateObjectPool;
        DataPool<GFXDescriptorSet, SHIP_MAX_DESCRIPTOR_SETS> m_DescriptorSetPool;

        // Ref counts for particular type of objects, of which there can be a limited number. Therefore,
        // with each creation, we can return an already allocated handle.
        shipUint32 m_SamplerHandleRefCounts[SHIP_MAX_SAMPLERS];
        shipUint32 m_GraphicsPipelineStateObjectHandleRefCounts[SHIP_MAX_GRAPHICS_PIPELINE_STATE_OBJECTS];
        shipUint32 m_ComputePipelineStateObjectHandleRefCounts[SHIP_MAX_COMPUTE_PIPELINE_STATE_OBJECTS];
    };
}