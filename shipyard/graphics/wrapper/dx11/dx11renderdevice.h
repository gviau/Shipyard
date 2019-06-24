#pragma once

#include <graphics/wrapper/renderdevice.h>

#include <graphics/wrapper/dx11/dx11_common.h>

#include <graphics/wrapper/dx11/dx11buffer.h>
#include <graphics/wrapper/dx11/dx11descriptorset.h>
#include <graphics/wrapper/dx11/dx11pipelinestateobject.h>
#include <graphics/wrapper/dx11/dx11rendertarget.h>
#include <graphics/wrapper/dx11/dx11rootsignature.h>
#include <graphics/wrapper/dx11/dx11shader.h>
#include <graphics/wrapper/dx11/dx11texture.h>

#include <system/datapool.h>

#include <windows.h>

struct ID3D11Device;
struct ID3D11DeviceContext;
struct IDXGISwapChain;

namespace Shipyard
{
    class SHIPYARD_API DX11RenderDevice : public BaseRenderDevice
    {
    public:
        DX11RenderDevice();
        ~DX11RenderDevice();

        bool Create();
        void Destroy();

        GFXVertexBufferHandle CreateVertexBuffer(uint32_t numVertices, VertexFormatType vertexFormatType, bool dynamic, void* initialData);
        void DestroyVertexBuffer(GFXVertexBufferHandle gfxVertexBufferHandle);
        GFXVertexBuffer& GetVertexBuffer(GFXVertexBufferHandle gfxVertexBufferHandle);
        const GFXVertexBuffer& GetVertexBuffer(GFXVertexBufferHandle gfxVertexBufferHandle) const;

        GFXIndexBufferHandle CreateIndexBuffer(uint32_t numIndices, bool uses2BytesPerIndex, bool dynamic, void* initialData);
        void DestroyIndexBuffer(GFXIndexBufferHandle gfxIndexBufferHandle);
        GFXIndexBuffer& GetIndexBuffer(GFXIndexBufferHandle gfxIndexBufferHandle);
        const GFXIndexBuffer& GetIndexBuffer(GFXIndexBufferHandle gfxIndexBufferHandle) const;

        GFXConstantBufferHandle CreateConstantBuffer(uint32_t dataSizeInBytes, bool dynamic, void* initialData);
        void DestroyConstantBuffer(GFXConstantBufferHandle gfxConstantBufferHandle);
        GFXConstantBuffer& GetConstantBuffer(GFXConstantBufferHandle gfxConstantBufferHandle);
        const GFXConstantBuffer& GetConstantBuffer(GFXConstantBufferHandle gfxConstantBufferHandle) const;

        GFXByteBufferHandle CreateByteBuffer(ByteBuffer::ByteBufferCreationFlags byteBufferCreationFlags, uint32_t dataSizeInBytes, bool dynamic, void* initialData);
        void DestroyByteBuffer(GFXByteBufferHandle gfxByteBufferHandle);
        GFXByteBuffer& GetByteBuffer(GFXByteBufferHandle gfxByteBufferHandle);
        const GFXByteBuffer& GetByteBuffer(GFXByteBufferHandle gfxByteBufferHandle) const;

        GFXTexture2DHandle CreateTexture2D(
                uint32_t width,
                uint32_t height,
                GfxFormat pixelFormat,
                bool dynamic,
                void* initialData,
                bool generateMips,
                TextureUsage textureUsage = TextureUsage::TextureUsage_Default);
        void DestroyTexture2D(GFXTexture2DHandle gfxTexture2dHandle);
        GFXTexture2D& GetTexture2D(GFXTexture2DHandle gfxTexture2dHandle);
        const GFXTexture2D& GetTexture2D(GFXTexture2DHandle gfxTexture2dHandle) const;

        GFXRenderTargetHandle CreateRenderTarget(GFXTexture2DHandle* texturesToAttach, uint32_t numTexturesToAttach);
        void DestroyRenderTarget(GFXRenderTargetHandle gfxRenderTargetHandle);
        GFXRenderTarget& GetRenderTarget(GFXRenderTargetHandle gfxRenderTargetHandle);
        const GFXRenderTarget& GetRenderTarget(GFXRenderTargetHandle gfxRenderTargetHandle) const;

