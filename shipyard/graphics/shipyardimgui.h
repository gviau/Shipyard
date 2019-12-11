#pragma once

#include <graphics/wrapper/wrapper_common.h>

#include <system/platform.h>

#include <extern/imgui/imgui.h>

namespace Shipyard
{
    shipBool SHIPYARD_API InitializeImGui(void* windowHandle, GFXRenderDevice& gfxRenderDevice);
    void SHIPYARD_API StartNewImGuiFrame();
    shipBool SHIPYARD_API ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT* shipyardMsgHandlingResult);
}