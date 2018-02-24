#include <common/wrapper/wrapper.h>

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

    Shipyard::RasterizerState rasterizerState;
    rasterizerState.m_AntialiasedLineEnable = false;
    rasterizerState.m_CullMode = Shipyard::CullMode::CullBackFace;
    rasterizerState.m_DepthBias = 0;
    rasterizerState.m_DepthBiasClamp = 0.0f;
    rasterizerState.m_DepthClipEnable = false;
    rasterizerState.m_FillMode = Shipyard::FillMode::Solid;
    rasterizerState.m_IsFrontCounterClockWise = false;
    rasterizerState.m_MultisampleEnable = false;
    rasterizerState.m_ScissorEnable = false;
    rasterizerState.m_SlopeScaledDepthBias = 0.0f;
    gfxRenderDeviceContext.SetRasterizerState(rasterizerState);

    Shipyard::DepthStencilState depthStencilState;
    depthStencilState.m_DepthEnable = true;
    depthStencilState.m_EnableDepthWrite = true;
    depthStencilState.m_DepthComparisonFunc = Shipyard::ComparisonFunc::Less;
    depthStencilState.m_StencilEnable = false;
    depthStencilState.m_StencilReadMask = 0;
    depthStencilState.m_StencilWriteMask = 0;
    gfxRenderDeviceContext.SetDepthStencilState(depthStencilState, 0);

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

    shared_ptr<Shipyard::GFXVertexBuffer> vertexBuffer(gfxRenderDevice.CreateVertexBuffer(8, Shipyard::VertexFormatType::Pos_UV, false, vertexBufferData));
    shared_ptr<Shipyard::GFXIndexBuffer> indexBuffer(gfxRenderDevice.CreateIndexBuffer(36, true, false, indices));
    shared_ptr<Shipyard::GFXConstantBuffer> constantBuffer(gfxRenderDevice.CreateConstantBuffer(sizeof(data), true, &data));
    shared_ptr<Shipyard::GFXTexture2D> texture(gfxRenderDevice.CreateTexture2D(2, 2, Shipyard::GfxFormat::R8G8B8A8_UNORM, false, textureData, false));

    Shipyard::String vertexShaderSource = "cbuffer constantBuffer : register(b0) { float4x4 mat; }; struct vs_input { float3 pos : POSITION; float2 uv : TEXCOORD; }; struct vs_output { float4 pos : SV_POSITION; float2 uv: TEXCOORD; }; "
        "vs_output main(vs_input input) { vs_output output; output.pos = mul(mat, float4(input.pos.xyz, 1.0)); output.uv = input.uv; return output; }";

    shared_ptr<Shipyard::GFXVertexShader> vertexShader(gfxRenderDevice.CreateVertexShader(vertexShaderSource));

    Shipyard::String pixelShaderSource = "SamplerState testSampler\n{\nFilter = MIN_MAG_MIP_LINEAR;\nAddressU = Wrap;\nAddressV = Wrap;\n};\nTexture2D tex : register(t0);\n"
        "struct ps_input\n{\nfloat4 pos : SV_POSITION;\nfloat2 uv: TEXCOORD;\n};\nstruct ps_output\n{\nfloat4 color : SV_TARGET;\n};\n"
        "ps_output main(ps_input input) {\nps_output output;\noutput.color = tex.Sample(testSampler, input.uv);\nreturn output;\n}";

    shared_ptr<Shipyard::GFXPixelShader> pixelShader(gfxRenderDevice.CreatePixelShader(pixelShaderSource));

    g_IsOpen = true;

    float theta = 0.0f;

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
            void* data = constantBuffer->Map(Shipyard::MapFlag::Write_Discard);

            glm::mat4 matrix(1.0f, 0.0f, 0.0f, 0.0f,
                             0.0f, 1.0f, 0.0f, 0.0f,
                             0.0f, 0.0f, 1.0f, 0.25f,
                             0.0f, 0.0f, 0.0f, 1.0f);
            ((SimpleConstantBuffer*)data)->m_Matrix = glm::rotate(matrix, theta, glm::vec3(0.0f, 1.0f, 0.0f));

            constantBuffer->Unmap();

            theta += 0.001f;

            gfxViewSurface.PreRender();

            gfxRenderDeviceContext.SetVertexShader(vertexShader.get());
            gfxRenderDeviceContext.SetPixelShader(pixelShader.get());
            gfxRenderDeviceContext.SetVertexShaderConstantBuffer(constantBuffer.get(), 0);
            gfxRenderDeviceContext.SetPixelShaderTexture(texture.get(), 0);
            gfxRenderDeviceContext.DrawIndexed(Shipyard::PrimitiveTopology::TriangleList, *(vertexBuffer.get()), *(indexBuffer.get()), 0, 0);

            gfxViewSurface.Flip();
        }
    }

    return 0;
}