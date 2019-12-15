#include <graphics/graphicsprecomp.h>

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

    for (shipUint32 i = 0; i < SHIP_MAX_GRAPHICS_PIPELINE_STATE_OBJECTS; i++)
    {
        m_GraphicsPipelineStateObjectHandleRefCounts[i] = 0;
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

    if (!m_Texture2dArrayPool.Create())
    {
        SHIP_LOG_ERROR("DX11RenderDevice::DX11RenderDevice() --> Couldn't create Texture2DArray pool.");
        return false;
    }

    if (!m_Texture3dPool.Create())
    {
        SHIP_LOG_ERROR("DX11RenderDevice::DX11RenderDevice() --> Couldn't create Texture3D pool.");
        return false;
    }

    if (!m_TextureCubePool.Create())
    {
        SHIP_LOG_ERROR("DX11RenderDevice::DX11RenderDevice() --> Couldn't create TextureCube pool.");
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

    if (!m_ComputeShaderPool.Create())
    {
        SHIP_LOG_ERROR("DX11RenderDevice::DX11RenderDevice() --> Couldn't create ComputeShader pool.");
        return false;
    }

    if (!m_RootSignaturePool.Create())
    {
        SHIP_LOG_ERROR("DX11RenderDevice::DX11RenderDevice() --> Couldn't create RootSignature pool.");
        return false;
    }

    if (!m_GraphicsPipelineStateObjectPool.Create())
    {
        SHIP_LOG_ERROR("DX11RenderDevice::DX11RenderDevice() --> Couldn't create GraphicsPipelineStateObject pool.");
        return false;
    }

    if (!m_ComputePipelineStateObjectPool.Create())
    {
        SHIP_LOG_ERROR("DX11RenderDevice::DX11RenderDevice() --> Couldn't create ComputePipelineStateObject pool.");
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
    shipUint16 generation = 0;
    if (m_VertexBufferPool.GetFirstAllocatedIndex(&indexToFree, &generation))
    {
        do
        {
            m_VertexBufferPool.GetItem(indexToFree, generation).Destroy();
            m_VertexBufferPool.ReleaseItem(indexToFree);
        } while (m_VertexBufferPool.GetNextAllocatedIndex(indexToFree, &indexToFree, &generation));
    }

    if (m_IndexBufferPool.GetFirstAllocatedIndex(&indexToFree, &generation))
    {
        do
        {
            m_IndexBufferPool.GetItem(indexToFree, generation).Destroy();
            m_IndexBufferPool.ReleaseItem(indexToFree);
        } while (m_IndexBufferPool.GetNextAllocatedIndex(indexToFree, &indexToFree, &generation));
    }

    if (m_ConstantBufferPool.GetFirstAllocatedIndex(&indexToFree, &generation))
    {
        do
        {
            m_ConstantBufferPool.GetItem(indexToFree, generation).Destroy();
            m_ConstantBufferPool.ReleaseItem(indexToFree);
        } while (m_ConstantBufferPool.GetNextAllocatedIndex(indexToFree, &indexToFree, &generation));
    }

    if (m_ByteBufferPool.GetFirstAllocatedIndex(&indexToFree, &generation))
    {
        do
        {
            m_ByteBufferPool.GetItem(indexToFree, generation).Destroy();
            m_ByteBufferPool.ReleaseItem(indexToFree);
        } while (m_ByteBufferPool.GetNextAllocatedIndex(indexToFree, &indexToFree, &generation));
    }

    if (m_Texture2dPool.GetFirstAllocatedIndex(&indexToFree, &generation))
    {
        do
        {
            m_Texture2dPool.GetItem(indexToFree, generation).Destroy();
            m_Texture2dPool.ReleaseItem(indexToFree);
        } while (m_Texture2dPool.GetNextAllocatedIndex(indexToFree, &indexToFree, &generation));
    }

    if (m_Texture2dArrayPool.GetFirstAllocatedIndex(&indexToFree, &generation))
    {
        do
        {
            m_Texture2dArrayPool.GetItem(indexToFree, generation).Destroy();
            m_Texture2dArrayPool.ReleaseItem(indexToFree);
        } while (m_Texture2dArrayPool.GetNextAllocatedIndex(indexToFree, &indexToFree, &generation));
    }

    if (m_Texture3dPool.GetFirstAllocatedIndex(&indexToFree, &generation))
    {
        do
        {
            m_Texture3dPool.GetItem(indexToFree, generation).Destroy();
            m_Texture3dPool.ReleaseItem(indexToFree);
        } while (m_Texture3dPool.GetNextAllocatedIndex(indexToFree, &indexToFree, &generation));
    }

    if (m_TextureCubePool.GetFirstAllocatedIndex(&indexToFree, &generation))
    {
        do
        {
            m_TextureCubePool.GetItem(indexToFree, generation).Destroy();
            m_TextureCubePool.ReleaseItem(indexToFree);
        } while (m_TextureCubePool.GetNextAllocatedIndex(indexToFree, &indexToFree, &generation));
    }

    if (m_SamplerPool.GetFirstAllocatedIndex(&indexToFree, &generation))
    {
        do
        {
            m_SamplerPool.GetItem(indexToFree, generation).Destroy();
            m_SamplerPool.ReleaseItem(indexToFree);
        } while (m_SamplerPool.GetNextAllocatedIndex(indexToFree, &indexToFree, &generation));
    }

    if (m_RenderTargetPool.GetFirstAllocatedIndex(&indexToFree, &generation))
    {
        do
        {
            m_RenderTargetPool.GetItem(indexToFree, generation).Destroy();
            m_RenderTargetPool.ReleaseItem(indexToFree);
        } while (m_RenderTargetPool.GetNextAllocatedIndex(indexToFree, &indexToFree, &generation));
    }

    if (m_DepthStencilRenderTargetPool.GetFirstAllocatedIndex(&indexToFree, &generation))
    {
        do
        {
            m_DepthStencilRenderTargetPool.GetItem(indexToFree, generation).Destroy();
            m_DepthStencilRenderTargetPool.ReleaseItem(indexToFree);
        } while (m_DepthStencilRenderTargetPool.GetNextAllocatedIndex(indexToFree, &indexToFree, &generation));
    }

    if (m_VertexShaderPool.GetFirstAllocatedIndex(&indexToFree, &generation))
    {
        do
        {
            m_VertexShaderPool.GetItem(indexToFree, generation).Destroy();
            m_VertexShaderPool.ReleaseItem(indexToFree);
        } while (m_VertexShaderPool.GetNextAllocatedIndex(indexToFree, &indexToFree, &generation));
    }

    if (m_PixelShaderPool.GetFirstAllocatedIndex(&indexToFree, &generation))
    {
        do
        {
            m_PixelShaderPool.GetItem(indexToFree, generation).Destroy();
            m_PixelShaderPool.ReleaseItem(indexToFree);
        } while (m_PixelShaderPool.GetNextAllocatedIndex(indexToFree, &indexToFree, &generation));
    }

    if (m_ComputeShaderPool.GetFirstAllocatedIndex(&indexToFree, &generation))
    {
        do
        {
            m_ComputeShaderPool.GetItem(indexToFree, generation).Destroy();
            m_ComputeShaderPool.ReleaseItem(indexToFree);
        } while (m_ComputeShaderPool.GetNextAllocatedIndex(indexToFree, &indexToFree, &generation));
    }

    if (m_RootSignaturePool.GetFirstAllocatedIndex(&indexToFree, &generation))
    {
        do
        {
            m_RootSignaturePool.GetItem(indexToFree, generation).Destroy();
            m_RootSignaturePool.ReleaseItem(indexToFree);
        } while (m_RootSignaturePool.GetNextAllocatedIndex(indexToFree, &indexToFree, &generation));
    }

    if (m_GraphicsPipelineStateObjectPool.GetFirstAllocatedIndex(&indexToFree, &generation))
    {
        do
        {
            m_GraphicsPipelineStateObjectPool.GetItem(indexToFree, generation).Destroy();
            m_GraphicsPipelineStateObjectPool.ReleaseItem(indexToFree);
        } while (m_GraphicsPipelineStateObjectPool.GetNextAllocatedIndex(indexToFree, &indexToFree, &generation));
    }

    if (m_ComputePipelineStateObjectPool.GetFirstAllocatedIndex(&indexToFree, &generation))
    {
        do
        {
            m_ComputePipelineStateObjectPool.GetItem(indexToFree, generation).Destroy();
            m_ComputePipelineStateObjectPool.ReleaseItem(indexToFree);
        } while (m_ComputePipelineStateObjectPool.GetNextAllocatedIndex(indexToFree, &indexToFree, &generation));
    }

    if (m_DescriptorSetPool.GetFirstAllocatedIndex(&indexToFree, &generation))
    {
        do
        {
            m_DescriptorSetPool.GetItem(indexToFree, generation).Destroy();
            m_DescriptorSetPool.ReleaseItem(indexToFree);
        } while (m_DescriptorSetPool.GetNextAllocatedIndex(indexToFree, &indexToFree, &generation));
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
    gfxVertexBufferHandle.handle = m_VertexBufferPool.GetNewItemIndex(&gfxVertexBufferHandle.generation);

    GFXVertexBuffer& gfxVertexBuffer = m_VertexBufferPool.GetItem(gfxVertexBufferHandle.handle, gfxVertexBufferHandle.generation);
    shipBool isValid = gfxVertexBuffer.Create(*m_Device, *m_ImmediateDeviceContext, numVertices, vertexFormatType, dynamic, initialData);

    SHIP_ASSERT(isValid);
    
    return gfxVertexBufferHandle;
}

void DX11RenderDevice::DestroyVertexBuffer(GFXVertexBufferHandle& gfxVertexBufferHandle)
{
    GFXVertexBuffer& gfxVertexBuffer = m_VertexBufferPool.GetItem(gfxVertexBufferHandle.handle, gfxVertexBufferHandle.generation);
    gfxVertexBuffer.Destroy();

    m_VertexBufferPool.ReleaseItem(gfxVertexBufferHandle.handle);
    gfxVertexBufferHandle.handle = InvalidGfxHandle;
    gfxVertexBufferHandle.generation += 1;
}

GFXVertexBuffer& DX11RenderDevice::GetVertexBuffer(GFXVertexBufferHandle gfxVertexBufferHandle)
{
    return m_VertexBufferPool.GetItem(gfxVertexBufferHandle.handle, gfxVertexBufferHandle.generation);
}

const GFXVertexBuffer& DX11RenderDevice::GetVertexBuffer(GFXVertexBufferHandle gfxVertexBufferHandle) const
{
    return m_VertexBufferPool.GetItem(gfxVertexBufferHandle.handle, gfxVertexBufferHandle.generation);
}

GFXIndexBufferHandle DX11RenderDevice::CreateIndexBuffer(shipUint32 numIndices, shipBool uses2BytesPerIndex, shipBool dynamic, void* initialData)
{
    GFXIndexBufferHandle gfxIndexBufferHandle;
    gfxIndexBufferHandle.handle = m_IndexBufferPool.GetNewItemIndex(&gfxIndexBufferHandle.generation);

    GFXIndexBuffer& gfxIndexBuffer = m_IndexBufferPool.GetItem(gfxIndexBufferHandle.handle, gfxIndexBufferHandle.generation);
    shipBool isValid = gfxIndexBuffer.Create(*m_Device, *m_ImmediateDeviceContext, numIndices, uses2BytesPerIndex, dynamic, initialData);

    SHIP_ASSERT(isValid);

    return gfxIndexBufferHandle;
}

void DX11RenderDevice::DestroyIndexBuffer(GFXIndexBufferHandle& gfxIndexBufferHandle)
{
    GFXIndexBuffer& gfxIndexBuffer = m_IndexBufferPool.GetItem(gfxIndexBufferHandle.handle, gfxIndexBufferHandle.generation);
    gfxIndexBuffer.Destroy();

    m_IndexBufferPool.ReleaseItem(gfxIndexBufferHandle.handle);
    gfxIndexBufferHandle.handle = InvalidGfxHandle;
    gfxIndexBufferHandle.generation += 1;
}

GFXIndexBuffer& DX11RenderDevice::GetIndexBuffer(GFXIndexBufferHandle gfxIndexBufferHandle)
{
    return m_IndexBufferPool.GetItem(gfxIndexBufferHandle.handle, gfxIndexBufferHandle.generation);
}

const GFXIndexBuffer& DX11RenderDevice::GetIndexBuffer(GFXIndexBufferHandle gfxIndexBufferHandle) const
{
    return m_IndexBufferPool.GetItem(gfxIndexBufferHandle.handle, gfxIndexBufferHandle.generation);
}

GFXConstantBufferHandle DX11RenderDevice::CreateConstantBuffer(shipUint32 dataSizeInBytes, shipBool dynamic, void* initialData)
{
    GFXConstantBufferHandle gfxConstantBufferHandle;

    if (dataSizeInBytes == 0)
    {
        return gfxConstantBufferHandle;
    }
    
    gfxConstantBufferHandle.handle = m_ConstantBufferPool.GetNewItemIndex(&gfxConstantBufferHandle.generation);

    GFXConstantBuffer& gfxConstantBuffer = m_ConstantBufferPool.GetItem(gfxConstantBufferHandle.handle, gfxConstantBufferHandle.generation);
    shipBool isValid = gfxConstantBuffer.Create(*m_Device, *m_ImmediateDeviceContext, dataSizeInBytes, dynamic, initialData);

    SHIP_ASSERT(isValid);

    return gfxConstantBufferHandle;
}

void DX11RenderDevice::DestroyConstantBuffer(GFXConstantBufferHandle& gfxConstantBufferHandle)
{
    GFXConstantBuffer& gfxConstantBuffer = m_ConstantBufferPool.GetItem(gfxConstantBufferHandle.handle, gfxConstantBufferHandle.generation);
    gfxConstantBuffer.Destroy();

    m_ConstantBufferPool.ReleaseItem(gfxConstantBufferHandle.handle);
    gfxConstantBufferHandle.handle = InvalidGfxHandle;
    gfxConstantBufferHandle.generation += 1;
}

GFXConstantBuffer& DX11RenderDevice::GetConstantBuffer(GFXConstantBufferHandle gfxConstantBufferHandle)
{
    return m_ConstantBufferPool.GetItem(gfxConstantBufferHandle.handle, gfxConstantBufferHandle.generation);
}

const GFXConstantBuffer& DX11RenderDevice::GetConstantBuffer(GFXConstantBufferHandle gfxConstantBufferHandle) const
{
    return m_ConstantBufferPool.GetItem(gfxConstantBufferHandle.handle, gfxConstantBufferHandle.generation);
}

GFXByteBufferHandle DX11RenderDevice::CreateByteBuffer(ByteBuffer::ByteBufferCreationFlags byteBufferCreationFlags, shipUint32 dataSizeInBytes, shipBool dynamic, void* initialData)
{
    GFXByteBufferHandle gfxByteBufferHandle;

    if (dataSizeInBytes == 0)
    {
        return gfxByteBufferHandle;
    }
    
    gfxByteBufferHandle.handle = m_ByteBufferPool.GetNewItemIndex(&gfxByteBufferHandle.generation);

    GFXByteBuffer& gfxByteBuffer = m_ByteBufferPool.GetItem(gfxByteBufferHandle.handle, gfxByteBufferHandle.generation);
    shipBool isValid = gfxByteBuffer.Create(*m_Device, *m_ImmediateDeviceContext, byteBufferCreationFlags, dataSizeInBytes, dynamic, initialData);

    SHIP_ASSERT(isValid);

    return gfxByteBufferHandle;
}

void DX11RenderDevice::DestroyByteBuffer(GFXByteBufferHandle& gfxByteBufferHandle)
{
    GFXByteBuffer& gfxByteBuffer = m_ByteBufferPool.GetItem(gfxByteBufferHandle.handle, gfxByteBufferHandle.generation);
    gfxByteBuffer.Destroy();

    m_ByteBufferPool.ReleaseItem(gfxByteBufferHandle.handle);
    gfxByteBufferHandle.handle = InvalidGfxHandle;
    gfxByteBufferHandle.generation += 1;
}

GFXByteBuffer& DX11RenderDevice::GetByteBuffer(GFXByteBufferHandle gfxByteBufferHandle)
{
    return m_ByteBufferPool.GetItem(gfxByteBufferHandle.handle, gfxByteBufferHandle.generation);
}

const GFXByteBuffer& DX11RenderDevice::GetByteBuffer(GFXByteBufferHandle gfxByteBufferHandle) const
{
    return m_ByteBufferPool.GetItem(gfxByteBufferHandle.handle, gfxByteBufferHandle.generation);
}

GFXByteBuffer* DX11RenderDevice::GetByteBufferPtr(GFXByteBufferHandle gfxByteBufferHandle)
{
    return m_ByteBufferPool.GetItemPtr(gfxByteBufferHandle.handle, gfxByteBufferHandle.generation);
}

const GFXByteBuffer* DX11RenderDevice::GetByteBufferPtr(GFXByteBufferHandle gfxByteBufferHandle) const
{
    return m_ByteBufferPool.GetItemPtr(gfxByteBufferHandle.handle, gfxByteBufferHandle.generation);
}

GFXTexture2DHandle DX11RenderDevice::CreateTexture2D(shipUint32 width, shipUint32 height, GfxFormat pixelFormat, shipBool dynamic, void* initialData, shipBool generateMips, TextureUsage textureUsage)
{
    GFXTexture2DHandle gfxTexture2dHandle;

    if (width == 0 && height == 0)
    {
        return gfxTexture2dHandle;
    }

    gfxTexture2dHandle.handle = m_Texture2dPool.GetNewItemIndex(&gfxTexture2dHandle.generation);

    GFXTexture2D& gfxTexture2d = m_Texture2dPool.GetItem(gfxTexture2dHandle.handle, gfxTexture2dHandle.generation);
    shipBool isValid = gfxTexture2d.Create(*m_Device, width, height, pixelFormat, dynamic, initialData, generateMips, textureUsage);

    SHIP_ASSERT(isValid);

    return gfxTexture2dHandle;
}

void DX11RenderDevice::DestroyTexture2D(GFXTexture2DHandle& gfxTexture2dHandle)
{
    GFXTexture2D& gfxTexture2d = m_Texture2dPool.GetItem(gfxTexture2dHandle.handle, gfxTexture2dHandle.generation);
    gfxTexture2d.Destroy();

    m_Texture2dPool.ReleaseItem(gfxTexture2dHandle.handle);
    gfxTexture2dHandle.handle = InvalidGfxHandle;
    gfxTexture2dHandle.generation += 1;
}

GFXTexture2D& DX11RenderDevice::GetTexture2D(GFXTexture2DHandle gfxTexture2dHandle)
{
    return m_Texture2dPool.GetItem(gfxTexture2dHandle.handle, gfxTexture2dHandle.generation);
}

const GFXTexture2D& DX11RenderDevice::GetTexture2D(GFXTexture2DHandle gfxTexture2dHandle) const
{
    return m_Texture2dPool.GetItem(gfxTexture2dHandle.handle, gfxTexture2dHandle.generation);
}

GFXTexture2D* DX11RenderDevice::GetTexture2DPtr(GFXTexture2DHandle gfxTexture2dHandle)
{
    return m_Texture2dPool.GetItemPtr(gfxTexture2dHandle.handle, gfxTexture2dHandle.generation);
}

const GFXTexture2D* DX11RenderDevice::GetTexture2DPtr(GFXTexture2DHandle gfxTexture2dHandle) const
{
    return m_Texture2dPool.GetItemPtr(gfxTexture2dHandle.handle, gfxTexture2dHandle.generation);
}

GFXTexture2DArrayHandle DX11RenderDevice::CreateTexture2DArray(
        shipUint32 width,
        shipUint32 height,
        shipUint32 numSlices,
        GfxFormat pixelFormat,
        shipBool dynamic,
        void* initialData,
        shipBool generateMips,
        TextureUsage textureUsage)
{
    GFXTexture2DArrayHandle gfxTexture2DArrayHandle;

    if (width == 0 && height == 0 && numSlices == 0)
    {
        return gfxTexture2DArrayHandle;
    }

    gfxTexture2DArrayHandle.handle = m_Texture3dPool.GetNewItemIndex(&gfxTexture2DArrayHandle.generation);

    GFXTexture2DArray& gfxTexture2dArray = m_Texture2dArrayPool.GetItem(gfxTexture2DArrayHandle.handle, gfxTexture2DArrayHandle.generation);
    shipBool isValid = gfxTexture2dArray.Create(*m_Device, width, height, numSlices, pixelFormat, dynamic, initialData, generateMips, textureUsage);

    SHIP_ASSERT(isValid);

    return gfxTexture2DArrayHandle;
}

void DX11RenderDevice::DestroyTexture2DArray(GFXTexture2DArrayHandle& gfxTexture2DArrayHandle)
{
    GFXTexture2DArray& gfxTexture2dArray = m_Texture2dArrayPool.GetItem(gfxTexture2DArrayHandle.handle, gfxTexture2DArrayHandle.generation);
    gfxTexture2dArray.Destroy();

    m_Texture2dArrayPool.ReleaseItem(gfxTexture2DArrayHandle.handle);
    gfxTexture2DArrayHandle.handle = InvalidGfxHandle;
    gfxTexture2DArrayHandle.generation += 1;
}

GFXTexture2DArray& DX11RenderDevice::GetTexture2DArray(GFXTexture2DArrayHandle gfxTexture2DArrayHandle)
{
    return m_Texture2dArrayPool.GetItem(gfxTexture2DArrayHandle.handle, gfxTexture2DArrayHandle.generation);
}

const GFXTexture2DArray& DX11RenderDevice::GetTexture2DArray(GFXTexture2DArrayHandle gfxTexture2DArrayHandle) const
{
    return m_Texture2dArrayPool.GetItem(gfxTexture2DArrayHandle.handle, gfxTexture2DArrayHandle.generation);
}

GFXTexture2DArray* DX11RenderDevice::GetTexture2DArrayPtr(GFXTexture2DArrayHandle gfxTexture2DArrayHandle)
{
    return m_Texture2dArrayPool.GetItemPtr(gfxTexture2DArrayHandle.handle, gfxTexture2DArrayHandle.generation);
}

const GFXTexture2DArray* DX11RenderDevice::GetTexture2DArrayPtr(GFXTexture2DArrayHandle gfxTexture2DArrayHandle) const
{
    return m_Texture2dArrayPool.GetItemPtr(gfxTexture2DArrayHandle.handle, gfxTexture2DArrayHandle.generation);
}

GFXTexture3DHandle DX11RenderDevice::CreateTexture3D(
        shipUint32 width,
        shipUint32 height,
        shipUint32 depth,
        GfxFormat pixelFormat,
        shipBool dynamic,
        void* initialData,
        shipBool generateMips,
        TextureUsage textureUsage)
{
    GFXTexture3DHandle gfxTexture3dHandle;

    if (width == 0 && height == 0 && depth == 0)
    {
        return gfxTexture3dHandle;
    }

    gfxTexture3dHandle.handle = m_Texture3dPool.GetNewItemIndex(&gfxTexture3dHandle.generation);

    GFXTexture3D& gfxTexture3d = m_Texture3dPool.GetItem(gfxTexture3dHandle.handle, gfxTexture3dHandle.generation);
    shipBool isValid = gfxTexture3d.Create(*m_Device, width, height, depth, pixelFormat, dynamic, initialData, generateMips, textureUsage);

    SHIP_ASSERT(isValid);

    return gfxTexture3dHandle;
}

void DX11RenderDevice::DestroyTexture3D(GFXTexture3DHandle& gfxTexture3DHandle)
{
    GFXTexture3D& gfxTexture3d = m_Texture3dPool.GetItem(gfxTexture3DHandle.handle, gfxTexture3DHandle.generation);
    gfxTexture3d.Destroy();

    m_Texture3dPool.ReleaseItem(gfxTexture3DHandle.handle);
    gfxTexture3DHandle.handle = InvalidGfxHandle;
    gfxTexture3DHandle.generation += 1;
}

GFXTexture3D& DX11RenderDevice::GetTexture3D(GFXTexture3DHandle gfxTexture3DHandle)
{
    return m_Texture3dPool.GetItem(gfxTexture3DHandle.handle, gfxTexture3DHandle.generation);
}

const GFXTexture3D& DX11RenderDevice::GetTexture3D(GFXTexture3DHandle gfxTexture3DHandle) const
{
    return m_Texture3dPool.GetItem(gfxTexture3DHandle.handle, gfxTexture3DHandle.generation);
}

GFXTexture3D* DX11RenderDevice::GetTexture3DPtr(GFXTexture3DHandle gfxTexture3DHandle)
{
    return m_Texture3dPool.GetItemPtr(gfxTexture3DHandle.handle, gfxTexture3DHandle.generation);
}

const GFXTexture3D* DX11RenderDevice::GetTexture3DPtr(GFXTexture3DHandle gfxTexture3DHandle) const
{
    return m_Texture3dPool.GetItemPtr(gfxTexture3DHandle.handle, gfxTexture3DHandle.generation);
}

GFXTextureCubeHandle DX11RenderDevice::CreateTextureCube(
        shipUint32 width,
        shipUint32 height,
        GfxFormat pixelFormat,
        shipBool dynamic,
        void* initialData,
        shipBool generateMips,
        TextureUsage textureUsage)
{
    GFXTextureCubeHandle gfxTextureCubeHandle;

    if (width == 0 && height == 0)
    {
        return gfxTextureCubeHandle;
    }

    gfxTextureCubeHandle.handle = m_TextureCubePool.GetNewItemIndex(&gfxTextureCubeHandle.generation);

    GFXTextureCube& gfxTextureCube = m_TextureCubePool.GetItem(gfxTextureCubeHandle.handle, gfxTextureCubeHandle.generation);
    shipBool isValid = gfxTextureCube.Create(*m_Device, width, height, pixelFormat, dynamic, initialData, generateMips, textureUsage);

    SHIP_ASSERT(isValid);

    return gfxTextureCubeHandle;
}

void DX11RenderDevice::DestroyTextureCube(GFXTextureCubeHandle& gfxTextureCubeHandle)
{
    GFXTextureCube& gfxTextureCube = m_TextureCubePool.GetItem(gfxTextureCubeHandle.handle, gfxTextureCubeHandle.generation);
    gfxTextureCube.Destroy();

    m_TextureCubePool.ReleaseItem(gfxTextureCubeHandle.handle);
    gfxTextureCubeHandle.handle = InvalidGfxHandle;
    gfxTextureCubeHandle.generation += 1;
}

GFXTextureCube& DX11RenderDevice::GetTextureCube(GFXTextureCubeHandle gfxTextureCubeHandle)
{
    return m_TextureCubePool.GetItem(gfxTextureCubeHandle.handle, gfxTextureCubeHandle.generation);
}

const GFXTextureCube& DX11RenderDevice::GetTextureCube(GFXTextureCubeHandle gfxTextureCubeHandle) const
{
    return m_TextureCubePool.GetItem(gfxTextureCubeHandle.handle, gfxTextureCubeHandle.generation);
}

GFXTextureCube* DX11RenderDevice::GetTextureCubePtr(GFXTextureCubeHandle gfxTextureCubeHandle)
{
    return m_TextureCubePool.GetItemPtr(gfxTextureCubeHandle.handle, gfxTextureCubeHandle.generation);
}

const GFXTextureCube* DX11RenderDevice::GetTextureCubePtr(GFXTextureCubeHandle gfxTextureCubeHandle) const
{
    return m_TextureCubePool.GetItemPtr(gfxTextureCubeHandle.handle, gfxTextureCubeHandle.generation);
}

shipUint32 GetSamplerIndexFromSamplerState(const GenerationalDataPool<GFXSampler, SHIP_MAX_SAMPLERS>& samplerPool, const SamplerState& samplerState)
{
    shipUint32 allocatedSamplerIndex = 0;
    shipUint16 allocatedSamplerGeneration = 0;
    if (samplerPool.GetFirstAllocatedIndex(&allocatedSamplerIndex, &allocatedSamplerGeneration))
    {
        do
        {
            if (samplerPool.GetItem(allocatedSamplerIndex, allocatedSamplerGeneration).GetSamplerState() == samplerState)
            {
                return allocatedSamplerIndex;
            }
        } while (samplerPool.GetNextAllocatedIndex(allocatedSamplerIndex, &allocatedSamplerIndex, &allocatedSamplerGeneration));
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
    gfxSamplerHandle.handle = m_SamplerPool.GetNewItemIndex(&gfxSamplerHandle.generation);

    GFXSampler& gfxSampler = m_SamplerPool.GetItem(gfxSamplerHandle.handle, gfxSamplerHandle.generation);
    shipBool isValid = gfxSampler.Create(*m_Device, samplerState);

    SHIP_ASSERT(isValid);

    m_SamplerHandleRefCounts[gfxSamplerHandle.handle] += 1;

    return gfxSamplerHandle;
}

void DX11RenderDevice::DestroySampler(GFXSamplerHandle& gfxSamplerHandle)
{
    SHIP_ASSERT(m_SamplerHandleRefCounts[gfxSamplerHandle.handle] > 0);
    m_SamplerHandleRefCounts[gfxSamplerHandle.handle] -= 1;

    if (m_SamplerHandleRefCounts[gfxSamplerHandle.handle] == 0)
    {
        GFXSampler& gfxSampler = m_SamplerPool.GetItem(gfxSamplerHandle.handle, gfxSamplerHandle.generation);
        gfxSampler.Destroy();

        m_SamplerPool.ReleaseItem(gfxSamplerHandle.handle);
        gfxSamplerHandle.handle = InvalidGfxHandle;
        gfxSamplerHandle.generation += 1;
    }
}

GFXSampler& DX11RenderDevice::GetSampler(GFXSamplerHandle gfxSamplerHandle)
{
    return m_SamplerPool.GetItem(gfxSamplerHandle.handle, gfxSamplerHandle.generation);
}

const GFXSampler& DX11RenderDevice::GetSampler(GFXSamplerHandle gfxSamplerHandle) const
{
    return m_SamplerPool.GetItem(gfxSamplerHandle.handle, gfxSamplerHandle.generation);
}

GFXSampler* DX11RenderDevice::GetSamplerPtr(GFXSamplerHandle gfxSamplerHandle)
{
    return m_SamplerPool.GetItemPtr(gfxSamplerHandle.handle, gfxSamplerHandle.generation);
}

const GFXSampler* DX11RenderDevice::GetSamplerPtr(GFXSamplerHandle gfxSamplerHandle) const
{
    return m_SamplerPool.GetItemPtr(gfxSamplerHandle.handle, gfxSamplerHandle.generation);
}

GFXRenderTargetHandle DX11RenderDevice::CreateRenderTarget(GFXTexture2DHandle* texturesToAttach, shipUint32 numTexturesToAttach)
{
    GFXRenderTargetHandle gfxRenderTargetHandle;
    gfxRenderTargetHandle.handle = m_RenderTargetPool.GetNewItemIndex(&gfxRenderTargetHandle.generation);

    GFXRenderTarget& gfxRenderTarget = m_RenderTargetPool.GetItem(gfxRenderTargetHandle.handle, gfxRenderTargetHandle.generation);

    GFXTexture2D* textures[GfxConstants_MaxRenderTargetsBound];
    for (shipUint32 i = 0; i < numTexturesToAttach; i++)
    {
        if (texturesToAttach[i].handle == InvalidGfxHandle)
        {
            textures[i] = nullptr;
        }
        else
        {
            textures[i] = &m_Texture2dPool.GetItem(texturesToAttach[i].handle, texturesToAttach[i].generation);
        }
    }

    shipBool isValid = gfxRenderTarget.Create(*m_Device, textures, numTexturesToAttach);

    SHIP_ASSERT(isValid);

    return gfxRenderTargetHandle;
}

void DX11RenderDevice::DestroyRenderTarget(GFXRenderTargetHandle& gfxRenderTargetHandle)
{
    GFXRenderTarget& gfxRenderTarget = m_RenderTargetPool.GetItem(gfxRenderTargetHandle.handle, gfxRenderTargetHandle.generation);
    gfxRenderTarget.Destroy();

    m_RenderTargetPool.ReleaseItem(gfxRenderTargetHandle.handle);
    gfxRenderTargetHandle.handle = InvalidGfxHandle;
    gfxRenderTargetHandle.generation += 1;
}

GFXRenderTarget& DX11RenderDevice::GetRenderTarget(GFXRenderTargetHandle gfxRenderTargetHandle)
{
    return m_RenderTargetPool.GetItem(gfxRenderTargetHandle.handle, gfxRenderTargetHandle.generation);
}

const GFXRenderTarget& DX11RenderDevice::GetRenderTarget(GFXRenderTargetHandle gfxRenderTargetHandle) const
{
    return m_RenderTargetPool.GetItem(gfxRenderTargetHandle.handle, gfxRenderTargetHandle.generation);
}

GFXDepthStencilRenderTargetHandle DX11RenderDevice::CreateDepthStencilRenderTarget(GFXTexture2DHandle depthStencilTexture)
{
    SHIP_ASSERT(depthStencilTexture.IsValid());

    GFXDepthStencilRenderTargetHandle gfxDepthStencilRenderTargetHandle;
    gfxDepthStencilRenderTargetHandle.handle = m_DepthStencilRenderTargetPool.GetNewItemIndex(&gfxDepthStencilRenderTargetHandle.generation);

    GFXDepthStencilRenderTarget& gfxDepthStencilRenderTarget = m_DepthStencilRenderTargetPool.GetItem(gfxDepthStencilRenderTargetHandle.handle, gfxDepthStencilRenderTargetHandle.generation);

    GFXTexture2D& gfxDepthStencilTexture = m_Texture2dPool.GetItem(depthStencilTexture.handle, depthStencilTexture.generation);

    shipBool isValid = gfxDepthStencilRenderTarget.Create(*m_Device, gfxDepthStencilTexture);

    SHIP_ASSERT(isValid);

    return gfxDepthStencilRenderTargetHandle;
}

void DX11RenderDevice::DestroyDepthStencilRenderTarget(GFXDepthStencilRenderTargetHandle& gfxDepthStencilRenderTargetHandle)
{
    GFXDepthStencilRenderTarget& gfxDepthStencilRenderTarget = m_DepthStencilRenderTargetPool.GetItem(gfxDepthStencilRenderTargetHandle.handle, gfxDepthStencilRenderTargetHandle.generation);
    gfxDepthStencilRenderTarget.Destroy();

    m_DepthStencilRenderTargetPool.ReleaseItem(gfxDepthStencilRenderTargetHandle.handle);
    gfxDepthStencilRenderTargetHandle.handle = InvalidGfxHandle;
    gfxDepthStencilRenderTargetHandle.generation += 1;
}

GFXDepthStencilRenderTarget& DX11RenderDevice::GetDepthStencilRenderTarget(GFXDepthStencilRenderTargetHandle gfxDepthStencilRenderTargetHandle)
{
    return m_DepthStencilRenderTargetPool.GetItem(gfxDepthStencilRenderTargetHandle.handle, gfxDepthStencilRenderTargetHandle.generation);
}

const GFXDepthStencilRenderTarget& DX11RenderDevice::GetDepthStencilRenderTarget(GFXDepthStencilRenderTargetHandle gfxDepthStencilRenderTargetHandle) const
{
    return m_DepthStencilRenderTargetPool.GetItem(gfxDepthStencilRenderTargetHandle.handle, gfxDepthStencilRenderTargetHandle.generation);
}

GFXVertexShaderHandle DX11RenderDevice::CreateVertexShader(void* shaderData, shipUint64 shaderDataSize)
{
    GFXVertexShaderHandle gfxVertexShaderHandle;
    gfxVertexShaderHandle.handle = m_VertexShaderPool.GetNewItemIndex(&gfxVertexShaderHandle.generation);

    GFXVertexShader& gfxVertexShader = m_VertexShaderPool.GetItem(gfxVertexShaderHandle.handle, gfxVertexShaderHandle.generation);
    shipBool isValid = gfxVertexShader.Create(*m_Device, shaderData, shaderDataSize);

    SHIP_ASSERT(isValid);

    return gfxVertexShaderHandle;
}

void DX11RenderDevice::DestroyVertexShader(GFXVertexShaderHandle& gfxVertexShaderHandle)
{
    GFXVertexShader& gfxVertexShader = m_VertexShaderPool.GetItem(gfxVertexShaderHandle.handle, gfxVertexShaderHandle.generation);
    gfxVertexShader.Destroy();

    m_VertexShaderPool.ReleaseItem(gfxVertexShaderHandle.handle);
    gfxVertexShaderHandle.handle = InvalidGfxHandle;
    gfxVertexShaderHandle.generation += 1;
}

GFXVertexShader& DX11RenderDevice::GetVertexShader(GFXVertexShaderHandle gfxVertexShaderHandle)
{
    return m_VertexShaderPool.GetItem(gfxVertexShaderHandle.handle, gfxVertexShaderHandle.generation);
}

const GFXVertexShader& DX11RenderDevice::GetVertexShader(GFXVertexShaderHandle gfxVertexShaderHandle) const
{
    return m_VertexShaderPool.GetItem(gfxVertexShaderHandle.handle, gfxVertexShaderHandle.generation);
}

GFXPixelShaderHandle DX11RenderDevice::CreatePixelShader(void* shaderData, shipUint64 shaderDataSize)
{
    GFXPixelShaderHandle gfxPixelShaderHandle;
    gfxPixelShaderHandle.handle = m_PixelShaderPool.GetNewItemIndex(&gfxPixelShaderHandle.generation);

    GFXPixelShader& gfxPixelShader = m_PixelShaderPool.GetItem(gfxPixelShaderHandle.handle, gfxPixelShaderHandle.generation);
    shipBool isValid = gfxPixelShader.Create(*m_Device, shaderData, shaderDataSize);

    SHIP_ASSERT(isValid);

    return gfxPixelShaderHandle;
}

void DX11RenderDevice::DestroyPixelShader(GFXPixelShaderHandle& gfxPixelShaderHandle)
{
    GFXPixelShader& gfxPixelShader = m_PixelShaderPool.GetItem(gfxPixelShaderHandle.handle, gfxPixelShaderHandle.generation);
    gfxPixelShader.Destroy();

    m_PixelShaderPool.ReleaseItem(gfxPixelShaderHandle.handle);
    gfxPixelShaderHandle.handle = InvalidGfxHandle;
    gfxPixelShaderHandle.generation += 1;
}

GFXPixelShader& DX11RenderDevice::GetPixelShader(GFXPixelShaderHandle gfxPixelShaderHandle)
{
    return m_PixelShaderPool.GetItem(gfxPixelShaderHandle.handle, gfxPixelShaderHandle.generation);
}

const GFXPixelShader& DX11RenderDevice::GetPixelShader(GFXPixelShaderHandle gfxPixelShaderHandle) const
{
    return m_PixelShaderPool.GetItem(gfxPixelShaderHandle.handle, gfxPixelShaderHandle.generation);
}

GFXComputeShaderHandle DX11RenderDevice::CreateComputeShader(void* shaderData, shipUint64 shaderDataSize)
{
    GFXComputeShaderHandle gfxComputeShaderHandle;
    gfxComputeShaderHandle.handle = m_ComputeShaderPool.GetNewItemIndex(&gfxComputeShaderHandle.generation);

    GFXComputeShader& gfxComputeShader = m_ComputeShaderPool.GetItem(gfxComputeShaderHandle.handle, gfxComputeShaderHandle.generation);
    shipBool isValid = gfxComputeShader.Create(*m_Device, shaderData, shaderDataSize);

    SHIP_ASSERT(isValid);

    return gfxComputeShaderHandle;
}

void DX11RenderDevice::DestroyComputeShader(GFXComputeShaderHandle& gfxComputeShaderHandle)
{
    GFXComputeShader& gfxComputeShader = m_ComputeShaderPool.GetItem(gfxComputeShaderHandle.handle, gfxComputeShaderHandle.generation);
    gfxComputeShader.Destroy();

    m_ComputeShaderPool.ReleaseItem(gfxComputeShaderHandle.handle);
    gfxComputeShaderHandle.handle = InvalidGfxHandle;
    gfxComputeShaderHandle.generation += 1;
}

GFXComputeShader& DX11RenderDevice::GetComputeShader(GFXComputeShaderHandle gfxComputeShaderHandle)
{
    return m_ComputeShaderPool.GetItem(gfxComputeShaderHandle.handle, gfxComputeShaderHandle.generation);
}

const GFXComputeShader& DX11RenderDevice::GetComputeShader(GFXComputeShaderHandle gfxComputeShaderHandle) const
{
    return m_ComputeShaderPool.GetItem(gfxComputeShaderHandle.handle, gfxComputeShaderHandle.generation);
}

GFXRootSignatureHandle DX11RenderDevice::CreateRootSignature(const Array<RootSignatureParameterEntry>& rootSignatureParameters)
{
    GFXRootSignatureHandle gfxRootSignatureHandle;
    gfxRootSignatureHandle.handle = m_RootSignaturePool.GetNewItemIndex(&gfxRootSignatureHandle.generation);

    GFXRootSignature& gfxRootSignature = m_RootSignaturePool.GetItem(gfxRootSignatureHandle.handle, gfxRootSignatureHandle.generation);
    shipBool isValid = gfxRootSignature.Create(rootSignatureParameters);

    SHIP_ASSERT(isValid);

    return gfxRootSignatureHandle;
}

void DX11RenderDevice::DestroyRootSignature(GFXRootSignatureHandle& gfxRootSignatureHandle)
{
    GFXRootSignature& gfxRootSignature = m_RootSignaturePool.GetItem(gfxRootSignatureHandle.handle, gfxRootSignatureHandle.generation);
    gfxRootSignature.Destroy();

    m_RootSignaturePool.ReleaseItem(gfxRootSignatureHandle.handle);
    gfxRootSignatureHandle.handle = InvalidGfxHandle;
    gfxRootSignatureHandle.generation += 1;
}

GFXRootSignature& DX11RenderDevice::GetRootSignature(GFXRootSignatureHandle gfxRootSignatureHandle)
{
    return m_RootSignaturePool.GetItem(gfxRootSignatureHandle.handle, gfxRootSignatureHandle.generation);
}

const GFXRootSignature& DX11RenderDevice::GetRootSignature(GFXRootSignatureHandle gfxRootSignatureHandle) const
{
    return m_RootSignaturePool.GetItem(gfxRootSignatureHandle.handle, gfxRootSignatureHandle.generation);
}

GFXRootSignature* DX11RenderDevice::GetRootSignaturePtr(GFXRootSignatureHandle gfxRootSignatureHandle)
{
    return m_RootSignaturePool.GetItemPtr(gfxRootSignatureHandle.handle, gfxRootSignatureHandle.generation);
}

const GFXRootSignature* DX11RenderDevice::GetRootSignaturePtr(GFXRootSignatureHandle gfxRootSignatureHandle) const
{
    return m_RootSignaturePool.GetItemPtr(gfxRootSignatureHandle.handle, gfxRootSignatureHandle.generation);
}

shipUint32 GetGraphicsPipelineStateObjectIndex(
        const GenerationalDataPool<GFXGraphicsPipelineStateObject, SHIP_MAX_GRAPHICS_PIPELINE_STATE_OBJECTS>& pipelineStateObjectPool,
        const GraphicsPipelineStateObjectCreationParameters& pipelineStateObjectCreationParameters)
{
    shipUint32 allocatedPipelineStateObjectIndex = 0;
    shipUint16 allocatedPipelineStateObjectGeneration = 0;
    if (pipelineStateObjectPool.GetFirstAllocatedIndex(&allocatedPipelineStateObjectIndex, &allocatedPipelineStateObjectGeneration))
    {
        do
        {
            if (pipelineStateObjectPool.GetItem(allocatedPipelineStateObjectIndex, allocatedPipelineStateObjectGeneration).GetCreationParameters() == pipelineStateObjectCreationParameters)
            {
                return allocatedPipelineStateObjectIndex;
            }
        } while (pipelineStateObjectPool.GetNextAllocatedIndex(allocatedPipelineStateObjectIndex, &allocatedPipelineStateObjectIndex, &allocatedPipelineStateObjectGeneration));
    }

    return InvalidGfxHandle;
}

GFXGraphicsPipelineStateObjectHandle DX11RenderDevice::CreateGraphicsPipelineStateObject(const GraphicsPipelineStateObjectCreationParameters& pipelineStateObjectCreationParameters)
{
    shipUint32 pipelineStateObjectIndex = GetGraphicsPipelineStateObjectIndex(m_GraphicsPipelineStateObjectPool, pipelineStateObjectCreationParameters);
    if (pipelineStateObjectIndex != InvalidGfxHandle)
    {
        m_GraphicsPipelineStateObjectHandleRefCounts[pipelineStateObjectIndex] += 1;

        return GFXGraphicsPipelineStateObjectHandle(pipelineStateObjectIndex);
    }

    GFXGraphicsPipelineStateObjectHandle gfxPipelineStateObjectHandle;
    gfxPipelineStateObjectHandle.handle = m_GraphicsPipelineStateObjectPool.GetNewItemIndex(&gfxPipelineStateObjectHandle.generation);

    GFXGraphicsPipelineStateObject& gfxPipelineStateObject = m_GraphicsPipelineStateObjectPool.GetItem(gfxPipelineStateObjectHandle.handle, gfxPipelineStateObjectHandle.generation);
    shipBool isValid = gfxPipelineStateObject.Create(pipelineStateObjectCreationParameters);

    SHIP_ASSERT(isValid);

    m_GraphicsPipelineStateObjectHandleRefCounts[gfxPipelineStateObjectHandle.handle] += 1;

    return gfxPipelineStateObjectHandle;
}

void DX11RenderDevice::DestroyGraphicsPipelineStateObject(GFXGraphicsPipelineStateObjectHandle& gfxPipelineStateObjectHandle)
{
    SHIP_ASSERT(m_GraphicsPipelineStateObjectHandleRefCounts[gfxPipelineStateObjectHandle.handle] > 0);
    m_GraphicsPipelineStateObjectHandleRefCounts[gfxPipelineStateObjectHandle.handle] -= 1;

    if (m_GraphicsPipelineStateObjectHandleRefCounts[gfxPipelineStateObjectHandle.handle] == 0)
    {
        GFXGraphicsPipelineStateObject& gfxPipelineStateObject = m_GraphicsPipelineStateObjectPool.GetItem(gfxPipelineStateObjectHandle.handle, gfxPipelineStateObjectHandle.generation);
        gfxPipelineStateObject.Destroy();

        m_GraphicsPipelineStateObjectPool.ReleaseItem(gfxPipelineStateObjectHandle.handle);
        gfxPipelineStateObjectHandle.handle = InvalidGfxHandle;
        gfxPipelineStateObjectHandle.generation += 1;
    }
}

GFXGraphicsPipelineStateObject& DX11RenderDevice::GetGraphicsPipelineStateObject(GFXGraphicsPipelineStateObjectHandle gfxPipelineStateObjectHandle)
{
    return m_GraphicsPipelineStateObjectPool.GetItem(gfxPipelineStateObjectHandle.handle, gfxPipelineStateObjectHandle.generation);
}

const GFXGraphicsPipelineStateObject& DX11RenderDevice::GetGraphicsPipelineStateObject(GFXGraphicsPipelineStateObjectHandle gfxPipelineStateObjectHandle) const
{
    return m_GraphicsPipelineStateObjectPool.GetItem(gfxPipelineStateObjectHandle.handle, gfxPipelineStateObjectHandle.generation);
}

shipUint32 GetComputePipelineStateObjectIndex(
        const GenerationalDataPool<GFXComputePipelineStateObject, SHIP_MAX_COMPUTE_PIPELINE_STATE_OBJECTS>& pipelineStateObjectPool,
        const ComputePipelineStateObjectCreationParameters& pipelineStateObjectCreationParameters)
{
    shipUint32 allocatedPipelineStateObjectIndex = 0;
    shipUint16 allocatedPipelineStateObjetGeneration = 0;
    if (pipelineStateObjectPool.GetFirstAllocatedIndex(&allocatedPipelineStateObjectIndex, &allocatedPipelineStateObjetGeneration))
    {
        do
        {
            if (pipelineStateObjectPool.GetItem(allocatedPipelineStateObjectIndex, allocatedPipelineStateObjetGeneration).GetCreationParameters() == pipelineStateObjectCreationParameters)
            {
                return allocatedPipelineStateObjectIndex;
            }
        } while (pipelineStateObjectPool.GetNextAllocatedIndex(allocatedPipelineStateObjectIndex, &allocatedPipelineStateObjectIndex, &allocatedPipelineStateObjetGeneration));
    }

    return InvalidGfxHandle;
}

GFXComputePipelineStateObjectHandle DX11RenderDevice::CreateComputePipelineStateObject(const ComputePipelineStateObjectCreationParameters& pipelineStateObjectCreationParameters)
{
    shipUint32 pipelineStateObjectIndex = GetComputePipelineStateObjectIndex(m_ComputePipelineStateObjectPool, pipelineStateObjectCreationParameters);
    if (pipelineStateObjectIndex != InvalidGfxHandle)
    {
        m_GraphicsPipelineStateObjectHandleRefCounts[pipelineStateObjectIndex] += 1;

        return GFXComputePipelineStateObjectHandle(pipelineStateObjectIndex);
    }

    GFXComputePipelineStateObjectHandle gfxPipelineStateObjectHandle;
    gfxPipelineStateObjectHandle.handle = m_ComputePipelineStateObjectPool.GetNewItemIndex(&gfxPipelineStateObjectHandle.generation);

    GFXComputePipelineStateObject& gfxPipelineStateObject = m_ComputePipelineStateObjectPool.GetItem(gfxPipelineStateObjectHandle.handle, gfxPipelineStateObjectHandle.generation);
    shipBool isValid = gfxPipelineStateObject.Create(pipelineStateObjectCreationParameters);

    SHIP_ASSERT(isValid);

    m_GraphicsPipelineStateObjectHandleRefCounts[gfxPipelineStateObjectHandle.handle] += 1;

    return gfxPipelineStateObjectHandle;
}

void DX11RenderDevice::DestroyComputePipelineStateObject(GFXComputePipelineStateObjectHandle& gfxPipelineStateObjectHandle)
{
    SHIP_ASSERT(m_ComputePipelineStateObjectHandleRefCounts[gfxPipelineStateObjectHandle.handle] > 0);
    m_ComputePipelineStateObjectHandleRefCounts[gfxPipelineStateObjectHandle.handle] -= 1;

    if (m_ComputePipelineStateObjectHandleRefCounts[gfxPipelineStateObjectHandle.handle] == 0)
    {
        GFXComputePipelineStateObject& gfxPipelineStateObject = m_ComputePipelineStateObjectPool.GetItem(gfxPipelineStateObjectHandle.handle, gfxPipelineStateObjectHandle.generation);
        gfxPipelineStateObject.Destroy();

        m_ComputePipelineStateObjectPool.ReleaseItem(gfxPipelineStateObjectHandle.handle);
        gfxPipelineStateObjectHandle.handle = InvalidGfxHandle;
        gfxPipelineStateObjectHandle.generation += 1;
    }
}

GFXComputePipelineStateObject& DX11RenderDevice::GetComputePipelineStateObject(GFXComputePipelineStateObjectHandle gfxPipelineStateObjectHandle)
{
    return m_ComputePipelineStateObjectPool.GetItem(gfxPipelineStateObjectHandle.handle, gfxPipelineStateObjectHandle.generation);
}

const GFXComputePipelineStateObject& DX11RenderDevice::GetComputePipelineStateObject(GFXComputePipelineStateObjectHandle gfxPipelineStateObjectHandle) const
{
    return m_ComputePipelineStateObjectPool.GetItem(gfxPipelineStateObjectHandle.handle, gfxPipelineStateObjectHandle.generation);
}

GFXDescriptorSetHandle DX11RenderDevice::CreateDescriptorSet(DescriptorSetType descriptorSetType, const Array<DescriptorSetEntryDeclaration>& descriptorSetEntryDeclarations)
{
    GFXDescriptorSetHandle gfxDescriptorSetHandle;
    gfxDescriptorSetHandle.handle = m_DescriptorSetPool.GetNewItemIndex(&gfxDescriptorSetHandle.generation);

    GFXDescriptorSet& gfxDescriptorSet = m_DescriptorSetPool.GetItem(gfxDescriptorSetHandle.handle, gfxDescriptorSetHandle.generation);
    
    shipBool isValid = gfxDescriptorSet.Create(descriptorSetType, descriptorSetEntryDeclarations);

    SHIP_ASSERT(isValid);

    return gfxDescriptorSetHandle;
}

void DX11RenderDevice::DestroyDescriptorSet(GFXDescriptorSetHandle& gfxDescriptorSetHandle)
{
    GFXDescriptorSet& gfxDescriptorSet = m_DescriptorSetPool.GetItem(gfxDescriptorSetHandle.handle, gfxDescriptorSetHandle.generation);
    gfxDescriptorSet.Destroy();

    m_DescriptorSetPool.ReleaseItem(gfxDescriptorSetHandle.handle);
    gfxDescriptorSetHandle.handle = InvalidGfxHandle;
    gfxDescriptorSetHandle.generation += 1;
}

GFXDescriptorSet& DX11RenderDevice::GetDescriptorSet(GFXDescriptorSetHandle gfxDescriptorSetHandle)
{
    return m_DescriptorSetPool.GetItem(gfxDescriptorSetHandle.handle, gfxDescriptorSetHandle.generation);
}

const GFXDescriptorSet& DX11RenderDevice::GetDescriptorSet(GFXDescriptorSetHandle gfxDescriptorSetHandle) const
{
    return m_DescriptorSetPool.GetItem(gfxDescriptorSetHandle.handle, gfxDescriptorSetHandle.generation);
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

    swapChainTextureHandle.handle = m_Texture2dPool.GetNewItemIndex(&swapChainTextureHandle.generation);

    GFXTexture2D& swapChainTexture = m_Texture2dPool.GetItem(swapChainTextureHandle.handle, swapChainTextureHandle.generation);

    swapChainTexture = DX11Texture2D(*m_Device, *backBufferTexture, format);

    backBufferTexture->Release();

    return swapChain;
}

ID3D11InputLayout* RegisterVertexFormatType(ID3D11Device* device, VertexFormatType vertexFormatType)
{
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