# Soma

Soma is a German-language 2D party RPG built with modern C++20, SFML 3, SpriterPlusPlus, and sfeMovie. Game definitions and presentation layout are loaded from JSON; simulation, application state, persistence, and SFML presentation are separate modules.

A concise overview of gameplay and player-facing differences from the original version is available in the [changelog](CHANGELOG.md).

The supported platform is Windows x64 with Visual Studio 2022/MSVC.

## Build

Run these commands from the repository root:

```powershell
cmake --preset windows-vs2022
cmake --build --preset windows-debug --parallel
ctest --test-dir build -C Debug --output-on-failure
cmake --build build --config Debug --target validate_content
```

For an optimized build:

```powershell
cmake --build --preset windows-release --parallel
```

The executables are written to `build/bin/Debug` and `build/bin/Release`. Run Soma from the repository root so development tools and relative asset paths are easy to understand:

```powershell
.\build\bin\Debug\Soma.exe
```

The application also searches upward from the executable for `Data/content/v1`, so it can locate the repository when started from Visual Studio or directly from the build directory.

## Development launchers

Soma normally opens the save-slot menu. Development flags can directly open a battle, level, or economy panel. The complete reference is in [Command-line options](docs/COMMAND_LINE.md).

Common examples:

```powershell
.\build\bin\Debug\Soma.exe --savegame .\Data\Saves\developer.json
.\build\bin\Debug\Soma.exe --explore level.bang.1 --seed 1234
.\build\bin\Debug\Soma.exe --enemies enemy.greg enemy.apache_kid --auto --exit-on-finish
.\build\bin\Debug\Soma.exe --skills
.\build\bin\Debug\Soma.exe --inventory
.\build\bin\Debug\Soma.exe --shop
.\build\bin\Debug\Soma.exe --rules
```

## Content

Runtime content lives in `Data/content/v1`. See [Content authoring](docs/CONTENT_AUTHORING.md) for the file map and workflows for adding or changing items, enemies, abilities, skills, encounters, levels, lootables, rule fragments, assets, and presentation layout.

After every content change, run:

```powershell
cmake --build build --config Debug --target validate_content
```

## Repository structure

- `src/gameplay`: shared IDs, events, item/combat value types, clocks, and random interfaces.
- `src/content`: typed JSON catalogs and validation.
- `src/simulation`: battle rules, enemy decisions, level generation, rules, loot, and rewards; no SFML dependency.
- `src/game`: campaign state and application services for economy, equipment, skills, progression, and rules.
- `src/persistence`: save serialization, validation, and save slots.
- `src/presentation`: renderer-neutral descriptions plus the SFML application and screens.
- `src/thirdparty`: the retained SpriterPlusPlus adapter sources.
- `Data`: JSON content and runtime fonts, music, sounds, videos, shaders, and sprites.
- `Dependencies`: bundled SFML, FFmpeg, and sfeMovie dependencies.
- `tools/content_validator`: standalone validation for all shipped JSON catalogs.
- `tests`: permanent simulation, content, economy, presentation, rules, world, and persistence regression tests.

## Dependencies

The default CMake preset uses the bundled SFML package and sfeMovie source. sfeMovie requires FFmpeg development files and runtime DLLs. CMake checks `Dependencies/ffmpeg` and then `C:/ffmpeg`; use `FFMPEG_ROOT` and `SOMA_FFMPEG_RUNTIME_DIR` when FFmpeg is installed elsewhere.

Set `SOMA_USE_BUNDLED_DEPENDENCIES=OFF` to use an installed SFML package. The project intentionally supports only Windows and MSVC.
