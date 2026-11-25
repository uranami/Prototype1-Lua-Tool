#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_dx9.h"
#include "../imgui/imgui_impl_win32.h"
#include "../imgui/ImGuiColorTextEdit/TextEditor.h"
#include <json.hpp>
#include "../sdk/lua_globals.hpp"
#include <filesystem>
#include <string>
#include <vector>

struct Prototype2Config
{
    int TimeScaleKey = 0;
    float TimeScale = 1.f;

    // Player hacks
    bool bInfiniteHealth = false;
    int  iHealthValue = 99999;
    bool bInfiniteClip = false;
    bool bInfiniteMass = false;
    bool bNoPushback = false;
    bool bInvisible = false;

    // Disguise
    bool bInfiniteDisguise   = false;  // ������ʶ�ƣ���� blown ��־ + ������ֵ��
    bool bForceDisguiseActive = false; // ǿ�Ʊ��� Active ״̬������ Alert/Red��

    // Consume
    bool bConsumeExplosion = false;    // �������ĺ���Ŀ��λ�ô��������ʳ����
    bool bChainConsume     = false;    // ���ĺ���������������������ĵ���

    // Visual Effects
    bool bResetEffectManager = false;  // ÿ֡���� rm_ResetEffectManager

    // AI / World
    bool bPlayerInvisibleToAI = false; // ai_SetPlayerInvisible �� AI ���������
    bool bDisableFriendlyFire = false; // ai_SetDisableFriendlyFire on all NPCs
    float fNpcDamageMultiplier = 1.0f; // ai_SetNpcDamageMultiplier
    bool bNpcDamageMultiplierEnabled = false;

    // Civilian / Pedestrian
    bool bCivilianIgnorePlayer    = false; // ·����ȫ������ң���� AutoTarget + Alert ������
    bool bCivilianNoAlertSpread   = false; // ����ϿֻŴ�������Ӱ�� AutoTarget��

    // Time of Day
    float fTimeOfDay  = 12.0f; // 0-24
    float fTodSpeed   = 1.0f;  // tod_SetSpeed
    bool  bTodEnabled = false;  // �Ƿ񸲸�ʱ��

    // Player transform
    bool  bPlayerInvincible = false; // go_SetHitable(player, false)

    // AI intensity / crowd
    int  iMaxCharacters = 40;         // ai_SetMaxCharacters
    bool bMaxCharactersEnabled = false;

    // Camera
    bool bDebugCamera = false;
    bool bDisableCameraShake = false; // cm_EnableCameraShake(false)

    // Spawn
    std::array<char, 128> spawnEntityName = { 'D','u','m','m','y','P','E','D',0 };
    std::array<char, 128> spawnModelName  = { 'c','i','v','i','l','i','a','n','_','f','e','m','a','l','e',0 };

    // Drawable
    std::array<char, 128> drawableName = { 0 };
};
inline Prototype2Config P2Config;

// --- Lua Loader: entry for each .lua file found in lua_p1/ ---
struct LuaLoaderEntry
{
    std::string filename;       // e.g. "my_script.lua"
    std::string fullPath;       // e.g. "lua_p1/my_script.lua"
    std::string cachedScript;   // script content cached on Loop start
    bool        isLooping;      // true = continuously execute in GOMUpdate
    std::string lastError;      // last execution error (empty = OK)
    bool        lastRunOk;      // result of last execution
    int         errorCount;     // consecutive error count (auto-stop after threshold)
};

class RenderImMenu
{
public:
    bool IsOpened = false;
    std::array<char, 256> FuncExec = { 0 };
    void RenderMenuBar()noexcept;
    void LoadConfig();
    void RenderLuaTab();
    void RenderDebugTab();
    void ExecLua(uintptr_t lua_State);

    // Lua Loader (standalone tab)
    void RenderLuaLoaderTab();              // standalone ImGui window
    void TickLuaLoader();                   // called every GOMUpdate tick
    void ScanLuaLoaderFolder();

private:

    struct MenuItem {
        int KeyBind = 0;
        std::array<char, 256> LuaFuncName = { 0 };

    };
    void RenderContextMenu() noexcept;
    void BuildLuaTrie();

    void SaveConfig();

    std::vector<MenuItem> LuaFunc;

    const char* configFilePath = "lua_seq.json";

    // Lua script editor state
    TextEditor luaEditor;
    bool luaEditorInitialized = false;
    std::unique_ptr<TextEditor::Trie> luaTrie;
    TextEditor::AutoCompleteConfig luaAutocompleteConfig;
    std::string luaOutput;
    std::array<char, 512> luaFilePathBuf = { 0 };
    std::array<char, 256> luaDumpPathBuf = { 'l','u','a','_','g','l','o','b','a','l','s','.','t','x','t',0 };
    std::array<char, 128> luaSaveNameBuf = { 'm','y','_','s','c','r','i','p','t','.','l','u','a',0 };

    // Lua Loader state
    std::vector<LuaLoaderEntry> luaLoaderEntries;
    bool luaLoaderScanned = false;

    struct
    {
        bool luatab = false;
        bool infotab = false;
        bool debugtab = false;
        bool configtab = false;
        bool creatortab = false;
        bool loadertab = false;
    } window;


};
inline RenderImMenu P1Menu;
namespace ImGui {

