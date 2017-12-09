#pragma once

#include <system/platform.h>

// TODO
// Move this in project configuration
#define DX11_RENDERER

// Uncomment the following to have virtual methods in each of the wrappers' base class to make sure that your implementation is valid
#define DEBUG_WRAPPER_INTERFACE_COMPILATION

namespace Shipyard
{
#ifdef DX11_RENDERER

class DX11ConstantBuffer;
class DX11IndexBuffer;
class DX11PixelShader;
class DX11RenderDevice;
class DX11RenderDeviceContext;
class DX11VertexBuffer;
class DX11VertexShader;
class DX11ViewSurface;

typedef DX11ConstantBuffer GFXConstantBuffer;
typedef DX11IndexBuffer GFXIndexBuffer;
typedef DX11PixelShader GFXPixelShader;
typedef DX11RenderDevice GFXRenderDevice;
typedef DX11RenderDeviceContext GFXRenderDeviceContext;
typedef DX11VertexBuffer GFXVertexBuffer;
typedef DX11VertexShader GFXVertexShader;
typedef DX11ViewSurface GFXViewSurface;
#endif // #ifdef DX11_RENDERER
}