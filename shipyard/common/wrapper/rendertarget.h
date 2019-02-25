#pragma once

#include <common/common.h>

#include <system/array.h>

namespace Shipyard
{
    class Texture2D;

    class SHIPYARD_API BaseRenderTarget
    {
    public:
        BaseRenderTarget();

#ifdef DEBUG_WRAPPER_INTERFACE_COMPILATION
        virtual bool IsValid() const = 0;
#endif // #ifdef DEBUG_WRAPPER_INTERFACE_COMPILATION

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
