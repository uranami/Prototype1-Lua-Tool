#pragma once

#include <safetyhook.hpp>
#include <d3d9.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#pragma comment(lib, "Dinput8.lib")
#pragma comment(lib, "Dxguid.lib")
#pragma comment (lib, "d3d9.lib" )
#pragma comment (lib, "d3dx9.lib" )

// Created with ReClass.NET 1.2 by KN4CK3R
inline HMODULE hModule;
inline HMODULE myhModule;
class N0000004E
{
public:
    char pad_0000[144]; //0x0000
    Vector3D N00000073; //0x0090
    Vector3D vecpos; //0x009C
    Vector3D N00000075; //0x00A8
    char pad_00B4[1956]; //0x00B4
}; //Size: 0x0858

namespace arisu::game::hooks
{
#define CREATE_HOOK_DEFINE_INLINE(name, type)                                                                                                                                                  \
    namespace name                                                                                                                                                                             \
    {                                                                                                                                                                                          \
        using fn = std::remove_pointer_t<type>;                                                                                                                                                \
        fn Fn;                                                                                                                                                                                 \
        inline safetyhook::InlineHook hk {};                                                                                                                                                   \
    };


    CREATE_HOOK_DEFINE_INLINE(DisguiseBlow, bool(__fastcall*)(void* ecx, void* edx, float a2));
    CREATE_HOOK_DEFINE_INLINE(GOMUpdate, int(__stdcall*)(int a1, int a2, float deltaTime));
    CREATE_HOOK_DEFINE_INLINE(EndScene, HRESULT(__stdcall*)(LPDIRECT3DDEVICE9));
    CREATE_HOOK_DEFINE_INLINE(Reset, HRESULT(__stdcall*)(LPDIRECT3DDEVICE9 pD3D9, D3DPRESENT_PARAMETERS* pPresentationParameters));
    CREATE_HOOK_DEFINE_INLINE(KeyBoardService, void(__fastcall*)(void* ecx, void* edx,int uMsg, UINT wParam_2, int lParam_1, char IsCapitalOn, char IsNumlockOn));
    CREATE_HOOK_DEFINE_INLINE(MouseService, void(__cdecl*)(int a1, unsigned int a2, unsigned int a3));
    CREATE_HOOK_DEFINE_INLINE(sub_705880, void (__fastcall*)(N0000004E* ptr, void* ecx, Vector3D* a2));
    CREATE_HOOK_DEFINE_INLINE(GetDeviceState, HRESULT(__stdcall*)(IDirectInputDevice8* pThis, DWORD cbData, LPVOID lpvData));
    CREATE_HOOK_DEFINE_INLINE(GetDeviceStateKB, HRESULT(__stdcall*)(IDirectInputDevice8* pThis, DWORD cbData, LPVOID lpvData));
    inline HWND hWindow;
    inline WNDPROC oWndProc;
    inline bool imgui_initialized = false;
    extern void Setup();
    extern LRESULT __stdcall WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void Destroy();

    inline float g_diWheelDelta = 0.f;
 
}
