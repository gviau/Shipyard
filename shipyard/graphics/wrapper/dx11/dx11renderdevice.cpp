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
    : m_Device(nullptr)
    , m_ImmediateDeviceContext(nullptr)
{
}

DX11RenderDevice::~DX11RenderDevice()
{
    Destroy();
}

bool DX11RenderDevice::Create()
{
    UINT flags = D3D11_CREATE_DEVICE_DEBUG;
    HRESULT hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, NULL, flags, nullptr, 0, D3D11_SDK_VERSION, &m_Device, nullptr, &m_ImmediateDeviceContext);
    if (FAILED(hr))
    {
        SHIP_LOG_ERROR("DX11RenderDevice::DX11RenderDevice() --> Couldn't create D3D11 device.");
        return false;
    }

    return true;
}

void DX11RenderDevice::Destroy()
{
    uint32_t indexToFree = 0;
    if (m_VertexBufferPool.GetFirstAllocatedIndex(&indexToFree))
    {
        do
        {
            m_VertexBufferPool.GetItem(indexToFree).Destroy();
            m_VertexBufferPool.ReleaseItem(indexToFree);
        } while (m_VertexBufferPool.GetNextAllocatedIndex(indexToFree, &indexToFree));
    }

    if (m_IndexBufferPool.GetFirstAllocatedIndex(&indexToFree))
    {
        do
        {
            m_IndexBufferPool.GetItem(indexToFree).Destroy();
            m_IndexBufferPool.ReleaseItem(indexToFree);
        } while (m_IndexBufferPool.GetNextAllocatedIndex(indexToFree, &indexToFree));
    }

    if (m_ConstantBufferPool.GetFirstAllocatedIndex(&indexToFree))
    {
        do
        {
            m_ConstantBufferPool.GetItem(indexToFree).Destroy();
            m_ConstantBufferPool.ReleaseItem(indexToFree);
        } while (m_ConstantBufferPool.GetNextAllocatedIndex(indexToFree, &indexToFree));
    }

    if (m_Texture2dPool.GetFirstAllocatedIndex(&indexToFree))
    {
        do
        {
            m_Texture2dPool.GetItem(indexToFree).Destroy();
            m_Texture2dPool.ReleaseItem(indexToFree);
        } while (m_Texture2dPool.GetNextAllocatedIndex(indexToFree, &indexToFree));
    }

    if (m_RenderTargetPool.GetFirstAllocatedIndex(&indexToFree))
    {
        do
        {
            m_RenderTargetPool.GetItem(indexToFree).Destroy();
            m_RenderTargetPool.ReleaseItem(indexToFree);
        } while (m_RenderTargetPool.GetNextAllocatedIndex(indexToFree, &indexToFree));
    }

    if (m_DepthStencilRenderTargetPool.GetFirstAllocatedIndex(&indexToFree))
    {
        do
        {
            m_DepthStencilRenderTargetPool.GetItem(indexToFree).Destroy();
            m_DepthStencilRenderTargetPool.ReleaseItem(indexToFree);
        } while (m_DepthStencilRenderTargetPool.GetNextAllocatedIndex(indexToFree, &indexToFree));
    }

    if (m_VertexShaderPool.GetFirstAllocatedIndex(&indexToFree))
    {
        do
        {
            m_VertexShaderPool.GetItem(indexToFree).Destroy();
            m_VertexShaderPool.ReleaseItem(indexToFree);
        } while (m_VertexShaderPool.GetNextAllocatedIndex(indexToFree, &indexToFree));
    }

    if (m_PixelShaderPool.GetFirstAllocatedIndex(&indexToFree))
    {
        do
        {
            m_PixelShaderPool.GetItem(indexToFree).Destroy();
            m_PixelShaderPool.ReleaseItem(indexToFree);
        } while (m_PixelShaderPool.GetNextAllocatedIndex(indexToFree, &indexToFree));
    }

    if (m_RootSignaturePool.GetFirstAllocatedIndex(&indexToFree))
    {
        do
        {
            m_RootSignaturePool.GetItem(indexToFree).Destroy();
            m_RootSignaturePool.ReleaseItem(indexToFree);
        } while (m_RootSignaturePool.GetNextAllocatedIndex(indexToFree, &indexToFree));
    }

    if (m_PipelineStateObjectPool.GetFirstAllocatedIndex(&indexToFree))
    {
        do
        {
            m_PipelineStateObjectPool.GetItem(indexToFree).Destroy();
            m_PipelineStateObjectPool.ReleaseItem(indexToFree);
        } while (m_PipelineStateObjectPool.GetNextAllocatedIndex(indexToFree, &indexToFree));
    }

    if (m_DescriptorSetPool.GetFirstAllocatedIndex(&indexToFree))
    {
        do
        {
            m_DescriptorSetPool.GetItem(indexToFree).Destroy();
            m_DescriptorSetPool.ReleaseItem(indexToFree);
        } while (m_DescriptorSetPool.GetNextAllocatedIndex(indexToFree, &indexToFree));
    }

    if (m_Device != nullptr)
    {
        m_Device->Release();
    }

    if (m_ImmediateDeviceContext != nullptr)
    {
        m_ImmediateDeviceContext->Release();
    }
}

