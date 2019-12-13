#include <tools/meshimporter.h>

#include <system/pathutils.h>

#include <tools/textureimporter.h>

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

struct MaterialTextures
{
    enum : shipUint32
    {
        NoTexture = 0xffffffff
    };

    MaterialTextures()
    {
        for (shipUint32 i = 0; i < shipUint32(GfxMaterialTextureType::Count); i++)
        {
            indexOfLoadedTextureForGfxMaterialTextureType[i] = NoTexture;
        }
    }

    shipUint32 indexOfLoadedTextureForGfxMaterialTextureType[shipUint32(GfxMaterialTextureType::Count)];
};

shipBool LoadMaterialTexture(aiMaterial* material, aiTextureType textureType, StringT* materialTexturePath)
{
    SHIP_ASSERT(materialTexturePath != nullptr);

    if (material->GetTextureCount(textureType) == 0)
    {
        return false;
    }

    // For now, only get the first texture we find for that type. How to handle several textures of the same type?
    aiString texturePath;
    material->GetTexture(textureType, 0, &texturePath);

    *materialTexturePath = texturePath.C_Str();

    return true;
}

void AddMaterialTexture(const StringT& materialTexturePath, Array<StringT>& textureFilenamesToLoad, MaterialTextures& materialTextures, GfxMaterialTextureType gfxMaterialTextureType)
{
    if (textureFilenamesToLoad.AddUnique(materialTexturePath))
    {
        materialTextures.indexOfLoadedTextureForGfxMaterialTextureType[shipUint32(gfxMaterialTextureType)] = textureFilenamesToLoad.Size() - 1;
    }
    else
    {
        materialTextures.indexOfLoadedTextureForGfxMaterialTextureType[shipUint32(gfxMaterialTextureType)] = textureFilenamesToLoad.FindIndex(materialTexturePath);
    }
}

void ProcessSubMeshMaterial(aiMaterial* material, Array<StringT>& textureFilenamesToLoad, Array<MaterialTextures>& materialTextures)
{
    MaterialTextures& newMaterialTextures = materialTextures.Grow();

    StringT materialTexturePath;
    if (LoadMaterialTexture(material, aiTextureType::aiTextureType_BASE_COLOR, &materialTexturePath))
    {
        AddMaterialTexture(materialTexturePath, textureFilenamesToLoad, newMaterialTextures, GfxMaterialTextureType::AlbedoMap);
    }
    else if (LoadMaterialTexture(material, aiTextureType_DIFFUSE, &materialTexturePath))
    {
        AddMaterialTexture(materialTexturePath, textureFilenamesToLoad, newMaterialTextures, GfxMaterialTextureType::AlbedoMap);
    }

    if (LoadMaterialTexture(material, aiTextureType::aiTextureType_NORMAL_CAMERA, &materialTexturePath))
    {
        AddMaterialTexture(materialTexturePath, textureFilenamesToLoad, newMaterialTextures, GfxMaterialTextureType::NormalMap);
    }
    else if (LoadMaterialTexture(material, aiTextureType_NORMALS, &materialTexturePath))
    {
        AddMaterialTexture(materialTexturePath, textureFilenamesToLoad, newMaterialTextures, GfxMaterialTextureType::NormalMap);
    }

    if (LoadMaterialTexture(material, aiTextureType::aiTextureType_METALNESS, &materialTexturePath))
    {
        AddMaterialTexture(materialTexturePath, textureFilenamesToLoad, newMaterialTextures, GfxMaterialTextureType::MetlanessMap);
    }

    if (LoadMaterialTexture(material, aiTextureType::aiTextureType_DIFFUSE_ROUGHNESS, &materialTexturePath))
    {
        AddMaterialTexture(materialTexturePath, textureFilenamesToLoad, newMaterialTextures, GfxMaterialTextureType::RoughnessMap);
    }
    else if (LoadMaterialTexture(material, aiTextureType::aiTextureType_LIGHTMAP, &materialTexturePath))
    {
        AddMaterialTexture(materialTexturePath, textureFilenamesToLoad, newMaterialTextures, GfxMaterialTextureType::RoughnessMap);
    }
}

void LoadSubMeshGeometries(const aiScene* assimpScene, ImportedMesh& importedMesh, Array<shipUint32>& materialIndicesToLoad)
{
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
                ImportedSubMesh& importedSubMesh = importedMesh.SubMeshes.Grow();

                ProcessSubMesh(mesh, assimpScene, &importedSubMesh);

                materialIndicesToLoad.AddUnique(mesh->mMaterialIndex);
            }
        }

        for (shipUint32 i = 0; i < currentNode->mNumChildren; i++)
        {
            aiNode* childNode = currentNode->mChildren[i];
            nodesToProcess.push(childNode);
        }
    }
}

