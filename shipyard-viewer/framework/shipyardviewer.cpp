#include "shipyardviewerlibprecomp.h"

#include "shipyardviewer.h"

#include <graphics/wrapper/wrapper.h>

#include <graphics/material/gfxmaterialunifiedconstantbuffer.h>

#include <graphics/shadercompiler/shadercompiler.h>
#include <graphics/shadercompiler/shaderwatcher.h>

#include <graphics/shader/shaderdatabase.h>
#include <graphics/shader/shaderfamilies.h>
#include <graphics/shader/shaderhandler.h>
#include <graphics/shader/shaderhandlermanager.h>
#include <graphics/shader/shaderinputprovider.h>
#include <graphics/shader/shaderresourcebinder.h>
#include <graphics/shader/shaderkey.h>

#include <graphics/utils/fullscreenhelper.h>

#include <graphics/defaulttextures.h>
#include <graphics/graphicssingletonstorer.h>
#include <graphics/rendercontext.h>
#include <graphics/renderer.h>
#include <graphics/shipyardimgui.h>

#include <system/logger.h>

#include <tools/meshimporter.h>

#include <extern/glm/gtc/matrix_transform.hpp>
#include <extern/glm/gtc/type_ptr.hpp>

#include <extern/imguizmo/ImGuizmo.h>