        GFXDepthStencilRenderTargetHandle CreateDepthStencilRenderTarget(GFXTexture2DHandle depthStencilTexture);
        void DestroyDepthStencilRenderTarget(GFXDepthStencilRenderTargetHandle gfxDepthStencilRenderTargetHandle);
        GFXDepthStencilRenderTarget& GetDepthStencilRenderTarget(GFXDepthStencilRenderTargetHandle gfxDepthStencilRenderTargetHandle);
        const GFXDepthStencilRenderTarget& GetDepthStencilRenderTarget(GFXDepthStencilRenderTargetHandle gfxDepthStencilRenderTargetHandle) const;

        GFXVertexShaderHandle CreateVertexShader(void* shaderData, uint64_t shaderDataSize);
        void DestroyVertexShader(GFXVertexShaderHandle gfxVertexShaderHandle);
        GFXVertexShader& GetVertexShader(GFXVertexShaderHandle gfxVertexShaderHandle);
        const GFXVertexShader& GetVertexShader(GFXVertexShaderHandle gfxVertexShaderHandle) const;

        GFXPixelShaderHandle CreatePixelShader(void* shaderData, uint64_t shaderDataSize);
        void DestroyPixelShader(GFXPixelShaderHandle gfxPixelShaderHandle);
        GFXPixelShader& GetPixelShader(GFXPixelShaderHandle gfxPixelShaderHandle);
        const GFXPixelShader& GetPixelShader(GFXPixelShaderHandle gfxPixelShaderHandle) const;

        GFXRootSignatureHandle CreateRootSignature(const Array<RootSignatureParameterEntry>& rootSignatureParameters);
        void DestroyRootSignature(GFXRootSignatureHandle gfxRootSignatureHandle);
        GFXRootSignature& GetRootSignature(GFXRootSignatureHandle gfxRootSignatureHandle);
        const GFXRootSignature& GetRootSignature(GFXRootSignatureHandle gfxRootSignatureHandle) const;

        GFXPipelineStateObjectHandle CreatePipelineStateObject(const PipelineStateObjectCreationParameters& pipelineStateObjectCreationParameters);
        void DestroyPipelineStateObject(GFXPipelineStateObjectHandle gfxPipelineStateObjectHandle);
        GFXPipelineStateObject& GetPipelineStateObject(GFXPipelineStateObjectHandle gfxPipelineStateObjectHandle);
        const GFXPipelineStateObject& GetPipelineStateObject(GFXPipelineStateObjectHandle gfxPipelineStateObjectHandle) const;

        GFXDescriptorSetHandle CreateDescriptorSet(DescriptorSetType descriptorSetType, const RootSignature& rootSignature);
        void DestroyDescriptorSet(GFXDescriptorSetHandle gfxDescriptorSetHandle);
        GFXDescriptorSet& GetDescriptorSet(GFXDescriptorSetHandle gfxDescriptorSetHandle);
        const GFXDescriptorSet& GetDescriptorSet(GFXDescriptorSetHandle gfxDescriptorSetHandle) const;

        IDXGISwapChain* CreateSwapchain(uint32_t width, uint32_t height, GfxFormat format, HWND hWnd, GFXTexture2DHandle& swapChainTextureHandle);

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
        DataPool<GFXRenderTarget, SHIP_MAX_RENDER_TARGETS> m_RenderTargetPool;
        DataPool<GFXDepthStencilRenderTarget, SHIP_MAX_DEPTH_STENCIL_RENDER_TARGETS> m_DepthStencilRenderTargetPool;
        DataPool<GFXVertexShader, SHIP_MAX_VERTEX_SHADERS> m_VertexShaderPool;
        DataPool<GFXPixelShader, SHIP_MAX_PIXEL_SHADERS> m_PixelShaderPool;
        DataPool<GFXRootSignature, SHIP_MAX_ROOT_SIGNATURES> m_RootSignaturePool;
        DataPool<GFXPipelineStateObject, SHIP_MAX_RENDER_PIPELINE_STATE_OBJECTS> m_PipelineStateObjectPool;
        DataPool<GFXDescriptorSet, SHIP_MAX_DESCRIPTOR_SETS> m_DescriptorSetPool;
    };
}