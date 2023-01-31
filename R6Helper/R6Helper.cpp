#include <iostream>
#include <cstdlib>
#include <windows.h>
#include "libs/imgui/imgui.h"
#include "libs/imgui/backends/imgui_impl_dx9.h"
#include "libs/imgui/backends/imgui_impl_win32.h"
#include <dinput.h>
#include <tchar.h>
#include <d3d9.h>
#include "byte_array.h"
#include "nav_elements.h"
#include "etc_elements.h"
#include "Options.h"
#include "Config.h"
#include "Helpers.h"
#include <imgui_console.h>
#include "R6Helper.h"

#pragma comment(lib, "d3d9.lib")
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

/*static void imvec4_setter(ImVec4& my_type, std::vector<int> vec)
{
    if (vec.size() < 4) return;

    my_type.x = vec[0] / 255.f;
    my_type.y = vec[1] / 255.f;
    my_type.z = vec[2] / 255.f;
    my_type.w = vec[3] / 255.f;
}*/

ImFont* medium;
ImFont* bold;
ImFont* tab_icons;
ImFont* logo;
ImFont* tab_title;
ImFont* tab_title_icon;
ImFont* subtab_title;
ImFont* combo_arrow;

enum heads {
    mainwind, configs, console
};

enum sub_heads {
    general, consolewnd
};

static LPDIRECT3D9              g_pD3D = NULL;
static LPDIRECT3DDEVICE9        g_pd3dDevice = NULL;
static D3DPRESENT_PARAMETERS    g_d3dpp = {};

bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void ResetDevice();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void R6Restarter::R6Start() {
    if (!g_Options.be) {
        switch (g_Options.platform) {
        case 0: ShellExecuteW(NULL, L"open", L"uplay://launch/1842/0", 0, 0, SW_HIDE); break;
        case 1: ShellExecuteW(NULL, L"open", L"steam://rungameid/359550", 0, 0, SW_HIDE); break;
        case 2: ShellExecuteW(NULL, L"open", L"here must be epic games starting link", 0, 0, SW_HIDE); break;
        }
    }
    else {
        switch (g_Options.platform)
        {
        case 0: ShellExecuteW(NULL, L"open", L"here must be uplay starting link", 0, 0, SW_HIDE); break;
        case 1: ShellExecuteW(NULL, L"open", L"steam://run/359550//belaunch -be/", 0, 0, SW_HIDE); break;
        case 2: ShellExecuteW(NULL, L"open", L"here must be epic games starting link", 0, 0, SW_HIDE); break;
        }
    }
}

