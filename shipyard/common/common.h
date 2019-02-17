#pragma once

#include <math/mathutilities.h>

#include <system/array.h>
#include <system/platform.h>
#include <system/string.h>

#include <cinttypes>

namespace Shipyard
{
    class PixelShader;
    class RootSignature;
    class VertexShader;

    enum class VertexFormatType : uint16_t;

    struct BytesArray
    {
        const char* m_Bytes;
        uint64_t m_BytesLength;
    };

    enum class PrimitiveTopology : uint8_t
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

    enum class GfxFormat : uint16_t
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

    enum class FillMode : uint8_t
    {
        Wireframe,
        Solid
    };

    enum class CullMode : uint8_t
    {
        CullNone,
        CullFrontFace,
        CullBackFace
    };

    struct RasterizerState
    {
        RasterizerState()
            : m_DepthBias(0)
            , m_DepthBiasClamp(0.0f)
            , m_SlopeScaledDepthBias(0.0f)

            , m_FillMode(FillMode::Solid)
            , m_CullMode(CullMode::CullBackFace)

            , m_IsFrontCounterClockwise(false)
            , m_DepthClipEnable(false)
            , m_ScissorEnable(false)
            , m_MultisampleEnable(false)
            , m_AntialiasedLineEnable(false)
        {

        }

        int m_DepthBias;
        float m_DepthBiasClamp;
        float m_SlopeScaledDepthBias;

        FillMode m_FillMode;
        CullMode m_CullMode;

        bool m_IsFrontCounterClockwise;
        bool m_DepthClipEnable;
        bool m_ScissorEnable;
        bool m_MultisampleEnable;
        bool m_AntialiasedLineEnable;

        bool operator== (const RasterizerState &rhs) const
        {
            return (m_DepthBias == rhs.m_DepthBias &&
                    IsAlmostEqual(m_DepthBiasClamp, rhs.m_DepthBiasClamp) &&
                    IsAlmostEqual(m_SlopeScaledDepthBias, rhs.m_SlopeScaledDepthBias) &&
                    m_FillMode == rhs.m_FillMode &&
                    m_CullMode == rhs.m_CullMode &&
                    m_IsFrontCounterClockwise == rhs.m_IsFrontCounterClockwise &&
                    m_DepthClipEnable == rhs.m_DepthClipEnable &&
                    m_ScissorEnable == rhs.m_ScissorEnable &&
                    m_MultisampleEnable == rhs.m_MultisampleEnable &&
                    m_AntialiasedLineEnable == rhs.m_AntialiasedLineEnable);
        }

        bool operator!= (const RasterizerState& rhs) const
        {
            return !(*this == rhs);
        }
    };

    enum class ComparisonFunc : uint8_t
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

    enum class StencilOperation : uint8_t
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
        DepthStencilState()
            : m_DepthComparisonFunc(ComparisonFunc::LessEqual)
            , m_StencilReadMask(0)
            , m_StencilWriteMask(0)

            , m_FrontFaceStencilFailOp(StencilOperation::Keep)
            , m_FrontFaceStencilDepthFailOp(StencilOperation::Keep)
            , m_FrontFaceStencilPassOp(StencilOperation::Keep)
            , m_FrontFaceStencilComparisonFunc(ComparisonFunc::Always)

            , m_BackFaceStencilFailOp(StencilOperation::Keep)
            , m_BackFaceStencilDepthFailOp(StencilOperation::Keep)
            , m_BackFaceStencilPassOp(StencilOperation::Keep)
            , m_BackFaceStencilComparisonFunc(ComparisonFunc::Always)

            , m_DepthEnable(true)
            , m_EnableDepthWrite(true)
            , m_StencilEnable(false)
        {
        }

        ComparisonFunc m_DepthComparisonFunc;
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

        bool m_DepthEnable;
        bool m_EnableDepthWrite;
        bool m_StencilEnable;

        bool operator== (const DepthStencilState& rhs) const
        {
            return (m_DepthComparisonFunc == rhs.m_DepthComparisonFunc &&
                    m_StencilReadMask == rhs.m_StencilReadMask &&
                    m_StencilWriteMask == rhs.m_StencilWriteMask &&
                    m_FrontFaceStencilFailOp == rhs.m_FrontFaceStencilFailOp &&
                    m_FrontFaceStencilDepthFailOp == rhs.m_FrontFaceStencilDepthFailOp &&
                    m_FrontFaceStencilPassOp == rhs.m_FrontFaceStencilPassOp &&
                    m_FrontFaceStencilComparisonFunc == m_FrontFaceStencilComparisonFunc &&
                    m_BackFaceStencilFailOp == rhs.m_BackFaceStencilFailOp &&
                    m_BackFaceStencilDepthFailOp == rhs.m_BackFaceStencilDepthFailOp &&
                    m_BackFaceStencilPassOp == rhs.m_BackFaceStencilPassOp &&
                    m_BackFaceStencilComparisonFunc == m_BackFaceStencilComparisonFunc &&
                    m_DepthEnable == rhs.m_DepthEnable &&
                    m_EnableDepthWrite == rhs.m_EnableDepthWrite &&
                    m_StencilEnable == rhs.m_StencilEnable);
        }

        bool operator!= (const DepthStencilState& rhs) const
        {
            return !(*this == rhs);
        }
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
        ShaderVisibility_None = 0x00,

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

    enum ShaderStage
    {
        ShaderStage_Vertex,
        ShaderStage_Pixel,
        ShaderStage_Hull,
        ShaderStage_Domain,
        ShaderStage_Geometry,
        ShaderStage_Compute,

        ShaderStage_Count
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
            , shaderVisibility(ShaderVisibility::ShaderVisibility_None)
        {
        }

        RootSignatureParameterType parameterType;

        RootDescriptorTable descriptorTable;
        RootDescriptor descriptor;

        ShaderVisibility shaderVisibility;
    };

    struct RenderStateBlock
    {
        RasterizerState rasterizerState;
        DepthStencilState depthStencilState;
    };

    struct PipelineStateObjectCreationParameters
    {
        PipelineStateObjectCreationParameters(RootSignature& rootSignatureToUse)
            : rootSignature(rootSignatureToUse)
            , vertexShader(nullptr)
            , pixelShader(nullptr)
            , numRenderTargets(0)
        {
        }

        RootSignature& rootSignature;

        VertexShader* vertexShader;
        PixelShader* pixelShader;

        RenderStateBlock renderStateBlock;
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

    enum GfxConstants
    {
        GfxConstants_MaxShaderResourceViewsBoundPerShaderStage = 64,
        GfxConstants_MaxConstantBufferViewsBoundPerShaderStage = 15,
        GfxConstants_MaxUnorderedAccessViewsBoundPerShaderStage = 8,
        GfxConstatns_MaxSamplersBoundPerShaderStage = 16
    };

    struct GfxViewport
    {
        GfxViewport()
            : topLeftX(0.0f)
            , topLeftY(0.0f)
            , width(1.0f)
            , height(1.0f)
            , minDepth(0.0f)
            , maxDepth(1.0f)
        {
        }

        float topLeftX;
        float topLeftY;
        float width;
        float height;
        float minDepth;
        float maxDepth;
    };
}