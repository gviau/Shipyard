#pragma once

#include <system/platform.h>

#include <graphics/graphicstypes.h>

#include <graphics/mesh/gfxmesh.h>

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
    class Renderer;
    class ShaderDatabase;

    struct SimpleConstantBufferProvider;

    class SHIPYARD_VIEWER_LIB_API ShipyardViewer
    {
    public:
        ~ShipyardViewer();

        shipBool CreateApp(HWND windowHandle, shipUint32 windowWidth, shipUint32 windowHeight);
        void ComputeOneFrame();

        shipBool OnWin32Msg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT* shipyardMsgHandlingResult);

    private:
        shipUint32 m_WindowWidth = 0;
        shipUint32 m_WindowHeight = 0;

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
        GFXVertexBufferHandle m_VertexBufferHandle;
        GFXIndexBufferHandle m_IndexBufferHandle;
        GFXTexture2DHandle m_TextureHandle;
        GFXTexture2DHandle m_TestTextureHandle;
        GFXRenderTargetHandle m_TestRenderTargetHandle;

        ShaderDatabase* m_pShaderDatabase = nullptr;

        SimpleConstantBufferProvider* m_pDataProvider = nullptr;
        Renderer* m_pRenderer = nullptr;

        GFXMesh* m_pGfxMesh = nullptr;
        GFXMaterial* m_pDefaultMaterial = nullptr;
        InplaceArray<GFXMaterial*, 8> m_LoadedMaterials;

        void ClearViewerAndLoadMesh(const shipChar* filename);
    };
}