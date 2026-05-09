#include "framework.h"
#include "hooks.hpp"
#include "Menu/Menu.hpp"

#define FindFormGlobal 0xFFFFD8EE  // LUA_GLOBALSINDEX = -10002
#define FindFormReg -10000          // LUA_REGISTRYINDEX

void arisu::game::hooks::Setup()
{
    while (GetModuleHandleA("d3d9.dll") == 0)
    {
        Sleep(100);
    }

    // --- Create a temporary D3D9 device to grab vtable pointers ---
    LPDIRECT3D9 d3d = NULL;
    LPDIRECT3DDEVICE9 D3dDevice = NULL;

    HWND tmpWnd = CreateWindowA("BUTTON", "Temp Window", WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 300, 300, NULL, NULL, hModule, NULL);
    if (tmpWnd == NULL)
    {
        logger.error("CreateWindowA failed for temp D3D window!");
        return;
    }

    d3d = Direct3DCreate9(D3D_SDK_VERSION);
    if (d3d == NULL)
    {
        logger.error("Direct3DCreate9 failed!");
        DestroyWindow(tmpWnd);
        return;
    }

    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory(&d3dpp, sizeof(d3dpp));
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.hDeviceWindow = tmpWnd;
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;

    // In ASI mode the game already owns the HAL device (possibly fullscreen/exclusive),
#ifdef BUILD_ASI
    HRESULT result = d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_NULLREF, tmpWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &D3dDevice);
#else
    HRESULT result = d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, tmpWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &D3dDevice);
#endif
    if (result != D3D_OK)
    {
        logger.error("CreateDevice failed for temp D3D device!");
        d3d->Release();
        DestroyWindow(tmpWnd);
        return;
    }

    // --- Create a temporary DirectInput mouse device to grab vtable pointer ---
    IDirectInput8* pDirectInput = nullptr;
    if (DirectInput8Create(myhModule, DIRECTINPUT_VERSION, IID_IDirectInput8, (LPVOID*)&pDirectInput, NULL) != DI_OK) {
        logger.error("DirectInput8Create failed!");
        D3dDevice->Release();
        d3d->Release();
        DestroyWindow(tmpWnd);
        return;
    }

    LPDIRECTINPUTDEVICE8 InputDevice8 = nullptr;
    if (pDirectInput->CreateDevice(GUID_SysMouse, &InputDevice8, NULL) != DI_OK) {
        logger.error("CreateDevice::SysMouse failed!");
        pDirectInput->Release();
        D3dDevice->Release();
        d3d->Release();
        DestroyWindow(tmpWnd);
        return;
    }

    LPDIRECTINPUTDEVICE8 InputDevice8KB = nullptr;
    if (pDirectInput->CreateDevice(GUID_SysKeyboard, &InputDevice8KB, NULL) != DI_OK) {
        logger.error("CreateDevice::SysKeyboard failed!");
        InputDevice8->Release();
        pDirectInput->Release();
        D3dDevice->Release();
        d3d->Release();
        DestroyWindow(tmpWnd);
        return;
    }


    if (GetDeviceState::hk = safetyhook::create_inline(Memory::get_vfunc_address(InputDevice8, 9).cast<void*>(), GetDeviceState::Fn); GetDeviceState::hk)
        logger.success("Hooked DirectInput::GetDeviceState");

    if (GetDeviceStateKB::hk = safetyhook::create_inline(Memory::get_vfunc_address(InputDevice8KB, 9).cast<void*>(), GetDeviceStateKB::Fn); GetDeviceStateKB::hk)
        logger.success("Hooked DirectInput::GetDeviceStateKB");

    // Wait for the game window to be created before subclassing it
    while ((hWindow = FindWindowA("prototypeWindowClass", nullptr)) == nullptr)
        Sleep(100);
    oWndProc = (WNDPROC)SetWindowLongPtr(hWindow, GWLP_WNDPROC, (LONG_PTR)WndProc);

    if (EndScene::hk = safetyhook::create_inline(Memory::get_vfunc_address(D3dDevice, 42).cast<void*>(), EndScene::Fn); EndScene::hk)
        logger.success("Hooked Directx::EndScene");

    if (Reset::hk = safetyhook::create_inline(Memory::get_vfunc_address(D3dDevice, 16).cast<void*>(), Reset::Fn); Reset::hk)
        logger.success("Hooked Directx::Reset");

     if (GOMUpdate::hk = safetyhook::create_inline(prototypeengine.find_pattern("53 55 56 57 E8 ? ? ? ? E8 ? ? ? ? E8").cast<void*>(), GOMUpdate::Fn); GOMUpdate::hk)
         logger.success("Hooked EngineSimulationTask");

    D3dDevice->Release();
    d3d->Release();
    DestroyWindow(tmpWnd);

}

