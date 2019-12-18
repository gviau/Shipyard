#include <graphics/graphicsprecomp.h>

#include <graphics/mesh/gfxmesh.h>

namespace Shipyard
{;

GFXSubMesh::GFXSubMesh()
    : m_GfxRenderDevice(nullptr)
    , m_GfxMaterial(nullptr)
{

}

GFXSubMesh::~GFXSubMesh()
{
    if (m_GfxVertexBufferHandle.IsValid())
    {
        m_GfxRenderDevice->DestroyVertexBuffer(m_GfxVertexBufferHandle);
    }

    if (m_GfxIndexBufferHandle.IsValid())
    {
        m_GfxRenderDevice->DestroyIndexBuffer(m_GfxIndexBufferHandle);
    }
}

void GFXSubMesh::Create(GFXRenderDevice& gfxRenderDevice, GFXVertexBufferHandle gfxVertexBufferHandle, GFXIndexBufferHandle gfxIndexBufferHandle, const GFXMaterial* gfxMaterial)
{
    m_GfxRenderDevice = &gfxRenderDevice;
    m_GfxVertexBufferHandle = gfxVertexBufferHandle;
    m_GfxIndexBufferHandle = gfxIndexBufferHandle;
    m_GfxMaterial = gfxMaterial;
}

GFXVertexBufferHandle GFXSubMesh::GetGfxVertexBufferHandle() const
{
    return m_GfxVertexBufferHandle;
}

GFXIndexBufferHandle GFXSubMesh::GetGfxIndexBufferHandle() const
{
    return m_GfxIndexBufferHandle;
}

const GFXMaterial* GFXSubMesh::GetGfxMaterial() const
{
    return m_GfxMaterial;
}

void GFXMesh::SetSubMeshes(GFXRenderDevice& gfxRenderDevice, const Array<GFXSubMeshCreationData>& gfxSubMeshes)
{
    m_GfxSubMeshes.Clear();
    m_GfxSubMeshes.Reserve(gfxSubMeshes.Size());

    for (const GFXSubMeshCreationData& gfxSubMeshCreationData : gfxSubMeshes)
    {
        GFXSubMesh& gfxSubMesh = m_GfxSubMeshes.Grow();
        gfxSubMesh.Create(gfxRenderDevice, gfxSubMeshCreationData.gfxVertexBufferHandle, gfxSubMeshCreationData.gfxIndexBufferHandle, gfxSubMeshCreationData.gfxMaterial);
    }
}

const Array<GFXSubMesh>& GFXMesh::GetGfxSubMeshes() const
{
    return m_GfxSubMeshes;
}

}