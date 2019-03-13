#pragma once

#include <graphics/graphicscommon.h>

namespace Shipyard
{
    class Texture2D;

    class SHIPYARD_API BaseRenderTarget
    {
    public:
        BaseRenderTarget();

        uint32_t GetWidth() const { return m_Width; }
        uint32_t GetHeight() const { return m_Height; }

    protected:
        uint32_t m_Width;
        uint32_t m_Height;
    };

    class SHIPYARD_API RenderTarget : public BaseRenderTarget
    {
    };

    class SHIPYARD_API DepthStencilRenderTarget : public BaseRenderTarget
    {

    };
}
