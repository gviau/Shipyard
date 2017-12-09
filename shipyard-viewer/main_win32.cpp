#include <common/wrapper/wrapper.h>

#include <memory>
using namespace std;

#include <windows.h>

bool g_IsOpen = false;

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

    Shipyard::RasterizerState rasterizerState;
    rasterizerState.m_AntialiasedLineEnable = false;
    rasterizerState.m_CullMode = Shipyard::CullMode::CullBackFace;
    rasterizerState.m_DepthBias = 0;
    rasterizerState.m_DepthBiasClamp = 0.0f;
    rasterizerState.m_DepthClipEnable = false;
    rasterizerState.m_FillMode = Shipyard::FillMode::Solid;
    rasterizerState.m_IsFrontCounterClockWise = true;
    rasterizerState.m_MultisampleEnable = false;
    rasterizerState.m_ScissorEnable = false;
    rasterizerState.m_SlopeScaledDepthBias = 0.0f;
    gfxRenderDeviceContext.SetRasterizerState(rasterizerState);

    gfxRenderDeviceContext.SetViewport(0.0f, 0.0f, 800.0f, 600.0f);

    Shipyard::Vertex_Pos_Color vertexBufferData[] =
    {
        { {  0.5f, -0.5f, 0.0f}, { 1.0f, 0.0f, 0.0f, } },
        { {  0.0f,  0.5f, 0.0f}, { 0.0f, 1.0f, 0.0f, } },
        { { -0.5f, -0.5f, 0.0f}, { 0.0f, 0.0f, 1.0f  } }
    };

    shared_ptr<Shipyard::GFXVertexBuffer> vertexBuffer(gfxRenderDevice.CreateVertexBuffer(3, Shipyard::VertexFormatType::Pos_Color, false, vertexBufferData));

    Shipyard::String vertexShaderSource = "struct vs_input { float2 pos : POSITION; float3 color : COLOR; }; struct vs_output { float4 pos : SV_POSITION; float3 color : TEXCOORD; }; "
        "vs_output main(vs_input input) { vs_output output; output.pos = float4(input.pos.xy, 0.0, 1.0); output.color = input.color; return output; }";

    shared_ptr<Shipyard::GFXVertexShader> vertexShader(gfxRenderDevice.CreateVertexShader(vertexShaderSource));

    Shipyard::String pixelShaderSource = "struct ps_input { float4 pos : SV_POSITION; float3 color : TEXCOORD; }; struct ps_output { float4 color : SV_TARGET; }; "
        "ps_output main(ps_input input) { ps_output output; output.color = float4(input.color, 1.0); return output; }";

    shared_ptr<Shipyard::GFXPixelShader> pixelShader(gfxRenderDevice.CreatePixelShader(pixelShaderSource));

    g_IsOpen = true;

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
            gfxViewSurface.PreRender();

            gfxRenderDeviceContext.SetVertexShader(vertexShader.get());
            gfxRenderDeviceContext.SetPixelShader(pixelShader.get());
            gfxRenderDeviceContext.Draw(Shipyard::PrimitiveTopology::TriangleList, *(vertexBuffer.get()), 0);

            gfxViewSurface.Flip();
        }
    }

    return 0;
}