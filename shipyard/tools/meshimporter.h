#pragma once

#include <graphics/vertexformat.h>

#include <system/array.h>
#include <system/string.h>

namespace Shipyard
{
    namespace MeshImporter
    {
        struct SHIPYARD_API ImportedSubMesh
        {
            VertexFormatType SubMeshVertexFormatType;

            // MeshVertices are to be interpreted depending on the VertexFormatType
            BigArray<shipUint8> SubMeshVertices;
            BigArray<shipUint8> SubMeshIndices;

            shipUint32 GetSubMeshNumVertices() const;
            shipUint32 GetSubMeshNumIndices() const;
            shipUint32 GetSubMeshIndicesSize() const;
        };

        struct SHIPYARD_API ImportedMesh
        {
            Array<ImportedSubMesh> SubMeshes;
        };

        enum class ErrorCode : shipUint8
        {
            None,
            FileNotFound
        };

        SHIPYARD_API ErrorCode LoadMeshFromFile(const shipChar* filename, ImportedMesh* importedMesh);
    }
}