namespace Shipyard
{;

struct SimpleConstantBufferProvider : public BaseShaderInputProvider<SimpleConstantBufferProvider>
{
    shipMat4x4 WorldProjectionMatrix;
    shipMat4x4 ViewFromWorldMatrix;
    shipMat4x4 WorldFromLocalMatrix;
};

SHIP_DECLARE_SHADER_INPUT_PROVIDER_BEGIN(SimpleConstantBufferProvider, PerInstance)
{
    SHIP_SCALAR_SHADER_INPUT("WorldProjectionMatrix", WorldProjectionMatrix);
    SHIP_SCALAR_SHADER_INPUT("ViewFromWorldMatrix", ViewFromWorldMatrix);
    SHIP_SCALAR_SHADER_INPUT("WorldFromLocalMatrix", WorldFromLocalMatrix);
}
SHIP_DECLARE_SHADER_INPUT_PROVIDER_END(SimpleConstantBufferProvider)

ShipyardViewer::~ShipyardViewer()
{
    SHIP_DELETE(m_pDefaultMaterial);
    SHIP_DELETE(m_pGfxMesh);

    SHIP_DELETE(m_pRenderer);

    SHIP_DELETE(m_pDataProvider);

    GetFullscreenHelper().DestroyResources(*m_pGfxRenderDevice);

    SHIP_DELETE(m_pGraphicsSingletonStorer);

    SHIP_DELETE(m_pShaderDatabase);

    SHIP_DELETE(m_pGfxViewSurface);
    SHIP_DELETE(m_pGfxDirectCommandQueue);
    SHIP_DELETE(m_pGfxCommandListAllocator);
    SHIP_DELETE(m_pGfxDirectRenderCommandList);
    SHIP_DELETE(m_pGfxRenderDevice);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
    DebugAllocator::GetInstance().Destroy();

    free(m_pDebugHeap);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

    GetLogger().CloseLog();

    m_FixedHeapAllocator.Destroy();
    m_PoolAllocator64.Destroy();
    m_PoolAllocator32.Destroy();
    m_PoolAllocator16.Destroy();

    GetGlobalAllocator().Destroy();

    free(m_pHeap);
}

shipBool ShipyardViewer::CreateApp(HWND windowHandle, shipUint32 windowWidth, shipUint32 windowHeight)
{
    GetLogger().OpenLog("shipyard_viewer.log");

    RECT rect;
    ::GetClientRect(windowHandle, &rect);
    m_WindowWidth = (rect.right - rect.left);
    m_WindowHeight = (rect.bottom - rect.top);

    windowWidth = m_WindowWidth;
    windowHeight = m_WindowHeight;

    size_t heapSize = 512 * 1024 * 1024;
    m_pHeap = malloc(heapSize);

    size_t numChunks = 256 * 1024;

    void* pHeap16 = reinterpret_cast<void*>(MemoryUtils::AlignAddress(size_t(m_pHeap), 16));
    void* pHeap32 = reinterpret_cast<void*>(MemoryUtils::AlignAddress(size_t(pHeap16) + numChunks * 16, 32));
    void* pHeap64 = reinterpret_cast<void*>(MemoryUtils::AlignAddress(size_t(pHeap32) + numChunks * 32, 64));

    void* pFixedHeap = reinterpret_cast<void*>(size_t(pHeap64) + numChunks * 64);
    size_t fixedHeapSize = heapSize - (size_t(pHeap64) + numChunks * 64 - size_t(m_pHeap));

    m_PoolAllocator16.Create(pHeap16, numChunks, 16);
    m_PoolAllocator32.Create(pHeap32, numChunks, 32);
    m_PoolAllocator64.Create(pHeap64, numChunks, 64);
    m_FixedHeapAllocator.Create(pFixedHeap, fixedHeapSize);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
    size_t debugHeapSize = 32 * 1024 * 1024;
    m_pDebugHeap = malloc(debugHeapSize);

    DebugAllocator::GetInstance().Create(m_pDebugHeap, debugHeapSize);

    m_PoolAllocator16.SetAllocatorDebugName("PoolAllocator 16 bytes");
    m_PoolAllocator32.SetAllocatorDebugName("PoolAllocator 32 bytes");
    m_PoolAllocator64.SetAllocatorDebugName("PoolAllocator 64 bytes");
    m_FixedHeapAllocator.SetAllocatorDebugName("FixedHeapAllocator");
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

    GlobalAllocator::AllocatorInitEntry allocatorInitEntries[4];
    allocatorInitEntries[0].pAllocator = &m_PoolAllocator16;
    allocatorInitEntries[0].maxAllocationSize = 16;
    allocatorInitEntries[1].pAllocator = &m_PoolAllocator32;
    allocatorInitEntries[1].maxAllocationSize = 32;
    allocatorInitEntries[2].pAllocator = &m_PoolAllocator64;
    allocatorInitEntries[2].maxAllocationSize = 64;
    allocatorInitEntries[3].pAllocator = &m_FixedHeapAllocator;
    allocatorInitEntries[3].maxAllocationSize = 0;

    GetGlobalAllocator().Create(allocatorInitEntries, 4);

    m_pGfxRenderDevice = SHIP_NEW(GFXRenderDevice, 1);
    m_pGfxRenderDevice->Create();

    GetShaderInputProviderManager().Initialize(*m_pGfxRenderDevice);

    m_pGraphicsSingletonStorer = SHIP_NEW(GraphicsSingletonStorer, 1);

    GetFullscreenHelper().CreateResources(*m_pGfxRenderDevice);

    m_pGfxDirectRenderCommandList = SHIP_NEW(GFXDirectRenderCommandList, 1)(*m_pGfxRenderDevice);
    m_pGfxDirectRenderCommandList->Create();

    m_pGfxCommandListAllocator = SHIP_NEW(GFXCommandListAllocator, 1)(*m_pGfxRenderDevice);
    m_pGfxCommandListAllocator->Create();

    m_pGfxDirectCommandQueue = SHIP_NEW(GFXCommandQueue, 1)(*m_pGfxRenderDevice, CommandQueueType::Direct);
    m_pGfxDirectCommandQueue->Create();

    m_pGfxViewSurface = SHIP_NEW(GFXViewSurface, 1);
    shipBool isValid = m_pGfxViewSurface->Create(*m_pGfxRenderDevice, windowWidth, windowHeight, GfxFormat::R8G8B8A8_UNORM, windowHandle);

    SHIP_ASSERT(isValid);

    m_GfxDepthTextureHandle = m_pGfxRenderDevice->CreateTexture2D(
        windowWidth,
        windowHeight,
        GfxFormat::D24_UNORM_S8_UINT,
        false,
        nullptr,
        false,
        TextureUsage::TextureUsage_DepthStencil);

    m_GfxDepthStencilRenderTargetHandle = m_pGfxRenderDevice->CreateDepthStencilRenderTarget(m_GfxDepthTextureHandle);

    m_pDataProvider = SHIP_NEW(SimpleConstantBufferProvider, 1);

    GetGFXMaterialUnifiedConstantBuffer().Create(*m_pGfxRenderDevice, nullptr, 4 * 1024 * 1024);

    m_pShaderDatabase = SHIP_NEW(ShaderDatabase, 1);
    m_pShaderDatabase->Load("ShipyardShaderDatabase.bin");

    GetShaderHandlerManager().Initialize(*m_pGfxRenderDevice, *m_pShaderDatabase);

    InitializeImGui(windowHandle, *m_pGfxRenderDevice);

    m_pRenderer = SHIP_NEW(Renderer, 1);
    m_pRenderer->Initialize(*m_pGfxRenderDevice);

    m_pDefaultMaterial = SHIP_NEW(GFXMaterial, 1);

    GFXTexture2DHandle defaultMaterialTextures[shipUint32(GfxMaterialTextureType::Count)] =
    {
        DefaultTextures::WhiteTexture,
        DefaultTextures::BlackTexture,
        DefaultTextures::BlackTexture,
        DefaultTextures::BlackTexture
    };

    m_pDefaultMaterial->Create(ShaderFamily::Error, defaultMaterialTextures);

    ImGuizmo::SetRect(0.0f, 0.0f, shipFloat(m_WindowWidth), shipFloat(m_WindowHeight));

    return true;
}

void ShipyardViewer::ComputeOneFrame()
{
    StartNewImGuiFrame();
    ImGuizmo::BeginFrame();

    GFXMaterialUnifiedConstantBuffer& gfxMaterialUnifiedConstantBuffer = GetGFXMaterialUnifiedConstantBuffer();
    gfxMaterialUnifiedConstantBuffer.PrepareForNextDrawCall();

    static shipFloat theta = 0.0f;

    GfxViewport gfxViewport;
    gfxViewport.topLeftX = 0.0f;
    gfxViewport.topLeftY = 0.0f;
    gfxViewport.width = shipFloat(m_WindowWidth);
    gfxViewport.height = shipFloat(m_WindowHeight);
    gfxViewport.minDepth = 0.0f;
    gfxViewport.maxDepth = 1.0f;

    m_pGfxDirectRenderCommandList->Reset(*m_pGfxCommandListAllocator, nullptr);

    shipMat4x4 viewFromWorldMatrix = glm::lookAtLH(shipVec3(0.25f, 0.25f, -0.25f), shipVec3(0.0f, 0.0f, 0.0f), shipVec3(0.0f, 1.0f, 0.0f));
    shipMat4x4 projectionFromViewMatrix = glm::perspectiveFovLH(glm::radians(90.0f), shipFloat(m_WindowWidth), shipFloat(m_WindowHeight), 0.001f, 1000.0f);

    static glm::mat4 worldFromLocalMatrix(1.0f, 0.0f, 0.0f, 0.0f,
                    0.0f, 1.0f, 0.0f, 0.0f,
                    0.0f, 0.0f, 1.0f, 0.25f,
                    0.0f, 0.0f, 0.0f, 1.0f);

    static ImGuizmo::OPERATION imguizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
    static ImGuizmo::MODE imguizmoMode = ImGuizmo::WORLD;

    // Taken from https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes..
    constexpr int keyQ = 0x51;
    constexpr int keyW = 0x57;
    constexpr int keyE = 0x45;
    constexpr int keyR = 0x52;
    if (ImGui::IsKeyPressed(keyQ))
    {
        imguizmoMode = ImGuizmo::MODE((int(imguizmoMode) + 1) % 2);
    }

    if (ImGui::IsKeyPressed(keyW))
    {
        imguizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
    }
    else if (ImGui::IsKeyPressed(keyE))
    {
        imguizmoOperation = ImGuizmo::OPERATION::ROTATE;
    }
    else if (ImGui::IsKeyPressed(keyR))
    {
        imguizmoOperation = ImGuizmo::OPERATION::SCALE;
    }

    if (imguizmoOperation == ImGuizmo::OPERATION::SCALE)
    {
        imguizmoMode = ImGuizmo::MODE::LOCAL;
    }

    ImGuizmo::Manipulate(
            reinterpret_cast<shipFloat*>(&viewFromWorldMatrix),
            reinterpret_cast<shipFloat*>(&projectionFromViewMatrix),
            imguizmoOperation,
            imguizmoMode,
            reinterpret_cast<shipFloat*>(&worldFromLocalMatrix));

    m_pDataProvider->WorldFromLocalMatrix = worldFromLocalMatrix;
    m_pDataProvider->WorldProjectionMatrix = viewFromWorldMatrix * worldFromLocalMatrix;
    m_pDataProvider->WorldProjectionMatrix = projectionFromViewMatrix * viewFromWorldMatrix * worldFromLocalMatrix;

    theta += 0.001f;

    GFXRenderTargetHandle gfxRenderTargetHandle = m_pGfxViewSurface->GetBackBufferRenderTargetHandle();

    ClearFullRenderTargetCommand* pClearFullRenderTargetCommand = m_pGfxDirectRenderCommandList->ClearFullRenderTarget();
    pClearFullRenderTargetCommand->gfxRenderTargetHandle = gfxRenderTargetHandle;
    pClearFullRenderTargetCommand->red = 0.0f;
    pClearFullRenderTargetCommand->green = 0.0f;
    pClearFullRenderTargetCommand->blue = 0.125f;
    pClearFullRenderTargetCommand->alpha = 0.0f;

    ClearDepthStencilRenderTargetCommand* pClearDepthStencilRenderTargetCommand = m_pGfxDirectRenderCommandList->ClearDepthStencilRenderTarget();
    pClearDepthStencilRenderTargetCommand->gfxDepthStencilRenderTargetHandle = m_GfxDepthStencilRenderTargetHandle;
    pClearDepthStencilRenderTargetCommand->depthStencilClearFlag = DepthStencilClearFlag::Depth;
    pClearDepthStencilRenderTargetCommand->depthValue = 1.0f;
    pClearDepthStencilRenderTargetCommand->stencilValue = 0;

    if (m_pGfxMesh != nullptr)
    {
        const Array<GFXSubMesh>& gfxSubMeshes = m_pGfxMesh->GetGfxSubMeshes();
        for (const GFXSubMesh& gfxSubMesh : gfxSubMeshes)
        {
            GFXVertexBufferHandle gfxVertexBufferHandle = gfxSubMesh.GetGfxVertexBufferHandle();
            GFXIndexBufferHandle gfxIndexBufferHandle = gfxSubMesh.GetGfxIndexBufferHandle();
            const GFXMaterial* gfxMaterial = gfxSubMesh.GetGfxMaterial();

            VertexFormatType vertexFormatType = m_pGfxRenderDevice->GetVertexBuffer(gfxVertexBufferHandle).GetVertexFormatType();

            ShaderKey shaderKey;
            shaderKey.SetShaderFamily(gfxMaterial->GetShaderFamily());

            SET_SHADER_OPTION(shaderKey, VERTEX_FORMAT_TYPE, shipUint32(vertexFormatType));

            ShaderHandler* pShaderHandler = GetShaderHandlerManager().GetShaderHandlerForShaderKey(shaderKey);

            InplaceArray<const ShaderInputProvider*, 2> shaderInputProviders;
            shaderInputProviders.Add(m_pDataProvider);
            shaderInputProviders.Add(&gfxMaterial->GetGfxMaterialShaderInputProvider());

            pShaderHandler->ApplyShaderInputProvidersForGraphics(*m_pGfxRenderDevice, *m_pGfxDirectRenderCommandList, shaderInputProviders);

            ShaderHandler::RenderState renderState;
            renderState.GfxRenderTargetHandle = gfxRenderTargetHandle;
            renderState.GfxDepthStencilRenderTargetHandle = m_GfxDepthStencilRenderTargetHandle;
            renderState.PrimitiveTopologyToRender = PrimitiveTopology::TriangleList;
            renderState.VertexFormatTypeToRender = vertexFormatType;
            renderState.OptionalRenderStateBlockStateOverride = nullptr;

            const ShaderHandler::ShaderRenderElementsForGraphics& shaderRenderElements = pShaderHandler->GetShaderRenderElementsForGraphicsCommands(*m_pGfxRenderDevice, renderState);

            DrawIndexedCommand* pDrawIndexedCommand = m_pGfxDirectRenderCommandList->DrawIndexed();
            pDrawIndexedCommand->gfxViewport = gfxViewport;
            pDrawIndexedCommand->gfxRenderTargetHandle = gfxRenderTargetHandle;
            pDrawIndexedCommand->gfxDepthStencilRenderTargetHandle = m_GfxDepthStencilRenderTargetHandle;
            pDrawIndexedCommand->gfxPipelineStateObjectHandle = shaderRenderElements.GfxGraphicsPipelineStateObjectHandle;
            pDrawIndexedCommand->gfxRootSignatureHandle = shaderRenderElements.GfxRootSignatureHandle;
            pDrawIndexedCommand->gfxDescriptorSetHandle = shaderRenderElements.GfxDescriptorSetHandle;
            pDrawIndexedCommand->gfxVertexBufferHandle = gfxVertexBufferHandle;
            pDrawIndexedCommand->gfxIndexBufferHandle = gfxIndexBufferHandle;
            pDrawIndexedCommand->vertexBufferOffset = 0;
            pDrawIndexedCommand->startIndexLocation = 0;
            pDrawIndexedCommand->indexBufferOffset = 0;
            pDrawIndexedCommand->baseVertexLocation = 0;
        }
    }

    RenderContext renderContext;
    renderContext.SetRenderDevice(m_pGfxRenderDevice);
    renderContext.SetRenderCommandList(m_pGfxDirectRenderCommandList);
    renderContext.SetViewSurface(m_pGfxViewSurface);

    m_pRenderer->PrepareRenderGraph();
    m_pRenderer->ExecuteRenderGraph(renderContext);

    m_pGfxDirectRenderCommandList->Close();

    GFXRenderCommandList* ppRenderCommandLists[] =
    {
        m_pGfxDirectRenderCommandList
    };

    m_pGfxDirectCommandQueue->ExecuteCommandLists(ppRenderCommandLists, 1);

    m_pGfxViewSurface->Flip();
}

shipBool ShipyardViewer::OnWin32Msg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT* shipyardMsgHandlingResult)
{
    if (msg == WM_DROPFILES)
    {
        HDROP dropHandle = (HDROP)wParam;

        constexpr UINT getNumberOfFilesDropped = 0xFFFFFFFF;
        UINT numberOfFileDropped = DragQueryFile(dropHandle, getNumberOfFilesDropped, nullptr, 0);

        for (UINT fileIndex = 0; fileIndex < numberOfFileDropped; fileIndex++)
        {
            constexpr shipUint32 maxFilenameLength = 1024;
            shipChar droppedFilename[maxFilenameLength];
            DragQueryFile(dropHandle, fileIndex, droppedFilename, maxFilenameLength);

            if (!MeshImporter::IsFileExtensionSupportedForMeshImport(droppedFilename))
            {
                continue;
            }

            ClearViewerAndLoadMesh(droppedFilename);
        }

        DragFinish(dropHandle);

        return true;
    }

    return ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam, shipyardMsgHandlingResult);
}

