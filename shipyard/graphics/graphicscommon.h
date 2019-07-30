#pragma once

#include <math/mathutilities.h>

#include <graphics/graphicsconfig.h>

#include <graphics/graphicstypes.h>

#include <system/array.h>
#include <system/bitfield.h>
#include <system/platform.h>
#include <system/string.h>

namespace Shipyard
{

    class DepthStencilRenderTarget;
    class DescriptorSet;
    class PixelShader;
    class RenderTarget;
    class RootSignature;
    class ShaderHandler;
    class VertexShader;

    enum class VertexFormatType : shipUint16;

    enum GfxConstants
    {
        GfxConstants_MaxShaderResourceViewsBoundPerShaderStage = 64,
        GfxConstants_MaxConstantBufferViewsBoundPerShaderStage = 15,
        GfxConstants_MaxUnorderedAccessViewsBoundPerShaderStage = 8,
        GfxConstants_MaxSamplersBoundPerShaderStage = 16,

        GfxConstants_MaxRenderTargetsBound = 8,

        GfxConstants_MaxVertexBuffers = 16,

        GfxConstants_MaxInputLayouts = 16,
    };

    struct BytesArray
    {
        const shipChar* m_Bytes;
        shipUint64 m_BytesLength;
    };

    enum class PrimitiveTopology : shipUint8
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

    enum class DepthStencilClearFlag : shipUint8
    {
        Depth,
        Stencil,
        DepthStencil
    };

    enum class GfxFormat : shipUint16
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
        R8_SINT,

        D16_UNORM,
        D24_UNORM_S8_UINT,
        D32_FLOAT,
        D32_FLOAT_S8X24_UINT,

