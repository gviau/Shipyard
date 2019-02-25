#include <common/wrapper/dx11/dx11renderdevice.h>

#include <common/wrapper/dx11/dx11buffer.h>
#include <common/wrapper/dx11/dx11descriptorset.h>
#include <common/wrapper/dx11/dx11pipelinestateobject.h>
#include <common/wrapper/dx11/dx11rendertarget.h>
#include <common/wrapper/dx11/dx11rootsignature.h>
#include <common/wrapper/dx11/dx11shader.h>
#include <common/wrapper/dx11/dx11texture.h>

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
        MessageBox(NULL, "DX11RenderDevice::DX11RenderDevice() failed", "DX11 error", MB_OK);
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
        MessageBox(NULL, "IDXGIFactor1::CreateSwapChain failed", "DX11 error", MB_OK);
        return nullptr;
    }

    return swapChain;
}

void DX11RenderDevice::CreateDepthStencilBuffer(uint32_t width, uint32_t height, ID3D11Texture2D*& depthStencilTexture, ID3D11DepthStencilView*& depthStencilTextureView)
{
    D3D11_TEXTURE2D_DESC depthStencilTextureDesc;
    depthStencilTextureDesc.Width = width;
    depthStencilTextureDesc.Height = height;
    depthStencilTextureDesc.MipLevels = 1;
    depthStencilTextureDesc.ArraySize = 1;
    depthStencilTextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilTextureDesc.SampleDesc.Count = 1;
    depthStencilTextureDesc.SampleDesc.Quality = 0;
    depthStencilTextureDesc.Usage = D3D11_USAGE_DEFAULT;
    depthStencilTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthStencilTextureDesc.CPUAccessFlags = 0;
    depthStencilTextureDesc.MiscFlags = 0;

    HRESULT hr = m_Device->CreateTexture2D(&depthStencilTextureDesc, nullptr, &depthStencilTexture);
    if (FAILED(hr))
    {
        MessageBox(NULL, "DepthStencilTexture creation failed", "DX11 error", MB_OK);
        return;
    }

    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
    depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDesc.Texture2D.MipSlice = 0;
    depthStencilViewDesc.Flags = 0;

    hr = m_Device->CreateDepthStencilView(depthStencilTexture, &depthStencilViewDesc, &depthStencilTextureView);
    if (FAILED(hr))
    {
        MessageBox(NULL, "CreateDepthStencilView failed", "DX11 error", MB_OK);
        return;
    }
}

}