#pragma once

#include <common/wrapper/wrapper_common.h>

#include <common/vertexformat.h>

namespace Shipyard
{
    class SHIPYARD_API BaseRenderDevice
    {
    public:
#ifdef DEBUG_WRAPPER_INTERFACE_COMPILATION
        virtual GFXVertexBuffer* CreateVertexBuffer(uint32_t numVertices, VertexFormatType vertexFormatType, bool dynamic, void* initialData) = 0;
        virtual GFXIndexBuffer* CreateIndexBuffer(uint32_t numIndices, bool uses2BytesPerIndex, bool dynamic, void* initialData) = 0;
        virtual GFXConstantBuffer* CreateConstantBuffer(uint32_t dataSizeInBytes, bool dynamic, void* initialData) = 0;
        virtual GFXTexture2D* CreateTexture2D(uint32_t width, uint32_t height, GfxFormat pixelFormat, bool dynamic, void* initialData, bool generateMips) = 0;
        virtual GFXVertexShader* CreateVertexShader(const String& source) = 0;
        virtual GFXPixelShader* CreatePixelShader(const String& source) = 0;
#endif // #ifdef DEBUG_WRAPPER_INTERFACE_COMPILATION
    };
}