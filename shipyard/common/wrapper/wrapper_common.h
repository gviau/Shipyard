#pragma once

#include <common/common.h>

#include <system/common.h>

// TODO
// Move this in project configuration
#define DX11_RENDERER

namespace Shipyard
{
#ifdef DX11_RENDERER

class DX11ConstantBuffer;
class DX11DepthStencilRenderTarget;
class DX11DescriptorSet;
class DX11IndexBuffer;
class DX11PipelineStateObject;
class DX11PixelShader;
class DX11RenderDevice;
class DX11RenderDeviceContext;
class DX11RenderTarget;
class DX11RootSignature;
class DX11Texture2D;
class DX11VertexBuffer;
class DX11VertexShader;
class DX11ViewSurface;

typedef DX11ConstantBuffer GFXConstantBuffer;
typedef DX11DepthStencilRenderTarget GFXDepthStencilRenderTarget;
typedef DX11DescriptorSet GFXDescriptorSet;
typedef DX11IndexBuffer GFXIndexBuffer;
typedef DX11PipelineStateObject GFXPipelineStateObject;
typedef DX11PixelShader GFXPixelShader;
typedef DX11RenderDevice GFXRenderDevice;
typedef DX11RenderDeviceContext GFXRenderDeviceContext;
typedef DX11RenderTarget GFXRenderTarget;
typedef DX11RootSignature GFXRootSignature;
typedef DX11Texture2D GFXTexture2D;
typedef DX11VertexBuffer GFXVertexBuffer;
typedef DX11VertexShader GFXVertexShader;
typedef DX11ViewSurface GFXViewSurface;
#endif // #ifdef DX11_RENDERER
}