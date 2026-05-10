# Rendering Module API

**Files:** `rendering/camera.h`, `rendering/mesh.h`, `rendering/shader.h`, `rendering/texture.h`, `rendering/renderer.h`, `rendering/framebuffer.h`, `rendering/material.h`, `rendering/light.h`, `rendering/skybox.h`

---

## `rendering/camera.h`

```cpp
namespace vge {
```

### `class Camera`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `Camera` | (ctor) | `void` | Constructor |
| `SetPosition` | `void` | `const Vec3& pos` | Set position |
| `GetPosition` | `Vec3` | `void` | Get position |
| `SetRotation` | `void` | `const Vec3& rot` | Set rotation (pitch, yaw, roll) |
| `GetRotation` | `Vec3` | `void` | Get rotation |
| `LookAt` | `void` | `const Vec3& target` | Look at target |
| `SetFOV` | `void` | `float fov` | Set field of view |
| `GetFOV` | `float` | `void` | Get FOV |
| `SetNearPlane` | `void` | `float near` | Set near plane |
| `SetFarPlane` | `void` | `float far` | Set far plane |
| `GetNearPlane` | `float` | `void` | Get near plane |
| `GetFarPlane` | `float` | `void` | Get far plane |
| `SetAspectRatio` | `void` | `float aspect` | Set aspect ratio |
| `GetAspectRatio` | `float` | `void` | Get aspect ratio |
| `GetViewMatrix` | `Mat4` | `void` | Get view matrix |
| `GetProjectionMatrix` | `Mat4` | `void` | Get projection matrix |
| `GetViewProjection` | `Mat4` | `void` | Get VP matrix |
| `MoveForward` | `void` | `float distance` | Move forward |
| `MoveRight` | `void` | `float distance` | Move right |
| `MoveUp` | `void` | `float distance` | Move up |
| `Rotate` | `void` | `float pitch, float yaw` | Rotate |
| `GetForward` | `Vec3` | `void` | Forward vector |
| `GetRight` | `Vec3` | `void` | Right vector |
| `GetUp` | `Vec3` | `void` | Up vector |
| `SetOrthographic` | `void` | `bool ortho` | Toggle orthographic |
| `IsOrthographic` | `bool` | `void` | Check orthographic |

---

## `rendering/mesh.h`

### `struct Vertex`
| Member | Type | Description |
|--------|------|-------------|
| `position` | `Vec3` | Vertex position |
| `normal` | `Vec3` | Vertex normal |
| `texCoord` | `Vec2` | Texture coordinates |
| `color` | `Vec3` | Vertex color |
| `tangent` | `Vec3` | Tangent vector |
| `bitangent` | `Vec3` | Bitangent vector |

### `struct MeshData`
| Member | Type | Description |
|--------|------|-------------|
| `vertices` | `std::vector<Vertex>` | Vertex data |
| `indices` | `std::vector<uint32_t>` | Index data |

### `class Mesh`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `Mesh` | (ctor) | `void` | Constructor |
| `Load` | `bool` | `const MeshData& data` | Load mesh data |
| `LoadFromFile` | `bool` | `const std::string& path` | Load from file |
| `Bind` | `void` | `void` | Bind for rendering |
| `Unbind` | `void` | `void` | Unbind |
| `Draw` | `void` | `void` | Draw mesh |
| `GetVertexCount` | `size_t` | `void` | Vertex count |
| `GetIndexCount` | `size_t` | `void` | Index count |
| `SetMaterial` | `void` | `Material* material` | Set material |
| `GetMaterial` | `Material*` | `void` | Get material |
| `Clear` | `void` | `void` | Clear data |
| `IsLoaded` | `bool` | `void` | Check loaded |

---

## `rendering/shader.h`

### `class Shader`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `Shader` | (ctor) | `void` | Constructor |
| `LoadFromFile` | `bool` | `const std::string& vertPath, const std::string& fragPath` | Load from files |
| `LoadFromSource` | `bool` | `const std::string& vertSrc, const std::string& fragSrc` | Load from strings |
| `Bind` | `void` | `void` | Bind shader |
| `Unbind` | `void` | `void` | Unbind |
| `SetInt` | `void` | `const std::string& name, int value` | Set uniform int |
| `SetFloat` | `void` | `const std::string& name, float value` | Set uniform float |
| `SetVec2` | `void` | `const std::string& name, const Vec2& value` | Set uniform vec2 |
| `SetVec3` | `void` | `const std::string& name, const Vec3& value` | Set uniform vec3 |
| `SetVec4` | `void` | `const std::string& name, const Vec4& value` | Set uniform vec4 |
| `SetMat4` | `void` | `const std::string& name, const Mat4& value` | Set uniform mat4 |
| `SetBool` | `void` | `const std::string& name, bool value` | Set uniform bool |
| `GetUniformLocation` | `int` | `const std::string& name` | Get uniform location |
| `IsLoaded` | `bool` | `void` | Check loaded |
| `GetError` | `std::string` | `void` | Get compile error |

