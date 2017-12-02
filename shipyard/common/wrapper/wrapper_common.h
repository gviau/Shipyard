#pragma once

namespace Shipyard
{
    // TODO
    // Move this in project configuration
#define DX11_RENDERER

    // Uncomment the following to have virtual methods in each of the wrappers' base class to make sure that your implementation is valid
#define DEBUG_WRAPPER_INTERFACE_COMPILATION

#ifdef DX11_RENDERER

class DX11ConstantBuffer;
class DX11IndexBuffer;
class DX11RenderDevice;
class DX11VertexBuffer;

#define GFXConstantBuffer DX11ConstantBuffer
#define GFXIndexBuffer DX11IndexBuffer
#define GFXRenderDevice DX11RenderDevice
#define GFXVertexBuffer DX11VertexBuffer
#endif // #ifdef DX11_RENDERER
}