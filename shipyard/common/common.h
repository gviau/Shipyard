#pragma once

#include <system/array.h>
#include <system/platform.h>
#include <system/string.h>

#include <cinttypes>

namespace Shipyard
{
    enum class PrimitiveTopology
    {
        LineList,
        LineStrip,
        PointList,
        TriangleList,
        TriangleStrip
    };

    enum class MapFlag
    {
        Read,
        Write,
        Read_Write,
        Write_Discard,
        Write_No_Overwrite
    };

    enum class GfxFormat
    {
        R32G32B32A32_FLOAT,
        R32G32B32A32_UINT,
        R32G32B32A32_SINT,
        R32G32B32_FLOAT,
        R32G32B32_UINT,
        R32G32B32_SINT,
        R16G16B16A16_FLOAT,
        R16G16B16A16_UNORM,
        R16G16B16A16_UINT,
        R16G16B16A16_SNORM,
        R16G16B16A16_SINT,
        R32G32_FLOAT,
        R32G32_UINT,
        R32G32_SINT,
        R10G10B10A2_UNORM,
        R10G10B10A2_UINT,
        R11G11B10_FLOAT,
        R8G8B8A8_UNORM,
        R8G8B8A8_UNORM_SRGB,
        R8G8B8A8_UINT,
        R8G8B8A8_SNORM,
        R8G8B8A8_SINT,
        R16G16_FLOAT,
        R16G16_UNORM,
        R16G16_UINT,
        R16G16_SNORM,
        R16G16_SINT,
        R32_FLOAT,
        R32_UINT,
        R32_SINT,
        R8G8_UNORM,
        R8G8_UINT,
        R8G8_SNORM,
        R8G8_SINT,
        R16_FLOAT,
        R16_UNORM,
        R16_UINT,
        R16_SNORM,
        R16_SINT,
        R8_UNORM,
        R8_UINT,
        R8_SNORM,
        R8_SINT
    };

    enum class FillMode
    {
        Wireframe,
        Solid
    };

    enum class CullMode
    {
        CullNone,
        CullFrontFace,
        CullBackFace
    };

    struct RasterizerState
    {
        FillMode m_FillMode;
        CullMode m_CullMode;
        bool m_IsFrontCounterClockWise;
        int m_DepthBias;
        float m_DepthBiasClamp;
        float m_SlopeScaledDepthBias;
        bool m_DepthClipEnable;
        bool m_ScissorEnable;
        bool m_MultisampleEnable;
        bool m_AntialiasedLineEnable;
    };

    enum class ComparisonFunc
    {
        Never,
        Less,
        Equal,
        LessEqual,
        Greater,
        NotEqual,
        GreaterEqual,
        Always
    };

    enum class StencilOperation
    {
        Keep,
        Zero,
        Replace,
        IncrSaturate,
        DecrSaturate,
        Invert,
        Incr,
        Decr
    };

    struct DepthStencilState
    {
        bool m_DepthEnable;
        bool m_EnableDepthWrite;
        ComparisonFunc m_DepthComparisonFunc;
        bool m_StencilEnable;
        uint8_t m_StencilReadMask;
        uint8_t m_StencilWriteMask;

        StencilOperation m_FrontFaceStencilFailOp;
        StencilOperation m_FrontFaceStencilDepthFailOp;
        StencilOperation m_FrontFaceStencilPassOp;
        ComparisonFunc m_FrontFaceStencilComparisonFunc;

        StencilOperation m_BackFaceStencilFailOp;
        StencilOperation m_BackFaceStencilDepthFailOp;
        StencilOperation m_BackFaceStencilPassOp;
        ComparisonFunc m_BackFaceStencilComparisonFunc;
    };

    enum class SemanticName
    {
        BiNormal,
        Bones,
        Color,
        Normal,
        Position,
        Tangent,
        TexCoord,
        Weights
    };

    struct InputLayout
    {
        SemanticName    m_SemanticName;
        uint32_t        m_SemanticIndex;
        GfxFormat       m_Format;
        uint32_t        m_InputSlot;
        uint32_t        m_ByteOffset;
        bool            m_IsDataPerInstance;
        uint32_t        m_InstanceDataStepRate;

        bool operator!= (const InputLayout& rhs)
        {
            return (m_SemanticName != rhs.m_SemanticName &&
                    m_SemanticIndex != rhs.m_SemanticIndex &&
                    m_Format != rhs.m_Format &&
                    m_InputSlot != rhs.m_InputSlot &&
                    m_ByteOffset != rhs.m_ByteOffset &&
                    m_IsDataPerInstance != rhs.m_IsDataPerInstance &&
                    m_InstanceDataStepRate != rhs.m_InstanceDataStepRate);
        }
    };
}