---

## `rendering/texture.h`

### `enum class TextureFormat`
| Value | Description |
|-------|-------------|
| `RGB` | 3 channels |
| `RGBA` | 4 channels |
| `Depth` | Depth only |
| `DepthStencil` | Depth + stencil |
| `R` | Single channel |
| `RG` | 2 channels |

### `enum class TextureFilter`
| Value | Description |
|-------|-------------|
| `Nearest` | Nearest neighbor |
| `Linear` | Linear interpolation |
| `NearestMipmapNearest` | Nearest with nearest mipmaps |
| `LinearMipmapNearest` | Linear with nearest mipmaps |
| `NearestMipmapLinear` | Nearest with linear mipmaps |
| `LinearMipmapLinear` | Trilinear |

### `enum class TextureWrap`
| Value | Description |
|-------|-------------|
| `Repeat` | Repeat texture |
| `ClampToEdge` | Clamp to edge |
| `ClampToBorder` | Clamp to border |
| `MirroredRepeat` | Mirrored repeat |

### `class Texture`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `Texture` | (ctor) | `void` | Constructor |
| `LoadFromFile` | `bool` | `const std::string& path, TextureFormat format = TextureFormat::RGBA` | Load from file |
| `LoadFromData` | `bool` | `const void* data, int width, int height, TextureFormat format` | Load from raw data |
| `Bind` | `void` | `uint32_t slot = 0` | Bind to slot |
| `Unbind` | `void` | `void` | Unbind |
| `SetFilter` | `void` | `TextureFilter min, TextureFilter mag` | Set filter |
| `SetWrap` | `void` | `TextureWrap s, TextureWrap t` | Set wrap mode |
| `GenerateMipmap` | `void` | `void` | Generate mipmaps |
| `GetWidth` | `int` | `void` | Get width |
| `GetHeight` | `int` | `void` | Get height |
| `GetFormat` | `TextureFormat` | `void` | Get format |
| `IsLoaded` | `bool` | `void` | Check loaded |
| `GetID` | `uint32_t` | `void` | Get OpenGL ID |

---

## `rendering/renderer.h`

### `class Renderer`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `GetInstance` | `Renderer&` | `void` | Singleton |
| `Initialize` | `bool` | `void` | Initialize |
| `Shutdown` | `void` | `void` | Shutdown |
| `BeginFrame` | `void` | `void` | Start frame |
| `EndFrame` | `void` | `void` | End frame |
| `Clear` | `void` | `const Vec3& color = Vec3(0,0,0)` | Clear screen |
| `SetViewport` | `void` | `int x, int y, int width, int height` | Set viewport |
| `GetViewport` | `std::tuple<int,int,int,int>` | `void` | Get viewport |
| `SetDepthTest` | `void` | `bool enabled` | Toggle depth test |
| `SetBlendMode` | `void` | `bool enabled` | Toggle blending |
| `SetCullFace` | `void` | `bool enabled` | Toggle culling |
| `SetWireframe` | `void` | `bool enabled` | Toggle wireframe |
| `RenderMesh` | `void` | `const Mesh& mesh, const Mat4& transform` | Render mesh |
| `RenderQuad` | `void` | `void` | Render fullscreen quad |
| `RenderCube` | `void` | `void` | Render cube |
| `SetShader` | `void` | `Shader* shader` | Set active shader |
| `GetShader` | `Shader*` | `void` | Get active shader |
| `SetCamera` | `void` | `const Camera& camera` | Set camera |
| `GetCamera` | `const Camera*` | `void` | Get camera |
| `GetFrameTime` | `float` | `void` | Get frame time |
| `GetFPS` | `float` | `void` | Get FPS |
| `GetDrawCalls` | `uint32_t` | `void` | Get draw calls |
| `GetTriangleCount` | `uint32_t` | `void` | Get triangle count |
| `EnableVSync` | `void` | `bool enabled` | Toggle VSync |
| `IsVSyncEnabled` | `bool` | `void` | Check VSync |

---

## `rendering/framebuffer.h`

