#include "../framework.h"
#include "Menu.hpp"
#include "../hooks.hpp"
#include <algorithm>
#include <array>
#include <string_view>
#include <unordered_set>
#include <Windows.h>
#include <fstream>
using json = nlohmann::json;

static void menuBarItem(const char* label, bool& enabled) noexcept
{
    if (ImGui::Selectable(label, enabled, ImGuiSelectableFlags_None, { ImGui::CalcTextSize(label, nullptr, true).x + ImGui::GetStyle().FramePadding.x, 0 }))
    {
        enabled = !enabled;
        if (enabled)
            ImGui::SetWindowFocus(label);
    }
}
void RenderImMenu::RenderContextMenu() noexcept
{
    if (ImGui::MenuItem("Close all"))
        window = {};

    ImGui::Separator();

    if (ImGui::MenuItem("Lua Console",  nullptr, window.luatab))    { window.luatab    = !window.luatab;    if (window.luatab)    ImGui::SetWindowFocus("Lua Console"); }
    if (ImGui::MenuItem("Lua Loader",   nullptr, window.loadertab)) { window.loadertab = !window.loadertab; if (window.loadertab) ImGui::SetWindowFocus("Lua Loader");   }
    if (ImGui::MenuItem("Debug Log",    nullptr, window.debugtab))  { window.debugtab  = !window.debugtab;  if (window.debugtab)  ImGui::SetWindowFocus("Debug Log");    }

    ImGui::Separator();


    if (ImGui::MenuItem("Clear Output Log"))
    {
        std::lock_guard<std::mutex> lock(arisu::game::Engine::luaOutputMutex);
        arisu::game::Engine::luaOutputLog.clear();
    }

    if (ImGui::MenuItem("Refresh Lua Scripts"))
        ScanLuaLoaderFolder();

    ImGui::Separator();

#ifndef BUILD_ASI
    if (ImGui::MenuItem("Unload  (F10)"))
        keybd_event(VK_F10, 0, 0, 0);
#endif
}

void RenderImMenu::RenderMenuBar() noexcept
{
    if (ImGui::BeginMainMenuBar())
    {

        menuBarItem("Lua", window.luatab);
        menuBarItem("Lua Loader", window.loadertab);
        menuBarItem("Debug Log", window.debugtab);
        ImGui::Separator();
        if (ImGui::BeginMenu("Context"))
        {
            RenderContextMenu();
            ImGui::EndMenu();
        }
        ImGui::Separator();
        ImGui::Text("owo?");
        ImGui::EndMainMenuBar();
    }
}

