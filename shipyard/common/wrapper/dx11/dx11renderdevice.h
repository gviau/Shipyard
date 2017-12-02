#pragma once

#include <common/wrapper/renderdevice.h>

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace Shipyard
{
    class DX11RenderDevice : public BaseRenderDevice
    {
    public:
        DX11RenderDevice();
        ~DX11RenderDevice();

        GFXVertexBuffer* CreateVertexBuffer(size_t numVertices, size_t vertexSizeInBytes, bool dynamic, void* initialData);
        GFXIndexBuffer* CreateIndexBuffer(size_t numIndices, size_t indexSizeInBytes, bool dynamic, void* initialData);
        GFXConstantBuffer* CreateConstantBuffer(size_t dataSizeInBytes, bool dynamic, void* initialData);

        ID3D11DeviceContext* GetImmediateDeviceContext() const { return m_ImmediateDeviceContext; }

    private:
        ID3D11Device* m_Device;
        ID3D11DeviceContext* m_ImmediateDeviceContext;
    };
}