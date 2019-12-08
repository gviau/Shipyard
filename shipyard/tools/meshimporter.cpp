#include <tools/meshimporter.h>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <queue>

namespace Shipyard
{;

namespace MeshImporter
{;

shipUint32 ImportedSubMesh::GetSubMeshNumVertices() const
{
    VertexFormat* vertexFormat = nullptr;
    GetVertexFormat(SubMeshVertexFormatType, vertexFormat);

    return (SubMeshVertices.Size() / vertexFormat->GetSize());
}

shipUint32 ImportedSubMesh::GetSubMeshNumIndices() const
{
    return (SubMeshIndices.Size() / GetSubMeshIndicesSize());
}

shipUint32 ImportedSubMesh::GetSubMeshIndicesSize() const
{
    return ((GetSubMeshNumVertices() < 65536) ? 2 : 4);
}

VertexFormatType GetVertexFormatTypeFromAssimpMesh(aiMesh* mesh)
{
    SHIP_ASSERT(mesh->HasPositions());

    if (mesh->HasNormals())
    {
        if (mesh->HasTextureCoords(0))
        {
            return VertexFormatType::Pos_UV_Normal;
        }
        else
        {
            return VertexFormatType::Pos_Normal;
        }
    }
    else
    {
        if (mesh->HasTextureCoords(0))
        {
            return VertexFormatType::Pos_UV;
        }
    }

    return VertexFormatType::Pos;
}

void ExtractVertexPos(aiMesh* mesh, shipUint32 vertexIdx, shipUint8*& vertexData)
{
    Vertex_Pos& vertex = *(Vertex_Pos*)vertexData;
    vertex.m_Position.x = mesh->mVertices[vertexIdx].x;
    vertex.m_Position.y = mesh->mVertices[vertexIdx].y;
    vertex.m_Position.z = mesh->mVertices[vertexIdx].z;

    vertexData += sizeof(vertex);
}

void ExtractVertexPosNormal(aiMesh* mesh, shipUint32 vertexIdx, shipUint8*& vertexData)
{
    Vertex_Pos_Normal& vertex = *(Vertex_Pos_Normal*)vertexData;
    vertex.m_Position.x = mesh->mVertices[vertexIdx].x;
    vertex.m_Position.y = mesh->mVertices[vertexIdx].y;
    vertex.m_Position.z = mesh->mVertices[vertexIdx].z;
    vertex.m_Normal.x = mesh->mNormals[vertexIdx].x;
    vertex.m_Normal.y = mesh->mNormals[vertexIdx].y;
    vertex.m_Normal.z = mesh->mNormals[vertexIdx].z;

    vertexData += sizeof(vertex);
}

void ExtractVertexPosUV(aiMesh* mesh, shipUint32 vertexIdx, shipUint8*& vertexData)
{
    Vertex_Pos_UV& vertex = *(Vertex_Pos_UV*)vertexData;
    vertex.m_Position.x = mesh->mVertices[vertexIdx].x;
    vertex.m_Position.y = mesh->mVertices[vertexIdx].y;
    vertex.m_Position.z = mesh->mVertices[vertexIdx].z;
    vertex.m_UV.x = mesh->mTextureCoords[0][vertexIdx].x;
    vertex.m_UV.y = mesh->mTextureCoords[0][vertexIdx].y;

    vertexData += sizeof(vertex);
}

void ExtractVertexPosUVNormal(aiMesh* mesh, shipUint32 vertexIdx, shipUint8*& vertexData)
{
    Vertex_Pos_UV_Normal& vertex = *(Vertex_Pos_UV_Normal*)vertexData;
    vertex.m_Position.x = mesh->mVertices[vertexIdx].x;
    vertex.m_Position.y = mesh->mVertices[vertexIdx].y;
    vertex.m_Position.z = mesh->mVertices[vertexIdx].z;
    vertex.m_UV.x = mesh->mTextureCoords[0][vertexIdx].x;
    vertex.m_UV.y = mesh->mTextureCoords[0][vertexIdx].y;
    vertex.m_Normal.x = mesh->mNormals[vertexIdx].x;
    vertex.m_Normal.y = mesh->mNormals[vertexIdx].y;
    vertex.m_Normal.z = mesh->mNormals[vertexIdx].z;

    vertexData += sizeof(vertex);
}

void ProcessSubMesh(aiMesh* mesh, const aiScene* scene, ImportedSubMesh* importedSubMesh)
{
    SHIP_ASSERT(importedSubMesh != nullptr);

    VertexFormatType vertexFormatType = GetVertexFormatTypeFromAssimpMesh(mesh);
    importedSubMesh->SubMeshVertexFormatType = vertexFormatType;

    VertexFormat* vertexFormat;
    GetVertexFormat(vertexFormatType, vertexFormat);

    importedSubMesh->SubMeshVertices.Resize(vertexFormat->GetSize() * mesh->mNumVertices);
    shipUint8* vertexData = &importedSubMesh->SubMeshVertices[0];

    for (shipUint32 vertexIdx = 0; vertexIdx < mesh->mNumVertices; vertexIdx++)
    {
        switch (vertexFormatType)
        {
        case VertexFormatType::Pos:
            ExtractVertexPos(mesh, vertexIdx, vertexData);
            break;

        case VertexFormatType::Pos_Normal:
            ExtractVertexPosNormal(mesh, vertexIdx, vertexData);
            break;

        case VertexFormatType::Pos_UV:
            ExtractVertexPosUV(mesh, vertexIdx, vertexData);
            break;

        case VertexFormatType::Pos_UV_Normal:
            ExtractVertexPosUVNormal(mesh, vertexIdx, vertexData);
            break;

        default:
            SHIP_ASSERT(!"Unsupported vertex format type in MehsImporter::ProcessSubMesh");
            break;
        }
    }

    shipUint32 subMeshIndicesSize = importedSubMesh->GetSubMeshIndicesSize();

    // We only process submeshes with triangles only. This is enforced through the combination of the aiProcess_Triangulate & aiProcess_SortByPType
    // post processes, and by ignoring sub meshes that aren't solely composed of triangles.
    shipUint32 numSubMeshIndices = mesh->mNumFaces * 3;

    importedSubMesh->SubMeshIndices.Resize(numSubMeshIndices * subMeshIndicesSize);
    shipUint8* indexData = &importedSubMesh->SubMeshIndices[0];

    for (shipUint32 faceIdx = 0; faceIdx < mesh->mNumFaces; faceIdx++)
    {
        const aiFace& subMeshFace = mesh->mFaces[faceIdx];

        for (shipUint32 idx = 0; idx < subMeshFace.mNumIndices; idx++)
        {
            shipUint32 index = subMeshFace.mIndices[idx];

            if (subMeshIndicesSize == 2)
            {
                shipUint16& indexAsWord = *(shipUint16*)indexData;
                indexAsWord = static_cast<shipUint16>(index);
                indexData += sizeof(indexAsWord);
            }
            else
            {
                shipUint32& indexAsDWord = *(shipUint32*)indexData;
                indexAsDWord = index;
                indexData += sizeof(indexAsDWord);
            }
        }
    }
}

ErrorCode LoadMeshFromFile(const shipChar* filename, ImportedMesh* importedMesh)
{
    SHIP_ASSERT(importedMesh != nullptr);

    Assimp::Importer meshImporter;

    const aiScene* assimpScene = meshImporter.ReadFile(filename, aiProcess_Triangulate | aiProcess_ConvertToLeftHanded | aiProcess_SortByPType);

    if (assimpScene == nullptr)
    {
        return ErrorCode::FileNotFound;
    }

    std::queue<aiNode*> nodesToProcess;
    nodesToProcess.push(assimpScene->mRootNode);

    while (nodesToProcess.size() > 0)
    {
        aiNode* currentNode = nodesToProcess.front();
        nodesToProcess.pop();

        for (shipUint32 i = 0; i < currentNode->mNumMeshes; i++)
        {
            aiMesh* mesh = assimpScene->mMeshes[currentNode->mMeshes[i]];

            if ((mesh->mPrimitiveTypes & aiPrimitiveType_TRIANGLE) > 0)
            {
                ImportedSubMesh& importedSubMesh = importedMesh->SubMeshes.Grow();

                ProcessSubMesh(mesh, assimpScene, &importedSubMesh);
            }
        }

        for (shipUint32 i = 0; i < currentNode->mNumChildren; i++)
        {
            aiNode* childNode = currentNode->mChildren[i];
            nodesToProcess.push(childNode);
        }
    }

    return ErrorCode::None;
}

}

}