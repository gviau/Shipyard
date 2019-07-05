#pragma once

#include <graphics/wrapper/gfxresource.h>

#include <graphics/vertexformat.h>

namespace Shipyard
{
    class SHIPYARD_API BaseBuffer : public GfxResource
    {
    public:
        BaseBuffer();
    };

    class SHIPYARD_API VertexBuffer
    {
    public:
        VertexBuffer();

        shipUint32 GetNumVertices() const { return m_NumVertices; }
        VertexFormatType GetVertexFormatType() const { return m_VertexFormatType; }

    protected:
        shipUint32 m_NumVertices;
        VertexFormatType m_VertexFormatType;
    };

    class SHIPYARD_API IndexBuffer
    {
    public:
        IndexBuffer();

        shipUint32 GetNumIndices() const { return m_NumIndices; }
        shipBool Uses2BytesPerIndex() const { return m_Uses2BytesPerIndex; }

    protected:
        shipUint32 m_NumIndices;
        shipBool m_Uses2BytesPerIndex;
    };

    class SHIPYARD_API ConstantBuffer
    {

    };

    class StructuredBuffer
    {

    };

    class SHIPYARD_API ByteBuffer
    {
    public:
        enum ByteBufferCreationFlags
        {
            ByteBufferCreationFlags_ShaderResourceView = 0x01
        };
    };
}