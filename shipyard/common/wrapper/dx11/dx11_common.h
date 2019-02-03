#pragma once

#include <common/common.h>

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
    const char* ConvertShipyardSemanticNameToDX11(SemanticName semanticName);
    D3D11_COMPARISON_FUNC ConvertShipyardComparisonFuncToDX11(ComparisonFunc func);
    D3D11_STENCIL_OP ConvertShipyardStencilOperationToDX11(StencilOperation operation);

    const char* GetD3DShaderVersion(D3D_FEATURE_LEVEL featureLevel);
}