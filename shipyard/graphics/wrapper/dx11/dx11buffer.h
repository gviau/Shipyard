#pragma once

#include <graphics/wrapper/buffer.h>

struct ID3D11Buffer;
struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11ShaderResourceView;
struct ID3D11UnorderedAccessView;

namespace Shipyard
{
    class SHIPYARD_GRAPHICS_API DX11BaseBuffer : public BaseBuffer
    {
    public:
        DX11BaseBuffer();

        void Destroy();

        ID3D11Buffer* GetBuffer() const { return m_Buffer; }
        ID3D11ShaderResourceView* GetShaderResourceView() { return m_ShaderResourceView; }
        ID3D11UnorderedAccessView* GetUnorderedAccessView() { return m_UnorderedAccessView; }

        size_t GetSize() const { return m_SizeInBytes; }

    protected:
        ID3D11DeviceContext* m_DeviceContext;
        ID3D11Buffer* m_Buffer;
        ID3D11ShaderResourceView* m_ShaderResourceView;
        ID3D11UnorderedAccessView* m_UnorderedAccessView;

        size_t m_SizeInBytes;
    };

    class SHIPYARD_GRAPHICS_API DX11VertexBuffer : public VertexBuffer, public DX11BaseBuffer
    {
    public:
        shipBool Create(ID3D11Device& device, ID3D11DeviceContext& deviceContext, shipUint32 numVertices, VertexFormatType vertexFormatType, shipBool dynamic, void* initialData);
    };

    class SHIPYARD_GRAPHICS_API DX11IndexBuffer : public IndexBuffer, public DX11BaseBuffer
    {
    public:
        shipBool Create(ID3D11Device& device, ID3D11DeviceContext& deviceContext, shipUint32 numIndices, shipBool uses2BytesPerIndex, shipBool dynamic, void* initialData);
    };

    class SHIPYARD_GRAPHICS_API DX11ConstantBuffer : public ConstantBuffer, public DX11BaseBuffer
    {
    public:
        shipBool Create(ID3D11Device& device, ID3D11DeviceContext& deviceContext, shipUint32 dataSizeInBytes, shipBool dynamic, void* initialData);
    };

    class SHIPYARD_GRAPHICS_API DX11ByteBuffer : public ByteBuffer, public DX11BaseBuffer
    {
    public:
        shipBool Create(
                ID3D11Device& device,
                ID3D11DeviceContext& deviceContext,
                ByteBufferCreationFlags byteBufferCreationFlags,
                shipUint32 dataSizeInBytes,
                shipBool dynamic,
                void* initialData);

    private:
        ID3D11UnorderedAccessView* m_UnorderedAccessView;
    };
}