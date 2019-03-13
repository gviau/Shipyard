#pragma once

namespace Shipyard
{
    const uint32_t InvalidGfxHandle = 0xffffffff;

    struct GFXVertexBufferHandle
    {
        uint32_t handle = InvalidGfxHandle;
    };

    struct GFXIndexBufferHandle
    {
        uint32_t handle = InvalidGfxHandle;
    };

    struct GFXConstantBufferHandle
    {
        uint32_t handle = InvalidGfxHandle;
    };

    struct GFXDescriptorSetHandle
    {
        uint32_t handle = InvalidGfxHandle;
    };

    struct GFXPipelineStateObjectHandle
    {
        uint32_t handle = InvalidGfxHandle;
    };

    struct GFXRenderTargetHandle
    {
        uint32_t handle = InvalidGfxHandle;
    };

    struct GFXDepthStencilRenderTargetHandle
    {
        uint32_t handle = InvalidGfxHandle;
    };

    struct GFXRootSignatureHandle
    {
        uint32_t handle = InvalidGfxHandle;
    };

    struct GFXVertexShaderHandle
    {
        uint32_t handle = InvalidGfxHandle;
    };

    struct GFXPixelShaderHandle
    {
        uint32_t handle = InvalidGfxHandle;
    };

    struct GFXTexture2DHandle
    {
        uint32_t handle = InvalidGfxHandle;
    };
}