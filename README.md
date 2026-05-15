# Aether Game Engine

A C++17 voxel-engine codebase under audit. The current verified path in this repository is the core library plus a small set of test executables; the windowed runtime remains dependency-gated on machines where `GLFW` is unavailable.

## Current Build Truth

Verified in this workspace on 2026-05-15 with Visual Studio 2022 and `C:/Program Files/CMake/bin/cmake.exe`:

- `voxel_engine_lib`
- `save_test`
- `world_test`
- `voxel_test`

Verified executables:

- `build-vs/Debug/save_test.exe`
- `build-vs/Debug/world_test.exe`
- `build-vs/Debug/voxel_test.exe`

Current dependency state in this shell:

- `OpenGL=TRUE`
- `GLFW=FALSE`
- `Lua=FALSE`

Result:

- the headless/core library path is verified
- `voxel_engine` is not currently verified in this shell because the GLFW-backed windowed runtime path is unavailable

## Canonical Module Map

- Rendering: `src/rendering/*`
  Legacy: `src/render/*`
- Input: `src/platform/input_manager.*`
  Legacy: `src/input/*`
  Compatibility-only outside the active library target: `src/platform/input.*`
- Application facade: `src/game/application.*`
  Legacy: `src/core/application.*`
- Entity layer: `src/entity/entity.*`, `src/entity/components.*`
  Legacy: `src/core/entity.*`
- Save stack: `src/core/save_system.*` for direct world/chunk IO, `src/voxel/world_serializer.*` for richer world snapshots, and `src/core/save_game.*` as the active higher-level wrapper
  Legacy: `src/game/save_system.*`, `src/game/serializer.*`

## Building

### Windows

```cmd
C:\Program Files\CMake\bin\cmake.exe -S . -B build-vs -G "Visual Studio 17 2022"
C:\Program Files\CMake\bin\cmake.exe --build build-vs --config Debug --target voxel_engine_lib save_test world_test voxel_test
build-vs\Debug\save_test.exe
build-vs\Debug\world_test.exe
build-vs\Debug\voxel_test.exe
```

### Notes

- `voxel_engine` is only expected to build when the GLFW-backed windowed runtime dependency path is available.
- Lua-backed scripting sources are excluded when Lua is not installed.
- Bundled Dear ImGui is present in-tree, but the optional editor/menu executable stack is still gated on the same windowed runtime dependencies.

## Public Entry Points

- Public include: `include/aether_engine.h`
- Compatibility include: `include/voxel_engine.h`
- Library target: `voxel_engine_lib`

## Documentation

- [docs/QUICKSTART.md](docs/QUICKSTART.md)
- [docs/WIKI.md](docs/WIKI.md)
- [docs/API-INDEX.md](docs/API-INDEX.md)

## Status

This repository still contains parallel legacy modules and partially implemented subsystems. Treat the verified build targets and the canonical module map above as the source of truth for the current audit state, not older feature claims elsewhere in the tree.
