#pragma once

#include <graphics/graphicscommon.h>

namespace Shipyard
{
    class Texture2D;

    class SHIPYARD_API BaseRenderTarget
    {
    public:
        BaseRenderTarget();

        shipUint32 GetWidth() const { return m_Width; }
        shipUint32 GetHeight() const { return m_Height; }

    protected:
        shipUint32 m_Width;
        shipUint32 m_Height;
    };

    class SHIPYARD_API RenderTarget : public BaseRenderTarget
    {
    };

    class SHIPYARD_API DepthStencilRenderTarget : public BaseRenderTarget
    {

    };
}