GFXVertexBufferHandle DX11RenderDevice::CreateVertexBuffer(uint32_t numVertices, VertexFormatType vertexFormatType, bool dynamic, void* initialData)
{
    GFXVertexBufferHandle gfxVertexBufferHandle;
    gfxVertexBufferHandle.handle = m_VertexBufferPool.GetNewItemIndex();

    GFXVertexBuffer& gfxVertexBuffer = m_VertexBufferPool.GetItem(gfxVertexBufferHandle.handle);
    bool isValid = gfxVertexBuffer.Create(*m_Device, *m_ImmediateDeviceContext, numVertices, vertexFormatType, dynamic, initialData);

    SHIP_ASSERT(isValid);
    
    return gfxVertexBufferHandle;
}

void DX11RenderDevice::DestroyVertexBuffer(GFXVertexBufferHandle gfxVertexBufferHandle)
{
    GFXVertexBuffer& gfxVertexBuffer = m_VertexBufferPool.GetItem(gfxVertexBufferHandle.handle);
    gfxVertexBuffer.Destroy();

    m_VertexBufferPool.ReleaseItem(gfxVertexBufferHandle.handle);
}

GFXVertexBuffer& DX11RenderDevice::GetVertexBuffer(GFXVertexBufferHandle gfxVertexBufferHandle)
{
    return m_VertexBufferPool.GetItem(gfxVertexBufferHandle.handle);
}

const GFXVertexBuffer& DX11RenderDevice::GetVertexBuffer(GFXVertexBufferHandle gfxVertexBufferHandle) const
{
    return m_VertexBufferPool.GetItem(gfxVertexBufferHandle.handle);
}

GFXIndexBufferHandle DX11RenderDevice::CreateIndexBuffer(uint32_t numIndices, bool uses2BytesPerIndex, bool dynamic, void* initialData)
{
    GFXIndexBufferHandle gfxIndexBufferHandle;
    gfxIndexBufferHandle.handle = m_IndexBufferPool.GetNewItemIndex();

    GFXIndexBuffer& gfxIndexBuffer = m_IndexBufferPool.GetItem(gfxIndexBufferHandle.handle);
    bool isValid = gfxIndexBuffer.Create(*m_Device, *m_ImmediateDeviceContext, numIndices, uses2BytesPerIndex, dynamic, initialData);

    SHIP_ASSERT(isValid);

    return gfxIndexBufferHandle;
}

void DX11RenderDevice::DestroyIndexBuffer(GFXIndexBufferHandle gfxIndexBufferHandle)
{
    GFXIndexBuffer& gfxIndexBuffer = m_IndexBufferPool.GetItem(gfxIndexBufferHandle.handle);
    gfxIndexBuffer.Destroy();

    m_IndexBufferPool.ReleaseItem(gfxIndexBufferHandle.handle);
}

GFXIndexBuffer& DX11RenderDevice::GetIndexBuffer(GFXIndexBufferHandle gfxIndexBufferHandle)
{
    return m_IndexBufferPool.GetItem(gfxIndexBufferHandle.handle);
}

