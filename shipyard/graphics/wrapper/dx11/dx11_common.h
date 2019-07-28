#pragma once

#include <graphics/graphicscommon.h>

#include <system/string.h>

#pragma warning( disable : 4005 )

#include <d3d11.h>

#pragma warning( default : 4005 )

namespace Shipyard
{
    D3D11_PRIMITIVE_TOPOLOGY ConvertShipyardPrimitiveTopologyToDX11(PrimitiveTopology primitiveTopology);
    DXGI_FORMAT ConvertShipyardFormatToDX11(GfxFormat format);
    D3D11_FILL_MODE ConvertShipyardFillModeToDX11(FillMode fillMode);
    D3D11_CULL_MODE ConvertShipyardCullModeToDX11(CullMode cullMode);
    const shipChar* ConvertShipyardSemanticNameToDX11(SemanticName semanticName);
    D3D11_COMPARISON_FUNC ConvertShipyardComparisonFuncToDX11(ComparisonFunc func);
    D3D11_STENCIL_OP ConvertShipyardStencilOperationToDX11(StencilOperation operation);
    D3D11_BLEND ConvertShipyardBlendFactorToDX11(BlendFactor blendFactor);
    D3D11_BLEND_OP ConvertShipyardBlendOperatorToDX11(BlendOperator blendOperator);
    shipUint8 ConvertShipyardRenderTargetWriteMaskToDX11(RenderTargetWriteMask renderTargetWriteMask);
    D3D11_MAP ConvertShipyardMapFlagToDX11(MapFlag mapFlag);
    D3D11_FILTER ConvertShipyardSamplingFilterToDX11(
            SamplingFilter minificationFilter,
            SamplingFilter magnificationFilter,
            SamplingFilter mipmapFilter,
            shipBool useAnisotropicFiltering,
            shipBool useComparisonFunction);
    D3D11_TEXTURE_ADDRESS_MODE ConvertShipyardTextureAddressModeToDX11(TextureAddressMode textureAddressMode);

    const shipChar* GetD3DShaderVersion(D3D_FEATURE_LEVEL featureLevel);
}