#include <graphics/wrapper/wrapper.h>

#include <graphics/shadercompiler/shadercompiler.h>
#include <graphics/shadercompiler/shaderwatcher.h>

#include <graphics/shaderdatabase.h>
#include <graphics/shaderfamilies.h>
#include <graphics/shaderhandler.h>
#include <graphics/shaderhandlermanager.h>
#include <graphics/shaderkey.h>
#include <graphics/shaderoptions.h>

#include <system/memory.h>
#include <system/memory/fixedheapallocator.h>
#include <system/memory/poolallocator.h>

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
#include <system/memory/debugallocator.h>
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

#include <system/singletonstorer.h>

#include <windows.h>

bool g_IsOpen = false;

#include <extern/glm/glm.hpp>
#include <extern/glm/gtc/matrix_transform.hpp>
#include <extern/glm/gtc/type_ptr.hpp>

struct SimpleConstantBuffer
{
    glm::mat4x4 m_Matrix;
};

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CLOSE:
        DestroyWindow(hWnd);
        g_IsOpen = false;
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }

    return 0;
}

#include <signal.h>

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    signal(SIGABRT, [](int)
    {
        __debugbreak();
    });

    WNDCLASSEX wndClass;
    ZeroMemory(&wndClass, sizeof(wndClass));
    wndClass.cbSize = sizeof(WNDCLASSEX);
    wndClass.style = CS_HREDRAW | CS_VREDRAW;
    wndClass.lpfnWndProc = WndProc;
    wndClass.hInstance = hInstance;
    wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndClass.hbrBackground = HBRUSH(COLOR_WINDOW);
    wndClass.lpszClassName = "ShipyardViewer";

    if (!RegisterClassEx(&wndClass))
    {
        MessageBox(NULL, "Couldn't register window class", "Win32 error", MB_OK);
        return 1;
    }

    uint32_t windowWidth = 800;
    uint32_t windowHeight = 600;

    HWND windowHandle = CreateWindowEx(NULL,"ShipyardViewer", "Shipyard Viewer", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, windowWidth, windowHeight, NULL, NULL, hInstance, NULL);
    if (windowHandle == NULL)
    {
        MessageBox(NULL, "Couldn't create window", "Win32 error", MB_OK);
        return 1;
    }

    ShowWindow(windowHandle, nShowCmd);

    size_t heapSize = 256 * 1024 * 1024;
    void* pHeap = malloc(heapSize);

    Shipyard::PoolAllocator poolAllocator16;
    Shipyard::PoolAllocator poolAllocator32;
    Shipyard::PoolAllocator poolAllocator64;
    Shipyard::FixedHeapAllocator fixedHeapAllocator;

    size_t numChunks = 256 * 1024;
    
    void* pHeap16 = reinterpret_cast<void*>(Shipyard::AlignAddress(size_t(pHeap), 16));
    void* pHeap32 = reinterpret_cast<void*>(Shipyard::AlignAddress(size_t(pHeap16) + numChunks * 16, 32));
    void* pHeap64 = reinterpret_cast<void*>(Shipyard::AlignAddress(size_t(pHeap32) + numChunks * 32, 64));

    void* pFixedHeap = reinterpret_cast<void*>(size_t(pHeap64) + numChunks * 64);
    size_t fixedHeapSize = heapSize - (size_t(pHeap64) + numChunks * 64 - size_t(pHeap));

    poolAllocator16.Create(pHeap16, numChunks, 16);
    poolAllocator32.Create(pHeap32, numChunks, 32);
    poolAllocator64.Create(pHeap64, numChunks, 64);
    fixedHeapAllocator.Create(pFixedHeap, fixedHeapSize);

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
    size_t debugHeapSize = 32 * 1024 * 1024;
    void* pDebugHeap = malloc(debugHeapSize);

    Shipyard::DebugAllocator::GetInstance().Create(pDebugHeap, debugHeapSize);

    poolAllocator16.SetAllocatorDebugName("PoolAllocator 16 bytes");
    poolAllocator32.SetAllocatorDebugName("PoolAllocator 32 bytes");
    poolAllocator64.SetAllocatorDebugName("PoolAllocator 64 bytes");
    fixedHeapAllocator.SetAllocatorDebugName("FixedHeapAllocator");
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

    Shipyard::GlobalAllocator::AllocatorInitEntry allocatorInitEntries[4];
    allocatorInitEntries[0].pAllocator = &poolAllocator16;
    allocatorInitEntries[0].maxAllocationSize = 16;
    allocatorInitEntries[1].pAllocator = &poolAllocator32;
    allocatorInitEntries[1].maxAllocationSize = 32;
    allocatorInitEntries[2].pAllocator = &poolAllocator64;
    allocatorInitEntries[2].maxAllocationSize = 64;
    allocatorInitEntries[3].pAllocator = &fixedHeapAllocator;
    allocatorInitEntries[3].maxAllocationSize = 0;

    Shipyard::GlobalAllocator::GetInstance().Create(allocatorInitEntries, 4);
    {
        Shipyard::SingletonStorer singletonStorer;

        Shipyard::GFXRenderDevice gfxRenderDevice;
        gfxRenderDevice.Create();

        Shipyard::GFXRenderDeviceContext gfxRenderDeviceContext(gfxRenderDevice);

        Shipyard::GFXViewSurface gfxViewSurface;
        bool isValid = gfxViewSurface.Create(gfxRenderDevice, gfxRenderDeviceContext, windowWidth, windowHeight, Shipyard::GfxFormat::R8G8B8A8_UNORM, windowHandle);

        SHIP_ASSERT(isValid);

        Shipyard::GFXTexture2DHandle gfxDepthTextureHandle = gfxRenderDevice.CreateTexture2D(
                windowWidth,
                windowHeight,
                Shipyard::GfxFormat::D24_UNORM_S8_UINT,
                false,
                nullptr,
                false,
                Shipyard::TextureUsage::TextureUsage_DepthStencil);

        Shipyard::GFXDepthStencilRenderTargetHandle gfxDepthStencilRenderTargetHandle = gfxRenderDevice.CreateDepthStencilRenderTarget(gfxDepthTextureHandle);

        Shipyard::Array<Shipyard::RootSignatureParameterEntry> rootSignatureParameters;
        rootSignatureParameters.Resize(2);

        rootSignatureParameters[0].parameterType = Shipyard::RootSignatureParameterType::ConstantBufferView;
        rootSignatureParameters[0].shaderVisibility = Shipyard::ShaderVisibility_Vertex;
        rootSignatureParameters[0].descriptor.shaderBindingSlot = 0;

        rootSignatureParameters[1].parameterType = Shipyard::RootSignatureParameterType::ShaderResourceView;
        rootSignatureParameters[1].shaderVisibility = Shipyard::ShaderVisibility_Pixel;
        rootSignatureParameters[1].descriptor.shaderBindingSlot = 0;

        Shipyard::GFXRootSignatureHandle gfxRootSignatureHandle = gfxRenderDevice.CreateRootSignature(rootSignatureParameters);
        Shipyard::GFXRootSignature& gfxRootSignature = gfxRenderDevice.GetRootSignature(gfxRootSignatureHandle);

        Shipyard::GfxViewport gfxViewport;
        gfxViewport.topLeftX = 0.0f;
        gfxViewport.topLeftY = 0.0f;
        gfxViewport.width = float(windowWidth);
        gfxViewport.height = float(windowHeight);
        gfxViewport.minDepth = 0.0f;
        gfxViewport.maxDepth = 1.0f;

        Shipyard::Vertex_Pos_UV vertexBufferData[] =
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

        SimpleConstantBuffer data =
        {
            glm::mat4x4(1.0f, 0.0f, 0.0f, 0.0f,
                        0.0f, 1.0f, 0.0f, 0.0f,
                        0.0f, 0.0f, 1.0f, 0.0f,
                        0.0f, 0.0f, 0.0f, 1.0f)
        };

        uint8_t textureData[] =
        {
            255, 255, 255, 255, 255, 0, 0, 255,
            255, 0, 0, 255, 255, 0, 0, 255
        };

        Shipyard::GFXVertexBufferHandle vertexBufferHandle = gfxRenderDevice.CreateVertexBuffer(8, Shipyard::VertexFormatType::Pos_UV, false, vertexBufferData);
        Shipyard::GFXIndexBufferHandle indexBufferHandle = gfxRenderDevice.CreateIndexBuffer(36, true, false, indices);
        Shipyard::GFXConstantBufferHandle constantBufferHandle = gfxRenderDevice.CreateConstantBuffer(sizeof(data), true, &data);
        Shipyard::GFXTexture2DHandle textureHandle = gfxRenderDevice.CreateTexture2D(2, 2, Shipyard::GfxFormat::R8G8B8A8_UNORM, false, textureData, false);

        Shipyard::GFXConstantBuffer& constantBuffer = gfxRenderDevice.GetConstantBuffer(constantBufferHandle);
        Shipyard::GFXTexture2D& texture = gfxRenderDevice.GetTexture2D(textureHandle);

        Shipyard::GFXDescriptorSetHandle gfxDescriptorSetHandle =
                gfxRenderDevice.CreateDescriptorSet(Shipyard::DescriptorSetType::ConstantBuffer_ShaderResource_UnorderedAccess_Views, gfxRootSignature);

        Shipyard::GFXDescriptorSet& gfxDescriptorSet = gfxRenderDevice.GetDescriptorSet(gfxDescriptorSetHandle);

        gfxDescriptorSet.SetDescriptorForRootIndex(0, constantBuffer);
        gfxDescriptorSet.SetDescriptorForRootIndex(1, texture);

        Shipyard::GFXTexture2DHandle testTextureHandle = gfxRenderDevice.CreateTexture2D(windowWidth, windowHeight, Shipyard::GfxFormat::R8G8B8A8_UNORM, false, nullptr, false, Shipyard::TextureUsage::TextureUsage_RenderTarget);
    
        Shipyard::GFXTexture2DHandle renderTargetTextures[] = { testTextureHandle };
        Shipyard::GFXRenderTargetHandle testRenderTargetHandle = gfxRenderDevice.CreateRenderTarget(&renderTargetTextures[0], 1);

        g_IsOpen = true;

        float theta = 0.0f;

        const char* shaderDirectory = "c:\\Sandbox\\shipyard\\shipyard-viewer\\shaders\\";
        Shipyard::ShaderCompiler::GetInstance().SetShaderDirectoryName(shaderDirectory);
        Shipyard::ShaderWatcher::GetInstance().SetShaderDirectoryName(shaderDirectory);

        Shipyard::ShaderDatabase shaderDatabase;
        shaderDatabase.Load("C:\\Sandbox\\shipyard\\generated-projects\\ShipyardShaderDatabase.bin");

        Shipyard::ShaderHandlerManager::GetInstance().Initialize(gfxRenderDevice, shaderDatabase);

        Shipyard::ShaderKey shaderKey;
        shaderKey.SetShaderFamily(Shipyard::ShaderFamily::Generic);

        MSG msg;
        while (g_IsOpen)
        {
            if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            else
            {
                void* mappedData = constantBuffer.Map(Shipyard::MapFlag::Write_Discard);

                glm::mat4 matrix(1.0f, 0.0f, 0.0f, 0.0f,
                                 0.0f, 1.0f, 0.0f, 0.0f,
                                 0.0f, 0.0f, 1.0f, 0.25f,
                                 0.0f, 0.0f, 0.0f, 1.0f);
                ((SimpleConstantBuffer*)mappedData)->m_Matrix = glm::rotate(matrix, theta, glm::vec3(0.0f, 1.0f, 0.0f));

                constantBuffer.Unmap();

                theta += 0.001f;

                Shipyard::GFXRenderTargetHandle gfxRenderTargetHandle = gfxViewSurface.GetBackBufferRenderTargetHandle();

                gfxRenderDeviceContext.ClearFullRenderTarget(gfxRenderTargetHandle, 0.0f, 0.0f, 0.125f, 1.0f);
                gfxRenderDeviceContext.ClearDepthStencilRenderTarget(gfxDepthStencilRenderTargetHandle, Shipyard::DepthStencilClearFlag::Depth, 1.0f, 0);

                static volatile uint32_t value = 0;

                SET_SHADER_OPTION(shaderKey, Test2Bits, value);

                Shipyard::ShaderHandler* shaderHandler = Shipyard::ShaderHandlerManager::GetInstance().GetShaderHandlerForShaderKey(shaderKey);

                Shipyard::DrawItem drawItem(
                        gfxRenderTargetHandle,
                        gfxDepthStencilRenderTargetHandle,
                        gfxViewport,
                        gfxRootSignatureHandle,
                        gfxDescriptorSetHandle,
                        *shaderHandler,
                        Shipyard::PrimitiveTopology::TriangleList);

                Shipyard::GFXVertexBufferHandle* gfxVertexBufferHandle = &vertexBufferHandle;
                uint32_t vertexBufferOffsets = 0;
                gfxRenderDeviceContext.DrawIndexed(drawItem, gfxVertexBufferHandle, 0, 1, &vertexBufferOffsets, indexBufferHandle, 0, 0, 0);

                gfxViewSurface.Flip();
            }
        }

        Shipyard::ShaderHandlerManager::GetInstance().Destroy();

        Shipyard::ShaderCompiler::GetInstance().StopThread();
        Shipyard::ShaderWatcher::GetInstance().StopThread();

        shaderDatabase.Close();
    }

#ifdef SHIP_ALLOCATOR_DEBUG_INFO
    Shipyard::DebugAllocator::GetInstance().Destroy();

    free(pDebugHeap);
#endif // #ifdef SHIP_ALLOCATOR_DEBUG_INFO

    fixedHeapAllocator.Destroy();
    poolAllocator64.Destroy();
    poolAllocator32.Destroy();
    poolAllocator16.Destroy();

    Shipyard::GlobalAllocator::GetInstance().Destroy();

    free(pHeap);

    return 0;
}