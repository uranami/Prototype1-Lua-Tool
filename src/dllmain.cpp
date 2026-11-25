#include "framework.h"
#include "hooks.hpp"

DWORD WINAPI unload(LPVOID parameter)
{
#ifndef BUILD_ASI
    while (!GetAsyncKeyState(VK_F10))
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    arisu::logger.destroy();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    arisu::game::hooks::Destroy();

    FreeLibraryAndExitThread(static_cast<HMODULE>(parameter), EXIT_SUCCESS);
#endif
    return 0;
}


DWORD WINAPI setup(const LPVOID parameter)
{
#ifdef BUILD_ASI
    std::this_thread::sleep_for(std::chrono::seconds(10)); // sleep 10s
#endif
    SetConsoleOutputCP(CP_UTF8);

    // Enable buffering to prevent VS from chopping up UTF-8 byte sequences
    setvbuf(stdout, nullptr, _IOFBF, 1000);
    setvbuf(stderr, nullptr, _IOFBF, 1000);

#ifdef BUILD_ASI
    arisu::logger.setup();
#endif
    arisu::game::SetupModules();
    arisu::game::Engine::Init();
    arisu::game::hooks::Setup();

    return true;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        myhModule = hModule;
        if (const auto thread = CreateThread(nullptr, 0, setup, hModule, 0, nullptr); thread != nullptr)
            CloseHandle(thread);
#ifndef BUILD_ASI
        if (const auto thread = CreateThread(nullptr, 0, unload, hModule, 0, nullptr); thread != nullptr)
            CloseHandle(thread);
#endif
    }
    else if (ul_reason_for_call == DLL_PROCESS_DETACH)
    {
        /*unload(hModule);*/
    }
    return TRUE;
}