void ShipyardViewer::ClearViewerAndLoadMesh(const shipChar* filename)
{
    SHIP_DELETE(m_pGfxMesh);
    m_pGfxMesh = nullptr;
    
    for (GFXMaterial* gfxMaterial : m_LoadedMaterials)
    {
        SHIP_DELETE(gfxMaterial);
    }

    m_LoadedMaterials.Clear();

    MeshImporter::ImportedMesh importedMesh;
    MeshImporter::LoadMeshFromFile(filename, *m_pGfxRenderDevice, MeshImporter::None, &importedMesh);

    if (importedMesh.SubMeshes.Empty())
    {
        return;
    }

    for (const MeshImporter::ImportedSubMeshMaterial& importedSubMeshMaterial : importedMesh.SubMeshMaterials)
    {
        GFXMaterial*& gfxMaterial = m_LoadedMaterials.Grow();
        gfxMaterial = SHIP_NEW(GFXMaterial, 1);

        GFXTexture2DHandle materialTextures[] =
        {
            (importedSubMeshMaterial.SubMeshMaterialTextures[0].IsValid()) ? importedSubMeshMaterial.SubMeshMaterialTextures[0] : DefaultTextures::WhiteTexture,
            (importedSubMeshMaterial.SubMeshMaterialTextures[1].IsValid()) ? importedSubMeshMaterial.SubMeshMaterialTextures[1] : DefaultTextures::BlackTexture,
            (importedSubMeshMaterial.SubMeshMaterialTextures[2].IsValid()) ? importedSubMeshMaterial.SubMeshMaterialTextures[2] : DefaultTextures::BlackTexture,
            (importedSubMeshMaterial.SubMeshMaterialTextures[3].IsValid()) ? importedSubMeshMaterial.SubMeshMaterialTextures[3] : DefaultTextures::BlackTexture
        };

        gfxMaterial->Create(ShaderFamily::Generic, materialTextures);
    }

    m_pGfxMesh = SHIP_NEW(GFXMesh, 1);

    InplaceArray<GFXMesh::GFXSubMeshCreationData, 16> gfxSubMeshCreationDatas;

    for (shipUint32 i = 0; i < importedMesh.SubMeshes.Size(); i++)
    {
        MeshImporter::ImportedSubMesh& importedSubMesh = importedMesh.SubMeshes[i];

        constexpr shipBool dynamic = false;
        GFXVertexBufferHandle gfxVertexBufferHandle = m_pGfxRenderDevice->CreateVertexBuffer(
                importedSubMesh.GetSubMeshNumVertices(),
                importedSubMesh.SubMeshVertexFormatType,
                dynamic,
                &importedSubMesh.SubMeshVertices[0]);

        GFXIndexBufferHandle gfxIndexBufferHandle = m_pGfxRenderDevice->CreateIndexBuffer(
                importedSubMesh.GetSubMeshNumIndices(),
                importedSubMesh.GetSubMeshIndicesSize() == 2,
                dynamic,
                &importedSubMesh.SubMeshIndices[0]);

        GFXMaterial* usedMaterial = nullptr;

        if (importedSubMesh.ReferencedSubMeshMaterial == nullptr)
        {
            usedMaterial = m_pDefaultMaterial;
        }
        else
        {
            shipUint32 j = 0;

            for (; j < importedMesh.SubMeshMaterials.Size(); j++)
            {
                MeshImporter::ImportedSubMeshMaterial& importedSubMeshMaterial = importedMesh.SubMeshMaterials[j];

                if (&importedSubMeshMaterial == importedSubMesh.ReferencedSubMeshMaterial)
                {
                    usedMaterial = m_LoadedMaterials[j];
                    break;
                }
            }

            if (j == importedMesh.SubMeshMaterials.Size())
            {
                usedMaterial = m_pDefaultMaterial;
            }
        }

        GFXMesh::GFXSubMeshCreationData& gfxSubMeshCreationData =  gfxSubMeshCreationDatas.Grow();
        gfxSubMeshCreationData.gfxVertexBufferHandle = gfxVertexBufferHandle;
        gfxSubMeshCreationData.gfxIndexBufferHandle = gfxIndexBufferHandle;
        gfxSubMeshCreationData.gfxMaterial = usedMaterial;
    }

    m_pGfxMesh->SetSubMeshes(*m_pGfxRenderDevice, gfxSubMeshCreationDatas);
}

}