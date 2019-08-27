#include <graphics/wrapper/dx11/dx11buffer.h>

#include <graphics/wrapper/dx11/dx11_common.h>

#include <system/logger.h>

#pragma warning( disable : 4005 )

#include <d3d11.h>

#pragma warning( default : 4005 )

namespace Shipyard
{;

DX11BaseBuffer::DX11BaseBuffer()
    : m_DeviceContext(nullptr)
    , m_Buffer(nullptr)
    , m_ShaderResourceView(nullptr)
    , m_SizeInBytes(0)
{

}

void DX11BaseBuffer::Destroy()
{
    SHIP_ASSERT_MSG(m_Buffer != nullptr, "Can't call Destroy on invalid buffer 0x%p", this);

    m_Buffer->Release();
    m_Buffer = nullptr;

    if (m_ShaderResourceView != nullptr)
    {
        m_ShaderResourceView->Release();
        m_ShaderResourceView = nullptr;
    }

    m_DeviceContext = nullptr;
}

shipBool DX11VertexBuffer::Create(ID3D11Device& device, ID3D11DeviceContext& deviceContext, shipUint32 numVertices, VertexFormatType vertexFormatType, shipBool dynamic, void* initialData)
{
    m_NumVertices = numVertices;
    m_VertexFormatType = vertexFormatType;
    m_DeviceContext = &deviceContext;

    D3D11_USAGE usage = (dynamic) ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;

    VertexFormat* vertexFormat = nullptr;
    GetVertexFormat(vertexFormatType, vertexFormat);

    m_SizeInBytes = vertexFormat->GetSize() * numVertices;

    D3D11_BUFFER_DESC desc;
    desc.Usage = usage;
    desc.ByteWidth = vertexFormat->GetSize() * numVertices;
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    desc.CPUAccessFlags = (dynamic) ? D3D11_CPU_ACCESS_WRITE : 0;
    desc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA data;
    data.pSysMem = initialData;
    data.SysMemPitch = 0;
    data.SysMemSlicePitch = 0;

    D3D11_SUBRESOURCE_DATA* pData = ((initialData == nullptr) ? nullptr : &data);

    HRESULT hr = device.CreateBuffer(&desc, pData, &m_Buffer);
    if (FAILED(hr))
    {
        SHIP_LOG_ERROR("DX11VertexBuffer::Create() --> Couldn't create vertex buffer.");
        return false;
    }

    return true;
}

shipBool DX11IndexBuffer::Create(ID3D11Device& device, ID3D11DeviceContext& deviceContext, shipUint32 numIndices, shipBool uses2BytesPerIndex, shipBool dynamic, void* initialData)
{
    m_NumIndices = numIndices;
    m_Uses2BytesPerIndex = uses2BytesPerIndex;
    m_DeviceContext = &deviceContext;

    D3D11_USAGE usage = (dynamic) ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;

    shipUint32 indexSizeInBytes = (uses2BytesPerIndex ? 2 : 4);

    m_SizeInBytes = numIndices * indexSizeInBytes;

    D3D11_BUFFER_DESC desc;
    desc.Usage = usage;
    desc.ByteWidth = numIndices * indexSizeInBytes;
    desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    desc.CPUAccessFlags = (dynamic) ? D3D11_CPU_ACCESS_WRITE : 0;
    desc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA data;
    data.pSysMem = initialData;
    data.SysMemPitch = 0;
    data.SysMemSlicePitch = 0;

    D3D11_SUBRESOURCE_DATA* pData = ((initialData == nullptr) ? nullptr : &data);

    HRESULT hr = device.CreateBuffer(&desc, pData, &m_Buffer);
    if (FAILED(hr))
    {
        SHIP_LOG_ERROR("DX11IndexBuffer::Create() --> Couldn't create index buffer.");
        return false;
    }

    return true;
}

shipBool DX11ConstantBuffer::Create(ID3D11Device& device, ID3D11DeviceContext& deviceContext, shipUint32 dataSizeInBytes, shipBool dynamic, void* initialData)
{
    m_DeviceContext = &deviceContext;
    m_SizeInBytes = dataSizeInBytes;

    D3D11_USAGE usage = (dynamic) ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;

    D3D11_BUFFER_DESC desc;
    desc.Usage = usage;
    desc.ByteWidth = dataSizeInBytes;
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = (dynamic) ? D3D11_CPU_ACCESS_WRITE : 0;
    desc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA data;
    D3D11_SUBRESOURCE_DATA* pData = &data;

    if (initialData != nullptr)
    {
        data.pSysMem = initialData;
        data.SysMemPitch = 0;
        data.SysMemSlicePitch = 0;
    }
    else
    {
        pData = nullptr;
    }

    HRESULT hr = device.CreateBuffer(&desc, pData, &m_Buffer);
    if (FAILED(hr))
    {
        SHIP_LOG_ERROR("DX11ConstantBuffer::Create() --> Couldn't create constant buffer.");
        return false;
    }

    m_ResourceType = GfxResourceType::ConstantBuffer;

    return true;
}

shipBool DX11ByteBuffer::Create(
        ID3D11Device& device,
        ID3D11DeviceContext& deviceContext,
        ByteBufferCreationFlags byteBufferCreationFlags,
        shipUint32 dataSizeInBytes,
        shipBool dynamic,
        void* initialData)
{
    m_DeviceContext = &deviceContext;
    m_SizeInBytes = dataSizeInBytes;

    D3D11_USAGE usage = (dynamic) ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;

    D3D11_BIND_FLAG bindFlag = D3D11_BIND_SHADER_RESOURCE;

    D3D11_BUFFER_DESC desc;
    desc.Usage = usage;
    desc.ByteWidth = dataSizeInBytes;
    desc.BindFlags = bindFlag;
    desc.CPUAccessFlags = (dynamic) ? D3D11_CPU_ACCESS_WRITE : 0;
    desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;

    D3D11_SUBRESOURCE_DATA data;
    D3D11_SUBRESOURCE_DATA* pData = &data;

    if (initialData != nullptr)
    {
        data.pSysMem = initialData;
        data.SysMemPitch = 0;
        data.SysMemSlicePitch = 0;
    }
    else
    {
        pData = nullptr;
    }

    HRESULT hr = device.CreateBuffer(&desc, pData, &m_Buffer);
    if (FAILED(hr))
    {
        SHIP_LOG_ERROR("DX11ByteBuffer::Create() --> Couldn't create byte buffer.");
        return false;
    }

    if ((byteBufferCreationFlags & ByteBufferCreationFlags::ByteBufferCreationFlags_ShaderResourceView) != 0)
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
        shaderResourceViewDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R32_TYPELESS;
        shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
        shaderResourceViewDesc.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;
        shaderResourceViewDesc.BufferEx.FirstElement = 0;
        shaderResourceViewDesc.BufferEx.NumElements = dataSizeInBytes / 4;

        hr = device.CreateShaderResourceView(m_Buffer, &shaderResourceViewDesc, &m_ShaderResourceView);
        if (FAILED(hr))
        {
            SHIP_LOG_ERROR("DX11ByteBuffer::Create() --> Couldn't create shader resource view.");
            return false;
        }
    }

    m_ResourceType = GfxResourceType::ByteBuffer;

    return true;
}

}