#pragma once

#include <graphics/material/gfxmaterial.h>

#include <graphics/wrapper/wrapper.h>

#include <graphics/graphicstypes.h>
#include <graphics/vertexformat.h>

#include <system/array.h>
#include <system/string.h>

namespace Shipyard
{
    namespace MeshImporter
    {
        struct SHIPYARD_API ImportedSubMeshMaterial
        {
            GFXTexture2DHandle SubMeshMaterialTextures[shipUint32(GfxMaterialTextureType::Count)];
        };

        struct SHIPYARD_API ImportedSubMesh
        {
            VertexFormatType SubMeshVertexFormatType;

            // MeshVertices are to be interpreted depending on the VertexFormatType
            BigArray<shipUint8> SubMeshVertices;
            BigArray<shipUint8> SubMeshIndices;

            ImportedSubMeshMaterial* ReferencedSubMeshMaterial;

            shipUint32 GetSubMeshNumVertices() const;
            shipUint32 GetSubMeshNumIndices() const;
            shipUint32 GetSubMeshIndicesSize() const;
        };

        struct SHIPYARD_API ImportedMesh
        {
            Array<ImportedSubMesh> SubMeshes;
            Array<ImportedSubMeshMaterial> SubMeshMaterials;
        };

        enum class ErrorCode : shipUint8
        {
            None,
            FileNotFound
        };

        // Used as a bit mask
        enum MeshImportFlags
        {
            None = 0x00,
            GenerateMipsForMaterialTextures = 0x01
        };

        SHIPYARD_API ErrorCode LoadMeshFromFile(const shipChar* filename, GFXRenderDevice& gfxRenderDevice, MeshImportFlags meshImportFlags, ImportedMesh* importedMesh);
    }
}