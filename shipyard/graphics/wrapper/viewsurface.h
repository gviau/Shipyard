#pragma once

#include <graphics/graphicscommon.h>

#include <graphics/wrapper/wrapper_common.h>

namespace Shipyard
{
    class SHIPYARD_API ViewSurface
    {
    public:
        ViewSurface();

#ifdef DEBUG_WRAPPER_INTERFACE_COMPILATION
        virtual void Flip() = 0;
#endif // #ifdef DEBUG_WRAPPER_INTERFACE_COMPILATION

        uint32_t GetWidth() const { return m_Width; }
        uint32_t GetHeight() const { return m_Height; }
        GfxFormat GetFormat() const { return m_ViewSurfaceFormat; }

    protected:
        uint32_t m_Width;
        uint32_t m_Height;
        GfxFormat m_ViewSurfaceFormat;
    };
}