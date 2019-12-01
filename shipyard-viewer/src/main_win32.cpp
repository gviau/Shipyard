#include "framework/shipyardviewer.h"

#include <cinttypes>

#include <windows.h>

bool g_IsOpen = false;
Shipyard::ShipyardViewer* g_ShipyardViewer = nullptr;

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    LRESULT shipyardMsgHandlingResult = 0;
    if (g_ShipyardViewer != nullptr && g_ShipyardViewer->OnWin32Msg(hWnd, msg, wParam, lParam, &shipyardMsgHandlingResult))
    {
        return shipyardMsgHandlingResult;
    }

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

    Shipyard::ShipyardViewer shipyardViewer;
    g_IsOpen = shipyardViewer.CreateApp(windowHandle, windowWidth, windowHeight);

    g_ShipyardViewer = &shipyardViewer;

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
            shipyardViewer.ComputeOneFrame();
        }
    }

    return 0;
}