const GFXIndexBuffer& DX11RenderDevice::GetIndexBuffer(GFXIndexBufferHandle gfxIndexBufferHandle) const
{
    return m_IndexBufferPool.GetItem(gfxIndexBufferHandle.handle);
}

GFXConstantBufferHandle DX11RenderDevice::CreateConstantBuffer(uint32_t dataSizeInBytes, bool dynamic, void* initialData)
{
    GFXConstantBufferHandle gfxConstantBufferHandle;
    gfxConstantBufferHandle.handle = m_ConstantBufferPool.GetNewItemIndex();

    GFXConstantBuffer& gfxConstantBuffer = m_ConstantBufferPool.GetItem(gfxConstantBufferHandle.handle);
    bool isValid = gfxConstantBuffer.Create(*m_Device, *m_ImmediateDeviceContext, dataSizeInBytes, dynamic, initialData);

    SHIP_ASSERT(isValid);

    return gfxConstantBufferHandle;
}

void DX11RenderDevice::DestroyConstantBuffer(GFXConstantBufferHandle gfxConstantBufferHandle)
{
    GFXConstantBuffer& gfxConstantBuffer = m_ConstantBufferPool.GetItem(gfxConstantBufferHandle.handle);
    gfxConstantBuffer.Destroy();

    m_ConstantBufferPool.ReleaseItem(gfxConstantBufferHandle.handle);
}

GFXConstantBuffer& DX11RenderDevice::GetConstantBuffer(GFXConstantBufferHandle gfxConstantBufferHandle)
{
    return m_ConstantBufferPool.GetItem(gfxConstantBufferHandle.handle);
}

const GFXConstantBuffer& DX11RenderDevice::GetConstantBuffer(GFXConstantBufferHandle gfxConstantBufferHandle) const
{
    return m_ConstantBufferPool.GetItem(gfxConstantBufferHandle.handle);
}

GFXTexture2DHandle DX11RenderDevice::CreateTexture2D(uint32_t width, uint32_t height, GfxFormat pixelFormat, bool dynamic, void* initialData, bool generateMips, TextureUsage textureUsage)
{
    GFXTexture2DHandle gfxTexture2dHandle;
    gfxTexture2dHandle.handle = m_Texture2dPool.GetNewItemIndex();

    GFXTexture2D& gfxTexture2d = m_Texture2dPool.GetItem(gfxTexture2dHandle.handle);
    bool isValid = gfxTexture2d.Create(*m_Device, width, height, pixelFormat, dynamic, initialData, generateMips, textureUsage);

    SHIP_ASSERT(isValid);

    return gfxTexture2dHandle;
}

void DX11RenderDevice::DestroyTexture2D(GFXTexture2DHandle gfxTexture2dHandle)
{
    GFXTexture2D& gfxTexture2d = m_Texture2dPool.GetItem(gfxTexture2dHandle.handle);
    gfxTexture2d.Destroy();

    m_Texture2dPool.ReleaseItem(gfxTexture2dHandle.handle);
}

GFXTexture2D& DX11RenderDevice::GetTexture2D(GFXTexture2DHandle gfxTexture2dHandle)
{
    return m_Texture2dPool.GetItem(gfxTexture2dHandle.handle);
}

const GFXTexture2D& DX11RenderDevice::GetTexture2D(GFXTexture2DHandle gfxTexture2dHandle) const
{
    return m_Texture2dPool.GetItem(gfxTexture2dHandle.handle);
}

GFXRenderTargetHandle DX11RenderDevice::CreateRenderTarget(GFXTexture2DHandle* texturesToAttach, uint32_t numTexturesToAttach)
{
    GFXRenderTargetHandle gfxRenderTargetHandle;
    gfxRenderTargetHandle.handle = m_RenderTargetPool.GetNewItemIndex();

    GFXRenderTarget& gfxRenderTarget = m_RenderTargetPool.GetItem(gfxRenderTargetHandle.handle);

    GFXTexture2D* textures[GfxConstants_MaxRenderTargetsBound];
    for (uint32_t i = 0; i < numTexturesToAttach; i++)
    {
        if (texturesToAttach[i].handle == InvalidGfxHandle)
        {
            textures[i] = nullptr;
        }
        else
        {
            textures[i] = &m_Texture2dPool.GetItem(texturesToAttach[i].handle);
        }
    }

    bool isValid = gfxRenderTarget.Create(*m_Device, textures, numTexturesToAttach);

    SHIP_ASSERT(isValid);

    return gfxRenderTargetHandle;
}

