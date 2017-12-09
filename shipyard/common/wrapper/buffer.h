#pragma once

#include <common/wrapper/wrapper_common.h>

#include <common/common.h>
#include <common/vertexformat.h>

namespace Shipyard
{
    class SHIPYARD_API BaseBuffer
    {
#ifdef DEBUG_WRAPPER_INTERFACE_COMPILATION
    public:
        virtual void* Map(MapFlag mapFlag) = 0;
        virtual void Unmap() = 0;
#endif // #ifdef DEBUG_WRAPPER_INTERFACE_COMPILATION
    };

    class SHIPYARD_API VertexBuffer
    {
    public:
        VertexBuffer(uint32_t numVertices, VertexFormatType vertexFormatType, bool dynamic, void* initialData);

        uint32_t GetNumVertices() const { return m_NumVertices; }
        VertexFormatType GetVertexFormatType() const { return m_VertexFormatType; }

    private:
        uint32_t m_NumVertices;
        VertexFormatType m_VertexFormatType;
    };

    class SHIPYARD_API IndexBuffer
    {
    public:
        IndexBuffer(uint32_t numIndices, uint32_t indexSizeInBytes, bool dynamic, void* initialData);

        uint32_t GetNumIndices() const { return m_NumIndices; }
        uint32_t GetIndexSizeInBytes() const { return m_IndexSizeInBytes; }

    private:
        uint32_t m_NumIndices;
        uint32_t m_IndexSizeInBytes;
    };

    class SHIPYARD_API ConstantBuffer
    {
    public:
        ConstantBuffer(uint32_t dataSizeInBytes, bool dynamic, void* initialData);
    };

    class StructuredBuffer
    {

    };

    class DataBuffer
    {

    };

}