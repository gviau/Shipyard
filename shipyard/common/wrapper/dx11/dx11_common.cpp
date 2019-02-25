#include <common/wrapper/dx11/dx11_common.h>

#include <cassert>

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
            assert(false);
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
        assert(false);
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
        assert(false);
    }

    return D3D11_CULL_NONE;
}

const char* ConvertShipyardSemanticNameToDX11(SemanticName semanticName)
{
    const char* dx11SemanticName = "";

    switch (semanticName)
    {
    case Shipyard::SemanticName::Color:     dx11SemanticName = "COLOR"; break;
    case Shipyard::SemanticName::Normal:    dx11SemanticName = "NORMAL"; break;
    case Shipyard::SemanticName::Position:  dx11SemanticName = "POSITION"; break;
    case Shipyard::SemanticName::TexCoord:  dx11SemanticName = "TEXCOORD"; break;
    }

    return dx11SemanticName;
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
    {

    };
    }

    return stencilOp;
}

const char* GetD3DShaderVersion(D3D_FEATURE_LEVEL featureLevel)
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