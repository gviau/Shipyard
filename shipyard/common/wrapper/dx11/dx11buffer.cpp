#include <common/wrapper/dx11/dx11buffer.h>

#include <cassert>

#include <common/wrapper/dx11/dx11_common.h>

#include <system/logger.h>

#pragma warning( disable : 4005 )

#include <d3d11.h>

#pragma warning( default : 4005 )

namespace Shipyard
{;

D3D11_MAP GetD3D11MapFlag(MapFlag mapFlag);

DX11BaseBuffer::DX11BaseBuffer(ID3D11DeviceContext& deviceContext)
    : m_DeviceContext(deviceContext)
    , m_Buffer(nullptr)
{

}

DX11BaseBuffer::~DX11BaseBuffer()
{
    if (m_Buffer != nullptr)
    {
        m_Buffer->Release();
    }
}

void* DX11BaseBuffer::Map(MapFlag mapFlag)
{
    if (m_Buffer == nullptr)
    {
        SHIP_LOG_WARNING("DX11BaseBuffer::Map() --> Calling map with a nullptr buffer.");
        return nullptr;
    }

    D3D11_MAP d3d11MapFlag = GetD3D11MapFlag(mapFlag);

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HRESULT hr = m_DeviceContext.Map(m_Buffer, 0, d3d11MapFlag, 0, &mappedResource);
    if (FAILED(hr))
    {
        SHIP_LOG_ERROR("DX11BaseBuffer::Map() --> Couldn't map buffer.");
        return nullptr;
    }

    return mappedResource.pData;
}

void DX11BaseBuffer::Unmap()
{
    if (m_Buffer != nullptr)
    {
        m_DeviceContext.Unmap(m_Buffer, 0);
    }
}

DX11VertexBuffer::DX11VertexBuffer(ID3D11Device& device, ID3D11DeviceContext& deviceContext, uint32_t numVertices, VertexFormatType vertexFormatType, bool dynamic, void* initialData)
    : DX11BaseBuffer(deviceContext)
    , VertexBuffer(numVertices, vertexFormatType, dynamic, initialData)
{
    D3D11_USAGE usage = (dynamic) ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;

    VertexFormat* vertexFormat = nullptr;
    GetVertexFormat(vertexFormatType, vertexFormat);

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
    }
}

DX11IndexBuffer::DX11IndexBuffer(ID3D11Device& device, ID3D11DeviceContext& deviceContext, uint32_t numIndices, bool uses2BytesPerIndex, bool dynamic, void* initialData)
    : DX11BaseBuffer(deviceContext)
    , IndexBuffer(numIndices, uses2BytesPerIndex, dynamic, initialData)
{
    D3D11_USAGE usage = (dynamic) ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;

    uint32_t indexSizeInBytes = (uses2BytesPerIndex ? 2 : 4);

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
    }
}

DX11ConstantBuffer::DX11ConstantBuffer(ID3D11Device& device, ID3D11DeviceContext& deviceContext, uint32_t dataSizeInBytes, bool dynamic, void* initialData)
    : DX11BaseBuffer(deviceContext)
    , ConstantBuffer(dataSizeInBytes, dynamic, initialData)
{
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
    }

    m_ResourceType = GfxResourceType::ConstantBuffer;
}

D3D11_MAP GetD3D11MapFlag(MapFlag mapFlag)
{
    switch (mapFlag)
    {
    case MapFlag::Read:                 return D3D11_MAP_READ;
    case MapFlag::Write:                return D3D11_MAP_WRITE;
    case MapFlag::Read_Write:           return D3D11_MAP_READ_WRITE;
    case MapFlag::Write_Discard:        return D3D11_MAP_WRITE_DISCARD;
    case MapFlag::Write_No_Overwrite:   return D3D11_MAP_WRITE_NO_OVERWRITE;
    default:
        // Should never happen
        assert(false);
        break;
    }

    return D3D11_MAP_READ;
}

}