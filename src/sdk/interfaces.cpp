#include "../framework.h"
#include "interfaces.hpp"
#include "../modules.hpp"
#include <fstream>
#include <sstream>
#include <mutex>


static int __cdecl meowhook_lua_print(int L)
{
    using namespace arisu::game::Engine;

    int nargs = LuaGetTop(L);
    std::string line;

    for (int i = 1; i <= nargs; i++)
    {
        // Use lua_tolstring directly — it handles number→string conversion
        // For other types, we call the global tostring() via pcall
        const char* s = LuaToLString(L, i, nullptr);
        if (!s)
        {
            // tolstring returned null — need tostring() metamethod
            LuaGetField(L, -10002, "tostring");
            LuaPushValue(L, i);
            if (LuaPcall(L, 1, 1, 0) == 0)
            {
                s = LuaToLString(L, -1, nullptr);
                LuaSetTop(L, -(1) - 1); // pop result
            }
            else
            {
                LuaSetTop(L, -(1) - 1); // pop error
                s = "???";
            }
        }

        if (i > 1)
            line += "\t";
        if (s)
            line += s;
    }

    {
        std::lock_guard<std::mutex> lock(luaOutputMutex);
        if (!luaOutputLog.empty() && luaOutputLog.back() != '\n')
            luaOutputLog += "\n";
        luaOutputLog += line;
    }

    return 0;
}
template <typename T>
T* GetFromSig(const arisu::impl::Module& mod, std::string_view pattern, std::string_view name, std::ptrdiff_t offset = 0, std::uint8_t deref = 0)
{
    auto res = mod.find_pattern(pattern).offset(offset).deref(deref);
    if (!res.is_valid())
    {
        arisu::logger.error("Failed to get address for \"{}\"", name);
        return {};
    }

    arisu::logger.success("{} ==> 0x{:x}", name, res.ptr);  

    return res.cast<T*>();

}
void arisu::game::Engine::InstallPrintHook(int L)
{
    if (!L || !LuaPushCClosure || !LuaSetField)
        return;

    // Use lua_setfield to set _G["print"] = our_function
    // lua_pushcfunction(L, fn) then lua_setfield(L, LUA_GLOBALSINDEX, "print")
    LuaPushCClosure(L, (void*)meowhook_lua_print, 0);
    LuaSetField(L, -10002, "print");

    LuaPushCClosure(L, (void*)meowhook_lua_print, 0);
    LuaSetField(L, -10002, "debug");

    logger.success("Installed Lua print hook");
}


