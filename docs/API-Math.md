# Math Module API
**Files:** src/math/mat4.h, src/math/noise.h, src/math/vec3.h

## `math/mat4.h`
```cpp
namespace vge {
```

### `struct Mat4`

## `math/noise.h`
```cpp
namespace aether {
```

### `class PerlinNoise`
| Method | Return Type | Parameters |
|--------|-------------|------------|
| `noise` | `float` | `float x, float y, float z` |
| `octaveNoise` | `float` | `float x, float y, float z, int octaves` |
| `fade` | `float` | `float t` |
| `lerp` | `float` | `float t, float a, float b` |
| `grad` | `float` | `int hash, float x, float y, float z` |

## `math/vec3.h`
```cpp
namespace vge {
```

### `struct Vec3`