HRESULT D3DAPI arisu::game::hooks::EndScene::Fn(LPDIRECT3DDEVICE9 d3d9)
{
    static bool init = true;
    if (init)
    {
        init = false;
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        ImGui_ImplWin32_Init(FindWindowA("prototypeWindowClass", NULL));
        ImGui_ImplDX9_Init(d3d9);
        imgui_initialized = true;
        P1Menu.ScanLuaLoaderFolder();
    }

    if (GetAsyncKeyState(VK_INSERT) & 1)
    {
        P1Menu.IsOpened = !P1Menu.IsOpened;
    }

    if (P1Menu.IsOpened)
    {
        ImGuiIO& io = ImGui::GetIO();

        // Mouse position — convert from screen to client space
        POINT pt;
        if (GetCursorPos(&pt) && ScreenToClient(hWindow, &pt))
            io.AddMousePosEvent((float)pt.x, (float)pt.y);

        // Mouse buttons
        static constexpr int vkButtons[] = { VK_LBUTTON, VK_RBUTTON, VK_MBUTTON, VK_XBUTTON1, VK_XBUTTON2 };
        for (int i = 0; i < 5; ++i)
            io.AddMouseButtonEvent(i, (GetAsyncKeyState(vkButtons[i]) & 0x8000) != 0);

        // Mouse wheel — accumulated from DirectInput in GetDeviceState::Fn
        if (g_diWheelDelta != 0.f)
        {
            io.AddMouseWheelEvent(0.f, g_diWheelDelta);
            g_diWheelDelta = 0.f;
        }
    }

    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();

    // Fix DisplaySize to match the actual backbuffer, not the window client rect.
    // This prevents font squishing when the backbuffer size differs from the window size.
    {
        IDirect3DSurface9* pBackBuffer = nullptr;
        if (SUCCEEDED(d3d9->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer)))
        {
            D3DSURFACE_DESC desc;
            if (SUCCEEDED(pBackBuffer->GetDesc(&desc)))
            {
                ImGuiIO& io = ImGui::GetIO();
                io.DisplaySize = ImVec2((float)desc.Width, (float)desc.Height);
            }
            pBackBuffer->Release();
        }
    }

    ImGui::NewFrame();

    if (P1Menu.IsOpened)
    {
        P1Menu.RenderMenuBar();
        P1Menu.RenderLuaTab();
        P1Menu.RenderLuaLoaderTab();
    }
    P1Menu.RenderDebugTab();
    ImGui::GetIO().MouseDrawCursor = P1Menu.IsOpened;

    ImGui::EndFrame();
    ImGui::Render();

    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());


    return hk.stdcall<HRESULT>(d3d9);
}

HRESULT D3DAPI arisu::game::hooks::Reset::Fn(LPDIRECT3DDEVICE9 pD3D9, D3DPRESENT_PARAMETERS* pPresentationParameters)
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
    auto hr = hk.stdcall<HRESULT>(pD3D9, pPresentationParameters);
    ImGui_ImplDX9_CreateDeviceObjects();
    return hr;
}
bool __fastcall arisu::game::hooks::DisguiseBlow::Fn(void* ecx, void* edx, float a2)
{
    // DisguiseBlow is no longer used as the main game tick.
    // All logic has been moved to GOMUpdate which runs every simulation sub-step.
    return hk.fastcall<bool>(ecx, edx, a2);
}