void LoadSubMeshMaterials(
        const shipChar* filename,
        GFXRenderDevice& gfxRenderDevice,
        MeshImportFlags meshImportFlags,
        const aiScene* assimpScene,
        const Array<shipUint32>& materialIndicesToLoad,
        ImportedMesh& importedMesh)
{
    InplaceArray<StringT, 64> textureFilenamesToLoad;

    InplaceArray<MaterialTextures, 32> subMeshMaterials;
    subMeshMaterials.Reserve(materialIndicesToLoad.Size());

    for (shipUint32 materialIndex : materialIndicesToLoad)
    {
        aiMaterial* material = assimpScene->mMaterials[materialIndex];

        ProcessSubMeshMaterial(material, textureFilenamesToLoad, subMeshMaterials);
    }

    InplaceArray<GFXTexture2DHandle, 64> loadedMeshTextures;
    loadedMeshTextures.Reserve(textureFilenamesToLoad.Size());

    InplaceStringT<256> fileDirectory;
    PathUtils::GetFileDirectory(filename, &fileDirectory);

    for (const StringT& textureFilename : textureFilenamesToLoad)
    {
        InplaceStringT<256> pathToTexture = fileDirectory + textureFilename;

        GFXTexture2DHandle gfxTexture2DHandle;
        TextureImporter::CreateGfxTextureFromFile(
                pathToTexture.GetBuffer(),
                gfxRenderDevice,
                ((meshImportFlags & MeshImportFlags::GenerateMipsForMaterialTextures) > 0) ? TextureImporter::GfxTextureCreationFlags::GenerateMips : TextureImporter::GfxTextureCreationFlags::DontGenerateMips,
                &gfxTexture2DHandle);

        loadedMeshTextures.Add(gfxTexture2DHandle);
    }

    importedMesh.SubMeshMaterials.Reserve(subMeshMaterials.Size());

    for (const MaterialTextures& materialTextures : subMeshMaterials)
    {
        ImportedSubMeshMaterial& importedSubMeshMaterial = importedMesh.SubMeshMaterials.Grow();

        for (shipUint32 i = 0; i < shipUint32(GfxMaterialTextureType::Count); i++)
        {
            if (materialTextures.indexOfLoadedTextureForGfxMaterialTextureType[i] == MaterialTextures::NoTexture)
            {
                continue;
            }

            importedSubMeshMaterial.SubMeshMaterialTextures[i] = loadedMeshTextures[materialTextures.indexOfLoadedTextureForGfxMaterialTextureType[i]];
        }
    }

    for (shipUint32 subMeshIndex = 0; subMeshIndex < materialIndicesToLoad.Size(); subMeshIndex++)
    {
        importedMesh.SubMeshes[subMeshIndex].ReferencedSubMeshMaterial = &importedMesh.SubMeshMaterials[materialIndicesToLoad[subMeshIndex]];
    }
}

ErrorCode LoadMeshFromFile(const shipChar* filename, GFXRenderDevice& gfxRenderDevice, MeshImportFlags meshImportFlags, ImportedMesh* importedMesh)
{
    SHIP_ASSERT(importedMesh != nullptr);

    Assimp::Importer meshImporter;

    const aiScene* assimpScene = meshImporter.ReadFile(filename, aiProcess_Triangulate | aiProcess_MakeLeftHanded | aiProcess_FlipWindingOrder | aiProcess_SortByPType);

    if (assimpScene == nullptr)
    {
        return ErrorCode::FileNotFound;
    }

    InplaceArray<shipUint32, 32> materialIndicesToLoad;
    materialIndicesToLoad.Reserve(assimpScene->mNumMaterials);

    LoadSubMeshGeometries(assimpScene, *importedMesh, materialIndicesToLoad);

    LoadSubMeshMaterials(filename, gfxRenderDevice, meshImportFlags, assimpScene, materialIndicesToLoad, *importedMesh);

    return ErrorCode::None;
}

shipBool IsFileExtensionSupportedForMeshImport(const shipChar* filename)
{
    InplaceArray<StringT, 2> stringParts;
    StringSplit(filename, '.', stringParts);

    Assimp::Importer meshImporter;

    return meshImporter.IsExtensionSupported(stringParts.Back().GetBuffer());
}

}

}