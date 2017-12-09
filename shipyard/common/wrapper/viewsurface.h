#pragma once

#include <common/common.h>

#include <common/wrapper/wrapper_common.h>

namespace Shipyard
{
    class SHIPYARD_API ViewSurface
    {
    public:
        ViewSurface(uint32_t width, uint32_t height, GfxFormat viewSurfaceFormat);
        ~ViewSurface();

#ifdef DEBUG_WRAPPER_INTERFACE_COMPILATION
        virtual void PreRender() = 0;
        virtual void Render() = 0;
        virtual void PostRender() = 0;
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