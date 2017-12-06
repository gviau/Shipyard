#pragma once

#include <common/common.h>

#include <dxgiformat.h>

namespace Shipyard
{
    DXGI_FORMAT ConvertShipyardFormatToDX11(GfxFormat format);
}