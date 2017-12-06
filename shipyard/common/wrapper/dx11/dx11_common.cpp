#include <common/wrapper/dx11/dx11_common.h>

#include <cassert>

namespace Shipyard
{;

DXGI_FORMAT ConvertShipyardFormatToDX11(GfxFormat format)
{
    switch (format)
    {
        case GfxFormat::R32G32B32A32_FLOAT:     return DXGI_FORMAT_R32G32B32A32_FLOAT;
        case GfxFormat::R32G32B32A32_UINT:      return DXGI_FORMAT_R32G32B32A32_UINT;
        case GfxFormat::R32G32B32A32_SINT:      return DXGI_FORMAT_R32G32B32A32_SINT;
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
        default:
            assert(false);
    }

    return DXGI_FORMAT_UNKNOWN;
}

}