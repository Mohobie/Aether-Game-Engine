# Aether Game Engine - Audit Review Snapshot

**Date:** 2026-05-15  
**Scope:** build truth and canonical architecture  
**Status:** partial verification only

## Verified In This Workspace

Using Visual Studio 2022 and `C:/Program Files/CMake/bin/cmake.exe`, the following targets were configured and built successfully on 2026-05-15:

- `voxel_engine_lib`
- `save_test`
- `world_test`
- `voxel_test`

The following executables were also run successfully:

- `build-vs/Debug/save_test.exe`
- `build-vs/Debug/world_test.exe`
- `build-vs/Debug/voxel_test.exe`

Current dependency state in this shell:

- `OpenGL=TRUE`
- `GLFW=FALSE`
- `Lua=FALSE`

## Not Verified In This Workspace

- `voxel_engine` was not built or run in this shell because the GLFW-backed windowed runtime path is currently unavailable.
- Lua-backed scripting paths were not verified because Lua is not available in this configuration.
- Many feature claims in older docs remain unverified at runtime even when related source files exist in-tree.

## Canonical Architecture Decisions From This Audit

- Rendering: `src/rendering/*` is canonical; `src/render/*` is legacy.
- Input: `src/platform/input_manager.*` is canonical for the active runtime build; `src/input/*` is legacy; `src/platform/input.*` is compatibility-only outside the active library target.
- Application facade: `src/game/application.*` is canonical; `src/core/application.*` is legacy.
- Entity layer: `src/entity/entity.*` and `src/entity/components.*` are canonical; `src/core/entity.*` is legacy.
- Save stack: `src/core/save_system.*` is the direct world/chunk API, `src/voxel/world_serializer.*` is the richer world snapshot layer, and `src/core/save_game.*` is the active higher-level wrapper; `src/game/save_system.*` and `src/game/serializer.*` are legacy.

## Build-System Findings Closed

- earlier duplicate source entries called out by the audit file are no longer present in `CMakeLists.txt`
- windowed application, render runtime, editor runtime, and `voxel_engine` now share the same `GLFW + OpenGL` gating boundary in `CMakeLists.txt`
- capability compile definitions are now target-scoped: `VGE_HAS_GLFW`, `VGE_HAS_OPENGL`, `VGE_HAS_LUA`
- the Windows recursion bug in `src/platform/platform_common.cpp` for `GetCurrentThreadId()` was fixed

## Remaining Risks

- the repository still contains many parallel legacy paths and stale docs
- the current build still emits non-fatal warnings across unrelated subsystems
- top-level runtime claims should continue to be treated cautiously until `voxel_engine` itself is re-enabled and revalidated on a machine with the full dependency set
- networking, rendering, scripting, UI, and save/load breadth beyond the verified tests are not proven by this audit snapshot

## Next Recommended Step

Choose one policy and make it explicit across the repo:

1. restore `GLFW` and finish the dependency inventory needed to verify `voxel_engine`
2. or formally standardize the current headless/core split as the supported default build on machines without windowed runtime dependencies
