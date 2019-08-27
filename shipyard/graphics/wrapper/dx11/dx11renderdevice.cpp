#include <graphics/wrapper/dx11/dx11renderdevice.h>

#include <graphics/shader/shadervertexformatgenerator.h>

#include <graphics/wrapper/dx11/dx11buffer.h>
#include <graphics/wrapper/dx11/dx11descriptorset.h>
#include <graphics/wrapper/dx11/dx11pipelinestateobject.h>
#include <graphics/wrapper/dx11/dx11rendertarget.h>
#include <graphics/wrapper/dx11/dx11rootsignature.h>
#include <graphics/wrapper/dx11/dx11shader.h>
#include <graphics/wrapper/dx11/dx11texture.h>

#include <graphics/defaulttextures.h>

#include <system/logger.h>
#include <system/memory.h>

#pragma warning( disable : 4005 )

#include <d3d11.h>
#include <d3dcompiler.h>

#pragma warning( default : 4005 )

namespace Shipyard
{;

// Used to not have to constantly create new input layouts
ID3D11InputLayout* g_RegisteredInputLayouts[shipUint32(VertexFormatType::VertexFormatType_Count)];

ID3D11InputLayout* RegisterVertexFormatType(ID3D11Device* device, VertexFormatType vertexFormatType);

DX11RenderDevice::DX11RenderDevice()
    : m_Device(nullptr)
    , m_ImmediateDeviceContext(nullptr)
{
    for (shipUint32 i = 0; i < SHIP_MAX_SAMPLERS; i++)
    {
        m_SamplerHandleRefCounts[i] = 0;
    }

    for (shipUint32 i = 0; i < SHIP_MAX_RENDER_PIPELINE_STATE_OBJECTS; i++)
    {
        m_PipelineStateObjectHandleRefCounts[i] = 0;
    }
}

DX11RenderDevice::~DX11RenderDevice()
{
    Destroy();
}

shipBool DX11RenderDevice::Create()
{
    UINT flags = D3D11_CREATE_DEVICE_DEBUG;
    HRESULT hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, NULL, flags, nullptr, 0, D3D11_SDK_VERSION, &m_Device, nullptr, &m_ImmediateDeviceContext);
    if (FAILED(hr))
    {
        SHIP_LOG_ERROR("DX11RenderDevice::DX11RenderDevice() --> Couldn't create D3D11 device.");
        return false;
    }

    for (shipUint32 i = 0; i < shipUint32(VertexFormatType::VertexFormatType_Count); i++)
    {
        g_RegisteredInputLayouts[i] = RegisterVertexFormatType(m_Device, VertexFormatType(i));
    }

    if (!m_VertexBufferPool.Create())
    {
        SHIP_LOG_ERROR("DX11RenderDevice::DX11RenderDevice() --> Couldn't create VertexBuffer pool.");
        return false;
    }

    if (!m_IndexBufferPool.Create())
    {
        SHIP_LOG_ERROR("DX11RenderDevice::DX11RenderDevice() --> Couldn't create IndexBuffer pool.");
        return false;
    }

    if (!m_ConstantBufferPool.Create())
    {
        SHIP_LOG_ERROR("DX11RenderDevice::DX11RenderDevice() --> Couldn't create ConstantBuffer pool.");
        return false;
    }

    if (!m_ByteBufferPool.Create())
    {
        SHIP_LOG_ERROR("DX11RenderDevice::DX11RenderDevice() --> Couldn't create ByteBuffer pool.");
        return false;
    }

    if (!m_Texture2dPool.Create())
    {
        SHIP_LOG_ERROR("DX11RenderDevice::DX11RenderDevice() --> Couldn't create Texture2D pool.");
        return false;
    }

    if (!m_SamplerPool.Create())
    {
        SHIP_LOG_ERROR("DX11RenderDevice::DX11RenderDevice() --> Couldn't create Sampler pool.");
        return false;
    }

    if (!m_RenderTargetPool.Create())
    {
        SHIP_LOG_ERROR("DX11RenderDevice::DX11RenderDevice() --> Couldn't create RenderTarget pool.");
        return false;
    }

    if (!m_DepthStencilRenderTargetPool.Create())
    {
        SHIP_LOG_ERROR("DX11RenderDevice::DX11RenderDevice() --> Couldn't create DepthStencilRenderTarget pool.");
        return false;
    }

    if (!m_VertexShaderPool.Create())
    {
        SHIP_LOG_ERROR("DX11RenderDevice::DX11RenderDevice() --> Couldn't create VertexShader pool.");
        return false;
    }

