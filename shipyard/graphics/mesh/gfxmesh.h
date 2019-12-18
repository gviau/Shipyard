#pragma once

#include <graphics/material/gfxmaterial.h>

#include <graphics/wrapper/wrapper.h>

#include <system/array.h>

namespace Shipyard
{
    class SHIPYARD_GRAPHICS_API GFXSubMesh
    {
    public:
        GFXSubMesh();
        ~GFXSubMesh();

        void Create(GFXRenderDevice& gfxRenderDevice, GFXVertexBufferHandle gfxVertexBufferHandle, GFXIndexBufferHandle gfxIndexBufferHandle, const GFXMaterial* gfxMaterial);

        GFXVertexBufferHandle GetGfxVertexBufferHandle() const;
        GFXIndexBufferHandle GetGfxIndexBufferHandle() const;
        const GFXMaterial* GetGfxMaterial() const;

    private:
        GFXRenderDevice* m_GfxRenderDevice;

        GFXVertexBufferHandle m_GfxVertexBufferHandle;
        GFXIndexBufferHandle m_GfxIndexBufferHandle;
        const GFXMaterial* m_GfxMaterial;
    };

    class SHIPYARD_GRAPHICS_API GFXMesh
    {
    public:
        struct GFXSubMeshCreationData
        {
            GFXVertexBufferHandle gfxVertexBufferHandle;
            GFXIndexBufferHandle gfxIndexBufferHandle;
            GFXMaterial* gfxMaterial;
        };

    public:
        void SetSubMeshes(GFXRenderDevice& gfxRenderDevice, const Array<GFXSubMeshCreationData>& gfxSubMeshes);

        const Array<GFXSubMesh>& GetGfxSubMeshes() const;

    private:
        Array<GFXSubMesh> m_GfxSubMeshes;
    };
}