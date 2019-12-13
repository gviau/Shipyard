#include <graphics/graphicsprecomp.h>

#include <graphics/wrapper/dx11/dx11_common.h>

namespace Shipyard
{;

D3D11_PRIMITIVE_TOPOLOGY ConvertShipyardPrimitiveTopologyToDX11(PrimitiveTopology primitiveTopology)
{
    D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

    switch (primitiveTopology)
    {
    case PrimitiveTopology::LineList: topology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST; break;
    case PrimitiveTopology::LineStrip: topology = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP; break;
    case PrimitiveTopology::PointList: topology = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST; break;
    case PrimitiveTopology::TriangleList: topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST; break;
    case PrimitiveTopology::TriangleStrip: topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP; break;
    }

    return topology;
}

DXGI_FORMAT ConvertShipyardFormatToDX11(GfxFormat format)
{
    switch (format)
    {
        case GfxFormat::R32G32B32A32_FLOAT:     return DXGI_FORMAT_R32G32B32A32_FLOAT;
        case GfxFormat::R32G32B32A32_UINT:      return DXGI_FORMAT_R32G32B32A32_UINT;
        case GfxFormat::R32G32B32A32_SINT:      return DXGI_FORMAT_R32G32B32A32_SINT;
        case GfxFormat::R32G32B32_FLOAT:        return DXGI_FORMAT_R32G32B32_FLOAT;
        case GfxFormat::R32G32B32_UINT:         return DXGI_FORMAT_R32G32B32_UINT;
        case GfxFormat::R32G32B32_SINT:         return DXGI_FORMAT_R32G32B32_SINT;
        case GfxFormat::R16G16B16A16_FLOAT:     return DXGI_FORMAT_R16G16B16A16_FLOAT;
        case GfxFormat::R16G16B16A16_UNORM:     return DXGI_FORMAT_R16G16B16A16_UNORM;
        case GfxFormat::R16G16B16A16_UINT:      return DXGI_FORMAT_R16G16B16A16_UINT;
        case GfxFormat::R16G16B16A16_SNORM:     return DXGI_FORMAT_R16G16B16A16_SNORM;
        case GfxFormat::R16G16B16A16_SINT:      return DXGI_FORMAT_R16G16B16A16_SINT;
        case GfxFormat::R32G32_FLOAT:           return DXGI_FORMAT_R32G32_FLOAT;
        case GfxFormat::R32G32_UINT:            return DXGI_FORMAT_R32G32_UINT;
        case GfxFormat::R32G32_SINT:            return DXGI_FORMAT_R32G32_SINT;
        case GfxFormat::R10G10B10A2_UNORM:      return DXGI_FORMAT_R10G10B10A2_UNORM;
        case GfxFormat::R10G10B10A2_UINT:       return DXGI_FORMAT_R10G10B10A2_UINT;
        case GfxFormat::R11G11B10_FLOAT:        return DXGI_FORMAT_R11G11B10_FLOAT;
        case GfxFormat::R8G8B8A8_UNORM:         return DXGI_FORMAT_R8G8B8A8_UNORM;
        case GfxFormat::R8G8B8A8_UNORM_SRGB:    return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        case GfxFormat::R8G8B8A8_UINT:          return DXGI_FORMAT_R8G8B8A8_UINT;
        case GfxFormat::R8G8B8A8_SNORM:         return DXGI_FORMAT_R8G8B8A8_SNORM;
        case GfxFormat::R8G8B8A8_SINT:          return DXGI_FORMAT_R8G8B8A8_SINT;
        case GfxFormat::R16G16_FLOAT:           return DXGI_FORMAT_R16G16_FLOAT;
        case GfxFormat::R16G16_UNORM:           return DXGI_FORMAT_R16G16_UNORM;
        case GfxFormat::R16G16_UINT:            return DXGI_FORMAT_R16G16_UINT;
        case GfxFormat::R16G16_SNORM:           return DXGI_FORMAT_R16G16_SNORM;
        case GfxFormat::R16G16_SINT:            return DXGI_FORMAT_R16G16_SINT;
        case GfxFormat::R32_FLOAT:              return DXGI_FORMAT_R32_FLOAT;
        case GfxFormat::R32_UINT:               return DXGI_FORMAT_R32_UINT;
        case GfxFormat::R32_SINT:               return DXGI_FORMAT_R32_SINT;
        case GfxFormat::R8G8_UNORM:             return DXGI_FORMAT_R8G8_UNORM;
        case GfxFormat::R8G8_UINT:              return DXGI_FORMAT_R8G8_UINT;
        case GfxFormat::R8G8_SNORM:             return DXGI_FORMAT_R8G8_SNORM;
        case GfxFormat::R8G8_SINT:              return DXGI_FORMAT_R8G8_SINT;
        case GfxFormat::R16_FLOAT:              return DXGI_FORMAT_R16_FLOAT;
        case GfxFormat::R16_UNORM:              return DXGI_FORMAT_R16_UNORM;
        case GfxFormat::R16_UINT:               return DXGI_FORMAT_R16_UINT;
        case GfxFormat::R16_SNORM:              return DXGI_FORMAT_R16_SNORM;
        case GfxFormat::R16_SINT:               return DXGI_FORMAT_R16_SINT;
        case GfxFormat::R8_UNORM:               return DXGI_FORMAT_R8_UNORM;
        case GfxFormat::R8_UINT:                return DXGI_FORMAT_R8_UINT;
        case GfxFormat::R8_SNORM:               return DXGI_FORMAT_R8_SNORM;
        case GfxFormat::R8_SINT:                return DXGI_FORMAT_R8_SINT;
        case GfxFormat::D16_UNORM:              return DXGI_FORMAT_D16_UNORM;
        case GfxFormat::D24_UNORM_S8_UINT:      return DXGI_FORMAT_D24_UNORM_S8_UINT;
        case GfxFormat::D32_FLOAT:              return DXGI_FORMAT_D32_FLOAT;
        case GfxFormat::D32_FLOAT_S8X24_UINT:   return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
        case GfxFormat::Unknown:                return DXGI_FORMAT_UNKNOWN;

        default:
            SHIP_ASSERT(false);
    }

    return DXGI_FORMAT_UNKNOWN;
}

D3D11_FILL_MODE ConvertShipyardFillModeToDX11(FillMode fillMode)
{
    switch (fillMode)
    {
    case FillMode::Solid:       return D3D11_FILL_SOLID;
    case FillMode::Wireframe:   return D3D11_FILL_WIREFRAME;
    default:
        SHIP_ASSERT(false);
    }

    return D3D11_FILL_SOLID;
}

D3D11_CULL_MODE ConvertShipyardCullModeToDX11(CullMode cullMode)
{
    switch (cullMode)
    {
    case CullMode::CullNone:        return D3D11_CULL_NONE;
    case CullMode::CullFrontFace:   return D3D11_CULL_FRONT;
    case CullMode::CullBackFace:    return D3D11_CULL_BACK;
    default:
        SHIP_ASSERT(false);
    }

    return D3D11_CULL_NONE;
}

D3D11_COMPARISON_FUNC ConvertShipyardComparisonFuncToDX11(ComparisonFunc func)
{
    D3D11_COMPARISON_FUNC comparisonFunc = D3D11_COMPARISON_ALWAYS;
    switch (func)
    {
    case ComparisonFunc::Always:        comparisonFunc = D3D11_COMPARISON_ALWAYS; break;
    case ComparisonFunc::Equal:         comparisonFunc = D3D11_COMPARISON_EQUAL; break;
    case ComparisonFunc::Greater:       comparisonFunc = D3D11_COMPARISON_GREATER; break;
    case ComparisonFunc::GreaterEqual:  comparisonFunc = D3D11_COMPARISON_GREATER_EQUAL; break;
    case ComparisonFunc::Less:          comparisonFunc = D3D11_COMPARISON_LESS; break;
    case ComparisonFunc::LessEqual:     comparisonFunc = D3D11_COMPARISON_LESS_EQUAL; break;
    case ComparisonFunc::Never:         comparisonFunc = D3D11_COMPARISON_NEVER; break;
    case ComparisonFunc::NotEqual:      comparisonFunc = D3D11_COMPARISON_NOT_EQUAL; break;
    }

    return comparisonFunc;
}

D3D11_STENCIL_OP ConvertShipyardStencilOperationToDX11(StencilOperation operation)
{
    D3D11_STENCIL_OP stencilOp = D3D11_STENCIL_OP_KEEP;

    switch (operation)
    {
    case StencilOperation::Decr:            stencilOp = D3D11_STENCIL_OP_DECR; break;
    case StencilOperation::DecrSaturate:    stencilOp = D3D11_STENCIL_OP_DECR_SAT; break;
    case StencilOperation::Incr:            stencilOp = D3D11_STENCIL_OP_INCR; break;
    case StencilOperation::IncrSaturate:    stencilOp = D3D11_STENCIL_OP_INCR_SAT; break;
    case StencilOperation::Invert:          stencilOp = D3D11_STENCIL_OP_INVERT; break;
    case StencilOperation::Keep:            stencilOp = D3D11_STENCIL_OP_KEEP; break;
    case StencilOperation::Replace:         stencilOp = D3D11_STENCIL_OP_REPLACE; break;
    case StencilOperation::Zero:            stencilOp = D3D11_STENCIL_OP_ZERO; break;
    }

    return stencilOp;
}

D3D11_BLEND ConvertShipyardBlendFactorToDX11(BlendFactor blendFactor)
{
    D3D11_BLEND blend = D3D11_BLEND_ONE;

    switch (blendFactor)
    {
    case BlendFactor::Zero:             blend = D3D11_BLEND_ZERO; break;
    case BlendFactor::One:              blend = D3D11_BLEND_ONE; break;
    case BlendFactor::SrcColor:         blend = D3D11_BLEND_SRC_COLOR; break;
    case BlendFactor::InvSrcColor:      blend = D3D11_BLEND_INV_SRC_COLOR; break;
    case BlendFactor::SrcAlpha:         blend = D3D11_BLEND_SRC_ALPHA; break;
    case BlendFactor::InvSrcAlpha:      blend = D3D11_BLEND_INV_SRC_ALPHA; break;
    case BlendFactor::DestAlpha:        blend = D3D11_BLEND_DEST_ALPHA; break;
    case BlendFactor::InvDestAlpha:     blend = D3D11_BLEND_INV_DEST_ALPHA; break;
    case BlendFactor::DestColor:        blend = D3D11_BLEND_DEST_COLOR; break;
    case BlendFactor::InvDestColor:     blend = D3D11_BLEND_INV_DEST_COLOR; break;
    case BlendFactor::SrcAlphaSat:      blend = D3D11_BLEND_SRC_ALPHA_SAT; break;
    case BlendFactor::UserFactor:       blend = D3D11_BLEND_BLEND_FACTOR; break;
    case BlendFactor::InvUserFactor:    blend = D3D11_BLEND_INV_BLEND_FACTOR; break;
    case BlendFactor::DualSrcColor:     blend = D3D11_BLEND_SRC1_COLOR; break;
    case BlendFactor::DualInvSrcColor:  blend = D3D11_BLEND_INV_SRC1_COLOR; break;
    case BlendFactor::DualSrcAlpha:     blend = D3D11_BLEND_SRC1_ALPHA; break;
    case BlendFactor::DualInvSrcAlpha:  blend = D3D11_BLEND_INV_SRC1_ALPHA; break;
    }

    return blend;
}

D3D11_BLEND_OP ConvertShipyardBlendOperatorToDX11(BlendOperator blendOperator)
{
    D3D11_BLEND_OP blendOp = D3D11_BLEND_OP_ADD;

    switch (blendOperator)
    {
    case Shipyard::BlendOperator::Add:                  blendOp = D3D11_BLEND_OP_ADD; break;
    case Shipyard::BlendOperator::Subtract:             blendOp = D3D11_BLEND_OP_SUBTRACT; break;
    case Shipyard::BlendOperator::ReverseSubstract:     blendOp = D3D11_BLEND_OP_REV_SUBTRACT; break;
    case Shipyard::BlendOperator::Min:                  blendOp = D3D11_BLEND_OP_MIN; break;
    case Shipyard::BlendOperator::Max:                  blendOp = D3D11_BLEND_OP_MAX; break;
    }

    return blendOp;
}

shipUint8 ConvertShipyardRenderTargetWriteMaskToDX11(RenderTargetWriteMask renderTargetWriteMask)
{
    shipUint8 mask = 0;

    if ((renderTargetWriteMask & RenderTargetWriteMask::RenderTargetWriteMask_R) > 0)
    {
        mask |= D3D11_COLOR_WRITE_ENABLE_RED;
    }

    if ((renderTargetWriteMask & RenderTargetWriteMask::RenderTargetWriteMask_G) > 0)
    {
        mask |= D3D11_COLOR_WRITE_ENABLE_GREEN;
    }

    if ((renderTargetWriteMask & RenderTargetWriteMask::RenderTargetWriteMask_B) > 0)
    {
        mask |= D3D11_COLOR_WRITE_ENABLE_BLUE;
    }

    if ((renderTargetWriteMask & RenderTargetWriteMask::RenderTargetWriteMask_A) > 0)
    {
        mask |= D3D11_COLOR_WRITE_ENABLE_ALPHA;
    }

    return mask;
}

D3D11_MAP ConvertShipyardMapFlagToDX11(MapFlag mapFlag)
{
    switch (mapFlag)
    {
    case MapFlag::Read:                 return D3D11_MAP_READ;
    case MapFlag::Write:                return D3D11_MAP_WRITE;
    case MapFlag::Read_Write:           return D3D11_MAP_READ_WRITE;
    case MapFlag::Write_Discard:        return D3D11_MAP_WRITE_DISCARD;
    case MapFlag::Write_No_Overwrite:   return D3D11_MAP_WRITE_NO_OVERWRITE;
    default:
        // Should never happen
        SHIP_ASSERT(false);
        break;
    }

    return D3D11_MAP_READ;
}

D3D11_FILTER ConvertShipyardSamplingFilterToDX11(
        SamplingFilter minificationFilter,
        SamplingFilter magnificationFilter,
        SamplingFilter mipmapFilter,
        shipBool useAnisotropicFiltering,
        shipBool useComparisonFunction)
{
    if (useAnisotropicFiltering)
    {
        if (useComparisonFunction)
        {
            return D3D11_FILTER_COMPARISON_ANISOTROPIC;
        }
        else
        {
            return D3D11_FILTER_ANISOTROPIC;
        }
    }

    if (minificationFilter == SamplingFilter::Nearest)
    {
        if (magnificationFilter == SamplingFilter::Nearest)
        {
            if (mipmapFilter == SamplingFilter::Nearest)
            {
                if (useComparisonFunction)
                {
                    return D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
                }
                else
                {
                    return D3D11_FILTER_MIN_MAG_MIP_POINT;
                }
            }
            else
            {
                if (useComparisonFunction)
                {
                    return D3D11_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR;
                }
                else
                {
                    return D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
                }
            }
        }
        else
        {
            if (mipmapFilter == SamplingFilter::Nearest)
            {
                if (useComparisonFunction)
                {
                    return D3D11_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT;
                }
                else
                {
                    return D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
                }
            }
            else
            {
                if (useComparisonFunction)
                {
                    return D3D11_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR;
                }
                else
                {
                    return D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
                }
            }
        }
    }
    else
    {
        if (magnificationFilter == SamplingFilter::Nearest)
        {
            if (mipmapFilter == SamplingFilter::Nearest)
            {
                if (useComparisonFunction)
                {
                    return D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT;
                }
                else
                {
                    return D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
                }
            }
            else
            {
                if (useComparisonFunction)
                {
                    return D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
                }
                else
                {
                    return D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
                }
            }
        }
        else
        {
            if (mipmapFilter == SamplingFilter::Nearest)
            {
                if (useComparisonFunction)
                {
                    return D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
                }
                else
                {
                    return D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
                }
            }
            else
            {
                if (useComparisonFunction)
                {
                    return D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
                }
                else
                {
                    return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
                }
            }
        }
    }
}

D3D11_TEXTURE_ADDRESS_MODE ConvertShipyardTextureAddressModeToDX11(TextureAddressMode textureAddressMode)
{
    switch (textureAddressMode)
    {
    case TextureAddressMode::Border:
        return D3D11_TEXTURE_ADDRESS_BORDER;

    case TextureAddressMode::Clamp:
        return D3D11_TEXTURE_ADDRESS_CLAMP;

    case TextureAddressMode::Mirror:
        return D3D11_TEXTURE_ADDRESS_MIRROR;

    case TextureAddressMode::Wrap:
        return D3D11_TEXTURE_ADDRESS_WRAP;

    default:
        break;
    }

    SHIP_ASSERT(!"Shouldn't happen");
    return D3D11_TEXTURE_ADDRESS_CLAMP;
}

const shipChar* GetD3DShaderVersion(D3D_FEATURE_LEVEL featureLevel)
{
    switch (featureLevel)
    {
    case D3D_FEATURE_LEVEL_11_0:
        return "5_0";
        break;

    case D3D_FEATURE_LEVEL_10_1:
        return "4_1";
        break;

    case D3D_FEATURE_LEVEL_10_0:
        return "4_0";
        break;

    case D3D_FEATURE_LEVEL_9_3:
    case D3D_FEATURE_LEVEL_9_2:
    case D3D_FEATURE_LEVEL_9_1:
        return "3_0";
        break;
    }

    return "UNKNOWN_D3D_SHADER_VERSION";
}

}