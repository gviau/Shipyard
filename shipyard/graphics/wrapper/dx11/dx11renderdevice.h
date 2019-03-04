#pragma once

#include <graphics/wrapper/renderdevice.h>

#include <graphics/wrapper/dx11/dx11_common.h>

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

        GFXVertexBuffer* CreateVertexBuffer(uint32_t numVertices, VertexFormatType vertexFormatType, bool dynamic, void* initialData);
        GFXIndexBuffer* CreateIndexBuffer(uint32_t numIndices, bool uses2BytesPerIndex, bool dynamic, void* initialData);
        GFXConstantBuffer* CreateConstantBuffer(uint32_t dataSizeInBytes, bool dynamic, void* initialData);

        GFXTexture2D* CreateTexture2D(
                uint32_t width,
                uint32_t height,
                GfxFormat pixelFormat,
                bool dynamic,
                void* initialData,
                bool generateMips,
                TextureUsage textureUsage = TextureUsage::TextureUsage_Default);

        GFXRenderTarget* CreateRenderTarget(GFXTexture2D** texturesToAttach, uint32_t numTexturesToAttach);
        GFXDepthStencilRenderTarget* CreateDepthStencilRenderTarget(GFXTexture2D& depthStencilTexture);

        GFXVertexShader* CreateVertexShader(void* shaderData, uint64_t shaderDataSize);
        GFXPixelShader* CreatePixelShader(void* shaderData, uint64_t shaderDataSize);

        GFXRootSignature* CreateRootSignature(const Array<RootSignatureParameterEntry>& rootSignatureParameters);
        GFXPipelineStateObject* CreatePipelineStateObject(const PipelineStateObjectCreationParameters& pipelineStateObjectCreationParameters);
        GFXDescriptorSet* CreateDescriptorSet(DescriptorSetType descriptorSetType, const RootSignature& rootSignature);

        IDXGISwapChain* CreateSwapchain(uint32_t width, uint32_t height, GfxFormat format, HWND hWnd);
        void CreateDepthStencilBuffer(uint32_t width, uint32_t height, ID3D11Texture2D*& depthStencilTexture, ID3D11DepthStencilView*& depthStencilTextureView);

        ID3D11Device* GetDevice() const { return m_Device; }
        ID3D11DeviceContext* GetImmediateDeviceContext() const { return m_ImmediateDeviceContext; }

    private:
        ID3D11Device* m_Device;
        ID3D11DeviceContext* m_ImmediateDeviceContext;
    };
}