#pragma once

#include <common/wrapper/buffer.h>

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11Buffer;

namespace Shipyard
{
    class DX11BaseBuffer : public BaseBuffer
    {
    public:
        DX11BaseBuffer(ID3D11DeviceContext& deviceContext);
        virtual ~DX11BaseBuffer();

        void* Map(MapFlag mapFlag);
        void Unmap();

        ID3D11Buffer* GetBuffer() const { return m_Buffer; }

    protected:
        ID3D11DeviceContext& m_DeviceContext;
        ID3D11Buffer* m_Buffer;
    };

    class DX11VertexBuffer : public VertexBuffer, public DX11BaseBuffer
    {
    public:
        DX11VertexBuffer(ID3D11Device& device, ID3D11DeviceContext& deviceContext, size_t numVertices, size_t vertexSizeInBytes, bool dynamic, void* initialData);
    };

    class DX11IndexBuffer : public IndexBuffer, public DX11BaseBuffer
    {
    public:
        DX11IndexBuffer(ID3D11Device& device, ID3D11DeviceContext& deviceContext, size_t numIndices, size_t indexSizeInBytes, bool dynamic, void* initialData);
    };

    class DX11ConstantBuffer : public ConstantBuffer, public DX11BaseBuffer
    {
    public:
        DX11ConstantBuffer(ID3D11Device& device, ID3D11DeviceContext& deviceContext, size_t dataSizeInBytes, bool dynamic, void* initialData);
    };
}