void RenderImMenu::RenderLuaTab() {

    if (!window.luatab)
        return;
    ImGui::Begin("Lua Console", &window.luatab, ImGuiWindowFlags_None);

    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Quick Function Call");
    ImGui::InputText("Function", FuncExec.data(), FuncExec.size());
    ImGui::SameLine();
    if (ImGui::Button("Call")) {
        int L = arisu::game::Engine::GetLuaState();
        if (L) {
            arisu::game::Engine::LuaGetField(L, 0xFFFFD8EE, FuncExec.data());
            arisu::game::Engine::LuaPcall(L, 0, 0, 0);
            luaOutput = "[OK] Called: " + std::string(FuncExec.data());
        } else {
            luaOutput = "[ERROR] lua_State is null";
        }
    }

    ImGui::Separator();

    if (!luaEditorInitialized) {
        luaEditor.SetLanguage(TextEditor::Language::Lua());
        luaEditor.SetShowLineNumbersEnabled(true);
        luaEditor.SetTabSize(4);
        luaEditor.SetText("");

        BuildLuaTrie();

        luaAutocompleteConfig.callback = [this](TextEditor::AutoCompleteState& state) {
            if (luaTrie) {
                luaTrie->findSuggestions(state.suggestions, state.searchTerm, 40);

                // Trie fuzzy search can yield duplicates via different match paths — deduplicate
                std::unordered_set<std::string> seen;
                auto it = std::remove_if(state.suggestions.begin(), state.suggestions.end(),
                    [&seen](const std::string& s) { return !seen.insert(s).second; });
                state.suggestions.erase(it, state.suggestions.end());

                if (state.suggestions.size() > 10)
                    state.suggestions.resize(10);
            }
            };
        luaEditor.SetAutoCompleteConfig(&luaAutocompleteConfig);


        luaEditor.SetChangeCallback([this]() {
            BuildLuaTrie();
            }, 2000);

        luaEditorInitialized = true;
    }

    ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), xorstr_("Prototype 1 Lua Editor By Rico1662 Discord@fastcall"));

    {
        int line, column;
        luaEditor.GetCurrentCursor(line, column);
        ImGui::Text("Ln %d, Col %d | %d lines | %s",
            line + 1, column + 1,
            luaEditor.GetLineCount(),
            luaEditor.IsOverwriteEnabled() ? "OVR" : "INS");
    }

    luaEditor.Render("##LuaScript", ImVec2(-1.0f, ImGui::GetTextLineHeight() * 20), true);

    if (ImGui::Button("Execute Script")) {
        std::string scriptText = luaEditor.GetText();
        std::string error;
        int result = arisu::game::Engine::ExecuteLuaScript(scriptText.c_str(), &error);
        if (result == 0) {
            luaOutput = "[OK] Script executed.";
        } else {
            luaOutput = "[ERROR] " + error;
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear Script")) {
        luaEditor.SetText("");
    }
    ImGui::SameLine();


    ImGui::SetNextItemWidth(140.0f);
    ImGui::InputText("##SaveName", luaSaveNameBuf.data(), luaSaveNameBuf.size());
    ImGui::SameLine();
    if (ImGui::Button("Save to lua_p1")) {
        std::string saveName(luaSaveNameBuf.data());
        if (!saveName.empty()) {

            std::error_code ec;
            std::filesystem::create_directory("lua_p1", ec);

            if (saveName.size() < 4 || saveName.substr(saveName.size() - 4) != ".lua")
                saveName += ".lua";

            std::string savePath = "lua_p1/" + saveName;
            std::ofstream outFile(savePath);
            if (outFile.is_open()) {
                outFile << luaEditor.GetText();
                outFile.close();
                luaOutput = "[OK] Saved to: " + savePath;
            } else {
                luaOutput = "[ERROR] Cannot write to: " + savePath;
            }
        }
    }

    ImGui::SameLine();
    ImGui::SetNextItemWidth(200.0f);
    ImGui::InputText("##LuaFilePath", luaFilePathBuf.data(), luaFilePathBuf.size());
    ImGui::SameLine();
    if (ImGui::Button("Run File")) {
        std::string error;
        int result = arisu::game::Engine::ExecuteLuaFile(luaFilePathBuf.data(), &error);
        if (result == 0) {
            luaOutput = "[OK] File executed: " + std::string(luaFilePathBuf.data());
        } else {
            luaOutput = "[ERROR] " + error;
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Load to Editor")) {
        std::ifstream file(luaFilePathBuf.data());
        if (file.is_open()) {
            std::string content((std::istreambuf_iterator<char>(file)),
                std::istreambuf_iterator<char>());
            luaEditor.SetText(content);
            luaOutput = "[OK] Loaded file into editor.";
        } else {
            luaOutput = "[ERROR] Cannot open file: " + std::string(luaFilePathBuf.data());
        }
    }

    if (!luaOutput.empty()) {
        bool isError = luaOutput.find("[ERROR]") != std::string::npos;
        ImGui::TextColored(isError ? ImVec4(1.0f, 0.3f, 0.3f, 1.0f) : ImVec4(0.3f, 1.0f, 0.3f, 1.0f),
            "%s", luaOutput.c_str());
    }

    ImGui::Separator();

    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.4f, 1.0f), "Output");
    ImGui::SameLine();
    if (ImGui::SmallButton("Clear Output")) {
        std::lock_guard<std::mutex> lock(arisu::game::Engine::luaOutputMutex);
        arisu::game::Engine::luaOutputLog.clear();
    }

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
    ImGui::BeginChild("##LuaOutput", ImVec2(-1.0f, ImGui::GetTextLineHeight() * 5), true);

    {
        std::lock_guard<std::mutex> lock(arisu::game::Engine::luaOutputMutex);
        if (!arisu::game::Engine::luaOutputLog.empty()) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.9f, 1.0f, 1.0f));
            ImGui::TextUnformatted(arisu::game::Engine::luaOutputLog.c_str());
            ImGui::PopStyleColor();

            if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 20.0f)
                ImGui::SetScrollHereY(1.0f);
        } else {
            ImGui::TextDisabled("(no output yet. use print() in Lua scripts)");
        }
    }

    ImGui::EndChild();
    ImGui::PopStyleColor();

    ImGui::Separator();


    ImGui::TextColored(ImVec4(0.8f, 0.6f, 1.0f, 1.0f), "Dump Lua Globals");
    if (ImGui::Button("Dump to Output")) {
        arisu::game::Engine::DumpLuaGlobals(nullptr);
        luaOutput = "[OK] Globals dumped to output.";
    }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(180.0f);
    ImGui::InputText("##DumpPath", luaDumpPathBuf.data(), luaDumpPathBuf.size());
    ImGui::SameLine();
    if (ImGui::Button("Dump to File")) {
        arisu::game::Engine::DumpLuaGlobals(luaDumpPathBuf.data());
        luaOutput = "[OK] Globals dumped to file: " + std::string(luaDumpPathBuf.data());
    }

    ImGui::Separator();

    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.4f, 1.0f), "Hotkey Bindings");

    if (ImGui::Button("+")) {
        LuaFunc.push_back(MenuItem());
    }

    ImGui::SameLine();

    if (ImGui::Button("-") && !LuaFunc.empty()) {
        LuaFunc.pop_back();
    }

    for (size_t i = 0; i < LuaFunc.size(); i++) {
        ImGui::PushID(static_cast<int>(i));

        ImGui::SetNextItemWidth(120.0f);
        ImGui::Hotkey("HotKey", &LuaFunc[i].KeyBind);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(200.0f);
        ImGui::InputText("Function", LuaFunc[i].LuaFuncName.data(), LuaFunc[i].LuaFuncName.size());

        ImGui::PopID();
    }

    if (ImGui::Button("Save")) {
        SaveConfig();
    }
    ImGui::SameLine();
    if (ImGui::Button("Load")) {
        LoadConfig();
    }

    ImGui::Separator();
    int L = arisu::game::Engine::GetLuaState();
    ImGui::Text("lua_State: 0x%08X", L);
    ImGui::Text("LuaScriptManager: 0x%p",
        arisu::game::Engine::g_pLuaScriptManager ? *arisu::game::Engine::g_pLuaScriptManager : nullptr);

    ImGui::End();
}

