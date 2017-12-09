#include <common/wrapper/buffer.h>

namespace Shipyard
{;

VertexBuffer::VertexBuffer(uint32_t numVertices, VertexFormatType vertexFormatType, bool dynamic, void* initialData)
    : m_NumVertices(numVertices)
    , m_VertexFormatType(vertexFormatType)
{

}

IndexBuffer::IndexBuffer(uint32_t numIndices, bool uses2BytesPerIndex, bool dynamic, void* initialData)
    : m_NumIndices(numIndices)
    , m_Uses2BytesPerIndex(uses2BytesPerIndex)
{

}

ConstantBuffer::ConstantBuffer(uint32_t dataSizeInBytes, bool dynamic, void* initialData)
{

}

}