void DX11RenderDevice::DestroyRenderTarget(GFXRenderTargetHandle gfxRenderTargetHandle)
{
    GFXRenderTarget& gfxRenderTarget = m_RenderTargetPool.GetItem(gfxRenderTargetHandle.handle);
    gfxRenderTarget.Destroy();

    m_RenderTargetPool.ReleaseItem(gfxRenderTargetHandle.handle);
}

GFXRenderTarget& DX11RenderDevice::GetRenderTarget(GFXRenderTargetHandle gfxRenderTargetHandle)
{
    return m_RenderTargetPool.GetItem(gfxRenderTargetHandle.handle);
}

const GFXRenderTarget& DX11RenderDevice::GetRenderTarget(GFXRenderTargetHandle gfxRenderTargetHandle) const
{
    return m_RenderTargetPool.GetItem(gfxRenderTargetHandle.handle);
}

GFXDepthStencilRenderTargetHandle DX11RenderDevice::CreateDepthStencilRenderTarget(GFXTexture2DHandle depthStencilTexture)
{
    SHIP_ASSERT(depthStencilTexture.handle != InvalidGfxHandle);

    GFXDepthStencilRenderTargetHandle gfxDepthStencilRenderTargetHandle;
    gfxDepthStencilRenderTargetHandle.handle = m_DepthStencilRenderTargetPool.GetNewItemIndex();

    GFXDepthStencilRenderTarget& gfxDepthStencilRenderTarget = m_DepthStencilRenderTargetPool.GetItem(gfxDepthStencilRenderTargetHandle.handle);

    GFXTexture2D& gfxDepthStencilTexture = m_Texture2dPool.GetItem(depthStencilTexture.handle);

    bool isValid = gfxDepthStencilRenderTarget.Create(*m_Device, gfxDepthStencilTexture);

    SHIP_ASSERT(isValid);

    return gfxDepthStencilRenderTargetHandle;
}

void DX11RenderDevice::DestroyDepthStencilRenderTarget(GFXDepthStencilRenderTargetHandle gfxDepthStencilRenderTargetHandle)
{
    GFXDepthStencilRenderTarget& gfxDepthStencilRenderTarget = m_DepthStencilRenderTargetPool.GetItem(gfxDepthStencilRenderTargetHandle.handle);
    gfxDepthStencilRenderTarget.Destroy();

    m_DepthStencilRenderTargetPool.ReleaseItem(gfxDepthStencilRenderTargetHandle.handle);
}

GFXDepthStencilRenderTarget& DX11RenderDevice::GetDepthStencilRenderTarget(GFXDepthStencilRenderTargetHandle gfxDepthStencilRenderTargetHandle)
{
    return m_DepthStencilRenderTargetPool.GetItem(gfxDepthStencilRenderTargetHandle.handle);
}

const GFXDepthStencilRenderTarget& DX11RenderDevice::GetDepthStencilRenderTarget(GFXDepthStencilRenderTargetHandle gfxDepthStencilRenderTargetHandle) const
{
    return m_DepthStencilRenderTargetPool.GetItem(gfxDepthStencilRenderTargetHandle.handle);
}

GFXVertexShaderHandle DX11RenderDevice::CreateVertexShader(void* shaderData, uint64_t shaderDataSize)
{
    GFXVertexShaderHandle gfxVertexShaderHandle;
    gfxVertexShaderHandle.handle = m_VertexShaderPool.GetNewItemIndex();

    GFXVertexShader& gfxVertexShader = m_VertexShaderPool.GetItem(gfxVertexShaderHandle.handle);
    bool isValid = gfxVertexShader.Create(*m_Device, shaderData, shaderDataSize);

    SHIP_ASSERT(isValid);

    return gfxVertexShaderHandle;
}