void RenderImMenu::RenderDebugTab() {

    if (!window.debugtab)
        return;


    ImGui::Begin("Debug Log", &window.debugtab,
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoNav);

    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.4f, 1.0f), "LUA print() Output");
    ImGui::SameLine();
    if (ImGui::SmallButton("Clear")) {
        std::lock_guard<std::mutex> lock(arisu::game::Engine::luaOutputMutex);
        arisu::game::Engine::luaOutputLog.clear();
    }
    ImGui::SameLine();

    int L = arisu::game::Engine::GetLuaState();
    ImGui::TextDisabled("lua_State: 0x%08X", L);

    ImGui::Separator();

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.08f, 0.08f, 0.08f, 1.0f));
    ImGui::BeginChild("##DebugLuaOutput", ImVec2(-1.0f, -1.0f), true);

    {
        std::lock_guard<std::mutex> lock(arisu::game::Engine::luaOutputMutex);
        if (!arisu::game::Engine::luaOutputLog.empty()) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.95f, 0.8f, 1.0f));
            ImGui::TextUnformatted(arisu::game::Engine::luaOutputLog.c_str());
            ImGui::PopStyleColor();

            if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 20.0f)
                ImGui::SetScrollHereY(1.0f);
        } else {
            ImGui::TextDisabled("(no output yet. use print() in Lua scripts)");
        }
    }

    ImGui::EndChild();
    ImGui::PopStyleColor();

    ImGui::End();
}



void RenderImMenu::ExecLua(uintptr_t lua_State) {
    int L = arisu::game::Engine::GetLuaState();
    if (!L) return;

    for (size_t i = 0; i < LuaFunc.size(); i++) {
        if (GetAsyncKeyState(LuaFunc[i].KeyBind) & 1)
        {
            arisu::game::Engine::LuaGetField(L, 0xFFFFD8EE, LuaFunc[i].LuaFuncName.data());
            arisu::game::Engine::LuaPcall(L, 0, 0, 0);
        }
    }
}


