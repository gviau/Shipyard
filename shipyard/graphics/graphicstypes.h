#pragma once

#include <system/platform.h>

namespace Shipyard
{
    const shipUint32 InvalidGfxHandle = 0xffffffff;

#define DEFINE_HANDLE_TYPE(HandleTypeName) \
    struct HandleTypeName \
    { \
        HandleTypeName() \
            : handle(InvalidGfxHandle) \
        { } \
        HandleTypeName(shipUint32 _handle) \
            : handle(_handle) \
        { } \
        shipBool operator== (const HandleTypeName& rhs) const \
        { \
            return handle == rhs.handle; \
        } \
        shipBool IsValid() const \
        { \
            return handle != InvalidGfxHandle; \
        } \
        shipUint32 handle; \
    }

    DEFINE_HANDLE_TYPE(GFXVertexBufferHandle);
    DEFINE_HANDLE_TYPE(GFXIndexBufferHandle);
    DEFINE_HANDLE_TYPE(GFXConstantBufferHandle);
    DEFINE_HANDLE_TYPE(GFXByteBufferHandle);
    DEFINE_HANDLE_TYPE(GFXDescriptorSetHandle);
    DEFINE_HANDLE_TYPE(GFXGraphicsPipelineStateObjectHandle);
    DEFINE_HANDLE_TYPE(GFXComputePipelineStateObjectHandle);
    DEFINE_HANDLE_TYPE(GFXRenderTargetHandle);
    DEFINE_HANDLE_TYPE(GFXDepthStencilRenderTargetHandle);
    DEFINE_HANDLE_TYPE(GFXRootSignatureHandle);
    DEFINE_HANDLE_TYPE(GFXVertexShaderHandle);
    DEFINE_HANDLE_TYPE(GFXPixelShaderHandle);
    DEFINE_HANDLE_TYPE(GFXComputeShaderHandle);
    DEFINE_HANDLE_TYPE(GFXTexture2DHandle);
    DEFINE_HANDLE_TYPE(GFXTexture2DArrayHandle);
    DEFINE_HANDLE_TYPE(GFXTexture3DHandle);
    DEFINE_HANDLE_TYPE(GFXTextureCubeHandle);
    DEFINE_HANDLE_TYPE(GFXSamplerHandle);
}