#include <graphics/wrapper/viewsurface.h>

namespace Shipyard
{;

ViewSurface::ViewSurface(uint32_t width, uint32_t height, GfxFormat viewSurfaceFormat)
    : m_Width(width)
    , m_Height(height)
    , m_ViewSurfaceFormat(viewSurfaceFormat)
{
}

ViewSurface::~ViewSurface()
{
}

}