void RenderImMenu::SaveConfig() {
    json j;
    for (size_t i = 0; i < LuaFunc.size(); i++) {

        size_t actualLength = 0;
        for (size_t k = 0; k < LuaFunc[i].LuaFuncName.size(); k++) {
            if (LuaFunc[i].LuaFuncName[k] == '\0') {
                break;
            }
            actualLength++;
        }

        if (actualLength > 0) {
            j.push_back({
                {"FUNC_KEY", LuaFunc[i].KeyBind},
                {"FUNC_NAME", std::string(LuaFunc[i].LuaFuncName.data(), actualLength)}
                });
        }
    }

    std::ofstream file(configFilePath);
    if (file.is_open()) {
        file << j.dump(4);
        file.close();
    }
}

void RenderImMenu::LoadConfig() {
    std::ifstream file(configFilePath);
    if (file.is_open()) {
        json j;
        file >> j;
        file.close();

        LuaFunc.clear();
        for (const auto& item : j) {
            MenuItem menuItem;
            menuItem.KeyBind = item["FUNC_KEY"];


            std::string inputStr = item["FUNC_NAME"];
            std::fill(menuItem.LuaFuncName.begin(), menuItem.LuaFuncName.end(), 0);
            std::copy(
                inputStr.begin(),
                inputStr.begin() + std::min(inputStr.length(), menuItem.LuaFuncName.size() - 1),
                menuItem.LuaFuncName.begin()
            );

            LuaFunc.push_back(menuItem);
        }
    }
}

void RenderImMenu::ScanLuaLoaderFolder()
{
    luaLoaderEntries.clear();
    const std::filesystem::path folder = "lua_p1";

    std::error_code ec;
    if (!std::filesystem::exists(folder, ec) || !std::filesystem::is_directory(folder, ec))
    {
        // Create the folder so the user knows where to put scripts
        std::filesystem::create_directory(folder, ec);
        return;
    }

    for (const auto& entry : std::filesystem::directory_iterator(folder, ec))
    {
        if (!entry.is_regular_file()) continue;
        auto ext = entry.path().extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        if (ext != ".lua") continue;

        LuaLoaderEntry le;
        le.filename   = entry.path().filename().string();
        le.fullPath   = entry.path().string();
        le.isLooping  = false;
        le.lastRunOk  = true;
        le.errorCount = 0;
        luaLoaderEntries.push_back(std::move(le));
    }

    // Sort alphabetically
    std::sort(luaLoaderEntries.begin(), luaLoaderEntries.end(),
        [](const LuaLoaderEntry& a, const LuaLoaderEntry& b) { return a.filename < b.filename; });
}

