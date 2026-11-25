# Prototype 1 Lua Tool

In-game Lua executor and editor for Prototype 1.
Made by Rico1662 (Discord: `@fastcall`)

## Features
- Lua editor with syntax highlighting and autocomplete (~900 game functions)
- Redirects `print()` / `debug()` output to an in-game log window
- Script loader (`lua_p1/` folder in game root) with one-shot run and tick loop
- Bind global Lua functions to hotkeys (`Config.json`)
- Dump `_G` functions to console or file

## Usage
Compile in `Release | x86` and inject the DLL into `prototypef.exe` (or compile with `BUILD_ASI` to use it as an `.asi`).

- `INSERT` - toggle menu
- `F10` - unload DLL

Put `.lua` scripts inside `<game folder>/lua_p1/` to load them from the Lua Loader tab.

## License
[GPL-3.0](LICENSE). If you use code from this repo, give proper credit and keep your fork open-source.