void DX11RenderDevice::DestroyVertexShader(GFXVertexShaderHandle gfxVertexShaderHandle)
{
    GFXVertexShader& gfxVertexShader = m_VertexShaderPool.GetItem(gfxVertexShaderHandle.handle);
    gfxVertexShader.Destroy();

    m_VertexShaderPool.ReleaseItem(gfxVertexShaderHandle.handle);
}

GFXVertexShader& DX11RenderDevice::GetVertexShader(GFXVertexShaderHandle gfxVertexShaderHandle)
{
    return m_VertexShaderPool.GetItem(gfxVertexShaderHandle.handle);
}

const GFXVertexShader& DX11RenderDevice::GetVertexShader(GFXVertexShaderHandle gfxVertexShaderHandle) const
{
    return m_VertexShaderPool.GetItem(gfxVertexShaderHandle.handle);
}

GFXPixelShaderHandle DX11RenderDevice::CreatePixelShader(void* shaderData, uint64_t shaderDataSize)
{
    GFXPixelShaderHandle gfxPixelShaderHandle;
    gfxPixelShaderHandle.handle = m_PixelShaderPool.GetNewItemIndex();

    GFXPixelShader& gfxPixelShader = m_PixelShaderPool.GetItem(gfxPixelShaderHandle.handle);
    bool isValid = gfxPixelShader.Create(*m_Device, shaderData, shaderDataSize);

    SHIP_ASSERT(isValid);

    return gfxPixelShaderHandle;
}

void DX11RenderDevice::DestroyPixelShader(GFXPixelShaderHandle gfxPixelShaderHandle)
{
    GFXPixelShader& gfxPixelShader = m_PixelShaderPool.GetItem(gfxPixelShaderHandle.handle);
    gfxPixelShader.Destroy();

    m_PixelShaderPool.ReleaseItem(gfxPixelShaderHandle.handle);
}

GFXPixelShader& DX11RenderDevice::GetPixelShader(GFXPixelShaderHandle gfxPixelShaderHandle)
{
    return m_PixelShaderPool.GetItem(gfxPixelShaderHandle.handle);
}

const GFXPixelShader& DX11RenderDevice::GetPixelShader(GFXPixelShaderHandle gfxPixelShaderHandle) const
{
    return m_PixelShaderPool.GetItem(gfxPixelShaderHandle.handle);
}

GFXRootSignatureHandle DX11RenderDevice::CreateRootSignature(const Array<RootSignatureParameterEntry>& rootSignatureParameters)
{
    GFXRootSignatureHandle gfxRootSignatureHandle;
    gfxRootSignatureHandle.handle = m_RootSignaturePool.GetNewItemIndex();

    GFXRootSignature& gfxRootSignature = m_RootSignaturePool.GetItem(gfxRootSignatureHandle.handle);
    bool isValid = gfxRootSignature.Create(rootSignatureParameters);

    SHIP_ASSERT(isValid);

    return gfxRootSignatureHandle;
}

void DX11RenderDevice::DestroyRootSignature(GFXRootSignatureHandle gfxRootSignatureHandle)
{
    GFXRootSignature& gfxRootSignature = m_RootSignaturePool.GetItem(gfxRootSignatureHandle.handle);
    gfxRootSignature.Destroy();

    m_RootSignaturePool.ReleaseItem(gfxRootSignatureHandle.handle);
}

GFXRootSignature& DX11RenderDevice::GetRootSignature(GFXRootSignatureHandle gfxRootSignatureHandle)
{
    return m_RootSignaturePool.GetItem(gfxRootSignatureHandle.handle);
}

const GFXRootSignature& DX11RenderDevice::GetRootSignature(GFXRootSignatureHandle gfxRootSignatureHandle) const
{
    return m_RootSignaturePool.GetItem(gfxRootSignatureHandle.handle);
}

