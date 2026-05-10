# Render Module API
**Files:** src/render/camera.h, src/render/mesh.h, src/render/renderer.h, src/render/shader.h, src/render/texture.h

## `render/camera.h`
```cpp
namespace aether {
```

### `class Camera`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `getViewMatrix` | `Mat4` | `` |
| `getProjectionMatrix` | `Mat4` | `` |

## `render/mesh.h`
```cpp
namespace aether {
```

### `struct Vertex`
| Member | Type |
|--------|------|
| `position` | `Vec3` |
| `texCoord` | `Vec2` |
| `color` | `uint32_t` |

### `class Mesh`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `addVertex` | `void` | `const Vertex& v` |
| `addTriangle` | `void` | `uint32_t a, uint32_t b, uint32_t c` |
| `clear` | `void` | `` |
| `isEmpty` | `bool` | `` |

## `render/renderer.h`
```cpp
namespace aether {
```

### `class Renderer`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `init` | `void` | `` |
| `shutdown` | `void` | `` |
| `beginFrame` | `void` | `` |
| `endFrame` | `void` | `` |
| `renderMesh` | `void` | `const Mesh& mesh, const Mat4& transform` |
| `setCamera` | `void` | `const Camera& camera` |

## `render/shader.h`
```cpp
namespace aether {
```

### `class Shader`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `load` | `bool` | `const std::string& vertexSource, const std::string& fragmentSource` |
| `bind` | `void` | `` |
| `setUniform` | `void` | `const std::string& name, float value` |

## `render/texture.h`
```cpp
namespace aether {
```

### `class Texture2D`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `loadFromFile` | `bool` | `const std::string& path` |
| `bind` | `void` | `uint32_t slot = 0` |
