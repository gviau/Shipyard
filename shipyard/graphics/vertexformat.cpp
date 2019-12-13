#include <graphics/graphicsprecomp.h>

#include <graphics/vertexformat.h>

namespace Shipyard
{;
// VertexFormat creation
VertexFormat_Pos g_VertexFormat_Pos;
VertexFormat_Pos_Color g_VertexFormat_Pos_Color;
VertexFormat_Pos_UV g_VertexFormat_Pos_UV;
VertexFormat_Pos_Normal g_VertexFormat_Pos_Normal;
VertexFormat_Pos_UV_Normal g_VertexFormat_Pos_UV_Normal;
VertexFormat_Pos_UV_Normal_Tangent g_VertexFormat_Pos_UV_Normal_Tangent;
VertexFormat_ImGui g_VertexFormat_ImGui;

VertexFormat_Pos::VertexFormat_Pos() 
{
    InputLayout position = NEW_INPUT_LAYOUT(SemanticName::Position, 0, GfxFormat::R32G32B32_FLOAT, 0, 0, false, 0);

    m_InputLayouts[m_NumInputLayouts++] = position;
}

VertexFormat_Pos_Color::VertexFormat_Pos_Color() {
    InputLayout position = NEW_INPUT_LAYOUT(SemanticName::Position, 0, GfxFormat::R32G32B32_FLOAT, 0, 0, false, 0);
    InputLayout color = NEW_INPUT_LAYOUT(SemanticName::Color, 0, GfxFormat::R32G32B32A32_FLOAT, 0, 12, false, 0);

    m_InputLayouts[m_NumInputLayouts++] = position;
    m_InputLayouts[m_NumInputLayouts++] = color;
}

VertexFormat_Pos_UV::VertexFormat_Pos_UV() {
    InputLayout position = NEW_INPUT_LAYOUT(SemanticName::Position, 0, GfxFormat::R32G32B32_FLOAT, 0, 0, false, 0);
    InputLayout uv = NEW_INPUT_LAYOUT(SemanticName::TexCoord, 0, GfxFormat::R32G32_FLOAT, 0, 12, false, 0);

    m_InputLayouts[m_NumInputLayouts++] = position;
    m_InputLayouts[m_NumInputLayouts++] = uv;
}

VertexFormat_Pos_Normal::VertexFormat_Pos_Normal()
{
    InputLayout position = NEW_INPUT_LAYOUT(SemanticName::Position, 0, GfxFormat::R32G32B32_FLOAT, 0, 0, false, 0);
    InputLayout normal = NEW_INPUT_LAYOUT(SemanticName::Normal, 0, GfxFormat::R32G32B32_FLOAT, 0, 12, false, 0);

    m_InputLayouts[m_NumInputLayouts++] = position;
    m_InputLayouts[m_NumInputLayouts++] = normal;
}

VertexFormat_Pos_UV_Normal::VertexFormat_Pos_UV_Normal()
{
    InputLayout position = NEW_INPUT_LAYOUT(SemanticName::Position, 0, GfxFormat::R32G32B32_FLOAT, 0, 0, false, 0);
    InputLayout uv = NEW_INPUT_LAYOUT(SemanticName::TexCoord, 0, GfxFormat::R32G32_FLOAT, 0, 12, false, 0);
    InputLayout normal = NEW_INPUT_LAYOUT(SemanticName::Normal, 0, GfxFormat::R32G32B32_FLOAT, 0, 20, false, 0);

    m_InputLayouts[m_NumInputLayouts++] = position;
    m_InputLayouts[m_NumInputLayouts++] = uv;
    m_InputLayouts[m_NumInputLayouts++] = normal;
}

VertexFormat_Pos_UV_Normal_Tangent::VertexFormat_Pos_UV_Normal_Tangent()
{
    InputLayout position = NEW_INPUT_LAYOUT(SemanticName::Position, 0, GfxFormat::R32G32B32_FLOAT, 0, 0, false, 0);
    InputLayout uv = NEW_INPUT_LAYOUT(SemanticName::TexCoord, 0, GfxFormat::R32G32_FLOAT, 0, 12, false, 0);
    InputLayout normal = NEW_INPUT_LAYOUT(SemanticName::Normal, 0, GfxFormat::R32G32B32_FLOAT, 0, 20, false, 0);
    InputLayout tangent = NEW_INPUT_LAYOUT(SemanticName::Tangent, 0, GfxFormat::R32G32B32_FLOAT, 0, 32, false, 0);

    m_InputLayouts[m_NumInputLayouts++] = position;
    m_InputLayouts[m_NumInputLayouts++] = uv;
    m_InputLayouts[m_NumInputLayouts++] = normal;
    m_InputLayouts[m_NumInputLayouts++] = tangent;
}

VertexFormat_ImGui::VertexFormat_ImGui()
{
    InputLayout position = NEW_INPUT_LAYOUT(SemanticName::Position, 0, GfxFormat::R32G32_FLOAT, 0, 0, false, 0);
    InputLayout uv = NEW_INPUT_LAYOUT(SemanticName::TexCoord, 0, GfxFormat::R32G32_FLOAT, 0, 8, false, 0);
    InputLayout color = NEW_INPUT_LAYOUT(SemanticName::Color, 0, GfxFormat::R8G8B8A8_UNORM, 0, 16, false, 0);

    m_InputLayouts[m_NumInputLayouts++] = position;
    m_InputLayouts[m_NumInputLayouts++] = uv;
    m_InputLayouts[m_NumInputLayouts++] = color;
}

void GetVertexFormat(VertexFormatType vertexFormatType, VertexFormat*& vertexFormat)
{
    SHIP_STATIC_ASSERT_MSG(shipUint32(VertexFormatType::VertexFormatType_Count) == 7, "Update the GetVertexFormat function if you add or remove vertex formats");

    switch (vertexFormatType)
    {
    case VertexFormatType::Pos:                                 vertexFormat = &g_VertexFormat_Pos; break;
    case VertexFormatType::Pos_Color:                           vertexFormat = &g_VertexFormat_Pos_Color; break;
    case VertexFormatType::Pos_UV:                              vertexFormat = &g_VertexFormat_Pos_UV; break;
    case VertexFormatType::Pos_Normal:                          vertexFormat = &g_VertexFormat_Pos_Normal; break;
    case VertexFormatType::Pos_UV_Normal:                       vertexFormat = &g_VertexFormat_Pos_UV_Normal; break;
    case VertexFormatType::Pos_UV_Normal_Tangent:               vertexFormat = &g_VertexFormat_Pos_UV_Normal_Tangent; break;
    case VertexFormatType::ImGui:                               vertexFormat = &g_VertexFormat_ImGui; break;
    }
}


shipBool VertexFormatTypeContainsColor(VertexFormatType type)
{
    SHIP_STATIC_ASSERT_MSG(shipUint32(VertexFormatType::VertexFormatType_Count) == 7, "Update the VertexFormatTypeContainsColor function if you add or remove vertex formats");

    switch (type)
    {
    case VertexFormatType::Pos_Color:
    case VertexFormatType::ImGui:
        return true;
    }

    return false;
}

shipBool VertexFormatTypeContainsUV(VertexFormatType type)
{
    SHIP_STATIC_ASSERT_MSG(shipUint32(VertexFormatType::VertexFormatType_Count) == 7, "Update the VertexFormatTypeContainsUV function if you add or remove vertex formats");

    switch (type)
    {
    case VertexFormatType::Pos_UV:
    case VertexFormatType::Pos_UV_Normal:
    case VertexFormatType::Pos_UV_Normal_Tangent:
    case VertexFormatType::ImGui:
        return true;
    }

    return false;
}

shipBool VertexFormatTypeContainsNormals(VertexFormatType type)
{
    SHIP_STATIC_ASSERT_MSG(shipUint32(VertexFormatType::VertexFormatType_Count) == 7, "Update the VertexFormatTypeContainsNormals function if you add or remove vertex formats");

    switch (type)
    {
    case VertexFormatType::Pos_Normal:
    case VertexFormatType::Pos_UV_Normal:
    case VertexFormatType::Pos_UV_Normal_Tangent:
        return true;
    }

    return false;
}

shipBool VertexFormatTypeContainsTangents(VertexFormatType type)
{
    SHIP_STATIC_ASSERT_MSG(shipUint32(VertexFormatType::VertexFormatType_Count) == 7, "Update the VertexFormatTypeContainsNormals function if you add or remove vertex formats");

    switch (type)
    {
    case VertexFormatType::Pos_UV_Normal_Tangent:
        return true;
    }

    return false;
}

const shipChar* GetVertexShaderInputName(SemanticName semanticName)
{
    SHIP_STATIC_ASSERT_MSG(shipUint32(SemanticName::Count) == 8, "Update the GetVertexShaderInputName function if you add or remove a SemanticName");

    switch (semanticName)
    {
    case SemanticName::BiNormal:    return "binormal";
    case SemanticName::Bones:       return "bone_indices";
    case SemanticName::Color:       return "color";
    case SemanticName::Normal:      return "normal";
    case SemanticName::Position:    return "position";
    case SemanticName::Tangent:     return "tangent";
    case SemanticName::TexCoord:    return "tex_coords";
    case SemanticName::Weights:     return "bone_weights";

    default:
        SHIP_ASSERT(!"Shouldn't happen");
    }

    return "INVALID_VERTEX_SHADER_INPUT_NAME";
}

const shipChar* GetVertexSemanticName(SemanticName semanticName)
{
    SHIP_STATIC_ASSERT_MSG(shipUint32(SemanticName::Count) == 8, "Update the GetVertexSemanticName function if you add or remove a SemanticName");

    switch (semanticName)
    {
    case SemanticName::BiNormal:    return "BINORMAL";
    case SemanticName::Bones:       return "BONE_INDICES";
    case SemanticName::Color:       return "COLOR";
    case SemanticName::Normal:      return "NORMAL";
    case SemanticName::Position:    return "POSITION";
    case SemanticName::Tangent:     return "TANGENT";
    case SemanticName::TexCoord:    return "TEXCOORDS";
    case SemanticName::Weights:     return "BONE_WEIGHTS";

    default:
        SHIP_ASSERT(!"Shouldn't happen");
    }

    return "INVALID_SEMANTIC_NAME";
}

}