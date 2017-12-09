#include <common/wrapper/dx11/dx11renderdevice.h>

#include <common/wrapper/dx11/dx11buffer.h>
#include <common/wrapper/dx11/dx11shader.h>

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

GFXIndexBuffer* DX11RenderDevice::CreateIndexBuffer(uint32_t numIndices, uint32_t indexSizeInBytes, bool dynamic, void* initialData)
{
    return MemAlloc(GFXIndexBuffer)(*m_Device, *m_ImmediateDeviceContext, numIndices, indexSizeInBytes, dynamic, initialData);
}

GFXConstantBuffer* DX11RenderDevice::CreateConstantBuffer(uint32_t dataSizeInBytes, bool dynamic, void* initialData)
{
    return MemAlloc(GFXConstantBuffer)(*m_Device, *m_ImmediateDeviceContext, dataSizeInBytes, dynamic, initialData);
}

GFXVertexShader* DX11RenderDevice::CreateVertexShader(const String& source)
{
    return MemAlloc(GFXVertexShader)(*m_Device, source);
}

GFXPixelShader* DX11RenderDevice::CreatePixelShader(const String& source)
{
    return MemAlloc(GFXPixelShader)(*m_Device, source);
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

}