        Unknown
    };

    shipBool IsDepthStencilFormat(GfxFormat format);

    enum class FillMode : shipUint8
    {
        Wireframe,
        Solid
    };

    enum class CullMode : shipUint8
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

        shipInt32 m_DepthBias;
        shipFloat m_DepthBiasClamp;
        shipFloat m_SlopeScaledDepthBias;

        FillMode m_FillMode;
        CullMode m_CullMode;

        shipBool m_IsFrontCounterClockwise;
        shipBool m_DepthClipEnable;
        shipBool m_ScissorEnable;
        shipBool m_MultisampleEnable;
        shipBool m_AntialiasedLineEnable;

        shipBool operator== (const RasterizerState &rhs) const
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

        shipBool operator!= (const RasterizerState& rhs) const
        {
            return !(*this == rhs);
        }
    };

    enum class ComparisonFunc : shipUint8
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

    enum class StencilOperation : shipUint8
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
        shipUint8 m_StencilReadMask;
        shipUint8 m_StencilWriteMask;

        StencilOperation m_FrontFaceStencilFailOp;
        StencilOperation m_FrontFaceStencilDepthFailOp;
        StencilOperation m_FrontFaceStencilPassOp;
        ComparisonFunc m_FrontFaceStencilComparisonFunc;

        StencilOperation m_BackFaceStencilFailOp;
        StencilOperation m_BackFaceStencilDepthFailOp;
        StencilOperation m_BackFaceStencilPassOp;
        ComparisonFunc m_BackFaceStencilComparisonFunc;

        shipBool m_DepthEnable;
        shipBool m_EnableDepthWrite;
        shipBool m_StencilEnable;

        shipBool operator== (const DepthStencilState& rhs) const
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

        shipBool operator!= (const DepthStencilState& rhs) const
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
        shipUint32        m_SemanticIndex;
        GfxFormat       m_Format;
        shipUint32        m_InputSlot;
        shipUint32        m_ByteOffset;
        shipBool            m_IsDataPerInstance;
        shipUint32        m_InstanceDataStepRate;

        shipBool operator!= (const InputLayout& rhs)
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

    enum class BlendFactor : shipUint8
    {
        Zero,
        One,
        SrcColor,
        InvSrcColor,
        SrcAlpha,
        InvSrcAlpha,
        DestAlpha,
        InvDestAlpha,
        DestColor,
        InvDestColor,
        SrcAlphaSat,
        UserFactor,
        InvUserFactor,
        DualSrcColor,
        DualInvSrcColor,
        DualSrcAlpha,
        DualInvSrcAlpha
    };

    enum class BlendOperator : shipUint8
    {
        Add,
        Subtract,
        ReverseSubstract,
        Min,
        Max
    };

    enum RenderTargetWriteMask : shipUint8
    {
        RenderTargetWriteMask_None = 0x00,
        RenderTargetWriteMask_R = 0x01,
        RenderTargetWriteMask_G = 0x02,
        RenderTargetWriteMask_B = 0x04,
        RenderTargetWriteMask_A = 0x08,

        RenderTargetWriteMask_RGBA = (RenderTargetWriteMask_R | RenderTargetWriteMask_G | RenderTargetWriteMask_B | RenderTargetWriteMask_A),
    };

    struct RenderTargetBlendState
    {
        shipBool m_BlendEnable = false;
        BlendFactor m_SourceBlend = BlendFactor::One;
        BlendFactor m_DestBlend = BlendFactor::Zero;
        BlendOperator m_BlendOperator = BlendOperator::Add;
        BlendFactor m_SourceAlphaBlend = BlendFactor::One;
        BlendFactor m_DestAlphaBlend = BlendFactor::Zero;
        BlendOperator m_AlphaBlendOperator = BlendOperator::Add;
        RenderTargetWriteMask m_RenderTargetWriteMask = RenderTargetWriteMask::RenderTargetWriteMask_RGBA;

        shipBool operator== (const RenderTargetBlendState& rhs) const
        {
            return (m_BlendEnable == rhs.m_BlendEnable &&
                    m_SourceBlend == rhs.m_SourceBlend &&
                    m_DestBlend == rhs.m_DestBlend &&
                    m_BlendOperator == rhs.m_BlendOperator &&
                    m_SourceAlphaBlend == rhs.m_SourceAlphaBlend &&
                    m_DestAlphaBlend == rhs.m_DestAlphaBlend &&
                    m_AlphaBlendOperator == rhs.m_AlphaBlendOperator &&
                    m_RenderTargetWriteMask == rhs.m_RenderTargetWriteMask);
        }

        shipBool operator!= (const RenderTargetBlendState& rhs) const
        {
            return !(*this == rhs);
        }
    };

    struct BlendState
    {
        RenderTargetBlendState renderTargetBlendStates[GfxConstants::GfxConstants_MaxRenderTargetsBound];

        // Those are only used if at least one render target uses a BlendFactor of BlendFactor::UserFactor or BlendFactor::InvUserFactor
        shipFloat m_RedBlendUserFactor = 1.0f;
        shipFloat m_GreenBlendUserFactor = 1.0f;
        shipFloat m_BlueBlendUserFactor = 1.0f;
        shipFloat m_AlphaBlendUserFactor = 1.0f;

        shipBool m_AlphaToCoverageEnable = false;
        shipBool m_IndependentBlendEnable = false;

        shipBool operator== (const BlendState& rhs) const
        {
            for (shipUint32 i = 0; i < GfxConstants::GfxConstants_MaxRenderTargetsBound; i++)
            {
                if (renderTargetBlendStates[i] != rhs.renderTargetBlendStates[i])
                {
                    return false;
                }
            }

            return (IsAlmostEqual(m_RedBlendUserFactor, rhs.m_RedBlendUserFactor) &&
                    IsAlmostEqual(m_GreenBlendUserFactor, rhs.m_GreenBlendUserFactor) &&
                    IsAlmostEqual(m_BlueBlendUserFactor, rhs.m_BlueBlendUserFactor) &&
                    IsAlmostEqual(m_AlphaBlendUserFactor, rhs.m_AlphaBlendUserFactor) &&
                    m_AlphaToCoverageEnable == rhs.m_AlphaToCoverageEnable &&
                    m_IndependentBlendEnable == rhs.m_IndependentBlendEnable);
        }

        shipBool operator!= (const BlendState& rhs) const
        {
            return !(*this == rhs);
        }
    };

    enum class SamplingFilter : shipUint8
    {
        Nearest,
        Linear
    };

    enum class TextureAddressMode : shipUint8
    {
        Wrap,
        Mirror,
        Clamp,
        Border
    };

    struct SamplerState
    {
        SamplingFilter MinificationFiltering = SamplingFilter::Nearest;
        SamplingFilter MagnificationFiltering = SamplingFilter::Nearest;
        SamplingFilter MipmapFiltering = SamplingFilter::Nearest;
        TextureAddressMode AddressModeU = TextureAddressMode::Clamp;
        TextureAddressMode AddressModeV = TextureAddressMode::Clamp;
        TextureAddressMode AddressModeW = TextureAddressMode::Clamp;
        ComparisonFunc ComparisonFunction = ComparisonFunc::Never;

        shipFloat MipLodBias = 0.0f;
        shipUint32 MaxAnisotropy = 1;
        shipFloat BorderRGBA[4];
        shipFloat MinLod = 0.0f;
        shipFloat MaxLod = 100.0f;

        // If set to true, the 3 sampling filter methods specified in this struct will be ignored.
        shipBool UseAnisotropicFiltering = false;

        shipBool operator== (const SamplerState &rhs) const
        {
            return (MinificationFiltering == rhs.MinificationFiltering &&
                    MagnificationFiltering == rhs.MagnificationFiltering &&
                    MipmapFiltering == rhs.MipmapFiltering &&
                    AddressModeU == rhs.AddressModeU &&
                    AddressModeV == rhs.AddressModeV &&
                    AddressModeW == rhs.AddressModeW &&
                    ComparisonFunction == rhs.ComparisonFunction &&
                    IsAlmostEqual(MipLodBias, rhs.MipLodBias) &&
                    UseAnisotropicFiltering == rhs.UseAnisotropicFiltering &&
                    (UseAnisotropicFiltering == false || MaxAnisotropy == rhs.MaxAnisotropy) &&
                    IsAlmostEqual(BorderRGBA[0], rhs.BorderRGBA[0]) &&
                    IsAlmostEqual(BorderRGBA[1], rhs.BorderRGBA[1]) &&
                    IsAlmostEqual(BorderRGBA[2], rhs.BorderRGBA[2]) &&
                    IsAlmostEqual(BorderRGBA[3], rhs.BorderRGBA[3]) &&
                    IsAlmostEqual(MinLod, rhs.MinLod) &&
                    IsAlmostEqual(MaxLod, rhs.MaxLod));
        }
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
                ShaderVisibility_Compute,
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
        shipUint32 numDescriptors;
        shipUint32 baseShaderRegister;
        shipUint32 registerSpace;
        shipUint32 offsetInDescriptorsFromTableStart;
    };

    struct RootDescriptorTable
    {
        Array<DescriptorRange> descriptorRanges;
    };

    struct RootDescriptor
    {
        RootDescriptor()
            : shaderBindingSlot(0)
            , registerSpace(0)
        {
        }

        shipUint32 shaderBindingSlot;
        shipUint32 registerSpace;
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
        BlendState blendState;
    };

    struct PipelineStateObjectCreationParameters
    {
        PipelineStateObjectCreationParameters()
            : rootSignature(nullptr)
            , numRenderTargets(0)
        {
        }

        const RootSignature* rootSignature;

        GFXVertexShaderHandle vertexShaderHandle;
        GFXPixelShaderHandle pixelShaderHandle;

        RenderStateBlock renderStateBlock;
        VertexFormatType vertexFormatType;
        PrimitiveTopology primitiveTopology;

        shipUint32 numRenderTargets;
        GfxFormat renderTargetsFormat[GfxConstants::GfxConstants_MaxRenderTargetsBound];
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

    struct DescriptorSetEntryDeclaration
    {
        static const shipUint16 InvalidDescriptorRangeIndex = shipUint16(-1);

        shipUint16 rootIndex = 0;
        shipUint16 numResources = 0;

        // Values other than InvalidDescriptorRangeIndex are considered to be to index inside of a descriptor table at rootIndex.
        shipUint16 descriptorRangeIndex = InvalidDescriptorRangeIndex;
    };

    // This class gives control to the programmer to override specific RenderStateBlock's state manually.
    // Those overridden states will have priority over every other way to set a state (inside of the shader for example).
    class SHIPYARD_API RenderStateBlockStateOverride
    {
    public:
        void ApplyOverridenValues(RenderStateBlock& renderStateBlock) const;

        void OverrideDepthBiasState(shipInt32 overrideValue);
        void OverrideDepthBiasClampState(shipFloat overrideValue);
        void OverrideSlopeScaledDepthBiasState(shipFloat overrideValue);

        void OverrideFillModeState(FillMode overrideValue);
        void OverrideCullModeState(CullMode overrideValue);

        void OverrideIsFrontCounterClockwiseState(shipBool overrideValue);
        void OverrideDepthClipEnableState(shipBool overrideValue);
        void OverrideScissorEnableState(shipBool overrideValue);
        void OverrideMultisampleEnableState(shipBool overrideValue);
        void OverrideAntialiasedLineEnableState(shipBool overrideValue);

        void OverrideDepthComparisonFuncState(ComparisonFunc overrideValue);
        void OverrideStencilReadMaskState(shipUint8 overrideValue);
        void OverrideStencilWriteMaskState(shipUint8 overrideValue);

        void OverrideFrontFaceStencilFailOpState(StencilOperation overrideValue);
        void OverrideFrontFaceStencilDepthFailOpState(StencilOperation overrideValue);
        void OverrideFrontFaceStencilPassOpState(StencilOperation overrideValue);
        void OverrideFrontFaceStencilComparisonFuncState(ComparisonFunc overrideValue);

        void OverrideBackFaceStencilFailOpState(StencilOperation overrideValue);
        void OverrideBackFaceStencilDepthFailOpState(StencilOperation overrideValue);
        void OverrideBackFaceStencilPassOpState(StencilOperation overrideValue);
        void OverrideBackFaceStencilComparisonFuncState(ComparisonFunc overrideValue);

        void OverrideDepthEnableState(shipBool overrideValue);
        void OverrideEnableDepthWriteState(shipBool overrideValue);
        void OverrideStencilEnableState(shipBool overrideValue);

        void OverrideRedBlendUserFactor(shipFloat overrideValue);
        void OverrideGreenBlendUserFactor(shipFloat overrideValue);
        void OverrideBlueBlendUserFactor(shipFloat overrideValue);
        void OverrideAlphaBlendUserFactor(shipFloat overrideValue);

        void OverrideAlphaToCoverageEnable(shipBool overrideValue);
        void OverrideIndependentBlendEnable(shipBool overrideValue);

        void OverrideBlendEnable(shipBool overrideValue, shipUint32 renderTargetIndex);
        void OverrideSourceBlend(BlendFactor overrideValue, shipUint32 renderTargetIndex);
        void OverrideDestBlend(BlendFactor overrideValue, shipUint32 renderTargetIndex);
        void OverrideBlendOperator(BlendOperator overrideValue, shipUint32 renderTargetIndex);
        void OverrideSourceAlphaBlend(BlendFactor overrideValue, shipUint32 renderTargetIndex);
        void OverrideDestAlphaBlend(BlendFactor overrideValue, shipUint32 renderTargetIndex);
        void OverrideAlphaBlendOperator(BlendOperator overrideValue, shipUint32 renderTargetIndex);
        void OverrideRenderTargetWriteMask(RenderTargetWriteMask overrideValue, shipUint32 renderTargetIndex);

    private:
        InplaceBitfield<128> m_OverridenState;
        RenderStateBlock m_RenderStateBlockOverride;
    };

    enum TextureUsage : shipUint8
    {
        TextureUsage_Default = 0x00,
        TextureUsage_RenderTarget = 0x01,
        TextureUsage_DepthStencil = 0x02
    };

    enum class GfxResourceType
    {
        ConstantBuffer,
        ByteBuffer,
        Texture,

        Unknown
    };

    struct GfxViewport
    {
        shipFloat topLeftX = 0.0f;
        shipFloat topLeftY = 0.0f;
        shipFloat width = 1.0f;
        shipFloat height = 1.0f;
        shipFloat minDepth = 0.0f;
        shipFloat maxDepth = 1.0f;

        shipBool operator== (const GfxViewport& rhs) const
        {
            return (IsAlmostEqual(topLeftX, rhs.topLeftX) &&
                    IsAlmostEqual(topLeftY, rhs.topLeftY) &&
                    IsAlmostEqual(width, rhs.width) &&
                    IsAlmostEqual(height, rhs.height) &&
                    IsAlmostEqual(minDepth, rhs.minDepth) &&
                    IsAlmostEqual(maxDepth, rhs.maxDepth));
        }

        shipBool operator!= (const GfxViewport& rhs) const
        {
            return (!IsAlmostEqual(topLeftX, rhs.topLeftX) ||
                    !IsAlmostEqual(topLeftY, rhs.topLeftY) ||
                    !IsAlmostEqual(width, rhs.width) ||
                    !IsAlmostEqual(height, rhs.height) ||
                    !IsAlmostEqual(minDepth, rhs.minDepth) ||
                    !IsAlmostEqual(maxDepth, rhs.maxDepth));
        }
    };

    enum class CommandQueueType : shipUint8
    {
        Direct,
        Compute,
        Copy
    };

    ShaderVisibility GetShaderVisibilityForShaderStage(ShaderStage shaderStage);
    ShaderStage GetShaderStageForShaderVisibility(ShaderVisibility shaderVisibility);
}