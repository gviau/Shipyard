#pragma once

#include <graphics/graphicscommon.h>

#include <system/systemcommon.h>

// TODO
// Move this in project configuration
#define DX11_RENDERER

namespace Shipyard
{
#ifdef DX11_RENDERER

class DX11BaseRenderCommandList;
class DX11ByteBuffer;
class DX11CommandListAllocator;
class DX11CommandQueue;
class DX11ComputePipelineStateObject;
class DX11ComputeRenderCommandList;
class DX11ComputeShader;
class DX11ConstantBuffer;
class DX11CopyRenderCommandList;
class DX11DepthStencilRenderTarget;
class DX11DescriptorSet;
class DX11DirectRenderCommandList;
class DX11GraphicsPipelineStateObject;
class DX11IndexBuffer;
class DX11PixelShader;
class DX11RenderDevice;
class DX11RenderTarget;
class DX11RootSignature;
class DX11Sampler;
class DX11Texture2D;
class DX11Texture2DArray;
class DX11Texture3D;
class DX11TextureCube;
class DX11VertexBuffer;
class DX11VertexShader;
class DX11ViewSurface;

typedef DX11BaseRenderCommandList GFXRenderCommandList;
typedef DX11ByteBuffer GFXByteBuffer;
typedef DX11CommandListAllocator GFXCommandListAllocator;
typedef DX11CommandQueue GFXCommandQueue;
typedef DX11ComputePipelineStateObject GFXComputePipelineStateObject;
typedef DX11ComputeRenderCommandList GFXComputeRenderCommandList;
typedef DX11ComputeShader GFXComputeShader;
typedef DX11ConstantBuffer GFXConstantBuffer;
typedef DX11CopyRenderCommandList GFXCopyRenderCommandList;
typedef DX11DepthStencilRenderTarget GFXDepthStencilRenderTarget;
typedef DX11DescriptorSet GFXDescriptorSet;
typedef DX11DirectRenderCommandList GFXDirectRenderCommandList;
typedef DX11GraphicsPipelineStateObject GFXGraphicsPipelineStateObject;
typedef DX11IndexBuffer GFXIndexBuffer;
typedef DX11PixelShader GFXPixelShader;
using GFXRenderDevice = DX11RenderDevice;
typedef DX11RenderTarget GFXRenderTarget;
typedef DX11RootSignature GFXRootSignature;
typedef DX11Sampler GFXSampler;
typedef DX11Texture2D GFXTexture2D;
typedef DX11Texture2DArray GFXTexture2DArray;
typedef DX11Texture3D GFXTexture3D;
typedef DX11TextureCube GFXTextureCube;
typedef DX11VertexBuffer GFXVertexBuffer;
typedef DX11VertexShader GFXVertexShader;
typedef DX11ViewSurface GFXViewSurface;
#endif // #ifdef DX11_RENDERER
}