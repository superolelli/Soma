# Soma

Soma is a 2D game and a small SFML-based game framework. The project now builds with CMake and targets SFML 3.

## Building on Windows

The repository currently includes prebuilt x64 dependencies under `Dependencies`, so Visual Studio 2022 and CMake 3.24 or newer are sufficient for the bundled build.

```powershell
cmake --preset windows-vs2022
cmake --build --preset windows-debug
```

The executable, runtime libraries, and copied `Data` directory are written to `build/bin`. Run the game with `build/bin/Soma.exe` so its relative asset paths resolve correctly.

The generated Visual Studio target sets its debugger working directory to `build/bin`, so launching `Soma` from the IDE resolves the same assets and DLLs as a direct launch.

sfeMovie is built from its bundled source and requires FFmpeg development files as well as its runtime DLLs. CMake looks for an FFmpeg prefix in `Dependencies/ffmpeg` and then `C:/ffmpeg`; set `FFMPEG_ROOT` when it is elsewhere. Runtime DLLs are copied from `Dependencies/ffmpeg/bin`, `C:/ffmpeg/bin`, or `SOMA_FFMPEG_RUNTIME_DIR`.

The build applies a local compatibility patch for AAC streams whose FFmpeg channel layout is unspecified. The generated source lives in the build tree; the ignored dependency checkout is not modified by a clean build.

Set `SOMA_USE_BUNDLED_DEPENDENCIES=OFF` to use installed packages instead. CMake then expects discoverable SFML 3, GLEW, OpenGL, and sfeMovie installations. Because sfeMovie does not install a CMake package configuration, its prefix can be supplied as `SFEMOVIE_ROOT`.

## Repository structure

- `src/engine`: first-party windowing, rendering, GUI, timing, and game-state framework code.
- `src/game`: Soma-specific gameplay, UI, state, and resource-management code.
- `third_party/SpriterPlusPlus`: vendored SpriterPlusPlus sources and its XML/JSON adapters.
- `third_party/json.hpp`: legacy standalone nlohmann/json header retained from the original project.
- `Dependencies`: binary development packages for SFML 3.1, GLEW, and the `jowie94/sfeMovie` fork.
- `Data`: runtime fonts, music, sounds, video, shaders, textures, and game definitions.
- `cmake`: package discovery modules for GLEW and sfeMovie.

CMake exposes separate `soma_engine`, `spriterplusplus`, and `Soma` targets. This keeps application, engine, and vendored compilation boundaries visible in IDEs and build diagnostics.

## Migration notes

The code has been updated for SFML 3's event variants, scoped enums, vector-valued coordinate APIs, angle type, `Rect` layout, resource constructors, renamed functions, and CMake targets. The old nine-patch quad geometry is rendered as triangle fans because SFML 3 removed the quad primitive. Obsolete source files that were not part of the original Visual Studio target remain in `src/game`, but are explicitly excluded from the CMake target.

## Open issues

- Replace `SfmlCompat` placeholder resources by passing real fonts, textures, and sound buffers into constructors. It is a migration bridge for SFML 2 classes that relied on removed default constructors.
- Remove global singleton resource managers and clarify resource ownership and initialization order.
- Replace owning raw pointers and `SAFE_DELETE` with `std::unique_ptr` or value semantics.
- Split the large game target into gameplay, UI, resources, and state libraries; add unit tests for non-rendering logic.
- Decide whether to repair or delete the obsolete `PlayerState`, `ItemRowPanel*`, and `ScrollableItemPanel*` variants excluded by CMake.
- Initialize GLEW once after context creation and report `glewInit` failures instead of initializing it in every `EquipmentConnections` object.
- Add explicit failure handling and logging for asset, shader, font, sound, music, video, XML, and Spriter file loading.
- Replace hard-coded `Data/...` paths with a configurable asset root and add install/package rules.
- Audit numeric narrowing warnings in layout and animation code, and use fixed-width types consistently.
- Fix mojibake already present in several German strings and normalize source/data encoding to UTF-8.
- Vendor or reproducibly acquire the exact FFmpeg runtime used by the prebuilt sfeMovie binary; CMake currently copies it from a configured local installation.
- Remove duplicate vendored JSON/XML implementations where possible and record exact third-party versions and licenses.
- Add continuous integration builds for Debug and Release on Windows, plus supported non-Windows platforms.