    if (!m_PixelShaderPool.Create())
    {
        SHIP_LOG_ERROR("DX11RenderDevice::DX11RenderDevice() --> Couldn't create PixelShader pool.");
        return false;
    }

    if (!m_RootSignaturePool.Create())
    {
        SHIP_LOG_ERROR("DX11RenderDevice::DX11RenderDevice() --> Couldn't create RootSignature pool.");
        return false;
    }

    if (!m_PipelineStateObjectPool.Create())
    {
        SHIP_LOG_ERROR("DX11RenderDevice::DX11RenderDevice() --> Couldn't create PipelineStateObject pool.");
        return false;
    }

    if (!m_DescriptorSetPool.Create())
    {
        SHIP_LOG_ERROR("DX11RenderDevice::DX11RenderDevice() --> Couldn't create DescriptorSet pool.");
        return false;
    }

    DefaultTextures::CreateDefaultTextures(*this);

    return true;
}

void DX11RenderDevice::Destroy()
{
    DefaultTextures::DestroyDefaultTextures(*this);

    for (shipUint32 i = 0; i < shipUint32(VertexFormatType::VertexFormatType_Count); i++)
    {
        if (g_RegisteredInputLayouts[i] != nullptr)
        {
            g_RegisteredInputLayouts[i]->Release();
        }
    }

    shipUint32 indexToFree = 0;
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

    if (m_ByteBufferPool.GetFirstAllocatedIndex(&indexToFree))
    {
        do
        {
            m_ByteBufferPool.GetItem(indexToFree).Destroy();
            m_ByteBufferPool.ReleaseItem(indexToFree);
        } while (m_ByteBufferPool.GetNextAllocatedIndex(indexToFree, &indexToFree));
    }

    if (m_Texture2dPool.GetFirstAllocatedIndex(&indexToFree))
    {
        do
        {
            m_Texture2dPool.GetItem(indexToFree).Destroy();
            m_Texture2dPool.ReleaseItem(indexToFree);
        } while (m_Texture2dPool.GetNextAllocatedIndex(indexToFree, &indexToFree));
    }

    if (m_SamplerPool.GetFirstAllocatedIndex(&indexToFree))
    {
        do
        {
            m_SamplerPool.GetItem(indexToFree).Destroy();
            m_SamplerPool.ReleaseItem(indexToFree);
        } while (m_SamplerPool.GetNextAllocatedIndex(indexToFree, &indexToFree));
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

GFXVertexBufferHandle DX11RenderDevice::CreateVertexBuffer(shipUint32 numVertices, VertexFormatType vertexFormatType, shipBool dynamic, void* initialData)
{
    GFXVertexBufferHandle gfxVertexBufferHandle;
    gfxVertexBufferHandle.handle = m_VertexBufferPool.GetNewItemIndex();

    GFXVertexBuffer& gfxVertexBuffer = m_VertexBufferPool.GetItem(gfxVertexBufferHandle.handle);
    shipBool isValid = gfxVertexBuffer.Create(*m_Device, *m_ImmediateDeviceContext, numVertices, vertexFormatType, dynamic, initialData);

    SHIP_ASSERT(isValid);
    
    return gfxVertexBufferHandle;
}

void DX11RenderDevice::DestroyVertexBuffer(GFXVertexBufferHandle gfxVertexBufferHandle)
{
    GFXVertexBuffer& gfxVertexBuffer = m_VertexBufferPool.GetItem(gfxVertexBufferHandle.handle);
    gfxVertexBuffer.Destroy();

    m_VertexBufferPool.ReleaseItem(gfxVertexBufferHandle.handle);
    gfxVertexBufferHandle.handle = InvalidGfxHandle;
}

GFXVertexBuffer& DX11RenderDevice::GetVertexBuffer(GFXVertexBufferHandle gfxVertexBufferHandle)
{
    return m_VertexBufferPool.GetItem(gfxVertexBufferHandle.handle);
}

const GFXVertexBuffer& DX11RenderDevice::GetVertexBuffer(GFXVertexBufferHandle gfxVertexBufferHandle) const
{
    return m_VertexBufferPool.GetItem(gfxVertexBufferHandle.handle);
}

GFXIndexBufferHandle DX11RenderDevice::CreateIndexBuffer(shipUint32 numIndices, shipBool uses2BytesPerIndex, shipBool dynamic, void* initialData)
{
    GFXIndexBufferHandle gfxIndexBufferHandle;
    gfxIndexBufferHandle.handle = m_IndexBufferPool.GetNewItemIndex();

    GFXIndexBuffer& gfxIndexBuffer = m_IndexBufferPool.GetItem(gfxIndexBufferHandle.handle);
    shipBool isValid = gfxIndexBuffer.Create(*m_Device, *m_ImmediateDeviceContext, numIndices, uses2BytesPerIndex, dynamic, initialData);

    SHIP_ASSERT(isValid);

    return gfxIndexBufferHandle;
}

void DX11RenderDevice::DestroyIndexBuffer(GFXIndexBufferHandle gfxIndexBufferHandle)
{
    GFXIndexBuffer& gfxIndexBuffer = m_IndexBufferPool.GetItem(gfxIndexBufferHandle.handle);
    gfxIndexBuffer.Destroy();

    m_IndexBufferPool.ReleaseItem(gfxIndexBufferHandle.handle);
    gfxIndexBufferHandle.handle = InvalidGfxHandle;
}

GFXIndexBuffer& DX11RenderDevice::GetIndexBuffer(GFXIndexBufferHandle gfxIndexBufferHandle)
{
    return m_IndexBufferPool.GetItem(gfxIndexBufferHandle.handle);
}

const GFXIndexBuffer& DX11RenderDevice::GetIndexBuffer(GFXIndexBufferHandle gfxIndexBufferHandle) const
{
    return m_IndexBufferPool.GetItem(gfxIndexBufferHandle.handle);
}

GFXConstantBufferHandle DX11RenderDevice::CreateConstantBuffer(shipUint32 dataSizeInBytes, shipBool dynamic, void* initialData)
{
    GFXConstantBufferHandle gfxConstantBufferHandle;
    gfxConstantBufferHandle.handle = m_ConstantBufferPool.GetNewItemIndex();

    GFXConstantBuffer& gfxConstantBuffer = m_ConstantBufferPool.GetItem(gfxConstantBufferHandle.handle);
    shipBool isValid = gfxConstantBuffer.Create(*m_Device, *m_ImmediateDeviceContext, dataSizeInBytes, dynamic, initialData);

    SHIP_ASSERT(isValid);

    return gfxConstantBufferHandle;
}

void DX11RenderDevice::DestroyConstantBuffer(GFXConstantBufferHandle gfxConstantBufferHandle)
{
    GFXConstantBuffer& gfxConstantBuffer = m_ConstantBufferPool.GetItem(gfxConstantBufferHandle.handle);
    gfxConstantBuffer.Destroy();

    m_ConstantBufferPool.ReleaseItem(gfxConstantBufferHandle.handle);
    gfxConstantBufferHandle.handle = InvalidGfxHandle;
}

GFXConstantBuffer& DX11RenderDevice::GetConstantBuffer(GFXConstantBufferHandle gfxConstantBufferHandle)
{
    return m_ConstantBufferPool.GetItem(gfxConstantBufferHandle.handle);
}

const GFXConstantBuffer& DX11RenderDevice::GetConstantBuffer(GFXConstantBufferHandle gfxConstantBufferHandle) const
{
    return m_ConstantBufferPool.GetItem(gfxConstantBufferHandle.handle);
}

GFXByteBufferHandle DX11RenderDevice::CreateByteBuffer(ByteBuffer::ByteBufferCreationFlags byteBufferCreationFlags, shipUint32 dataSizeInBytes, shipBool dynamic, void* initialData)
{
    GFXByteBufferHandle gfxByteBufferHandle;
    gfxByteBufferHandle.handle = m_ByteBufferPool.GetNewItemIndex();

    GFXByteBuffer& gfxByteBuffer = m_ByteBufferPool.GetItem(gfxByteBufferHandle.handle);
    shipBool isValid = gfxByteBuffer.Create(*m_Device, *m_ImmediateDeviceContext, byteBufferCreationFlags, dataSizeInBytes, dynamic, initialData);

    SHIP_ASSERT(isValid);

    return gfxByteBufferHandle;
}

void DX11RenderDevice::DestroyByteBuffer(GFXByteBufferHandle gfxByteBufferHandle)
{
    GFXByteBuffer& gfxByteBuffer = m_ByteBufferPool.GetItem(gfxByteBufferHandle.handle);
    gfxByteBuffer.Destroy();

    m_ByteBufferPool.ReleaseItem(gfxByteBufferHandle.handle);
    gfxByteBufferHandle.handle = InvalidGfxHandle;
}

GFXByteBuffer& DX11RenderDevice::GetByteBuffer(GFXByteBufferHandle gfxByteBufferHandle)
{
    return m_ByteBufferPool.GetItem(gfxByteBufferHandle.handle);
}

const GFXByteBuffer& DX11RenderDevice::GetByteBuffer(GFXByteBufferHandle gfxByteBufferHandle) const
{
    return m_ByteBufferPool.GetItem(gfxByteBufferHandle.handle);
}

GFXByteBuffer* DX11RenderDevice::GetByteBufferPtr(GFXByteBufferHandle gfxByteBufferHandle)
{
    return m_ByteBufferPool.GetItemPtr(gfxByteBufferHandle.handle);
}

const GFXByteBuffer* DX11RenderDevice::GetByteBufferPtr(GFXByteBufferHandle gfxByteBufferHandle) const
{
    return m_ByteBufferPool.GetItemPtr(gfxByteBufferHandle.handle);
}

GFXTexture2DHandle DX11RenderDevice::CreateTexture2D(shipUint32 width, shipUint32 height, GfxFormat pixelFormat, shipBool dynamic, void* initialData, shipBool generateMips, TextureUsage textureUsage)
{
    GFXTexture2DHandle gfxTexture2dHandle;
    gfxTexture2dHandle.handle = m_Texture2dPool.GetNewItemIndex();

    GFXTexture2D& gfxTexture2d = m_Texture2dPool.GetItem(gfxTexture2dHandle.handle);
    shipBool isValid = gfxTexture2d.Create(*m_Device, width, height, pixelFormat, dynamic, initialData, generateMips, textureUsage);

    SHIP_ASSERT(isValid);

    return gfxTexture2dHandle;
}

void DX11RenderDevice::DestroyTexture2D(GFXTexture2DHandle gfxTexture2dHandle)
{
    GFXTexture2D& gfxTexture2d = m_Texture2dPool.GetItem(gfxTexture2dHandle.handle);
    gfxTexture2d.Destroy();

    m_Texture2dPool.ReleaseItem(gfxTexture2dHandle.handle);
    gfxTexture2dHandle.handle = InvalidGfxHandle;
}

GFXTexture2D& DX11RenderDevice::GetTexture2D(GFXTexture2DHandle gfxTexture2dHandle)
{
    return m_Texture2dPool.GetItem(gfxTexture2dHandle.handle);
}

const GFXTexture2D& DX11RenderDevice::GetTexture2D(GFXTexture2DHandle gfxTexture2dHandle) const
{
    return m_Texture2dPool.GetItem(gfxTexture2dHandle.handle);
}

GFXTexture2D* DX11RenderDevice::GetTexture2DPtr(GFXTexture2DHandle gfxTexture2dHandle)
{
    return m_Texture2dPool.GetItemPtr(gfxTexture2dHandle.handle);
}

const GFXTexture2D* DX11RenderDevice::GetTexture2DPtr(GFXTexture2DHandle gfxTexture2dHandle) const
{
    return m_Texture2dPool.GetItemPtr(gfxTexture2dHandle.handle);
}

shipUint32 GetSamplerIndexFromSamplerState(const DataPool<GFXSampler, SHIP_MAX_SAMPLERS>& samplerPool, const SamplerState& samplerState)
{
    shipUint32 allocatedSamplerIndex = 0;
    if (samplerPool.GetFirstAllocatedIndex(&allocatedSamplerIndex))
    {
        do
        {
            if (samplerPool.GetItem(allocatedSamplerIndex).GetSamplerState() == samplerState)
            {
                return allocatedSamplerIndex;
            }
        } while (samplerPool.GetNextAllocatedIndex(allocatedSamplerIndex, &allocatedSamplerIndex));
    }

    return InvalidGfxHandle;
}

GFXSamplerHandle DX11RenderDevice::CreateSampler(const SamplerState& samplerState)
{
    shipUint32 samplerIndex = GetSamplerIndexFromSamplerState(m_SamplerPool, samplerState);
    if (samplerIndex != InvalidGfxHandle)
    {
        m_SamplerHandleRefCounts[samplerIndex] += 1;

        return GFXSamplerHandle(samplerIndex);
    }

    GFXSamplerHandle gfxSamplerHandle;
    gfxSamplerHandle.handle = m_SamplerPool.GetNewItemIndex();

    GFXSampler& gfxSampler = m_SamplerPool.GetItem(gfxSamplerHandle.handle);
    shipBool isValid = gfxSampler.Create(*m_Device, samplerState);

    SHIP_ASSERT(isValid);

    m_SamplerHandleRefCounts[gfxSamplerHandle.handle] += 1;

    return gfxSamplerHandle;
}

void DX11RenderDevice::DestroySampler(GFXSamplerHandle gfxSamplerHandle)
{
    SHIP_ASSERT(m_SamplerHandleRefCounts[gfxSamplerHandle.handle] > 0);
    m_SamplerHandleRefCounts[gfxSamplerHandle.handle] -= 1;

    if (m_SamplerHandleRefCounts[gfxSamplerHandle.handle] == 0)
    {
        GFXSampler& gfxSampler = m_SamplerPool.GetItem(gfxSamplerHandle.handle);
        gfxSampler.Destroy();

        m_SamplerPool.ReleaseItem(gfxSamplerHandle.handle);
        gfxSamplerHandle.handle = InvalidGfxHandle;
    }
}

GFXSampler& DX11RenderDevice::GetSampler(GFXSamplerHandle gfxSamplerHandle)
{
    return m_SamplerPool.GetItem(gfxSamplerHandle.handle);
}

const GFXSampler& DX11RenderDevice::GetSampler(GFXSamplerHandle gfxSamplerHandle) const
{
    return m_SamplerPool.GetItem(gfxSamplerHandle.handle);
}

GFXSampler* DX11RenderDevice::GetSamplerPtr(GFXSamplerHandle gfxSamplerHandle)
{
    return m_SamplerPool.GetItemPtr(gfxSamplerHandle.handle);
}

const GFXSampler* DX11RenderDevice::GetSamplerPtr(GFXSamplerHandle gfxSamplerHandle) const
{
    return m_SamplerPool.GetItemPtr(gfxSamplerHandle.handle);
}

GFXRenderTargetHandle DX11RenderDevice::CreateRenderTarget(GFXTexture2DHandle* texturesToAttach, shipUint32 numTexturesToAttach)
{
    GFXRenderTargetHandle gfxRenderTargetHandle;
    gfxRenderTargetHandle.handle = m_RenderTargetPool.GetNewItemIndex();

    GFXRenderTarget& gfxRenderTarget = m_RenderTargetPool.GetItem(gfxRenderTargetHandle.handle);

    GFXTexture2D* textures[GfxConstants_MaxRenderTargetsBound];
    for (shipUint32 i = 0; i < numTexturesToAttach; i++)
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

    shipBool isValid = gfxRenderTarget.Create(*m_Device, textures, numTexturesToAttach);

    SHIP_ASSERT(isValid);

    return gfxRenderTargetHandle;
}

void DX11RenderDevice::DestroyRenderTarget(GFXRenderTargetHandle gfxRenderTargetHandle)
{
    GFXRenderTarget& gfxRenderTarget = m_RenderTargetPool.GetItem(gfxRenderTargetHandle.handle);
    gfxRenderTarget.Destroy();

    m_RenderTargetPool.ReleaseItem(gfxRenderTargetHandle.handle);
    gfxRenderTargetHandle.handle = InvalidGfxHandle;
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

    shipBool isValid = gfxDepthStencilRenderTarget.Create(*m_Device, gfxDepthStencilTexture);

    SHIP_ASSERT(isValid);

    return gfxDepthStencilRenderTargetHandle;
}

void DX11RenderDevice::DestroyDepthStencilRenderTarget(GFXDepthStencilRenderTargetHandle gfxDepthStencilRenderTargetHandle)
{
    GFXDepthStencilRenderTarget& gfxDepthStencilRenderTarget = m_DepthStencilRenderTargetPool.GetItem(gfxDepthStencilRenderTargetHandle.handle);
    gfxDepthStencilRenderTarget.Destroy();

    m_DepthStencilRenderTargetPool.ReleaseItem(gfxDepthStencilRenderTargetHandle.handle);
    gfxDepthStencilRenderTargetHandle.handle = InvalidGfxHandle;
}

GFXDepthStencilRenderTarget& DX11RenderDevice::GetDepthStencilRenderTarget(GFXDepthStencilRenderTargetHandle gfxDepthStencilRenderTargetHandle)
{
    return m_DepthStencilRenderTargetPool.GetItem(gfxDepthStencilRenderTargetHandle.handle);
}

const GFXDepthStencilRenderTarget& DX11RenderDevice::GetDepthStencilRenderTarget(GFXDepthStencilRenderTargetHandle gfxDepthStencilRenderTargetHandle) const
{
    return m_DepthStencilRenderTargetPool.GetItem(gfxDepthStencilRenderTargetHandle.handle);
}

GFXVertexShaderHandle DX11RenderDevice::CreateVertexShader(void* shaderData, shipUint64 shaderDataSize)
{
    GFXVertexShaderHandle gfxVertexShaderHandle;
    gfxVertexShaderHandle.handle = m_VertexShaderPool.GetNewItemIndex();

    GFXVertexShader& gfxVertexShader = m_VertexShaderPool.GetItem(gfxVertexShaderHandle.handle);
    shipBool isValid = gfxVertexShader.Create(*m_Device, shaderData, shaderDataSize);

    SHIP_ASSERT(isValid);

    return gfxVertexShaderHandle;
}

void DX11RenderDevice::DestroyVertexShader(GFXVertexShaderHandle gfxVertexShaderHandle)
{
    GFXVertexShader& gfxVertexShader = m_VertexShaderPool.GetItem(gfxVertexShaderHandle.handle);
    gfxVertexShader.Destroy();

    m_VertexShaderPool.ReleaseItem(gfxVertexShaderHandle.handle);
    gfxVertexShaderHandle.handle = InvalidGfxHandle;
}

GFXVertexShader& DX11RenderDevice::GetVertexShader(GFXVertexShaderHandle gfxVertexShaderHandle)
{
    return m_VertexShaderPool.GetItem(gfxVertexShaderHandle.handle);
}

const GFXVertexShader& DX11RenderDevice::GetVertexShader(GFXVertexShaderHandle gfxVertexShaderHandle) const
{
    return m_VertexShaderPool.GetItem(gfxVertexShaderHandle.handle);
}

GFXPixelShaderHandle DX11RenderDevice::CreatePixelShader(void* shaderData, shipUint64 shaderDataSize)
{
    GFXPixelShaderHandle gfxPixelShaderHandle;
    gfxPixelShaderHandle.handle = m_PixelShaderPool.GetNewItemIndex();

    GFXPixelShader& gfxPixelShader = m_PixelShaderPool.GetItem(gfxPixelShaderHandle.handle);
    shipBool isValid = gfxPixelShader.Create(*m_Device, shaderData, shaderDataSize);

    SHIP_ASSERT(isValid);

    return gfxPixelShaderHandle;
}

void DX11RenderDevice::DestroyPixelShader(GFXPixelShaderHandle gfxPixelShaderHandle)
{
    GFXPixelShader& gfxPixelShader = m_PixelShaderPool.GetItem(gfxPixelShaderHandle.handle);
    gfxPixelShader.Destroy();

    m_PixelShaderPool.ReleaseItem(gfxPixelShaderHandle.handle);
    gfxPixelShaderHandle.handle = InvalidGfxHandle;
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
    shipBool isValid = gfxRootSignature.Create(rootSignatureParameters);

    SHIP_ASSERT(isValid);

    return gfxRootSignatureHandle;
}

void DX11RenderDevice::DestroyRootSignature(GFXRootSignatureHandle gfxRootSignatureHandle)
{
    GFXRootSignature& gfxRootSignature = m_RootSignaturePool.GetItem(gfxRootSignatureHandle.handle);
    gfxRootSignature.Destroy();

    m_RootSignaturePool.ReleaseItem(gfxRootSignatureHandle.handle);
    gfxRootSignatureHandle.handle = InvalidGfxHandle;
}

GFXRootSignature& DX11RenderDevice::GetRootSignature(GFXRootSignatureHandle gfxRootSignatureHandle)
{
    return m_RootSignaturePool.GetItem(gfxRootSignatureHandle.handle);
}

const GFXRootSignature& DX11RenderDevice::GetRootSignature(GFXRootSignatureHandle gfxRootSignatureHandle) const
{
    return m_RootSignaturePool.GetItem(gfxRootSignatureHandle.handle);
}

GFXRootSignature* DX11RenderDevice::GetRootSignaturePtr(GFXRootSignatureHandle gfxRootSignatureHandle)
{
    return m_RootSignaturePool.GetItemPtr(gfxRootSignatureHandle.handle);
}

const GFXRootSignature* DX11RenderDevice::GetRootSignaturePtr(GFXRootSignatureHandle gfxRootSignatureHandle) const
{
    return m_RootSignaturePool.GetItemPtr(gfxRootSignatureHandle.handle);
}

shipUint32 GetPipelineStateObjectIndex(
        const DataPool<GFXPipelineStateObject, SHIP_MAX_RENDER_PIPELINE_STATE_OBJECTS>& pipelineStateObjectPool,
        const PipelineStateObjectCreationParameters& pipelineStateObjectCreationParameters)
{
    shipUint32 allocatedPipelineStateObjectIndex = 0;
    if (pipelineStateObjectPool.GetFirstAllocatedIndex(&allocatedPipelineStateObjectIndex))
    {
        do
        {
            if (pipelineStateObjectPool.GetItem(allocatedPipelineStateObjectIndex).GetCreationParameters() == pipelineStateObjectCreationParameters)
            {
                return allocatedPipelineStateObjectIndex;
            }
        } while (pipelineStateObjectPool.GetNextAllocatedIndex(allocatedPipelineStateObjectIndex, &allocatedPipelineStateObjectIndex));
    }

    return InvalidGfxHandle;
}

GFXPipelineStateObjectHandle DX11RenderDevice::CreatePipelineStateObject(const PipelineStateObjectCreationParameters& pipelineStateObjectCreationParameters)
{
    shipUint32 pipelineStateObjectIndex = GetPipelineStateObjectIndex(m_PipelineStateObjectPool, pipelineStateObjectCreationParameters);
    if (pipelineStateObjectIndex != InvalidGfxHandle)
    {
        m_PipelineStateObjectHandleRefCounts[pipelineStateObjectIndex] += 1;

        return GFXPipelineStateObjectHandle(pipelineStateObjectIndex);
    }

    GFXPipelineStateObjectHandle gfxPipelineStateObjectHandle;
    gfxPipelineStateObjectHandle.handle = m_PipelineStateObjectPool.GetNewItemIndex();

    GFXPipelineStateObject& gfxPipelineStateObject = m_PipelineStateObjectPool.GetItem(gfxPipelineStateObjectHandle.handle);
    shipBool isValid = gfxPipelineStateObject.Create(pipelineStateObjectCreationParameters);

    SHIP_ASSERT(isValid);

    m_PipelineStateObjectHandleRefCounts[gfxPipelineStateObjectHandle.handle] += 1;

    return gfxPipelineStateObjectHandle;
}

void DX11RenderDevice::DestroyPipelineStateObject(GFXPipelineStateObjectHandle gfxPipelineStateObjectHandle)
{
    SHIP_ASSERT(m_PipelineStateObjectHandleRefCounts[gfxPipelineStateObjectHandle.handle] > 0);
    m_PipelineStateObjectHandleRefCounts[gfxPipelineStateObjectHandle.handle] -= 1;

    if (m_PipelineStateObjectHandleRefCounts[gfxPipelineStateObjectHandle.handle] == 0)
    {
        GFXPipelineStateObject& gfxPipelineStateObject = m_PipelineStateObjectPool.GetItem(gfxPipelineStateObjectHandle.handle);
        gfxPipelineStateObject.Destroy();

        m_PipelineStateObjectPool.ReleaseItem(gfxPipelineStateObjectHandle.handle);
        gfxPipelineStateObjectHandle.handle = InvalidGfxHandle;
    }
}

GFXPipelineStateObject& DX11RenderDevice::GetPipelineStateObject(GFXPipelineStateObjectHandle gfxPipelineStateObjectHandle)
{
    return m_PipelineStateObjectPool.GetItem(gfxPipelineStateObjectHandle.handle);
}

const GFXPipelineStateObject& DX11RenderDevice::GetPipelineStateObject(GFXPipelineStateObjectHandle gfxPipelineStateObjectHandle) const
{
    return m_PipelineStateObjectPool.GetItem(gfxPipelineStateObjectHandle.handle);
}

GFXDescriptorSetHandle DX11RenderDevice::CreateDescriptorSet(DescriptorSetType descriptorSetType, const Array<DescriptorSetEntryDeclaration>& descriptorSetEntryDeclarations)
{
    GFXDescriptorSetHandle gfxDescriptorSetHandle;
    gfxDescriptorSetHandle.handle = m_DescriptorSetPool.GetNewItemIndex();

    GFXDescriptorSet& gfxDescriptorSet = m_DescriptorSetPool.GetItem(gfxDescriptorSetHandle.handle);
    
    shipBool isValid = gfxDescriptorSet.Create(descriptorSetType, descriptorSetEntryDeclarations);

    SHIP_ASSERT(isValid);

    return gfxDescriptorSetHandle;
}

void DX11RenderDevice::DestroyDescriptorSet(GFXDescriptorSetHandle gfxDescriptorSetHandle)
{
    GFXDescriptorSet& gfxDescriptorSet = m_DescriptorSetPool.GetItem(gfxDescriptorSetHandle.handle);
    gfxDescriptorSet.Destroy();

    m_DescriptorSetPool.ReleaseItem(gfxDescriptorSetHandle.handle);
    gfxDescriptorSetHandle.handle = InvalidGfxHandle;
}

GFXDescriptorSet& DX11RenderDevice::GetDescriptorSet(GFXDescriptorSetHandle gfxDescriptorSetHandle)
{
    return m_DescriptorSetPool.GetItem(gfxDescriptorSetHandle.handle);
}

const GFXDescriptorSet& DX11RenderDevice::GetDescriptorSet(GFXDescriptorSetHandle gfxDescriptorSetHandle) const
{
    return m_DescriptorSetPool.GetItem(gfxDescriptorSetHandle.handle);
}

IDXGISwapChain* DX11RenderDevice::CreateSwapchain(shipUint32 width, shipUint32 height, GfxFormat format, HWND hWnd, GFXTexture2DHandle& swapChainTextureHandle)
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

ID3D11InputLayout* RegisterVertexFormatType(ID3D11Device* device, VertexFormatType vertexFormatType)
{
    SHIP_STATIC_ASSERT_MSG(shipUint32(VertexFormatType::VertexFormatType_Count) == 6, "Update the RegisterVertexFormatType function if you add or remove vertex formats");

    shipUint32 idx = shipUint32(vertexFormatType);
    SHIP_ASSERT(g_RegisteredInputLayouts[idx] == nullptr);

    // Create a dummy shader just to validate the input layout
    StringA dummyShaderSource = GetShaderVertexInputForVertexFormat(vertexFormatType);

    dummyShaderSource +=
        "struct vs_output {\n"
        "float4 position : SV_POSITION;\n"
        "};\n"

        "vs_output main(vs_input input) {\n"
        "vs_output output = (vs_output)0;\n"
        "output.position = float4(input.position.x, 0.0, 0.0, 1.0);\n"
        "return output;\n"
        "}\n";

    ID3D10Blob* shaderBlob = nullptr;
    ID3D10Blob* error = nullptr;

    D3D_FEATURE_LEVEL featureLevel = device->GetFeatureLevel();
    StringA shaderVersion = GetD3DShaderVersion(featureLevel);

    StringA version = ("vs_" + shaderVersion);
    shipUint32 flags = 0;

#ifdef _DEBUG
    flags = D3DCOMPILE_DEBUG;
#endif

    HRESULT hr = D3DCompile(dummyShaderSource.GetBuffer(), dummyShaderSource.Size(), nullptr, nullptr, nullptr, "main", version.GetBuffer(), flags, 0, &shaderBlob, &error);
    if (FAILED(hr))
    {
        if (error != nullptr)
        {
            shipChar* errorMsg = (shipChar*)error->GetBufferPointer();
            SHIP_LOG_ERROR(errorMsg);

            error->Release();
        }

        SHIP_LOG_ERROR("RegisterVertexFormatType() --> Couldn't compile dummy shader for registering vertex layout.");

        return nullptr;
    }

    ID3D11VertexShader* shader = nullptr;
    shipBool success = true;
    hr = device->CreateVertexShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, &shader);
    if (FAILED(hr))
    {
        success = false;

        SHIP_LOG_ERROR("RegisterVertexFormatType() --> Couldn't create dummy vertex shader for registering vertex layout.");
    }

    if (error != nullptr)
    {
        error->Release();
    }

    if (!success)
    {
        shader->Release();
        return nullptr;
    }

    VertexFormat* vertexFormat = nullptr;
    GetVertexFormat(vertexFormatType, vertexFormat);

    const InputLayout* inputLayouts = vertexFormat->GetInputLayouts();
    shipUint32 numInputLayouts = vertexFormat->GetNumInputLayouts();

    Array<D3D11_INPUT_ELEMENT_DESC> inputElements;
    for (shipUint32 i = 0; i < numInputLayouts; i++)
    {
        const InputLayout& inputLayout = inputLayouts[i];

        D3D11_INPUT_ELEMENT_DESC inputElement;
        inputElement.SemanticName = GetVertexSemanticName(inputLayout.m_SemanticName);
        inputElement.SemanticIndex = inputLayout.m_SemanticIndex;
        inputElement.Format = ConvertShipyardFormatToDX11(inputLayout.m_Format);
        inputElement.InputSlot = inputLayout.m_InputSlot;
        inputElement.AlignedByteOffset = inputLayout.m_ByteOffset;
        inputElement.InputSlotClass = (inputLayout.m_IsDataPerInstance) ? D3D11_INPUT_PER_INSTANCE_DATA : D3D11_INPUT_PER_VERTEX_DATA;
        inputElement.InstanceDataStepRate = inputLayout.m_InstanceDataStepRate;

        inputElements.Add(inputElement);
    }

    ID3D11InputLayout* inputLayout = nullptr;
    hr = device->CreateInputLayout(&inputElements[0], inputElements.Size(), shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), &inputLayout);
    if (FAILED(hr))
    {
        SHIP_LOG_ERROR("RegisterVertexFormatType() --> Couldn't create input layout.");

        shader->Release();
        return nullptr;
    }

    shader->Release();
    return inputLayout;
}

}