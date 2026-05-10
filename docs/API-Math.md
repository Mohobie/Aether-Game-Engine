# Math Module API

**File:** `math/vec3.h`

```cpp
namespace vge {
```

## `struct Vec3`
3D vector with x, y, z components.

### Members
| Member | Type | Description |
|--------|------|-------------|
| `x` | `float` | X component |
| `y` | `float` | Y component |
| `z` | `float` | Z component |

### Constructors
| Constructor | Parameters | Description |
|-------------|------------|-------------|
| `Vec3()` | `void` | Default (0,0,0) |
| `Vec3(float x, float y, float z)` | x, y, z values | Initialize with values |
| `Vec3(float v)` | single value | All components = v |

### Methods
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `length` | `float` | `void` | Vector magnitude |
| `lengthSquared` | `float` | `void` | Squared magnitude |
| `normalized` | `Vec3` | `void` | Unit vector |
| `dot` | `float` | `const Vec3& other` | Dot product |
| `cross` | `Vec3` | `const Vec3& other` | Cross product |
| `distance` | `float` | `const Vec3& other` | Distance to other |
| `lerp` | `Vec3` | `const Vec3& other, float t` | Linear interpolation |
| `operator+` | `Vec3` | `const Vec3& other` | Addition |
| `operator-` | `Vec3` | `const Vec3& other` | Subtraction |
| `operator*` | `Vec3` | `float scalar` | Scalar multiply |
| `operator/` | `Vec3` | `float scalar` | Scalar divide |
| `operator+=` | `Vec3&` | `const Vec3& other` | Add assign |
| `operator-=` | `Vec3&` | `const Vec3& other` | Sub assign |
| `operator*=` | `Vec3&` | `float scalar` | Mul assign |
| `operator/=` | `Vec3&` | `float scalar` | Div assign |
| `operator==` | `bool` | `const Vec3& other` | Equality |
| `operator!=` | `bool` | `const Vec3& other` | Inequality |
| `operator[]` | `float&` | `int index` | Index access |

### Static Methods
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `Vec3::Up` | `Vec3` | `void` | (0, 1, 0) |
| `Vec3::Right` | `Vec3` | `void` | (1, 0, 0) |
| `Vec3::Forward` | `Vec3` | `void` | (0, 0, 1) |
| `Vec3::Zero` | `Vec3` | `void` | (0, 0, 0) |
| `Vec3::One` | `Vec3` | `void` | (1, 1, 1) |

---

**File:** `math/mat4.h`

## `struct Mat4`
4x4 matrix for transformations.

### Constructors
| Constructor | Parameters | Description |
|-------------|------------|-------------|
| `Mat4()` | `void` | Identity matrix |
| `Mat4(float diagonal)` | diagonal value | Diagonal matrix |

### Methods
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `Identity` | `Mat4` | `void` | Static identity |
| `Translate` | `Mat4` | `const Vec3& translation` | Translation matrix |
| `RotateX` | `Mat4` | `float angle` | X rotation (radians) |
| `RotateY` | `Mat4` | `float angle` | Y rotation (radians) |
| `RotateZ` | `Mat4` | `float angle` | Z rotation (radians) |
| `Rotate` | `Mat4` | `float angle, const Vec3& axis` | Axis rotation |
| `Scale` | `Mat4` | `const Vec3& scale` | Scale matrix |
| `Perspective` | `Mat4` | `float fov, float aspect, float near, float far` | Perspective projection |
| `Orthographic` | `Mat4` | `float left, float right, float bottom, float top, float near, float far` | Orthographic projection |
| `LookAt` | `Mat4` | `const Vec3& eye, const Vec3& center, const Vec3& up` | View matrix |
| `Transpose` | `Mat4` | `void` | Transpose |
| `Inverse` | `Mat4` | `void` | Inverse |
| `operator*` | `Mat4` | `const Mat4& other` | Matrix multiply |
| `operator*` | `Vec3` | `const Vec3& vec` | Transform vector |
| `operator[]` | `float*` | `int row` | Row access |
| `GetData` | `const float*` | `void` | Raw data (16 floats) |

---

**File:** `math/noise.h`

## `class Noise`
Perlin/Simplex noise generation.

| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `Noise` | (ctor) | `uint32_t seed = 0` | Constructor with seed |
| `Seed` | `void` | `uint32_t seed` | Set seed |
| `Perlin2D` | `float` | `float x, float y` | 2D Perlin noise |
| `Perlin3D` | `float` | `float x, float y, float z` | 3D Perlin noise |
| `Simplex2D` | `float` | `float x, float y` | 2D Simplex noise |
| `Simplex3D` | `float` | `float x, float y, float z` | 3D Simplex noise |
| `FBM2D` | `float` | `float x, float y, int octaves, float persistence` | Fractal Brownian Motion |
| `FBM3D` | `float` | `float x, float y, float z, int octaves, float persistence` | 3D FBM |
| `Ridged2D` | `float` | `float x, float y, int octaves` | Ridged multifractal |
| `Turbulence2D` | `float` | `float x, float y, int octaves` | Turbulence |
