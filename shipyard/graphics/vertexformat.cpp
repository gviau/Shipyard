#include <graphics/vertexformat.h>

namespace Shipyard
{;
// VertexFormat creation
VertexFormat_Pos g_VertexFormat_Pos;
VertexFormat_Pos_Color g_VertexFormat_Pos_Color;
VertexFormat_Pos_UV g_VertexFormat_Pos_UV;
VertexFormat_Pos_Normal g_VertexFormat_Pos_Normal;
VertexFormat_Pos_UV_Normal g_VertexFormat_Pos_UV_Normal;

VertexFormat_Pos::VertexFormat_Pos() 
{
    InputLayout position = NEW_INPUT_LAYOUT(SemanticName::Position, 0, GfxFormat::R32G32B32_FLOAT, 0, 0, false, 0);

    m_InputLayouts.Add(position);
}

VertexFormat_Pos_Color::VertexFormat_Pos_Color() {
    InputLayout position = NEW_INPUT_LAYOUT(SemanticName::Position, 0, GfxFormat::R32G32B32_FLOAT, 0, 0, false, 0);
    InputLayout color = NEW_INPUT_LAYOUT(SemanticName::Color, 0, GfxFormat::R32G32B32_FLOAT, 0, 12, false, 0);

    m_InputLayouts.Add(position);
    m_InputLayouts.Add(color);
}

VertexFormat_Pos_UV::VertexFormat_Pos_UV() {
    InputLayout position = NEW_INPUT_LAYOUT(SemanticName::Position, 0, GfxFormat::R32G32B32_FLOAT, 0, 0, false, 0);
    InputLayout uv = NEW_INPUT_LAYOUT(SemanticName::TexCoord, 0, GfxFormat::R32G32_FLOAT, 0, 12, false, 0);

    m_InputLayouts.Add(position);
    m_InputLayouts.Add(uv);
}

VertexFormat_Pos_Normal::VertexFormat_Pos_Normal()
{
    InputLayout position = NEW_INPUT_LAYOUT(SemanticName::Position, 0, GfxFormat::R32G32B32_FLOAT, 0, 0, false, 0);
    InputLayout normal = NEW_INPUT_LAYOUT(SemanticName::Normal, 0, GfxFormat::R32G32B32_FLOAT, 0, 12, false, 0);

    m_InputLayouts.Add(position);
    m_InputLayouts.Add(normal);
}

VertexFormat_Pos_UV_Normal::VertexFormat_Pos_UV_Normal()
{
    InputLayout position = NEW_INPUT_LAYOUT(SemanticName::Position, 0, GfxFormat::R32G32B32_FLOAT, 0, 0, false, 0);
    InputLayout uv = NEW_INPUT_LAYOUT(SemanticName::TexCoord, 0, GfxFormat::R32G32_FLOAT, 0, 12, false, 0);
    InputLayout normal = NEW_INPUT_LAYOUT(SemanticName::Normal, 0, GfxFormat::R32G32B32_FLOAT, 0, 20, false, 0);

    m_InputLayouts.Add(position);
    m_InputLayouts.Add(uv);
    m_InputLayouts.Add(normal);
}

void GetVertexFormat(VertexFormatType vertexFormatType, VertexFormat*& vertexFormat)
{
    static_assert(uint32_t(VertexFormatType::VertexFormatType_Count) == 5, "Update the GetVertexFormat function if you add or remove vertex formats");

    switch (vertexFormatType)
    {
    case VertexFormatType::Pos:                               vertexFormat = &g_VertexFormat_Pos; break;
    case VertexFormatType::Pos_Color:                         vertexFormat = &g_VertexFormat_Pos_Color; break;
    case VertexFormatType::Pos_UV:                            vertexFormat = &g_VertexFormat_Pos_UV; break;
    case VertexFormatType::Pos_Normal:                        vertexFormat = &g_VertexFormat_Pos_Normal; break;
    case VertexFormatType::Pos_UV_Normal:                     vertexFormat = &g_VertexFormat_Pos_UV_Normal; break;
    }
}


bool VertexFormatTypeContainsColor(VertexFormatType type)
{
    static_assert(uint32_t(VertexFormatType::VertexFormatType_Count) == 5, "Update the VertexFormatTypeContainsColor function if you add or remove vertex formats");

    return (type == VertexFormatType::Pos_Color);
}

bool VertexFormatTypeContainsUV(VertexFormatType type)
{
    static_assert(uint32_t(VertexFormatType::VertexFormatType_Count) == 5, "Update the VertexFormatTypeContainsUV function if you add or remove vertex formats");

    switch (type)
    {
    case VertexFormatType::Pos_UV:
    case VertexFormatType::Pos_UV_Normal:
        return true;
    }

    return false;
}

bool VertexFormatTypeContainsNormals(VertexFormatType type)
{
    static_assert(uint32_t(VertexFormatType::VertexFormatType_Count) == 5, "Update the VertexFormatTypeContainsNormals function if you add or remove vertex formats");

    switch (type)
    {
    case VertexFormatType::Pos_Normal:
    case VertexFormatType::Pos_UV_Normal:
        return true;
    }

    return false;
}
}