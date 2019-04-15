#pragma once

namespace Shipyard
{
    const uint32_t InvalidGfxHandle = 0xffffffff;

    struct GFXVertexBufferHandle
    {
        GFXVertexBufferHandle()
            : handle(InvalidGfxHandle)
        {

        }

        GFXVertexBufferHandle(uint32_t _handle)
            : handle(_handle)
        {

        }

        uint32_t handle;
    };

    struct GFXIndexBufferHandle
    {
        GFXIndexBufferHandle()
            : handle(InvalidGfxHandle)
        {

        }

        GFXIndexBufferHandle(uint32_t _handle)
            : handle(_handle)
        {

        }

        uint32_t handle;
    };

    struct GFXConstantBufferHandle
    {
        GFXConstantBufferHandle()
            : handle(InvalidGfxHandle)
        {

        }

        GFXConstantBufferHandle(uint32_t _handle)
            : handle(_handle)
        {

        }

        uint32_t handle;
    };

    struct GFXDescriptorSetHandle
    {
        GFXDescriptorSetHandle()
            : handle(InvalidGfxHandle)
        {

        }

        GFXDescriptorSetHandle(uint32_t _handle)
            : handle(_handle)
        {

        }

        uint32_t handle;
    };

    struct GFXPipelineStateObjectHandle
    {
        GFXPipelineStateObjectHandle()
            : handle(InvalidGfxHandle)
        {

        }

        GFXPipelineStateObjectHandle(uint32_t _handle)
            : handle(_handle)
        {

        }

        uint32_t handle;
    };

    struct GFXRenderTargetHandle
    {
        GFXRenderTargetHandle()
            : handle(InvalidGfxHandle)
        {

        }

        GFXRenderTargetHandle(uint32_t _handle)
            : handle(_handle)
        {

        }

        uint32_t handle;
    };

    struct GFXDepthStencilRenderTargetHandle
    {
        GFXDepthStencilRenderTargetHandle()
            : handle(InvalidGfxHandle)
        {

        }

        GFXDepthStencilRenderTargetHandle(uint32_t _handle)
            : handle(_handle)
        {

        }

        uint32_t handle;
    };

    struct GFXRootSignatureHandle
    {
        GFXRootSignatureHandle()
            : handle(InvalidGfxHandle)
        {

        }

        GFXRootSignatureHandle(uint32_t _handle)
            : handle(_handle)
        {

        }

        uint32_t handle;
    };

    struct GFXVertexShaderHandle
    {
        GFXVertexShaderHandle()
            : handle(InvalidGfxHandle)
        {

        }

        GFXVertexShaderHandle(uint32_t _handle)
            : handle(_handle)
        {

        }

        uint32_t handle;
    };

    struct GFXPixelShaderHandle
    {
        GFXPixelShaderHandle()
            : handle(InvalidGfxHandle)
        {

        }

        GFXPixelShaderHandle(uint32_t _handle)
            : handle(_handle)
        {

        }

        uint32_t handle;
    };

    struct GFXTexture2DHandle
    {
        GFXTexture2DHandle()
            : handle(InvalidGfxHandle)
        {

        }

        GFXTexture2DHandle(uint32_t _handle)
            : handle(_handle)
        {

        }

        uint32_t handle;
    };
}