int __stdcall arisu::game::hooks::GOMUpdate::Fn(int a1, int a2, float deltaTime)
{
    int L = arisu::game::Engine::GetLuaState();

    if (L != 0 && arisu::game::Engine::LuaGetField && arisu::game::Engine::LuaPcall)
    {
        static std::atomic<int> s_hookedState { 0 };
        if (s_hookedState.exchange(L) != L)
        {
            arisu::game::Engine::InstallPrintHook(L);
        }

        // Hotkey Lua function calls
        P1Menu.ExecLua(L);

        // Lua Loader — execute looping scripts every tick
        P1Menu.TickLuaLoader();
    }
    return hk.stdcall<int>(a1, a2, deltaTime);
}
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void __fastcall arisu::game::hooks::KeyBoardService::Fn(void* ecx, void* edx, int uMsg, UINT wParam, int lParam, char IsCapitalOn, char IsNumlockOn)
{

    hk.fastcall(ecx, edx, uMsg, wParam, lParam, IsCapitalOn, IsNumlockOn);
}

HRESULT __stdcall arisu::game::hooks::GetDeviceState::Fn(IDirectInputDevice8* pThis, DWORD cbData, LPVOID lpvData) {
    HRESULT result = hk.stdcall<HRESULT>(pThis, cbData, lpvData);

    if (P1Menu.IsOpened)
    {
        if (result == DI_OK && lpvData)
        {

            if (cbData >= sizeof(DIMOUSESTATE2))
            {
                auto* ms = reinterpret_cast<DIMOUSESTATE2*>(lpvData);
                // lZ is the wheel axis: positive = scroll up, negative = scroll down.
                // Scale to ImGui convention (WHEEL_DELTA = 120 per notch).
                if (ms->lZ != 0)
                    g_diWheelDelta += (float)ms->lZ / WHEEL_DELTA;
            }
            else if (cbData >= sizeof(DIMOUSESTATE))
            {
                auto* ms = reinterpret_cast<DIMOUSESTATE*>(lpvData);
                if (ms->lZ != 0)
                    g_diWheelDelta += (float)ms->lZ / WHEEL_DELTA;
            }
            memset(lpvData, 0, cbData);
        }
    }
    return result;
}

HRESULT __stdcall arisu::game::hooks::GetDeviceStateKB::Fn(IDirectInputDevice8* pThis, DWORD cbData, LPVOID lpvData) {
    HRESULT result = hk.stdcall<HRESULT>(pThis, cbData, lpvData);

    if (P1Menu.IsOpened)
    {
        if (result == DI_OK && lpvData) {
            memset(lpvData, 0, cbData);
        }
    }
    return result;
}
void __cdecl arisu::game::hooks::MouseService::Fn(int a1, unsigned int a2, unsigned int a3)
{

    hk.ccall(a1, a2,a3);

}
void __fastcall arisu::game::hooks::sub_705880::Fn(N0000004E* ptr, void* ecx, Vector3D* angle)
{
   hk.fastcall(ptr,ecx, angle);


}
LRESULT __stdcall arisu::game::hooks::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

    if (uMsg == WM_ACTIVATEAPP)
    {
        P1Menu.IsOpened = false;
    }

    if (P1Menu.IsOpened && ImGui_ImplWin32_WndProcHandler(hWindow, uMsg, wParam, lParam))
        return FALSE;


    return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

void arisu::game::hooks::Destroy()
{
    GOMUpdate::hk.reset();
    DisguiseBlow::hk.reset();
    KeyBoardService::hk.reset();
    EndScene::hk.reset();

    if (imgui_initialized)
    {
        imgui_initialized = false;
        ImGui_ImplDX9_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }

    if (hWindow && oWndProc)
        SetWindowLongPtr(hWindow, GWLP_WNDPROC, (LONG_PTR)oWndProc);
}