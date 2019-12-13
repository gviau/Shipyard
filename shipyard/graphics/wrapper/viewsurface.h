#pragma once

#include <graphics/graphicscommon.h>

#include <graphics/wrapper/wrapper_common.h>

namespace Shipyard
{
    class SHIPYARD_GRAPHICS_API ViewSurface
    {
    public:
        ViewSurface();

#ifdef DEBUG_WRAPPER_INTERFACE_COMPILATION
        virtual void Flip() = 0;
#endif // #ifdef DEBUG_WRAPPER_INTERFACE_COMPILATION

        shipUint32 GetWidth() const { return m_Width; }
        shipUint32 GetHeight() const { return m_Height; }
        GfxFormat GetFormat() const { return m_ViewSurfaceFormat; }

    protected:
        shipUint32 m_Width;
        shipUint32 m_Height;
        GfxFormat m_ViewSurfaceFormat;
    };
}