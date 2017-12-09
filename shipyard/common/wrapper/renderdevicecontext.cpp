#include <common/wrapper/renderdevicecontext.h>

namespace Shipyard
{;

RenderDeviceContext::RenderDeviceContext(const GFXRenderDevice& renderDevice)
{

}

RenderDeviceContext::~RenderDeviceContext()
{

}

void RenderDeviceContext::ClearFirstRenderTarget(float red, float green, float blue, float alpha)
{
    ClearRenderTarget(red, green, blue, alpha, 0);
}

void RenderDeviceContext::ClearRenderTargets(float red, float green, float blue, float alpha, uint32_t start, uint32_t count)
{
    for (uint32_t i = start; i < count; i++)
    {
        ClearRenderTarget(red, green, blue, alpha, i);
    }
}

}