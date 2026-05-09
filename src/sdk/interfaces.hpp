#pragma once

#include <d3d9.h>
#include <string>
#include <mutex>

namespace arisu::game::Engine
{

    // lua_pcall(L, nargs, nresults, errfunc)
    typedef int(__cdecl* lua_pcallFn)(int lua_State, int nargs, int nresults, int errfunc);
    inline lua_pcallFn LuaPcall;

    // lua_getfield(L, idx, k)
    typedef void(__cdecl* lua_getfieldFn)(int lua_State, int idx, const char* k);
    inline lua_getfieldFn LuaGetField;

    // lua_settop(L, idx)
    typedef void(__cdecl* lua_settopFn)(int lua_State, int idx);
    inline lua_settopFn LuaSetTop;

    // lua_gettop(L)
    typedef int(__cdecl* lua_gettopFn)(int lua_State);
    inline lua_gettopFn LuaGetTop;

    // lua_tolstring(L, idx, len)
    typedef const char*(__cdecl* lua_tolstringFn)(int lua_State, int idx, size_t* len);
    inline lua_tolstringFn LuaToLString;

    // luaL_loadbuffer(L, buff, sz, name)
    typedef int(__cdecl* luaL_loadbufferFn)(int lua_State, const char* buff, size_t sz, const char* name);
    inline luaL_loadbufferFn LuaLoadBuffer;

    // lua_pushcclosure(L, fn, n)
    typedef void(__cdecl* lua_pushcclosureFn)(int lua_State, void* fn, int n);
    inline lua_pushcclosureFn LuaPushCClosure;

    // lua_pushlstring(L, s, len)
    typedef void(__cdecl* lua_pushlstringFn)(int lua_State, const char* s, size_t len);
    inline lua_pushlstringFn LuaPushLString;

    // lua_pushvalue(L, idx)
    typedef void(__cdecl* lua_pushvalueFn)(int lua_State, int idx);
    inline lua_pushvalueFn LuaPushValue;

    // lua_call(L, nargs, nresults) - we use lua_pcall wrapper instead
    typedef void(__cdecl* lua_callFn)(int lua_State, int nargs, int nresults);
    inline lua_callFn LuaCall;

    // lua_setfield(L, idx, k) — sets t[k] = v, pops value
    typedef void(__cdecl* lua_setfieldFn)(int lua_State, int idx, const char* k);
    inline lua_setfieldFn LuaSetField;

    // lua_rawset(L, idx) (used as lua_settable equivalent)
    typedef void(__cdecl* lua_settableFn)(int lua_State, int idx);
    inline lua_settableFn LuaSetTable;

    // lua_error is not directly needed; we use luaL_error pattern instead
    typedef int(__cdecl* lua_errorFn)(int lua_State);
    inline lua_errorFn LuaError;

    // --- Output capture ---
    inline std::string luaOutputLog;
    inline bool luaOutputCapture = false;
    inline std::mutex luaOutputMutex;

    void InstallPrintHook(int L);

    // --- LuaScriptManager ---

    // LuaScriptManager::DoLuaBuffer
    // int __thiscall (void* this, int L, const char* buff, int sz, const char* name)
    typedef int(__thiscall* DoLuaBufferFn)(void* thisPtr, int lua_State, const char* buff, unsigned int sz, const char* name);
    inline DoLuaBufferFn DoLuaBuffer;

    // g_pLuaScriptManager — global data pointer, resolved via base+offset
    // (no sigscan: data pointer requires xref resolution)
    // LuaScriptManager object pointer. lua_State is at offset 0x0C (this[3]).
    inline void** g_pLuaScriptManager;

    inline uintptr_t lua_State;

    // Prototype 1 does not expose SimulationDriver/CameraManager the same way.
    inline SimulationDriver_t* SimulationDriver;
    inline uintptr_t* pCameraManager;

    // --- High-level helpers ---
    int ExecuteLuaScript(const char* script, std::string* outError = nullptr);
    int ExecuteLuaFile(const char* filePath, std::string* outError = nullptr);
    void DumpLuaGlobals(const char* saveToFile = nullptr);
    int GetLuaState();

    void Init();
    void cleanptr();
}
