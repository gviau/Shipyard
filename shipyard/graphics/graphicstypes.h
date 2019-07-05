#pragma once

#include <system/platform.h>

namespace Shipyard
{
    const shipUint32 InvalidGfxHandle = 0xffffffff;

    struct GFXVertexBufferHandle
    {
        GFXVertexBufferHandle()
            : handle(InvalidGfxHandle)
        {

        }

        GFXVertexBufferHandle(shipUint32 _handle)
            : handle(_handle)
        {

        }

        shipUint32 handle;
    };

    struct GFXIndexBufferHandle
    {
        GFXIndexBufferHandle()
            : handle(InvalidGfxHandle)
        {

        }

        GFXIndexBufferHandle(shipUint32 _handle)
            : handle(_handle)
        {

        }

        shipUint32 handle;
    };

    struct GFXConstantBufferHandle
    {
        GFXConstantBufferHandle()
            : handle(InvalidGfxHandle)
        {

        }

        GFXConstantBufferHandle(shipUint32 _handle)
            : handle(_handle)
        {

        }

        shipUint32 handle;
    };

    struct GFXByteBufferHandle
    {
        GFXByteBufferHandle()
            : handle(InvalidGfxHandle)
        {

        }

        GFXByteBufferHandle(shipUint32 _handle)
            : handle(_handle)
        {

        }

        shipUint32 handle;
    };

    struct GFXDescriptorSetHandle
    {
        GFXDescriptorSetHandle()
            : handle(InvalidGfxHandle)
        {

        }

        GFXDescriptorSetHandle(shipUint32 _handle)
            : handle(_handle)
        {

        }

        shipUint32 handle;
    };

    struct GFXPipelineStateObjectHandle
    {
        GFXPipelineStateObjectHandle()
            : handle(InvalidGfxHandle)
        {

        }

        GFXPipelineStateObjectHandle(shipUint32 _handle)
            : handle(_handle)
        {

        }

        shipUint32 handle;
    };

    struct GFXRenderTargetHandle
    {
        GFXRenderTargetHandle()
            : handle(InvalidGfxHandle)
        {

        }

        GFXRenderTargetHandle(shipUint32 _handle)
            : handle(_handle)
        {

        }

        shipUint32 handle;
    };

    struct GFXDepthStencilRenderTargetHandle
    {
        GFXDepthStencilRenderTargetHandle()
            : handle(InvalidGfxHandle)
        {

        }

        GFXDepthStencilRenderTargetHandle(shipUint32 _handle)
            : handle(_handle)
        {

        }

        shipUint32 handle;
    };

    struct GFXRootSignatureHandle
    {
        GFXRootSignatureHandle()
            : handle(InvalidGfxHandle)
        {

        }

        GFXRootSignatureHandle(shipUint32 _handle)
            : handle(_handle)
        {

        }

        shipUint32 handle;
    };

    struct GFXVertexShaderHandle
    {
        GFXVertexShaderHandle()
            : handle(InvalidGfxHandle)
        {

        }

        GFXVertexShaderHandle(shipUint32 _handle)
            : handle(_handle)
        {

        }

        shipUint32 handle;
    };

    struct GFXPixelShaderHandle
    {
        GFXPixelShaderHandle()
            : handle(InvalidGfxHandle)
        {

        }

        GFXPixelShaderHandle(shipUint32 _handle)
            : handle(_handle)
        {

        }

        shipUint32 handle;
    };

    struct GFXTexture2DHandle
    {
        GFXTexture2DHandle()
            : handle(InvalidGfxHandle)
        {

        }

        GFXTexture2DHandle(shipUint32 _handle)
            : handle(_handle)
        {

        }

        shipUint32 handle;
    };
}