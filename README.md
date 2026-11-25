# Rogue2

Rogue2 is an experimental turn-based roguelike written in C++ for Windows. Explore procedurally generated maps, fight monsters in melee or at range, cast spells, and pick up equipment for your character.

The project uses artwork from *Ancient Domains of Mystery* (ADOM).

## Play on Windows

Download [`rogue2-win32.zip`](https://github.com/Pavelius/rogue2/releases/download/latest-build/rogue2-win32.zip), extract the archive, and run `rogue2.exe`. The release is a 32-bit Windows build.

## Controls

| Key | Action |
| --- | --- |
| Arrow keys | Move north, south, east, or west |
| Home / Page Up | Move northwest / northeast |
| End / Page Down | Move southwest / southeast |
| I | Open inventory |
| U | Open backpack |
| P | View and pick up items on the ground |
| D | Drop an item |
| S | Shoot |
| Ctrl+M | Open the in-game manual |

## Build from source

Open `rogue2.sln` in Visual Studio with the Desktop development with C++ workload, the MSVC v143 toolset, and a Windows 10 SDK. Select **Release** and **Win32**, then build the solution.

To use MSBuild, run this command from the repository root in a Visual Studio Developer Command Prompt:

```cmd
msbuild source/rogue2.vcxproj /m /p:Configuration=Release /p:Platform=Win32
```

The executable is written to `source/Release/rogue2.exe`.

## Design notes

- [Combat](help/combat.md)
- [Quest types](help/quest.md)
- [Village buildings](help/sites.md)

## License

See [LICENSE.txt](LICENSE.txt) for the Apache License 2.0 text.
