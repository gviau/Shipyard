#pragma once

#include <system/array.h>
#include <system/platform.h>
#include <system/string.h>

#include <cinttypes>

namespace Shipyard
{
    class RootSignature;

    enum class VertexFormatType : uint16_t;

    struct BytesArray
    {
        const char* m_Bytes;
        uint64_t m_BytesLength;
    };

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
        R8G8B8_UNORM,
        R8G8B8_SNORM,
        R8G8B8_UINT,
        R8G8B8_SINT,
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

    enum class BlendFactor
    {

    };

    enum class BlendOperator
    {
        Add,
        Subtract,
        ReverseSubstract,
        Min,
        Max
    };

    struct RenderTargetBlendState
    {

    };

    enum class RootSignatureParameterType
    {
        DescriptorTable,

        Constant,

        ConstantBufferView,
        ShaderResourceView,
        UnorderedAccessView,

        Unknown
    };

    enum ShaderVisibility
    {
        ShaderVisibility_Unknown = 0x00,

        ShaderVisibility_Vertex = 0x01,
        ShaderVisibility_Pixel = 0x02,
        ShaderVisibility_Hull = 0x04,
        ShaderVisibility_Domain = 0x08,
        ShaderVisibility_Geometry = 0x10,
        ShaderVisibility_Compute = 0x20,

        ShaderVisibility_All =
                ShaderVisibility_Vertex |
                ShaderVisibility_Pixel |
                ShaderVisibility_Hull |
                ShaderVisibility_Domain |
                ShaderVisibility_Geometry |
                ShaderVisibility_Compute
    };

    enum class DescriptorRangeType
    {
        ShaderResourceView,
        UnorderedAccessView,
        ConstantBufferView,
        Sampler,

        Unknown
    };

    struct DescriptorRange
    {
        DescriptorRange()
            : descriptorRangeType(DescriptorRangeType::Unknown)
            , numDescriptors(0)
            , baseShaderRegister(0)
            , registerSpace(0)
            , offsetInDescriptorsFromTableStart(0)
        {
        }

        DescriptorRangeType descriptorRangeType;
        uint32_t numDescriptors;
        uint32_t baseShaderRegister;
        uint32_t registerSpace;
        uint32_t offsetInDescriptorsFromTableStart;
    };

    struct RootDescriptorTable
    {
        RootDescriptorTable()
            : numDescriptorRanges(0)
            , descriptorRanges(nullptr)
        {
        }

        uint32_t numDescriptorRanges;
        const DescriptorRange* descriptorRanges;
    };

    struct RootDescriptor
    {
        RootDescriptor()
            : shaderBindingSlot(0)
            , registerSpace(0)
        {
        }

        uint32_t shaderBindingSlot;
        uint32_t registerSpace;
    };

    struct RootSignatureParameterEntry
    {
        RootSignatureParameterEntry()
            : parameterType(RootSignatureParameterType::Unknown)
            , shaderVisibility(ShaderVisibility::ShaderVisibility_Unknown)
        {
        }

        RootSignatureParameterType parameterType;

        union
        {
            RootDescriptorTable descriptorTable;
            RootDescriptor descriptor;
        };

        ShaderVisibility shaderVisibility;
    };

    struct PipelineStateObjectCreationParameters
    {
        PipelineStateObjectCreationParameters(RootSignature& rootSignatureToUse)
            : rootSignature(rootSignatureToUse)
            , numRenderTargets(0)
        {
        }

        RootSignature& rootSignature;

        RasterizerState rasterizerState;
        DepthStencilState depthStencilState;
        VertexFormatType vertexFormatType;
        PrimitiveTopology primitiveTopology;

        uint32_t numRenderTargets;
        GfxFormat renderTargetsFormat[8];
        GfxFormat depthStencilFormat;
    };

    enum class DescriptorSetType
    {
        ConstantBuffer_ShaderResource_UnorderedAccess_Views,
        Samplers,
        RenderTargetViews,
        DepthStencilView,

        Unknown
    };

    enum class GfxResourceType
    {
        ConstantBuffer,
        Texture,

        Unknown
    };
}