int main(int, char**)
{
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("R6 Helper"), NULL };
    ::RegisterClassEx(&wc);
    HWND hwnd = ::CreateWindow(wc.lpszClassName, _T(""), WS_POPUP, 100, 100, 730, 460, NULL, NULL, wc.hInstance, NULL);
    
    FreeConsole();
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClass(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    ImGuiConsole Console;
    R6Restarter R6;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX9_Init(g_pd3dDevice);

    // Load Fonts
    ImFontConfig font_config;
    font_config.PixelSnapH = false;
    font_config.OversampleH = 5;
    font_config.OversampleV = 5;
    font_config.RasterizerMultiply = 1.2f;

    static const ImWchar ranges[] =
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x0400, 0x052F, // Cyrillic + Cyrillic Supplement
        0x2DE0, 0x2DFF, // Cyrillic Extended-A
        0xA640, 0xA69F, // Cyrillic Extended-B
        0xE000, 0xE226, // icons
        0,
    };

    // Log example information:
    Console.System().Log(csys::ItemType::INFO) << "R6 Helper | By Yulogoth\nhttps://github.com/Yulogoth" << csys::endl;



    font_config.GlyphRanges = ranges;

    medium = io.Fonts->AddFontFromMemoryTTF(PTRootUIMedium, sizeof(PTRootUIMedium), 15.0f, &font_config, ranges);
    bold = io.Fonts->AddFontFromMemoryTTF(PTRootUIBold, sizeof(PTRootUIBold), 15.0f, &font_config, ranges);

    tab_icons = io.Fonts->AddFontFromMemoryTTF(clarityfont, sizeof(clarityfont), 15.0f, &font_config, ranges);
    logo = io.Fonts->AddFontFromMemoryTTF(clarityfont, sizeof(clarityfont), 21.0f, &font_config, ranges);

    tab_title = io.Fonts->AddFontFromMemoryTTF(PTRootUIBold, sizeof(PTRootUIBold), 19.0f, &font_config, ranges);
    tab_title_icon = io.Fonts->AddFontFromMemoryTTF(clarityfont, sizeof(clarityfont), 18.0f, &font_config, ranges);

    subtab_title = io.Fonts->AddFontFromMemoryTTF(PTRootUIBold, sizeof(PTRootUIBold), 15.0f, &font_config, ranges);

    combo_arrow = io.Fonts->AddFontFromMemoryTTF(combo, sizeof(combo), 9.0f, &font_config, ranges);

    ImVec4 clear_color = ImVec4(0, 0, 0, 0);

    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the Win32 backend.
        MSG msg;
        while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        // Start the Dear ImGui frame
        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        static heads tab{ mainwind };
        static sub_heads subtab{ general };
        const char* soft_name = "R6 Helper";
        const char* tab_name = tab == mainwind ? soft_name : tab == configs ? soft_name : tab == console ? soft_name : 0;
        const char* tab_icon = tab == mainwind ? "G" : tab == configs ? "E" : tab == console ? "H" : 0;

        static bool cfg_use, boolean_1 = false;

        const char* platform_pick[3] = { "Uplay", "Steam", "Epic Games" };
        ImGui::SetNextWindowPos({ 0, 0 });
        ImGui::SetNextWindowSize({ 730, 460 });
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });

        ImGui::Begin("R6 Helper", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove); {
            auto draw = ImGui::GetWindowDrawList();

            auto pos = ImGui::GetWindowPos();
            auto size = ImGui::GetWindowSize();

            ImGuiStyle style = ImGui::GetStyle();

            draw->AddRectFilled(pos, ImVec2(pos.x + 210, pos.y + size.y), ImColor(24, 24, 26), style.WindowRounding, ImDrawFlags_RoundCornersLeft);
            draw->AddLine(ImVec2(pos.x + 210, pos.y + 2), ImVec2(pos.x + 210, pos.y + size.y - 2), ImColor(1.0f, 1.0f, 1.0f, 0.03f));
            draw->AddLine(ImVec2(pos.x + 47, pos.y + 2), ImVec2(pos.x + 47, pos.y + size.y - 2), ImColor(1.0f, 1.0f, 1.0f, 0.03f));
            draw->AddLine(ImVec2(pos.x + 2, pos.y + 47), ImVec2(pos.x + 47, pos.y + 47), ImColor(1.0f, 1.0f, 1.0f, 0.03f));
            draw->AddLine(ImVec2(pos.x + 63, pos.y + 47), ImVec2(pos.x + 195, pos.y + 47), ImColor(1.0f, 1.0f, 1.0f, 0.03f));
            draw->AddText(logo, 21.0f, ImVec2(pos.x + 14, pos.y + 12), ImColor(147, 190, 66), "A");

            draw->AddText(tab_title_icon, 18.0f, ImVec2(pos.x + 65, pos.y + 14), ImColor(147, 190, 66), tab_icon);
            draw->AddText(tab_title, 19.0f, ImVec2(pos.x + 93, pos.y + 15), ImColor(1.0f, 1.0f, 1.0f), tab_name);

            draw->AddRect(pos + ImVec2(1, 1), pos + size - ImVec2(1, 1), ImColor(1.0f, 1.0f, 1.0f, 0.03f), style.WindowRounding);
            //draw->AddCircleFilled(ImVec2(size.x-17, 10), 4,ImColor(255, 0, 0));

            ImGui::SetCursorPos({ 8, 56 });
            ImGui::BeginGroup(); {
                if (elements::tab("G", tab == mainwind)) { tab = mainwind; }
                //if (elements::tab("C", tab == antiaim)) { tab = antiaim; }
                //if (elements::tab("D", tab == visuals)) { tab = visuals; }
                //if (elements::tab("E", tab == settings)) { tab = settings; }
                //if (elements::tab("F", tab == skins)) { tab = skins; }
                if (elements::tab("H", tab == console)) { tab = console; }
                if (elements::tab("E", tab == configs)) { tab = configs; }
            } ImGui::EndGroup();

            switch (tab) {
            case mainwind:
                draw->AddText(subtab_title, 15.0f, ImVec2(pos.x + 72, pos.y + 60), ImColor(1.0f, 1.0f, 1.0f, 0.4f), "MAIN");
                subtab = general;
                ImGui::SetCursorPos({ 57, 86 });
                ImGui::BeginGroup(); {
                    if (elements::subtab("Restarter", subtab == general)) { subtab = general; }
                    //if (elements::subtab("Accuracy", subtab == accuracy)) { subtab = accuracy; }
                    //if (elements::subtab("Exploits", subtab == exploits)) { subtab = exploits; }
                } ImGui::EndGroup();

                switch (subtab) {
                case general:
                    ImGui::SetCursorPos({ 226, 16 });
                    e_elements::begin_child("Restarter", ImVec2(240, 300)); {
                        if (!cfg_use) {
                            ImGui::Combo("Platform", g_Options.platform, platform_pick, IM_ARRAYSIZE(platform_pick));
                            ImGui::SliderInt("Restart Timeout", g_Options.timeout, 0, 20, "%d", ImGuiSliderFlags_None);
                            ImGui::Checkbox("BE Disable", g_Options.be);
                        }
                        else {
                            Config::LoadCFG("cfg.r6h");
                            ImGui::Text("Platform: %s", platform_pick[g_Options.platform]);
                            ImGui::Text("Timeout: %d", g_Options.timeout);
                        }
                        ImGui::NewLine();
                        if (ImGui::Button("Restart", ImVec2(210, 25))) {
                            system("taskkill /im RainbowSix_BE.exe");
                            system("taskkill /im RainbowSix.exe");
                            //cout << "R6 must be closed\nStarting in 2 seconds...";
                            for (int i = 0; i < g_Options.timeout; i++) Sleep(1000);
                            R6.R6Start();
                        }
                    }
                    e_elements::end_child();

                    ImGui::SetCursorPos({ 476, 16 });
                    e_elements::begin_child("Settings", ImVec2(240, 240)); {
                        ImGui::Checkbox("Use config", &cfg_use);
                        if (ImGui::Button("Save CFG")) Config::SaveCFG("cfg.r6h");
                        if (ImGui::Button("Exit")) {
                            exit(0);
                        }
                        if (ImGui::Button("R6 Stop")) {
                            if (Helpers::CheckProc(L"RainbowSix.exe")) {
                                system("taskkill /im RainbowSix_BE.exe");
                                system("taskkill /im RainbowSix.exe");
                            }
                            else {
                                //smthng
                            }
                        }
                        
                    }
                    e_elements::end_child();

                    /*ImGui::SetCursorPos({226, 332});
                    e_elements::begin_child("water in", ImVec2(240, 114)); {

                    }
                    e_elements::end_child();*/

                    ImGui::SetCursorPos({ 476, 272 });
                    e_elements::begin_child("Credits", ImVec2(240, 174)); {
                        ImGui::Text("R6 Helper | By Yulogoth\nhttps://github.com/Yulogoth");
                    }
                    e_elements::end_child();
                    break;
                }
                break;

            case console:
                draw->AddText(subtab_title, 15.0f, ImVec2(pos.x + 72, pos.y + 60), ImColor(1.0f, 1.0f, 1.0f, 0.4f), "CONSOLE");
                subtab = consolewnd;
                ImGui::SetCursorPos({ 57, 86 });
                ImGui::BeginGroup(); {
                    if (elements::subtab("Console", subtab == consolewnd)) { subtab = consolewnd; }
                } ImGui::EndGroup();

                switch (subtab) {
                case consolewnd:
                    Console.Draw();
                    break;
                }
                break;
            }
        }
        ImGui::End();

        ImGui::PopStyleVar();

        // Rendering
        ImGui::EndFrame();
        g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
        D3DCOLOR clear_col_dx = D3DCOLOR_RGBA((int)(clear_color.x * clear_color.w * 255.0f), (int)(clear_color.y * clear_color.w * 255.0f), (int)(clear_color.z * clear_color.w * 255.0f), (int)(clear_color.w * 255.0f));
        g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);
        if (g_pd3dDevice->BeginScene() >= 0)
        {
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            g_pd3dDevice->EndScene();
        }
        HRESULT result = g_pd3dDevice->Present(NULL, NULL, NULL, NULL);

        // Handle loss of D3D9 device
        if (result == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
            ResetDevice();
    }

    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClass(wc.lpszClassName, wc.hInstance);

    return 0;
}

// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
    if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
        return false;

    // Create the D3DDevice
    ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
    g_d3dpp.Windowed = TRUE;
    g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN; // Need to use an explicit format with alpha if needing per-pixel alpha composition.
    g_d3dpp.EnableAutoDepthStencil = TRUE;
    g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;           // Present with vsync
    //g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present without vsync, maximum unthrottled framerate
    if (g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0)
        return false;

    return true;
}

void CleanupDeviceD3D()
{
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
    if (g_pD3D) { g_pD3D->Release(); g_pD3D = NULL; }
}

void ResetDevice()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
    HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
    if (hr == D3DERR_INVALIDCALL)
        IM_ASSERT(0);
    ImGui_ImplDX9_CreateDeviceObjects();
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
        {
            g_d3dpp.BackBufferWidth = LOWORD(lParam);
            g_d3dpp.BackBufferHeight = HIWORD(lParam);
            ResetDevice();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}