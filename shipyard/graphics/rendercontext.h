#pragma once

#include <system/platform.h>

#include <graphics/wrapper/wrapper_common.h>

namespace Shipyard
{
    class SHIPYARD_GRAPHICS_API RenderContext
    {
    public:
        void SetRenderDevice(GFXRenderDevice* gfxRenderDevice);
        GFXRenderDevice* GetRenderDevice();

        void SetRenderCommandList(GFXDirectRenderCommandList* gfxDirectRenderCommandList);
        GFXDirectRenderCommandList* GetRenderCommandList();

        void SetViewSurface(GFXViewSurface* gfxViewSurface);
        GFXViewSurface* GetViewSurface() const;

    protected:
        GFXRenderDevice* m_RenderDevice = nullptr;
        GFXDirectRenderCommandList* m_DirectRenderCommandList = nullptr;
        GFXViewSurface* m_ViewSurface = nullptr;
    };
}