#pragma once

#include <common/wrapper/wrapper_common.h>

namespace Shipyard
{
    class BaseRenderDevice
    {
    public:
#ifdef DEBUG_WRAPPER_INTERFACE_COMPILATION
        virtual GFXVertexBuffer* CreateVertexBuffer(size_t numVertices, size_t vertexSizeInBytes, bool dynamic, void* initialData) = 0;
        virtual GFXIndexBuffer* CreateIndexBuffer(size_t numIndices, size_t indexSizeInBytes, bool dynamic, void* initialData) = 0;
        virtual GFXConstantBuffer* CreateConstantBuffer(size_t dataSizeInBytes, bool dynamic, void* initialData) = 0;
#endif // #ifdef DEBUG_WRAPPER_INTERFACE_COMPILATION
    };
}