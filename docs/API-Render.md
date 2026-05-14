# Render Module API

This module family is **legacy**.

**Legacy files:** `src/render/camera.h`, `src/render/mesh.h`, `src/render/renderer.h`, `src/render/shader.h`, `src/render/texture.h`  
**Canonical replacement:** `src/rendering/*` documented in [API-Rendering](API-Rendering)

The 2026-05-14 architecture audit keeps `rendering/` as the supported renderer path. This page remains only as a pointer for older references that still mention `render/`.

## Legacy note

- Prefer `rendering/camera.h` over `render/camera.h`
- Prefer `rendering/mesh.h` over `render/mesh.h`
- Prefer `rendering/renderer.h` over `render/renderer.h`
- Prefer `rendering/shader.h` over `render/shader.h`
- Prefer `rendering/texture.h` over `render/texture.h`

## Namespace

Older generated docs for this module often show `namespace aether`, but the active codebase uses `namespace vge`.
