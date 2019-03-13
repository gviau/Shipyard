#pragma once

#include <graphics/wrapper/gfxresource.h>

#include <graphics/vertexformat.h>

namespace Shipyard
{
    class SHIPYARD_API BaseBuffer : public GfxResource
    {
    public:
        BaseBuffer();

#ifdef DEBUG_WRAPPER_INTERFACE_COMPILATION
        virtual void* Map(MapFlag mapFlag) = 0;
        virtual void Unmap() = 0;
#endif // #ifdef DEBUG_WRAPPER_INTERFACE_COMPILATION
    };

    class SHIPYARD_API VertexBuffer
    {
    public:
        VertexBuffer();

        uint32_t GetNumVertices() const { return m_NumVertices; }
        VertexFormatType GetVertexFormatType() const { return m_VertexFormatType; }

    protected:
        uint32_t m_NumVertices;
        VertexFormatType m_VertexFormatType;
    };

    class SHIPYARD_API IndexBuffer
    {
    public:
        IndexBuffer();

        uint32_t GetNumIndices() const { return m_NumIndices; }
        bool Uses2BytesPerIndex() const { return m_Uses2BytesPerIndex; }

    protected:
        uint32_t m_NumIndices;
        bool m_Uses2BytesPerIndex;
    };

    class SHIPYARD_API ConstantBuffer
    {

    };

    class StructuredBuffer
    {

    };

    class DataBuffer
    {

    };
}