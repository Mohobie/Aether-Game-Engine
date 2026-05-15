# Aether Game Engine

Aether Game Engine is a modular C++17 voxel-engine codebase for building sandbox, survival, and other world-driven games. It combines voxel world systems, gameplay modules, rendering paths, UI, audio, physics, AI, networking, and scripting-oriented infrastructure in a single repository.

## Highlights

- Voxel world foundation: blocks, chunks, world generation, block registry, chunk management, and save/load layers
- Modular gameplay systems: crafting, inventory, combat, quests, NPC dialog, survival systems, tools, furnaces, and world interaction
- Rendering stack in-tree: camera, meshes, shaders, lighting, sky, weather, particles, post-processing, and world rendering paths
- Engine subsystems: audio, AI, networking, UI, resource management, platform abstractions, and editor-facing utilities
- Public engine library target: `voxel_engine_lib`

## Build Status

The repository contains both always-built core systems and optional runtime/editor paths that depend on external libraries being available on the host machine.

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

What that means:

- the core engine library and the three verified tests build cleanly in this environment
- the windowed runtime target `voxel_engine` remains optional and only builds when the GLFW-backed runtime dependency path is available
- Lua-backed scripting sources are excluded when Lua is not available

## Canonical Module Map

The current supported public architecture in the codebase is:

- Rendering: `src/rendering/*`
- Input: `src/platform/input_manager.*`
- Application facade: `src/game/application.*`
- Entity layer: `src/entity/entity.*`, `src/entity/components.*`
- Save stack: `src/core/save_system.*`, `src/voxel/world_serializer.*`, `src/core/save_game.*`

Legacy parallel paths still exist in-tree for some subsystems, but the paths above are the current source of truth for the active architecture.

## Building

### Windows

```cmd
C:\Program Files\CMake\bin\cmake.exe -S . -B build-vs -G "Visual Studio 17 2022"
C:\Program Files\CMake\bin\cmake.exe --build build-vs --config Debug --target voxel_engine_lib save_test world_test voxel_test
build-vs\Debug\save_test.exe
build-vs\Debug\world_test.exe
build-vs\Debug\voxel_test.exe
```

If GLFW-backed window/runtime dependencies are available on your machine, you can also configure and build the executable target:

```cmd
C:\Program Files\CMake\bin\cmake.exe --build build-vs --config Debug --target voxel_engine
```

## Public Entry Points

- Public include: `include/aether_engine.h`
- Compatibility include: `include/voxel_engine.h`
- Primary library target: `voxel_engine_lib`

## Documentation

- [docs/QUICKSTART.md](docs/QUICKSTART.md)
- [docs/WIKI.md](docs/WIKI.md)
- [docs/API-INDEX.md](docs/API-INDEX.md)
- [REVIEW_REPORT.md](REVIEW_REPORT.md)

## Project Direction

The repository remains an active engine codebase rather than a minimal library skeleton. It includes both production-oriented core systems and parallel or partially integrated subsystems that are still being consolidated. The verified build targets and documented canonical module map above are the best guide to what is currently supported.
