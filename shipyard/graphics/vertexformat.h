#pragma once

#include <graphics/graphicscommon.h>

#include <system/array.h>

#include <extern/glm/vec2.hpp>
#include <extern/glm/vec3.hpp>
#include <extern/glm/vec4.hpp>
using namespace glm;

namespace Shipyard
{
    ///////////////////////////////////////////////////////////////////////////////
    // VERTEX TYPES
    ///////////////////////////////////////////////////////////////////////////////
    struct Vertex_Pos
    {
        vec3 m_Position;
    };

    struct Vertex_Pos_Color
    {
        vec3 m_Position;
        vec3 m_Color;
    };

    struct Vertex_Pos_UV
    {
        vec3 m_Position;
        vec2 m_UV;
    };

    struct Vertex_Pos_Normal
    {
        vec3 m_Position;
        vec3 m_Normal;
    };

    struct Vertex_Pos_UV_Normal
    {
        vec3 m_Position;
        vec2 m_UV;
        vec3 m_Normal;
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

        VertexFormatType_Count
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

    void GetVertexFormat(VertexFormatType vertexFormatType, VertexFormat*& vertexFormat);
    shipBool VertexFormatTypeContainsColor(VertexFormatType type);
    shipBool VertexFormatTypeContainsUV(VertexFormatType type);
    shipBool VertexFormatTypeContainsNormals(VertexFormatType type);
}