#pragma once

#include <graphics/graphicstypes.h>

#include <graphics/graphicssingleton.h>

#include <system/array.h>

namespace Shipyard
{
    class BaseAllocator;

    class BaseRenderDevice;
    class DirectRenderCommandList;

    class ShaderInputProvider;

    // Materials' constant buffers are managed as a single huge buffer.
    class SHIPYARD_GRAPHICS_API GFXMaterialUnifiedConstantBuffer : public GraphicsSingleton<GFXMaterialUnifiedConstantBuffer>
    {
    public:
        GFXMaterialUnifiedConstantBuffer();
        ~GFXMaterialUnifiedConstantBuffer();

        // maxConstantBufferPoolHeapSize sets a limit on the allocated buffer for the unified material constant buffer.
        // If the declared ShaderInputProviders' total memory usage * SHIP_MAX_BATCHED_DRAW_CALLS_PER_MATERIAL is greater
        // than this value, the method will return false, since it won't be able to allocate the buffer.
        shipBool Create(BaseRenderDevice& renderDevice, BaseAllocator* pAllocator, size_t maxConstantBufferPoolHeapSize);
        void Destroy();

        // This must be called before a draw call. Not thread-safe.
        void PrepareForNextDrawCall();

        // Thread-safe
        void ApplyShaderInputProviders(const Array<const ShaderInputProvider*>& shaderInputProviders);
        void ApplyShaderInputProvider(const ShaderInputProvider& shaderInputProvider);

        GFXByteBufferHandle BindMaterialUnfiedConstantBuffer(DirectRenderCommandList& directRenderCommandList);

    private:
        // This method is thread-safe.
        void* MapBufferForShaderInputProvider(const ShaderInputProvider& shaderInputProvider);

        GFXByteBufferHandle GetUpdatedMaterialUnifiedConstantBuffer(DirectRenderCommandList& gfxDirectRenderCommandList);

        BaseRenderDevice* m_pGfxRenderDevice;
        BaseAllocator* m_pAllocator;
        void* m_pConstantBufferHeap;
        size_t m_ConstantBufferHeapSize;
        shipBool m_IsDirty;

        Array<shipUint32> m_ShaderInputProviderRequiredSizes;
        Array<size_t> m_WriteOffsetsPerShaderInputProvider;

        // Constant buffer emulated through a big byte buffer.
        GFXByteBufferHandle m_MaterialUnifiedConstantBuffer;
    };

    SHIPYARD_GRAPHICS_API GFXMaterialUnifiedConstantBuffer& GetGFXMaterialUnifiedConstantBuffer();
}