void RenderImMenu::RenderLuaLoaderTab()
{
    if (!window.loadertab)
        return;

    ImGui::Begin("Lua Loader", &window.loadertab,
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoNav);

    ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f), "Lua Loader (lua_p1/)");

    if (ImGui::Button("Refresh Scripts"))
    {
        ScanLuaLoaderFolder();
        luaLoaderScanned = true;
    }

    // Auto-scan on first render is now handled at ImGui init time (hooks.cpp)

    if (luaLoaderEntries.empty())
    {
        ImGui::TextDisabled("No .lua files found in lua_p1/ folder.");
        ImGui::TextDisabled("Place your scripts there and click Refresh.");
        ImGui::End();
        return;
    }

    ImGui::SameLine();
    ImGui::TextDisabled("(%d scripts)", (int)luaLoaderEntries.size());

    // Count active looping scripts
    int activeCount = 0;
    for (auto& e : luaLoaderEntries)
        if (e.isLooping) activeCount++;
    if (activeCount > 0)
    {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.4f, 1.0f), " | %d active", activeCount);
    }

    ImGui::Separator();

    // Table header
    ImGui::Columns(3, "##LuaLoaderCols", true);
    ImGui::SetColumnWidth(0, ImGui::GetWindowWidth() * 0.45f);
    ImGui::SetColumnWidth(1, ImGui::GetWindowWidth() * 0.30f);
    ImGui::SetColumnWidth(2, ImGui::GetWindowWidth() * 0.25f);
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Script");
    ImGui::NextColumn();
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Run Once");
    ImGui::NextColumn();
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Loop");
    ImGui::NextColumn();
    ImGui::Separator();

    for (size_t i = 0; i < luaLoaderEntries.size(); i++)
    {
        auto& entry = luaLoaderEntries[i];
        ImGui::PushID(static_cast<int>(i));

        // Script name with status color indicator
        if (entry.isLooping)
        {
            // Pulsing green highlight for active looping scripts
            float pulse = (sinf((float)ImGui::GetTime() * 3.0f) + 1.0f) * 0.5f;
            ImVec4 col = ImVec4(0.1f + 0.2f * pulse, 0.8f + 0.2f * pulse, 0.2f + 0.2f * pulse, 1.0f);
            ImGui::TextColored(col, "%s", entry.filename.c_str());
        }
        else if (!entry.lastRunOk)
        {
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "%s", entry.filename.c_str());
        }
        else
        {
            ImGui::Text("%s", entry.filename.c_str());
        }

        // Tooltip with full path and error info
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::Text("Path: %s", entry.fullPath.c_str());
            if (entry.isLooping && !entry.cachedScript.empty())
                ImGui::TextDisabled("Cached: %d bytes", (int)entry.cachedScript.size());
            if (!entry.lastError.empty())
                ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Error: %s", entry.lastError.c_str());
            if (entry.errorCount > 0 && entry.isLooping)
                ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f), "Error count: %d/10", entry.errorCount);
            ImGui::EndTooltip();
        }

        ImGui::NextColumn();

        // "Run Once" button
        if (ImGui::Button("Execute"))
        {
            std::string error;
            int result = arisu::game::Engine::ExecuteLuaFile(entry.fullPath.c_str(), &error);
            entry.lastRunOk = (result == 0);
            entry.lastError = (result == 0) ? "" : error;
        }
        ImGui::NextColumn();

        // Loop toggle button with color feedback
        if (entry.isLooping)
        {
            ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered,  ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,   ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
            if (ImGui::Button("Stop"))
            {
                entry.isLooping = false;
            }
            ImGui::PopStyleColor(3);
        }
        else
        {
            ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered,  ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,   ImVec4(0.1f, 0.5f, 0.1f, 1.0f));
            if (ImGui::Button("Loop"))
            {
                // Read and cache script content on start
                std::ifstream file(entry.fullPath);
                if (file.is_open()) {
                    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
                    entry.cachedScript = std::move(content);
                    entry.isLooping = true;
                    entry.lastRunOk = true;
                    entry.lastError.clear();
                    entry.errorCount = 0;
                } else {
                    entry.lastRunOk = false;
                    entry.lastError = "Cannot open file: " + entry.fullPath;
                }
            }
            ImGui::PopStyleColor(3);
        }

        ImGui::NextColumn();
        ImGui::PopID();
    }

    ImGui::Columns(1);

    ImGui::End();
}


void RenderImMenu::TickLuaLoader()
{
    constexpr int kMaxConsecutiveErrors = 10;

    for (auto& entry : luaLoaderEntries)
    {
        if (!entry.isLooping) continue;

        std::string error;
        int result = arisu::game::Engine::ExecuteLuaScript(entry.cachedScript.c_str(), &error);
        entry.lastRunOk = (result == 0);

        if (result != 0)
        {
            entry.lastError = error;
            entry.errorCount++;
            if (entry.errorCount >= kMaxConsecutiveErrors)
            {
                entry.isLooping = false;
                entry.lastError += " (auto-stopped after " + std::to_string(kMaxConsecutiveErrors) + " errors)";
            }
        }
        else
        {
            entry.lastError.clear();
            entry.errorCount = 0;
        }
    }
}

void RenderImMenu::BuildLuaTrie() {
    luaTrie = std::make_unique<TextEditor::Trie>();

    // Hardcoded engine globals
    for (const auto& fn : GetLuaGlobals())
        luaTrie->insert(fn);

    // Language keywords and identifiers
    if (auto* lang = TextEditor::Language::Lua()) {
        for (auto& kw : lang->keywords)    luaTrie->insert(kw);
        for (auto& kw : lang->declarations) luaTrie->insert(kw);
        for (auto& kw : lang->identifiers) luaTrie->insert(kw);
    }

    // Identifiers from the current document
    luaEditor.IterateIdentifiers([this](const std::string& id) {
        luaTrie->insert(id);
        });
}
