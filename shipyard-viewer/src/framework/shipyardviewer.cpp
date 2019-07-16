#include "shipyardviewer.h"

#include <graphics/wrapper/wrapper.h>

#include <graphics/material/materialunifiedconstantbuffer.h>

#include <graphics/shadercompiler/shadercompiler.h>
#include <graphics/shadercompiler/shaderwatcher.h>

#include <graphics/shader/shaderdatabase.h>
#include <graphics/shader/shaderfamilies.h>
#include <graphics/shader/shaderhandler.h>
#include <graphics/shader/shaderhandlermanager.h>
#include <graphics/shader/shaderinputprovider.h>
#include <graphics/shader/shaderresourcebinder.h>
#include <graphics/shader/shaderkey.h>
#include <graphics/shader/shaderoptions.h>

#include <graphics/graphicssingletonstorer.h>

#include <system/logger.h>

#include <extern/glm/gtc/matrix_transform.hpp>
#include <extern/glm/gtc/type_ptr.hpp>

namespace Shipyard
{;

struct SimpleConstantBufferProvider : public BaseShaderInputProvider<SimpleConstantBufferProvider>
{
    shipMat4x4 Matrix;
    GFXTexture2DHandle TestTexture;
};

SHIP_DECLARE_SHADER_INPUT_PROVIDER_BEGIN(SimpleConstantBufferProvider, PerInstance)
{
    SHIP_SCALAR_SHADER_INPUT("Test", Matrix);
    SHIP_TEXTURE2D_SHADER_INPUT(ShaderInputScalarType::Float4, "TestTexture", TestTexture);
}
SHIP_DECLARE_SHADER_INPUT_PROVIDER_END(SimpleConstantBufferProvider)

ShipyardViewer::~ShipyardViewer()
{
    SHIP_DELETE(m_pDataProvider);

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

    m_WindowWidth = windowWidth;
    m_WindowHeight = windowHeight;

    size_t heapSize = 256 * 1024 * 1024;
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

    m_pGraphicsSingletonStorer = SHIP_NEW(GraphicsSingletonStorer, 1);

    m_pGfxRenderDevice = SHIP_NEW(GFXRenderDevice, 1);
    m_pGfxRenderDevice->Create();

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

    Vertex_Pos_UV vertexBufferData[] =
    {
        { { -0.5f, -0.5f, -0.25f }, { 0.0f, 0.0f } },
        { { -0.5f,  0.5f, -0.25f }, { 0.0f, 1.0f } },
        { {  0.5f,  0.5f, -0.25f }, { 1.0f, 1.0f } },
        { {  0.5f, -0.5f, -0.25f }, { 1.0f, 0.0f } },

        { { -0.5f, -0.5f,  0.25f }, { 0.0f, 0.0f } },
        { { -0.5f,  0.5f,  0.25f }, { 0.0f, 1.0f } },
        { {  0.5f,  0.5f,  0.25f }, { 1.0f, 1.0f } },
        { {  0.5f, -0.5f,  0.25f }, { 1.0f, 0.0f } }
    };

    unsigned short indices[] =
    {
        0, 1, 2,
        0, 2, 3,

        3, 2, 6,
        3, 6, 7,

        4, 5, 1,
        4, 1, 0,

        1, 5, 6,
        1, 6, 2,

        4, 0, 3,
        4, 3, 7,

        7, 6, 5,
        7, 5, 4
    };

    shipUint8 textureData[] =
    {
        255, 255, 255, 255, 255, 0, 0, 255,
        255, 0, 0, 255, 255, 0, 0, 255
    };

    m_VertexBufferHandle = m_pGfxRenderDevice->CreateVertexBuffer(8, VertexFormatType::Pos_UV, false, vertexBufferData);
    m_IndexBufferHandle = m_pGfxRenderDevice->CreateIndexBuffer(36, true, false, indices);
    m_TextureHandle = m_pGfxRenderDevice->CreateTexture2D(2, 2, GfxFormat::R8G8B8A8_UNORM, false, textureData, false);

    InplaceArray<DescriptorSetEntryDeclaration, 2> descriptorSetEntryDeclarations;
    DescriptorSetEntryDeclaration& unifiedBufferEntry = descriptorSetEntryDeclarations.Grow();
    unifiedBufferEntry.rootIndex = 0;
    unifiedBufferEntry.numResources = 1;
    unifiedBufferEntry.descriptorRangeIndex = 0;

    DescriptorSetEntryDeclaration& textureEntry = descriptorSetEntryDeclarations.Grow();
    textureEntry.rootIndex = 0;
    textureEntry.numResources = 1;
    textureEntry.descriptorRangeIndex = 1;

    m_GfxDescriptorSetHandle =
        m_pGfxRenderDevice->CreateDescriptorSet(DescriptorSetType::ConstantBuffer_ShaderResource_UnorderedAccess_Views, descriptorSetEntryDeclarations);

    m_TestTextureHandle = m_pGfxRenderDevice->CreateTexture2D(windowWidth, windowHeight, GfxFormat::R8G8B8A8_UNORM, false, nullptr, false, TextureUsage::TextureUsage_RenderTarget);

    GFXTexture2DHandle renderTargetTextures[] = { m_TestTextureHandle };
    m_TestRenderTargetHandle = m_pGfxRenderDevice->CreateRenderTarget(&renderTargetTextures[0], 1);

    GetShaderInputProviderManager().Initialize(*m_pGfxRenderDevice);

    m_pDataProvider = SHIP_NEW(SimpleConstantBufferProvider, 1);

    GetGFXMaterialUnifiedConstantBuffer().Create(*m_pGfxRenderDevice, nullptr, 4 * 1024 * 1024);

    m_pShaderDatabase = SHIP_NEW(ShaderDatabase, 1);
    m_pShaderDatabase->Load("ShipyardShaderDatabase.bin");

    GetShaderHandlerManager().Initialize(*m_pGfxRenderDevice, *m_pShaderDatabase);

    ShaderKey shaderKey;
    shaderKey.SetShaderFamily(ShaderFamily::Generic);
    GetShaderHandlerManager().GetShaderHandlerForShaderKey(shaderKey);

    Sleep(2000);

    return true;
}

void ShipyardViewer::ComputeOneFrame()
{
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

    static ShaderKey shaderKey;
    shaderKey.SetShaderFamily(ShaderFamily::Generic);

    m_pGfxDirectRenderCommandList->Reset(*m_pGfxCommandListAllocator, nullptr);

    glm::mat4 matrix(1.0f, 0.0f, 0.0f, 0.0f,
                     0.0f, 1.0f, 0.0f, 0.0f,
                     0.0f, 0.0f, 1.0f, 0.25f,
                     0.0f, 0.0f, 0.0f, 1.0f);

    m_pDataProvider->Matrix = glm::rotate(matrix, theta, shipVec3(0.0f, 1.0f, 0.0f));
    m_pDataProvider->TestTexture = m_TextureHandle;

    InplaceArray<ShaderInputProvider*, 1> shaderInputProviders;
    shaderInputProviders.Add(m_pDataProvider);

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

    static volatile shipUint32 value = 0;

    SET_SHADER_OPTION(shaderKey, Test2Bits, value);

    ShaderHandler* pShaderHandler = GetShaderHandlerManager().GetShaderHandlerForShaderKey(shaderKey);

    pShaderHandler->ApplyShaderInputProviders(*m_pGfxRenderDevice, *m_pGfxDirectRenderCommandList, shaderInputProviders);

    shipUint32 vertexBufferOffsets = 0;

    DrawIndexedCommand* pDrawIndexedCommand = m_pGfxDirectRenderCommandList->DrawIndexed();
    pDrawIndexedCommand->gfxViewport = gfxViewport;
    pDrawIndexedCommand->shaderKey = shaderKey;
    pDrawIndexedCommand->gfxRenderTargetHandle = gfxRenderTargetHandle;
    pDrawIndexedCommand->gfxDepthStencilRenderTargetHandle = m_GfxDepthStencilRenderTargetHandle;
    pDrawIndexedCommand->gfxDescriptorSetHandle = m_GfxDescriptorSetHandle;
    pDrawIndexedCommand->primitiveTopologyToUse = PrimitiveTopology::TriangleList;
    pDrawIndexedCommand->pRenderStateBlockStateOverride = nullptr;
    pDrawIndexedCommand->pGfxVertexBufferHandles = &m_VertexBufferHandle;
    pDrawIndexedCommand->gfxIndexBufferHandle = m_IndexBufferHandle;
    pDrawIndexedCommand->startSlot = 0;
    pDrawIndexedCommand->numVertexBuffers = 1;
    pDrawIndexedCommand->startVertexLocation = 0;
    pDrawIndexedCommand->pVertexBufferOffsets = &vertexBufferOffsets;
    pDrawIndexedCommand->startVertexLocation = 0;
    pDrawIndexedCommand->startIndexLocation = 0;
    pDrawIndexedCommand->indexBufferOffset = 0;

    m_pGfxDirectRenderCommandList->Close();

    GFXRenderCommandList* ppRenderCommandLists[] =
    {
        m_pGfxDirectRenderCommandList
    };

    m_pGfxDirectCommandQueue->ExecuteCommandLists(ppRenderCommandLists, 1);

    m_pGfxViewSurface->Flip();
}

}