GFXPipelineStateObjectHandle DX11RenderDevice::CreatePipelineStateObject(const PipelineStateObjectCreationParameters& pipelineStateObjectCreationParameters)
{
    GFXPipelineStateObjectHandle gfxPipelineStateObjectHandle;
    gfxPipelineStateObjectHandle.handle = m_PipelineStateObjectPool.GetNewItemIndex();

    GFXPipelineStateObject& gfxPipelineStateObject = m_PipelineStateObjectPool.GetItem(gfxPipelineStateObjectHandle.handle);
    bool isValid = gfxPipelineStateObject.Create(pipelineStateObjectCreationParameters);

    SHIP_ASSERT(isValid);

    return gfxPipelineStateObjectHandle;
}

void DX11RenderDevice::DestroyPipelineStateObject(GFXPipelineStateObjectHandle gfxPipelineStateObjectHandle)
{
    GFXPipelineStateObject& gfxPipelineStateObject = m_PipelineStateObjectPool.GetItem(gfxPipelineStateObjectHandle.handle);
    gfxPipelineStateObject.Destroy();

    m_PipelineStateObjectPool.ReleaseItem(gfxPipelineStateObjectHandle.handle);
}

GFXPipelineStateObject& DX11RenderDevice::GetPipelineStateObject(GFXPipelineStateObjectHandle gfxPipelineStateObjectHandle)
{
    return m_PipelineStateObjectPool.GetItem(gfxPipelineStateObjectHandle.handle);
}

const GFXPipelineStateObject& DX11RenderDevice::GetPipelineStateObject(GFXPipelineStateObjectHandle gfxPipelineStateObjectHandle) const
{
    return m_PipelineStateObjectPool.GetItem(gfxPipelineStateObjectHandle.handle);
}

GFXDescriptorSetHandle DX11RenderDevice::CreateDescriptorSet(DescriptorSetType descriptorSetType, const RootSignature& rootSignature)
{
    GFXDescriptorSetHandle gfxDescriptorSetHandle;
    gfxDescriptorSetHandle.handle = m_DescriptorSetPool.GetNewItemIndex();

    GFXDescriptorSet& gfxDescriptorSet = m_DescriptorSetPool.GetItem(gfxDescriptorSetHandle.handle);
    bool isValid = gfxDescriptorSet.Create(descriptorSetType, rootSignature);

    SHIP_ASSERT(isValid);

    return gfxDescriptorSetHandle;
}

void DX11RenderDevice::DestroyDescriptorSet(GFXDescriptorSetHandle gfxDescriptorSetHandle)
{
    GFXDescriptorSet& gfxDescriptorSet = m_DescriptorSetPool.GetItem(gfxDescriptorSetHandle.handle);
    gfxDescriptorSet.Destroy();

    m_DescriptorSetPool.ReleaseItem(gfxDescriptorSetHandle.handle);
}

GFXDescriptorSet& DX11RenderDevice::GetDescriptorSet(GFXDescriptorSetHandle gfxDescriptorSetHandle)
{
    return m_DescriptorSetPool.GetItem(gfxDescriptorSetHandle.handle);
}

const GFXDescriptorSet& DX11RenderDevice::GetDescriptorSet(GFXDescriptorSetHandle gfxDescriptorSetHandle) const
{
    return m_DescriptorSetPool.GetItem(gfxDescriptorSetHandle.handle);
}

IDXGISwapChain* DX11RenderDevice::CreateSwapchain(uint32_t width, uint32_t height, GfxFormat format, HWND hWnd, GFXTexture2DHandle& swapChainTextureHandle)
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

    ID3D11Texture2D* backBufferTexture = nullptr;
    hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBufferTexture);
    if (FAILED(hr))
    {
        swapChain->Release();

        SHIP_LOG_ERROR("DX11RenderDevice::CreateSwapchain() --> Couldn't create view surface.");
        return nullptr;
    }

    swapChainTextureHandle.handle = m_Texture2dPool.GetNewItemIndex();

    GFXTexture2D& swapChainTexture = m_Texture2dPool.GetItem(swapChainTextureHandle.handle);

    swapChainTexture = DX11Texture2D(*m_Device, *backBufferTexture, format);

    backBufferTexture->Release();

    return swapChain;
}

}