### `class Framebuffer`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `Framebuffer` | (ctor) | `void` | Constructor |
| `Create` | `bool` | `int width, int height` | Create framebuffer |
| `Destroy` | `void` | `void` | Destroy |
| `Bind` | `void` | `void` | Bind for rendering |
| `Unbind` | `void` | `void` | Unbind |
| `GetColorTexture` | `Texture*` | `void` | Get color attachment |
| `GetDepthTexture` | `Texture*` | `void` | Get depth attachment |
| `Resize` | `void` | `int width, int height` | Resize |
| `GetWidth` | `int` | `void` | Get width |
| `GetHeight` | `int` | `void` | Get height |
| `IsValid` | `bool` | `void` | Check valid |
| `BlitToScreen` | `void` | `void` | Copy to screen |

---

## `rendering/material.h`

### `struct MaterialProperties`
| Member | Type | Description |
|--------|------|-------------|
| `diffuseColor` | `Vec3` | Diffuse color |
| `specularColor` | `Vec3` | Specular color |
| `ambientColor` | `Vec3` | Ambient color |
| `emissiveColor` | `Vec3` | Emissive color |
| `shininess` | `float` | Specular shininess |
| `opacity` | `float` | Opacity (0-1) |
| `metallic` | `float` | Metallic factor |
| `roughness` | `float` | Roughness factor |

### `class Material`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `Material` | (ctor) | `void` | Constructor |
| `SetDiffuseTexture` | `void` | `Texture* texture` | Set diffuse map |
| `GetDiffuseTexture` | `Texture*` | `void` | Get diffuse map |
| `SetNormalTexture` | `void` | `Texture* texture` | Set normal map |
| `GetNormalTexture` | `Texture*` | `void` | Get normal map |
| `SetSpecularTexture` | `void` | `Texture* texture` | Set specular map |
| `GetSpecularTexture` | `Texture*` | `void` | Get specular map |
| `SetProperties` | `void` | `const MaterialProperties& props` | Set properties |
| `GetProperties` | `MaterialProperties` | `void` | Get properties |
| `Bind` | `void` | `Shader* shader` | Bind to shader |
| `Unbind` | `void` | `void` | Unbind |
| `Clone` | `Material*` | `void` | Create copy |

---

## `rendering/light.h`

### `enum class LightType`
| Value | Description |
|-------|-------------|
| `Directional` | Sun/moon light |
| `Point` | Omnidirectional |
| `Spot` | Cone light |
| `Ambient` | Global ambient |

### `struct Light`
| Member | Type | Description |
|--------|------|-------------|
| `type` | `LightType` | Light type |
| `position` | `Vec3` | Position (point/spot) |
| `direction` | `Vec3` | Direction (directional/spot) |
| `color` | `Vec3` | Light color |
| `intensity` | `float` | Brightness |
| `range` | `float` | Attenuation range |
| `spotAngle` | `float` | Spot cone angle |
| `spotSoftness` | `float` | Spot edge softness |
| `castShadows` | `bool` | Casts shadows |
| `shadowMapSize` | `int` | Shadow resolution |

### `class LightManager`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `GetInstance` | `LightManager&` | `void` | Singleton |
| `AddLight` | `void` | `const Light& light` | Add light |
| `RemoveLight` | `void` | `size_t index` | Remove light |
| `GetLight` | `Light*` | `size_t index` | Get light |
| `GetLightCount` | `size_t` | `void` | Count |
| `SetAmbientLight` | `void` | `const Vec3& color, float intensity` | Set ambient |
| `GetAmbientLight` | `Vec3` | `void` | Get ambient color |
| `GetAmbientIntensity` | `float` | `void` | Get ambient intensity |
| `Clear` | `void` | `void` | Remove all |
| `UpdateShader` | `void` | `Shader* shader` | Upload to shader |

---

## `rendering/skybox.h`

### `class Skybox`
| Method | Return Type | Parameters | Description |
|--------|-------------|------------|-------------|
| `Skybox` | (ctor) | `void` | Constructor |
| `Load` | `bool` | `const std::string& right, const std::string& left, const std::string& top, const std::string& bottom, const std::string& front, const std::string& back` | Load cubemap |
| `LoadEquirectangular` | `bool` | `const std::string& path` | Load equirectangular |
| `Render` | `void` | `const Camera& camera` | Render skybox |
| `Bind` | `void` | `uint32_t slot` | Bind cubemap |
| `SetRotation` | `void` | `const Vec3& rotation` | Rotate skybox |
| `GetRotation` | `Vec3` | `void` | Get rotation |
| `SetTint` | `void` | `const Vec3& tint` | Set color tint |
| `GetTint` | `Vec3` | `void` | Get tint |
| `IsLoaded` | `bool` | `void` | Check loaded |