    static const char* KeyNames[] =
    {
        "NONE",
        "MOUSE1",
        "MOUSE2",
        "Control-break",
        "MOUSE3",
        "MOUSE4",
        "MOUSE5",
        "Undefined",
        "BACKSPACE",
        "TAB",
        "Reserved",
        "Reserved",
        "Clear",
        "ENTER",
        "Reserved",
        "Reserved",
        "SHIFT",
        "CTRL",
        "ALT",
        "Pause",
        "CAPS",
        "IME",
        "IME",
        "IME",
        "IME",
        "IME",
        "IME",
        "ESC",
        "IME",
        "IME",
        "IME",
        "IME",
        "SPACE",
        "PAGEUP",
        "PAGEDOWN",
        "END",
        "HOME",
        "LEFT",
        "UP",
        "RIGHT",
        "DOWN",
        "Select",
        "Print",
        "Execute",
        "Print Screen",
        "INSERT",
        "DELETE",
        "HELP",
        "0",
        "1",
        "2",
        "3",
        "4",
        "5",
        "6",
        "7",
        "8",
        "9",
        "Undefined",
        "Undefined",
        "Undefined",
        "Undefined",
        "Undefined",
        "Undefined",
        "Undefined",
        "A",
        "B",
        "C",
        "D",
        "E",
        "F",
        "G",
        "H",
        "I",
        "J",
        "K",
        "L",
        "M",
        "N",
        "O",
        "P",
        "Q",
        "R",
        "S",
        "T",
        "U",
        "V",
        "W",
        "X",
        "Y",
        "Z",
        "Left Window",
        "Right Window",
        "Applications",
        "Reserved",
        "Sleep",
        "Numpad 0",
        "Numpad 1",
        "Numpad 2",
        "Numpad 3",
        "Numpad 4",
        "Numpad 5",
        "Numpad 6",
        "Numpad 7",
        "Numpad 8",
        "Numpad 9",
        "Numpad *",
        "Numpad +",
        "Numpad Separator",
        "Numpad -",
        "Numpad .",
        "Numpad /",
        "F1",
        "F2",
        "F3",
        "F4",
        "F5",
        "F6",
        "F7",
        "F8",
        "F9",
        "F10",
        "F11",
        "F12",
        "F13",
        "F14",
        "F15",
        "F16",
        "F17",
        "F18",
        "F19",
        "F20",
        "F21",
        "F22",
        "F23",
        "F24",
        "Unassigned",
        "Unassigned",
        "Unassigned",
        "Unassigned",
        "Unassigned",
        "Unassigned",
        "Unassigned",
        "Unassigned",
        "Numlock",
        "SCROLL",
        "OEM",
        "OEM",
        "OEM",
        "OEM",
        "OEM",
        "Unassigned",
        "Unassigned",
        "Unassigned",
        "Unassigned",
        "Unassigned",
        "Unassigned",
        "Unassigned",
        "Unassigned",
        "Unassigned",
        "Left Shift",
        "Right Shift",
        "Left Control",
        "Right Control",
        "Left Menu",
        "Right Menu",
        "Browser back",
        "Browser forward",
        "Browser refresh",
        "Browser stop",
        "Browser search",
        "Browser favorites",
        "Browser home",
        "Volume mute",
        "Volume down",
        "Volume up",
        "Next track",
        "Previous track",
        "Stop track",
        "Play/Pause track",
        "Start mail",
        "Select media",
        "Start app1",
        "Start app2",
        "Reserved",
        "Reserved",
        ";:",
        "+",
        ",",
        "-",
        ".",
        "/?",
        "`~",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Unassigned",
        "Unassigned",
        "Unassigned",
        "[{",
        "\\|",
        "]}",
        "\'\"",
        "OEM",
        "Reserved",
        "OEM",
        "OEM",
        "OEM",
        "OEM",
        "IME",
        "OEM",
        "VK_PACKET",
        "Unassigned",
        "OEM",
        "OEM",
        "OEM",
        "OEM",
        "OEM",
        "OEM",
        "OEM",
        "OEM",
        "OEM",
        "OEM",
        "OEM",
        "OEM",
        "OEM",
        "Attn",
        "CrSel",
        "ExSel",
        "EOF",
        "Play",
        "Zoom",
        "Noname",
        "PA1",
        "Clear",
        "None"
    };

    struct HotkeyData {
        int key;
        bool waiting;
        char label[128];
    };

    inline bool Hotkey(const char* label, int* k) {
        static std::map<ImGuiID, HotkeyData> hotkeyData;
        bool changed = false;

        ImGuiID id = ImGui::GetID(label);

        if (hotkeyData.find(id) == hotkeyData.end()) {
            hotkeyData[id] = HotkeyData{ *k, false, "" };
        }

        HotkeyData& data = hotkeyData[id];

        if (!data.waiting) {
            if (*k == 0 || *k >= sizeof(KeyNames) / sizeof(KeyNames[0])) {
                strcpy_s(data.label, sizeof(data.label), "[ None ]");
            }
            else {
                snprintf(data.label, sizeof(data.label), "[ %s ]", KeyNames[*k]);
            }
        }
        else {
            strcpy_s(data.label, sizeof(data.label), "[ Press key... ]");
        }

        if (ImGui::Button(data.label, ImVec2(0, 0))) {
            data.waiting = true;
        }

        if (data.waiting) {
            for (int i = 1; i < 256; i++) {
                if (GetAsyncKeyState(i) & 0x8000) {
                    if (i == VK_ESCAPE) {
                        data.waiting = false;
                        *k = 0;
                    }
                    else {
                        *k = i;
                        data.key = i;
                        changed = true;
                        data.waiting = false;
                    }
                    break;
                }
            }
        }

        if (data.waiting && ImGui::IsMouseClicked(0) && !ImGui::IsItemHovered()) {
            data.waiting = false;
        }

        return changed;
    }

} // namespace ImGui