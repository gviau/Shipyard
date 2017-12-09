#include <common/wrapper/buffer.h>

namespace Shipyard
{;

VertexBuffer::VertexBuffer(uint32_t numVertices, VertexFormatType vertexFormatType, bool dynamic, void* initialData)
    : m_NumVertices(numVertices)
    , m_VertexFormatType(vertexFormatType)
{

}

IndexBuffer::IndexBuffer(uint32_t numIndices, uint32_t indexSizeInBytes, bool dynamic, void* initialData)
    : m_NumIndices(numIndices)
    , m_IndexSizeInBytes(indexSizeInBytes)
{

}

ConstantBuffer::ConstantBuffer(uint32_t dataSizeInBytes, bool dynamic, void* initialData)
{

}

}