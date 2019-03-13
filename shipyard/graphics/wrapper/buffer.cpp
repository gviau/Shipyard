#include <graphics/wrapper/buffer.h>

namespace Shipyard
{;

BaseBuffer::BaseBuffer()
    : GfxResource(GfxResourceType::Unknown)
{

}

VertexBuffer::VertexBuffer()
    : m_NumVertices(0)
    , m_VertexFormatType(VertexFormatType::VertexFormatType_Count)
{

}

IndexBuffer::IndexBuffer()
    : m_NumIndices(0)
    , m_Uses2BytesPerIndex(true)
{

}

}