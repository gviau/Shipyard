#include <common/wrapper/wrapper.h>

#include <common/shadercompiler/shadercompiler.h>
#include <common/shadercompiler/shaderwatcher.h>

#include <common/shaderdatabase.h>
#include <common/shaderfamilies.h>
#include <common/shaderhandler.h>
#include <common/shaderhandlermanager.h>
#include <common/shaderkey.h>
#include <common/shaderoptions.h>

#include <system/singletonstorer.h>

#include <memory>
using namespace std;

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

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
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

    HWND windowHandle = CreateWindowEx(NULL,"ShipyardViewer", "Shipyard Viewer", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, NULL, NULL, hInstance, NULL);
    if (windowHandle == NULL)
    {
        MessageBox(NULL, "Couldn't create window", "Win32 error", MB_OK);
        return 1;
    }

    ShowWindow(windowHandle, nShowCmd);

    Shipyard::GFXRenderDevice gfxRenderDevice;
    Shipyard::GFXRenderDeviceContext gfxRenderDeviceContext(gfxRenderDevice);
    Shipyard::GFXViewSurface gfxViewSurface(gfxRenderDevice, gfxRenderDeviceContext, 800, 600, Shipyard::GfxFormat::R8G8B8A8_UNORM, windowHandle);

    Shipyard::Array<Shipyard::RootSignatureParameterEntry> rootSignatureParameters;
    rootSignatureParameters.Resize(2);

    rootSignatureParameters[0].parameterType = Shipyard::RootSignatureParameterType::ConstantBufferView;
    rootSignatureParameters[0].shaderVisibility = Shipyard::ShaderVisibility_Vertex;
    rootSignatureParameters[0].descriptor.shaderBindingSlot = 0;

    rootSignatureParameters[1].parameterType = Shipyard::RootSignatureParameterType::ShaderResourceView;
    rootSignatureParameters[1].shaderVisibility = Shipyard::ShaderVisibility_Pixel;
    rootSignatureParameters[1].descriptor.shaderBindingSlot = 0;

    unique_ptr<Shipyard::GFXRootSignature> gfxRootSignature(gfxRenderDevice.CreateRootSignature(rootSignatureParameters));

    gfxRenderDeviceContext.SetViewport(0.0f, 0.0f, 800.0f, 600.0f);

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

    unique_ptr<Shipyard::GFXVertexBuffer> vertexBuffer(gfxRenderDevice.CreateVertexBuffer(8, Shipyard::VertexFormatType::Pos_UV, false, vertexBufferData));
    unique_ptr<Shipyard::GFXIndexBuffer> indexBuffer(gfxRenderDevice.CreateIndexBuffer(36, true, false, indices));
    unique_ptr<Shipyard::GFXConstantBuffer> constantBuffer(gfxRenderDevice.CreateConstantBuffer(sizeof(data), true, &data));
    unique_ptr<Shipyard::GFXTexture2D> texture(gfxRenderDevice.CreateTexture2D(2, 2, Shipyard::GfxFormat::R8G8B8A8_UNORM, false, textureData, false));

    unique_ptr<Shipyard::GFXDescriptorSet> gfxDescriptorSet(
            gfxRenderDevice.CreateDescriptorSet(Shipyard::DescriptorSetType::ConstantBuffer_ShaderResource_UnorderedAccess_Views, *gfxRootSignature.get()));

    gfxDescriptorSet->SetDescriptorForRootIndex(0, *constantBuffer.get());
    gfxDescriptorSet->SetDescriptorForRootIndex(1, *texture.get());

    g_IsOpen = true;

    float theta = 0.0f;

    Shipyard::SingletonStorer singletonStorer;

    Shipyard::StringT shaderDirectory = "c:\\Sandbox\\shipyard\\shipyard-viewer\\shaders\\";
    Shipyard::ShaderCompiler::GetInstance().SetShaderDirectoryName(shaderDirectory);
    Shipyard::ShaderWatcher::GetInstance().SetShaderDirectoryName(shaderDirectory);

    Shipyard::ShaderDatabase shaderDatabase;
    shaderDatabase.Load("C:\\Sandbox\\shipyard\\generated-projects\\ShipyardShaderDatabase.bin");

    Shipyard::ShaderHandlerManager::GetInstance().SetShaderDatabase(shaderDatabase);

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
            void* mappedData = constantBuffer->Map(Shipyard::MapFlag::Write_Discard);

            glm::mat4 matrix(1.0f, 0.0f, 0.0f, 0.0f,
                             0.0f, 1.0f, 0.0f, 0.0f,
                             0.0f, 0.0f, 1.0f, 0.25f,
                             0.0f, 0.0f, 0.0f, 1.0f);
            ((SimpleConstantBuffer*)mappedData)->m_Matrix = glm::rotate(matrix, theta, glm::vec3(0.0f, 1.0f, 0.0f));

            constantBuffer->Unmap();

            theta += 0.001f;

            gfxViewSurface.PreRender();

            static volatile uint32_t value = 0;

            SET_SHADER_OPTION(shaderKey, Test2Bits, value);

            Shipyard::ShaderHandler* shaderHandler = Shipyard::ShaderHandlerManager::GetInstance().GetShaderHandlerForShaderKey(shaderKey, gfxRenderDevice);

            Shipyard::DrawItem drawItem(*gfxRootSignature.get(), *gfxDescriptorSet.get(), *shaderHandler, Shipyard::PrimitiveTopology::TriangleList);

            gfxRenderDeviceContext.DrawIndexed(drawItem, *(vertexBuffer.get()), *(indexBuffer.get()), 0, 0);

            gfxViewSurface.Flip();
        }
    }

    return 0;
}