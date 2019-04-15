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
    , m_SizeInBytes(0)
{

}

void DX11BaseBuffer::Destroy()
{
    SHIP_ASSERT_MSG(m_Buffer != nullptr, "Can't call Destroy on invalid buffer 0x%p", this);

    m_Buffer->Release();
    m_Buffer = nullptr;

    m_DeviceContext = nullptr;
}

bool DX11VertexBuffer::Create(ID3D11Device& device, ID3D11DeviceContext& deviceContext, uint32_t numVertices, VertexFormatType vertexFormatType, bool dynamic, void* initialData)
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

    HRESULT hr = device.CreateBuffer(&desc, &data, &m_Buffer);
    if (FAILED(hr))
    {
        SHIP_LOG_ERROR("DX11VertexBuffer::DX11VertexBuffer() --> Couldn't create vertex buffer.");
        return false;
    }

    return true;
}

bool DX11IndexBuffer::Create(ID3D11Device& device, ID3D11DeviceContext& deviceContext, uint32_t numIndices, bool uses2BytesPerIndex, bool dynamic, void* initialData)
{
    m_NumIndices = numIndices;
    m_Uses2BytesPerIndex = uses2BytesPerIndex;
    m_DeviceContext = &deviceContext;

    D3D11_USAGE usage = (dynamic) ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;

    uint32_t indexSizeInBytes = (uses2BytesPerIndex ? 2 : 4);

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

    HRESULT hr = device.CreateBuffer(&desc, &data, &m_Buffer);
    if (FAILED(hr))
    {
        SHIP_LOG_ERROR("DX11IndexBuffer::DX11IndexBuffer() --> Couldn't create index buffer.");
        return false;
    }

    return true;
}

bool DX11ConstantBuffer::Create(ID3D11Device& device, ID3D11DeviceContext& deviceContext, uint32_t dataSizeInBytes, bool dynamic, void* initialData)
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
        SHIP_LOG_ERROR("DX11ConstantBuffer::DX11ConstantBuffer() --> Couldn't create constant buffer.");
        return false;
    }

    m_ResourceType = GfxResourceType::ConstantBuffer;

    return true;
}

}