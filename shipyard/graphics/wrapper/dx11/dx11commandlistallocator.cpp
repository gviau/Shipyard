#include <graphics/graphicsprecomp.h>

#include <graphics/wrapper/dx11/dx11commandlistallocator.h>

#include <system/memory.h>

namespace Shipyard
{;

DX11CommandListAllocator::DX11CommandListAllocator(GFXRenderDevice& gfxRenderDevice)
    : CommandListAllocator(gfxRenderDevice)
{

}

shipBool DX11CommandListAllocator::Create()
{
    return true;
}

void DX11CommandListAllocator::Destroy()
{

}

}