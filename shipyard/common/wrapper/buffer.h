#pragma once

#include <common/wrapper/wrapper_common.h>

namespace Shipyard
{
    enum class MapFlag
    {
        Read,
        Write,
        Read_Write,
        Write_Discard,
        Write_No_Overwrite
    };

    class BaseBuffer
    {
#ifdef DEBUG_WRAPPER_INTERFACE_COMPILATION
    public:
        virtual void* Map(MapFlag mapFlag) = 0;
        virtual void Unmap() = 0;
#endif // #ifdef DEBUG_WRAPPER_INTERFACE_COMPILATION
    };

    class VertexBuffer
    {
    public:
        VertexBuffer(size_t numVertices, size_t vertexSizeInBytes, bool dynamic, void* initialData);
        
    };

    class IndexBuffer
    {
    public:
        IndexBuffer(size_t numIndices, size_t indexSizeInBytes, bool dynamic, void* initialData);
    };

    class ConstantBuffer
    {
    public:
        ConstantBuffer(size_t dataSizeInBytes, bool dynamic, void* initialData);
    };

    class StructuredBuffer
    {

    };

    class DataBuffer
    {

    };

}