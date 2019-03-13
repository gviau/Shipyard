#pragma once

#include <graphics/wrapper/buffer.h>

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11Buffer;

namespace Shipyard
{
    class SHIPYARD_API DX11BaseBuffer : public BaseBuffer
    {
    public:
        DX11BaseBuffer();

        void Destroy();

        void* Map(MapFlag mapFlag);
        void Unmap();

        ID3D11Buffer* GetBuffer() const { return m_Buffer; }

    protected:
        ID3D11DeviceContext* m_DeviceContext;
        ID3D11Buffer* m_Buffer;
    };

    class SHIPYARD_API DX11VertexBuffer : public VertexBuffer, public DX11BaseBuffer
    {
    public:
        bool Create(ID3D11Device& device, ID3D11DeviceContext& deviceContext, uint32_t numVertices, VertexFormatType vertexFormatType, bool dynamic, void* initialData);
    };

    class SHIPYARD_API DX11IndexBuffer : public IndexBuffer, public DX11BaseBuffer
    {
    public:
        bool Create(ID3D11Device& device, ID3D11DeviceContext& deviceContext, uint32_t numIndices, bool uses2BytesPerIndex, bool dynamic, void* initialData);
    };

    class SHIPYARD_API DX11ConstantBuffer : public ConstantBuffer, public DX11BaseBuffer
    {
    public:
        bool Create(ID3D11Device& device, ID3D11DeviceContext& deviceContext, uint32_t dataSizeInBytes, bool dynamic, void* initialData);
    };
}