void arisu::game::Engine::Init()
{
    auto& mod = prototypeengine;
    if (mod.base() == 0)
    {
        arisu::logger.error("prototypeenginef.dll not loaded!");
        return;
    }

    uintptr_t base = mod.base();
    arisu::logger.info("prototypeenginef.dll base = {:#x}", base);

    #define RESOLVE_OR_FAIL(name, sig) \
        do { \
            auto addr_ = mod.find_pattern(sig); \
            if (addr_.is_valid()) { \
                name = (decltype(name))addr_.ptr; \
                arisu::logger.success(#name " @ {:#x}", (uintptr_t)name); \
            } else { \
                arisu::logger.error(#name " NOT FOUND (sig: " sig ")"); \
            } \
        } while(0)

    RESOLVE_OR_FAIL(LuaPcall,        "8B 4C 24 ? 83 EC ? 85 C9 56");
    RESOLVE_OR_FAIL(LuaGetField,     "8B 4C 24 ? 83 EC ? 53 56 8B 74 24 ? 57 8B D6 E8 ? ? ? ? 8B 54 24 ? 8B F8 8B C2 8D 58 ? 8A 08 83 C0 ? 84 C9 75 ? 2B C3 50 52 56 E8 ? ? ? ? 89 44 24 ? 8B 46 ? 50");
    RESOLVE_OR_FAIL(LuaSetTop,       "8B 4C 24 ? 85 C9 8B 44 24 ? 7C");
    RESOLVE_OR_FAIL(LuaGetTop,       "8B 4C 24 ? 8B 41 ? 2B 41 ? C1 F8");
    RESOLVE_OR_FAIL(LuaToLString,    "56 8B 74 24 ? 57 8B 7C 24 ? 8B CF 8B D6");
    RESOLVE_OR_FAIL(LuaLoadBuffer,   "55 8B EC 83 EC ? 8B 45 ? 8B 55");
    RESOLVE_OR_FAIL(LuaPushCClosure, "56 8B 74 24 ? 8B 46 ? 8B 48 ? 3B 48 ? 57 72 ? 56 E8 ? ? ? ? 83 C4 ? 8B C6");
    RESOLVE_OR_FAIL(LuaPushLString,  "56 8B 74 24 ? 8B 46 ? 8B 48 ? 3B 48 ? 57 72 ? 56 E8 ? ? ? ? 83 C4 ? 8B 54 24 ? 8B 44 24 ? 8B 7E ? 52 50 56 E8 ? ? ? ? 83 C4 ? 89 07 C7 47 ? ? ? ? ? 83 46 ? ? 5F 5E C3 8B 54 24");
    RESOLVE_OR_FAIL(LuaPushValue,    "8B 4C 24 ? 56 8B 74 24 ? 8B D6 E8 ? ? ? ? 8B 4E ? 8B 10 89 11");
    RESOLVE_OR_FAIL(LuaSetTable,     "8B 4C 24 ? 53 56 8B 74 24 ? 57 8B D6 E8 ? ? ? ? 8B 7E ? 8B D8 8B 0B"); // lua_rawset
    RESOLVE_OR_FAIL(LuaSetField,     "8B 4C 24 ? 83 EC ? 53 56 8B 74 24 ? 57 8B D6 E8 ? ? ? ? 8B 54 24 ? 8B F8 8B C2 8D 58 ? 8A 08 83 C0 ? 84 C9 75 ? 2B C3 50 52 56 E8 ? ? ? ? 89 44 24 ? 8B 46 ? 83 E8"); // lua_setfield

    #undef RESOLVE_OR_FAIL

    LuaCall  = nullptr; // Not needed, we use LuaPcall everywhere
    LuaError = nullptr; // Not critical for basic operation

    // DoLuaBuffer via sigscan
    {
        auto addr = mod.find_pattern("8B 41 ? 53 56 57 50");
        if (addr.is_valid())
        {
            DoLuaBuffer = (DoLuaBufferFn)addr.ptr;
            arisu::logger.success("DoLuaBuffer     @ {:#x}", (uintptr_t)DoLuaBuffer);
        }
        else
        {
            arisu::logger.error("DoLuaBuffer NOT FOUND");
        }
    }

    // g_pLuaScriptManager is a global pointer at base+0x1284254
    // It points to the LuaScriptManager singleton object.
    // lua_State* is at object offset 0x0C (this[3]).
    // (kept as base+offset — data pointer, cannot sigscan without xref)

    g_pLuaScriptManager = GetFromSig<void*>(arisu::game::prototypeengine, "8B 0D ? ? ? ? 85 C9 74 ? 8B 01 8B 10 6A ? FF D2 A1", "g_pLuaScriptManager", 2, 1);



    SimulationDriver = nullptr;
    pCameraManager = nullptr;

    arisu::logger.success("arisu::Engine::Init done.");
}

void arisu::game::Engine::cleanptr()
{
    LuaPcall = nullptr;
    LuaGetField = nullptr;
    LuaSetTop = nullptr;
    LuaGetTop = nullptr;
    LuaToLString = nullptr;
    LuaLoadBuffer = nullptr;
    LuaPushCClosure = nullptr;
    LuaPushLString = nullptr;
    LuaPushValue = nullptr;
    LuaCall = nullptr;
    LuaSetTable = nullptr;
    LuaSetField = nullptr;
    LuaError = nullptr;
    DoLuaBuffer = nullptr;
    g_pLuaScriptManager = nullptr;

    lua_State = 0;
    SimulationDriver = nullptr;
}

int arisu::game::Engine::GetLuaState()
{
    if (!g_pLuaScriptManager || !*g_pLuaScriptManager)
        return 0;

    // Prototype 1: lua_State is at offset 0x0C in the LuaScriptManager object
    // (this[3] in the decompiled code: LuaScriptManager_Init uses this+3 as lua_State ptr)
    return *(int*)((uintptr_t)*g_pLuaScriptManager + 0x0C);
}

int arisu::game::Engine::ExecuteLuaScript(const char* script, std::string* outError)
{
    int L = GetLuaState();
    if (!L)
    {
        if (outError) *outError = "lua_State is null (LuaScriptManager not initialized)";
        return -1;
    }

    if (!LuaLoadBuffer || !LuaPcall || !LuaGetTop || !LuaSetTop || !LuaToLString)
    {
        if (outError) *outError = "Lua API function pointers not resolved";
        return -2;
    }

    luaOutputCapture = true;

    int top = LuaGetTop(L);

    size_t len = strlen(script);
    int loadResult = LuaLoadBuffer(L, script, len, "=owo");

    if (loadResult != 0)
    {
        if (outError)
        {
            const char* err = LuaToLString(L, -1, nullptr);
            *outError = err ? err : "Unknown compile error";
        }
        LuaSetTop(L, top);
        luaOutputCapture = false;
        return loadResult;
    }

    int callResult = LuaPcall(L, 0, -1, 0);

    if (callResult != 0)
    {
        if (outError)
        {
            const char* err = LuaToLString(L, -1, nullptr);
            *outError = err ? err : "Unknown runtime error";
        }
        LuaSetTop(L, top);
        luaOutputCapture = false;
        return callResult;
    }

    int newTop = LuaGetTop(L);
    if (newTop > top)
    {
        std::string returns;
        for (int i = top + 1; i <= newTop; i++)
        {
            const char* s = LuaToLString(L, i, nullptr);
            if (s)
            {
                if (!returns.empty()) returns += "\t";
                returns += s;
            }
        }
        if (!returns.empty())
        {
            std::lock_guard<std::mutex> lock(luaOutputMutex);
            if (!luaOutputLog.empty() && luaOutputLog.back() != '\n')
                luaOutputLog += "\n";
            luaOutputLog += "=> " + returns;
        }
    }

    LuaSetTop(L, top);
    luaOutputCapture = false;
    return 0;
}

int arisu::game::Engine::ExecuteLuaFile(const char* filePath, std::string* outError)
{
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        if (outError) *outError = std::string("Failed to open file: ") + filePath;
        return -1;
    }

    std::stringstream ss;
    ss << file.rdbuf();
    std::string content = ss.str();

    return ExecuteLuaScript(content.c_str(), outError);
}

void arisu::game::Engine::DumpLuaGlobals(const char* saveToFile)
{
    // This Lua script iterates _G and all sub-tables (1 level deep),
    // collecting every name. Outputs in C++ vector<string> format
    // compatible with GetLuaGlobals() in lua_globals.hpp.
    static const char* dumpScript = R"LUA(
local results = {}
local seen = {}

for k, v in pairs(_G) do
    local tp = type(v)
    local name = tostring(k)
    if tp == "function" then
        results[#results + 1] = name
    elseif tp == "table" and v ~= _G and not seen[v] then
        seen[v] = true
        for sk, sv in pairs(v) do
            if type(sv) == "function" then
                results[#results + 1] = name .. "." .. tostring(sk)
            end
        end
    end
end

table.sort(results)

print("inline const std::vector<std::string>& GetLuaGlobals()")
print("{")
print("    static const std::vector<std::string> kList = {")

for _, name in ipairs(results) do
    print('        "' .. name .. '",')
end

print("    };")
print("    return kList;")
print("}")
print("")
print("// Total: " .. #results .. " functions")

return #results
)LUA";

    std::string error;
    {
        std::lock_guard<std::mutex> lock(luaOutputMutex);
        luaOutputLog.clear();
    }

    int result = ExecuteLuaScript(dumpScript, &error);

    if (result != 0)
    {
        std::lock_guard<std::mutex> lock(luaOutputMutex);
        luaOutputLog += "\n[ERROR] Dump failed: " + error;
        return;
    }

    if (saveToFile && saveToFile[0])
    {
        std::string snapshot;
        {
            std::lock_guard<std::mutex> lock(luaOutputMutex);
            snapshot = luaOutputLog;
        }
        std::ofstream file(saveToFile);
        if (file.is_open())
        {
            file << snapshot;
            file.close();
            std::lock_guard<std::mutex> lock(luaOutputMutex);
            luaOutputLog += "\n[Saved to " + std::string(saveToFile) + "]";
        }
        else
        {
            std::lock_guard<std::mutex> lock(luaOutputMutex);
            luaOutputLog += "\n[ERROR] Failed to save to " + std::string(saveToFile);
        }
    }
}