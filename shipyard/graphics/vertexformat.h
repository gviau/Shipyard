#pragma once

#include <graphics/graphicscommon.h>

#include <system/array.h>

#include <math/mathtypes.h>

namespace Shipyard
{
    ///////////////////////////////////////////////////////////////////////////////
    // VERTEX TYPES
    ///////////////////////////////////////////////////////////////////////////////
    struct Vertex_Pos
    {
        shipVec3 m_Position;
    };

    struct Vertex_Pos_Color
    {
        shipVec3 m_Position;
        shipVec4 m_Color;
    };

    struct Vertex_Pos_UV
    {
        shipVec3 m_Position;
        shipVec2 m_UV;
    };

    struct Vertex_Pos_Normal
    {
        shipVec3 m_Position;
        shipVec3 m_Normal;
    };

    struct Vertex_Pos_UV_Normal
    {
        shipVec3 m_Position;
        shipVec2 m_UV;
        shipVec3 m_Normal;
    };

    struct Vertex_Pos_UV_Normal_Tangent
    {
        shipVec3 m_Position;
        shipVec2 m_UV;
        shipVec3 m_Normal;
        shipVec3 m_Tangent;
    };

    struct Vertex_ImGui
    {
        shipVec2 m_Position;
        shipVec2 m_UV;
        shipUint32 m_Color;
    };

    ///////////////////////////////////////////////////////////////////////////////
    // VERTEX FORMATS
    ///////////////////////////////////////////////////////////////////////////////
#define NEW_INPUT_LAYOUT(semanticName, semanticIndex, format, inputSlot, byteOffset, isDataPerInstance, instanceDataStepRate) \
    { semanticName, semanticIndex, format, inputSlot, byteOffset, isDataPerInstance, instanceDataStepRate }

    enum class VertexFormatType : shipUint16
    {
        Pos,
        Pos_Color,
        Pos_UV,
        Pos_Normal,
        Pos_UV_Normal,
        Pos_UV_Normal_Tangent,
        ImGui,

        VertexFormatType_Count,
        Invalid = VertexFormatType_Count
    };

    class VertexFormat
    {
    public:
        VertexFormat()
            : m_NumInputLayouts(0)
        {
        }

        const InputLayout* GetInputLayouts() const { return m_InputLayouts; }
        shipUint32 GetNumInputLayouts() const { return m_NumInputLayouts; }

        virtual shipUint32 GetSize() const = 0;

    protected:
        InputLayout m_InputLayouts[GfxConstants::GfxConstants_MaxInputLayouts];
        shipUint32 m_NumInputLayouts;
    };

    class VertexFormat_Pos : public VertexFormat
    {
    public:
        VertexFormat_Pos();
        virtual shipUint32 GetSize() const override { return sizeof(Vertex_Pos); }
    };

    class VertexFormat_Pos_Color : public VertexFormat
    {
    public:
        VertexFormat_Pos_Color();
        virtual shipUint32 GetSize() const override { return sizeof(Vertex_Pos_Color); }
    };

    class VertexFormat_Pos_UV : public VertexFormat
    {
    public:
        VertexFormat_Pos_UV();
        virtual shipUint32 GetSize() const override { return sizeof(Vertex_Pos_UV); }
    };

    class VertexFormat_Pos_Normal : public VertexFormat
    {
    public:
        VertexFormat_Pos_Normal();
        virtual shipUint32 GetSize() const override { return sizeof(Vertex_Pos_Normal); }
    };

    class VertexFormat_Pos_UV_Normal : public VertexFormat
    {
    public:
        VertexFormat_Pos_UV_Normal();
        virtual shipUint32 GetSize() const override { return sizeof(Vertex_Pos_UV_Normal); }
    };

    class VertexFormat_Pos_UV_Normal_Tangent : public VertexFormat
    {
    public:
        VertexFormat_Pos_UV_Normal_Tangent();
        virtual shipUint32 GetSize() const override { return sizeof(Vertex_Pos_UV_Normal_Tangent); }
    };

    class VertexFormat_ImGui : public VertexFormat
    {
    public:
        VertexFormat_ImGui();
        virtual shipUint32 GetSize() const override { return sizeof(Vertex_ImGui); }
    };

    SHIPYARD_GRAPHICS_API void GetVertexFormat(VertexFormatType vertexFormatType, VertexFormat*& vertexFormat);
    shipBool VertexFormatTypeContainsColor(VertexFormatType type);
    shipBool VertexFormatTypeContainsUV(VertexFormatType type);
    shipBool VertexFormatTypeContainsNormals(VertexFormatType type);
    shipBool VertexFormatTypeContainsTangents(VertexFormatType type);

    const shipChar* GetVertexShaderInputName(SemanticName semanticName);
    const shipChar* GetVertexSemanticName(SemanticName semanticName);
}