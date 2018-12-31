#pragma once

#include <common/common.h>

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

    enum class VertexFormatType : uint16_t
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
        const Array<InputLayout>& GetInputLayouts() const { return m_InputLayouts; }
        virtual uint32_t GetSize() const = 0;

    protected:
        Array<InputLayout> m_InputLayouts;
    };

    class VertexFormat_Pos : public VertexFormat
    {
    public:
        VertexFormat_Pos();
        virtual uint32_t GetSize() const override { return sizeof(Vertex_Pos); }
    };

    class VertexFormat_Pos_Color : public VertexFormat
    {
    public:
        VertexFormat_Pos_Color();
        virtual uint32_t GetSize() const override { return sizeof(Vertex_Pos_Color); }
    };

    class VertexFormat_Pos_UV : public VertexFormat
    {
    public:
        VertexFormat_Pos_UV();
        virtual uint32_t GetSize() const override { return sizeof(Vertex_Pos_UV); }
    };

    class VertexFormat_Pos_Normal : public VertexFormat
    {
    public:
        VertexFormat_Pos_Normal();
        virtual uint32_t GetSize() const override { return sizeof(Vertex_Pos_Normal); }
    };

    class VertexFormat_Pos_UV_Normal : public VertexFormat
    {
    public:
        VertexFormat_Pos_UV_Normal();
        virtual uint32_t GetSize() const override { return sizeof(Vertex_Pos_UV_Normal); }
    };

    void GetVertexFormat(VertexFormatType vertexFormatType, VertexFormat*& vertexFormat);
    bool VertexFormatTypeContainsColor(VertexFormatType type);
    bool VertexFormatTypeContainsUV(VertexFormatType type);
    bool VertexFormatTypeContainsNormals(VertexFormatType type);
}