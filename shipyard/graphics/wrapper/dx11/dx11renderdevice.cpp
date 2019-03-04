#include <graphics/wrapper/dx11/dx11renderdevice.h>

#include <graphics/wrapper/dx11/dx11buffer.h>
#include <graphics/wrapper/dx11/dx11descriptorset.h>
#include <graphics/wrapper/dx11/dx11pipelinestateobject.h>
#include <graphics/wrapper/dx11/dx11rendertarget.h>
#include <graphics/wrapper/dx11/dx11rootsignature.h>
#include <graphics/wrapper/dx11/dx11shader.h>
#include <graphics/wrapper/dx11/dx11texture.h>

#include <system/logger.h>
#include <system/memory.h>

#pragma warning( disable : 4005 )

#include <d3d11.h>

#pragma warning( default : 4005 )

namespace Shipyard
{;

DX11RenderDevice::DX11RenderDevice()
{
    UINT flags = D3D11_CREATE_DEVICE_DEBUG;
    HRESULT hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, NULL, flags, nullptr, 0, D3D11_SDK_VERSION, &m_Device, nullptr, &m_ImmediateDeviceContext);
    if (FAILED(hr))
    {
        SHIP_LOG_ERROR("DX11RenderDevice::DX11RenderDevice() --> Couldn't create D3D11 device.");
    }
}

DX11RenderDevice::~DX11RenderDevice()
{
    if (m_Device != nullptr)
    {
        m_Device->Release();
    }

    if (m_ImmediateDeviceContext != nullptr)
    {
        m_ImmediateDeviceContext->Release();
    }
}

GFXVertexBuffer* DX11RenderDevice::CreateVertexBuffer(uint32_t numVertices, VertexFormatType vertexFormatType, bool dynamic, void* initialData)
{
    return MemAlloc(GFXVertexBuffer)(*m_Device, *m_ImmediateDeviceContext, numVertices, vertexFormatType, dynamic, initialData);
}

GFXIndexBuffer* DX11RenderDevice::CreateIndexBuffer(uint32_t numIndices, bool uses2BytesPerIndex, bool dynamic, void* initialData)
{
    return MemAlloc(GFXIndexBuffer)(*m_Device, *m_ImmediateDeviceContext, numIndices, uses2BytesPerIndex, dynamic, initialData);
}

GFXConstantBuffer* DX11RenderDevice::CreateConstantBuffer(uint32_t dataSizeInBytes, bool dynamic, void* initialData)
{
    return MemAlloc(GFXConstantBuffer)(*m_Device, *m_ImmediateDeviceContext, dataSizeInBytes, dynamic, initialData);
}

GFXTexture2D* DX11RenderDevice::CreateTexture2D(uint32_t width, uint32_t height, GfxFormat pixelFormat, bool dynamic, void* initialData, bool generateMips, TextureUsage textureUsage)
{
    return MemAlloc(GFXTexture2D)(*m_Device, width, height, pixelFormat, dynamic, initialData, generateMips, textureUsage);
}

GFXRenderTarget* DX11RenderDevice::CreateRenderTarget(GFXTexture2D** texturesToAttach, uint32_t numTexturesToAttach)
{
    return MemAlloc(GFXRenderTarget)(*m_Device, texturesToAttach, numTexturesToAttach);
}

GFXDepthStencilRenderTarget* DX11RenderDevice::CreateDepthStencilRenderTarget(GFXTexture2D& depthStencilTexture)
{
    return MemAlloc(GFXDepthStencilRenderTarget)(*m_Device, depthStencilTexture);
}

GFXVertexShader* DX11RenderDevice::CreateVertexShader(void* shaderData, uint64_t shaderDataSize)
{
    return MemAlloc(GFXVertexShader)(*m_Device, shaderData, shaderDataSize);
}

GFXPixelShader* DX11RenderDevice::CreatePixelShader(void* shaderData, uint64_t shaderDataSize)
{
    return MemAlloc(GFXPixelShader)(*m_Device, shaderData, shaderDataSize);
}

GFXRootSignature* DX11RenderDevice::CreateRootSignature(const Array<RootSignatureParameterEntry>& rootSignatureParameters)
{
    return MemAlloc(GFXRootSignature)(rootSignatureParameters);
}

GFXPipelineStateObject* DX11RenderDevice::CreatePipelineStateObject(const PipelineStateObjectCreationParameters& pipelineStateObjectCreationParameters)
{
    return MemAlloc(GFXPipelineStateObject)(pipelineStateObjectCreationParameters);
}

GFXDescriptorSet* DX11RenderDevice::CreateDescriptorSet(DescriptorSetType descriptorSetType, const RootSignature& rootSignature)
{
    return MemAlloc(GFXDescriptorSet)(descriptorSetType, rootSignature);
}

IDXGISwapChain* DX11RenderDevice::CreateSwapchain(uint32_t width, uint32_t height, GfxFormat format, HWND hWnd)
{
    DXGI_FORMAT backBufferFormat = ConvertShipyardFormatToDX11(format);

    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
    swapChainDesc.BufferCount = 1;
    swapChainDesc.BufferDesc.Format = backBufferFormat;
    swapChainDesc.BufferDesc.Height = height;
    swapChainDesc.BufferDesc.Width = width;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swapChainDesc.OutputWindow = hWnd;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.Windowed = true;

    IDXGIDevice1* dxgiDevice = nullptr;
    m_Device->QueryInterface(__uuidof(IDXGIDevice1), (void**)&dxgiDevice);

    IDXGIAdapter* dxgiAdapter = nullptr;
    dxgiDevice->GetAdapter(&dxgiAdapter);

    IDXGIFactory1* dxgiFactory = nullptr;
    dxgiAdapter->GetParent(__uuidof(IDXGIFactory1), (void**)&dxgiFactory);

    IDXGISwapChain* swapChain = nullptr;
    HRESULT hr = dxgiFactory->CreateSwapChain(m_Device, &swapChainDesc, &swapChain);

    dxgiDevice->Release();
    dxgiAdapter->Release();
    dxgiFactory->Release();

    if (FAILED(hr))
    {
        SHIP_LOG_ERROR("DX11RenderDevice::CreateSwapchain() --> Couldn't create swapchain.");
        return nullptr;
    }

    return swapChain;
}

}