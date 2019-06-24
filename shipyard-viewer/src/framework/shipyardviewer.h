#pragma once

#include <system/platform.h>

#include <graphics/graphicstypes.h>

#include <graphics/wrapper/wrapper_common.h>

#include <system/memory.h>
#include <system/memory/fixedheapallocator.h>
#include <system/memory/poolallocator.h>

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
#include <system/memory/debugallocator.h>
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

#include <windows.h>

namespace Shipyard
{
    class GraphicsSingletonStorer;
    class ShaderDatabase;

    struct SimpleConstantBufferProvider;

    class ShipyardViewer
    {
    public:
        ~ShipyardViewer();

        bool CreateApp(HWND windowHandle, uint32_t windowWidth, uint32_t windowHeight);
        void ComputeOneFrame();

    private:
        uint32_t m_WindowWidth = 0;
        uint32_t m_WindowHeight = 0;

        GraphicsSingletonStorer* m_pGraphicsSingletonStorer = nullptr;

        void* m_pHeap = nullptr;
        PoolAllocator m_PoolAllocator16;
        PoolAllocator m_PoolAllocator32;
        PoolAllocator m_PoolAllocator64;
        FixedHeapAllocator m_FixedHeapAllocator;

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
        void* m_pDebugHeap = nullptr;
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

        GFXRenderDevice* m_pGfxRenderDevice = nullptr;
        GFXDirectRenderCommandList* m_pGfxDirectRenderCommandList = nullptr;
        GFXCommandListAllocator* m_pGfxCommandListAllocator = nullptr;
        GFXCommandQueue* m_pGfxDirectCommandQueue = nullptr;
        GFXViewSurface* m_pGfxViewSurface = nullptr;

        GFXTexture2DHandle m_GfxDepthTextureHandle;
        GFXDepthStencilRenderTargetHandle m_GfxDepthStencilRenderTargetHandle;
        GFXRootSignatureHandle m_GfxRootSignatureHandle;
        GFXVertexBufferHandle m_VertexBufferHandle;
        GFXIndexBufferHandle m_IndexBufferHandle;
        GFXTexture2DHandle m_TextureHandle;
        GFXDescriptorSetHandle m_GfxDescriptorSetHandle;
        GFXTexture2DHandle m_TestTextureHandle;
        GFXRenderTargetHandle m_TestRenderTargetHandle;

        ShaderDatabase* m_pShaderDatabase = nullptr;

        SimpleConstantBufferProvider* m_pDataProvider = nullptr;
    };
}