# Rendering Module API

This page is only a partial reference for the canonical `src/rendering/*` tree.

**Canonical runtime note:** the active windowed renderer path is dependency-gated behind `GLFW + OpenGL` in `CMakeLists.txt`, and `game/application.h` includes `rendering/modern_renderer_integration.h` so `vge::Renderer` resolves through the modern renderer integration layer in the current public runtime path.

**Representative files:** `src/rendering/camera.h`, `src/rendering/framebuffer_renderer.h`, `src/rendering/lighting.h`, `src/rendering/material.h`, `src/rendering/mesh.h`, `src/rendering/renderer.h`, `src/rendering/modern_renderer_integration.h`, `src/rendering/shader.h`, `src/rendering/texture.h`

## `rendering/camera.h`
```cpp
namespace vge {
```

### `class Camera`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `SetPosition` | `void` | `const Vec3& pos` |
| `Move` | `void` | `const Vec3& delta` |
| `SetRotation` | `void` | `float yaw, float pitch, float roll` |
| `Rotate` | `void` | `float deltaYaw, float deltaPitch, float deltaRoll` |
| `GetViewMatrix` | `Mat4` | `` |
| `GetProjectionMatrix` | `Mat4` | `` |
| `GetForward` | `Vec3` | `` |
| `GetRight` | `Vec3` | `` |
| `GetUp` | `Vec3` | `` |

## `rendering/framebuffer_renderer.h`
```cpp
namespace vge {
```

### `class FramebufferRenderer`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `Initialize` | `bool` | `` |
| `Shutdown` | `void` | `` |
| `Clear` | `void` | `uint32_t color` |
| `SetPixel` | `void` | `int x, int y, uint32_t color` |
| `DrawRect` | `void` | `int x, int y, int w, int h, uint32_t color` |
| `DrawLine` | `void` | `int x0, int y0, int x1, int y1, uint32_t color` |
| `SwapBuffers` | `void` | `` |

## `rendering/lighting.h`
```cpp
namespace vge {
```

### `class LightEngine`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `PropagateBlockLight` | `void` | `Chunk* chunk` |
| `PropagateLightFromSource` | `void` | `Chunk* chunk, int sourceX, int sourceY, int sourceZ, int intensity` |
| `GetBlockLightEmission` | `int` | `BlockType type` |
| `InitializeChunk` | `void` | `Chunk* chunk` |
| `UpdateBlockLight` | `void` | `Chunk* chunk, int x, int y, int z` |
| `RecalculateChunk` | `void` | `Chunk* chunk` |

## `rendering/material.h`
```cpp
namespace vge {
```

### `struct Material`
| Member | Type |
|--------|------|
| `diffuseColor` | `Vec3` |
| `specularColor` | `Vec3` |
| `shininess` | `float` |
| `diffuseTexture` | `Texture*` |

## `rendering/mesh.h`
```cpp
namespace vge {
```

### `struct Vec2`

### `struct Vertex`
| Member | Type |
|--------|------|
| `position` | `Vec3` |
| `normal` | `Vec3` |
| `color` | `Vec3` |
| `texCoord` | `Vec2` |

### `class Mesh`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `AddVertex` | `void` | `const Vertex& v` |
| `AddIndex` | `void` | `uint32_t i` |

## `rendering/renderer.h`
```cpp
namespace vge {
```

`rendering/renderer.h` still exists in-tree, but the active application-facing runtime alias also flows through `rendering/modern_renderer_integration.h`.

### `class Renderer`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `RenderWorldASCII` | `void` | `const World& world, const Camera& camera` |
| `RenderWorldFB` | `void` | `const World& world, const Camera& camera` |
| `GetBlockColor` | `uint32_t` | `BlockType type` |
| `Initialize` | `bool` | `` |
| `Shutdown` | `void` | `` |
| `BeginFrame` | `void` | `` |
| `EndFrame` | `void` | `` |
| `SetClearColor` | `void` | `float r, float g, float b, float a` |
| `SetViewport` | `void` | `int x, int y, int w, int h` |
| `RenderMesh` | `void` | `const Mesh& mesh, const Shader& shader, const Camera& camera` |
| `RenderWorld` | `void` | `const World& world, const Camera& camera` |

## `rendering/modern_renderer_integration.h`
```cpp
namespace vge {
```

### Runtime alias

- `using Renderer = ModernRenderer`

## `rendering/shader.h`
```cpp
namespace vge {
```

### `class Shader`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `LoadFromFile` | `bool` | `const std::string& vertexPath, const std::string& fragmentPath` |
| `LoadFromSource` | `bool` | `const std::string& vertexSource, const std::string& fragmentSource` |
| `Bind` | `void` | `` |
| `SetInt` | `void` | `const std::string& name, int value` |
| `SetFloat` | `void` | `const std::string& name, float value` |
| `SetVec3` | `void` | `const std::string& name, const Vec3& value` |
| `SetMat4` | `void` | `const std::string& name, const Mat4& value` |

## `rendering/texture.h`
```cpp
namespace vge {
```

### `class Texture`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `LoadFromFile` | `bool` | `const std::string& path` |
| `LoadFromData` | `bool` | `const unsigned char* data, int w, int h, int channels` |
| `Bind` | `void` | `int slot = 0` |
| `Unbind` | `void` | `` |
