# Rendering System

The engine uses OpenGL for rendering with modern features like PBR, shadows, and post-processing.

## Renderer Setup

```cpp
#include "rendering/renderer.h"

// Create renderer
vge::Renderer renderer;
renderer.Initialize(1920, 1080);  // Window size

// Main render loop
while (running) {
    // Clear screen
    renderer.Clear(0.2f, 0.3f, 0.4f);  // RGB background color
    
    // Render world
    renderer.RenderWorld(world, camera);
    
    // Render UI
    renderer.RenderUI(uiSystem);
    
    // Swap buffers
    renderer.Present();
}
```

## Camera

```cpp
#include "rendering/camera.h"

// Create camera
vge::Camera camera;
camera.SetPosition(vge::Vec3(0, 10, 20));
camera.SetLookAt(vge::Vec3(0, 0, 0));
camera.SetFOV(60.0f);
camera.SetNearFar(0.1f, 1000.0f);

// Update aspect ratio on window resize
camera.SetAspectRatio(width / (float)height);

// Get view and projection matrices
vge::Mat4 view = camera.GetViewMatrix();
vge::Mat4 proj = camera.GetProjectionMatrix();

// Camera movement
void UpdateCamera(float dt) {
    // WASD movement
    vge::Vec3 forward = camera.GetForward();
    vge::Vec3 right = camera.GetRight();
    
    if (input.GetKey(Key::W)) camera.Move(forward * speed * dt);
    if (input.GetKey(Key::S)) camera.Move(-forward * speed * dt);
    if (input.GetKey(Key::A)) camera.Move(-right * speed * dt);
    if (input.GetKey(Key::D)) camera.Move(right * speed * dt);
    
    // Mouse look
    float mouseX = input.GetMouseDeltaX();
    float mouseY = input.GetMouseDeltaY();
    camera.Rotate(mouseX * sensitivity, -mouseY * sensitivity);
}
```

## Meshes

```cpp
#include "rendering/mesh.h"

// Create mesh from vertices
vge::Mesh mesh;

// Define vertices (position, normal, UV)
std::vector<Vertex> vertices = {
    // Front face
    {{-0.5f, -0.5f,  0.5f}, {0, 0, 1}, {0, 0}},
    {{ 0.5f, -0.5f,  0.5f}, {0, 0, 1}, {1, 0}},
    {{ 0.5f,  0.5f,  0.5f}, {0, 0, 1}, {1, 1}},
    {{-0.5f,  0.5f,  0.5f}, {0, 0, 1}, {0, 1}},
    // ... more vertices
};

// Define indices
std::vector<uint32_t> indices = {
    0, 1, 2, 2, 3, 0,  // Front face
    // ... more indices
};

mesh.SetVertices(vertices);
mesh.SetIndices(indices);
mesh.UploadToGPU();

// Render mesh
mesh.Bind();
mesh.Draw();
```

## Shaders

```cpp
#include "rendering/shader.h"

// Load shader from files
vge::Shader shader;
shader.LoadFromFiles("shaders/vertex.glsl", "shaders/fragment.glsl");

// Use shader
shader.Bind();

// Set uniforms
shader.SetMat4("uViewProj", viewProj);
shader.SetVec3("uCameraPos", camera.GetPosition());
shader.SetFloat("uTime", time);

// Set texture
shader.SetInt("uDiffuseMap", 0);  // Texture unit 0
texture.Bind(0);

// Draw
mesh.Draw();

// Unbind
shader.Unbind();
```

### Basic Vertex Shader
```glsl
#version 330 core
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 uModel;
uniform mat4 uViewProj;

out vec3 vWorldPos;
out vec3 vNormal;
out vec2 vTexCoord;

void main() {
    vec4 worldPos = uModel * vec4(aPosition, 1.0);
    vWorldPos = worldPos.xyz;
    vNormal = mat3(transpose(inverse(uModel))) * aNormal;
    vTexCoord = aTexCoord;
    gl_Position = uViewProj * worldPos;
}
```

### Basic Fragment Shader
```glsl
#version 330 core
in vec3 vWorldPos;
in vec3 vNormal;
in vec2 vTexCoord;

uniform vec3 uCameraPos;
uniform sampler2D uDiffuseMap;

out vec4 FragColor;

void main() {
    vec3 normal = normalize(vNormal);
    vec3 viewDir = normalize(uCameraPos - vWorldPos);
    
    // Simple diffuse lighting
    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
    float diff = max(dot(normal, lightDir), 0.0);
    
    vec3 color = texture(uDiffuseMap, vTexCoord).rgb;
    vec3 ambient = color * 0.3;
    vec3 diffuse = color * diff * 0.7;
    
    FragColor = vec4(ambient + diffuse, 1.0);
}
```

## Textures

```cpp
#include "rendering/texture.h"

// Load texture from file
vge::Texture texture;
texture.LoadFromFile("textures/stone.png");
texture.SetFilter(TextureFilter::Linear);
texture.SetWrap(TextureWrap::Repeat);

// Use texture
texture.Bind(0);  // Bind to texture unit 0

// Create texture from data
vge::Texture createTexture;
createTexture.CreateFromData(width, height, TextureFormat::RGBA, pixelData);
```

## Materials (PBR)

```cpp
#include "rendering/pbr_material.h"

// Create PBR material
vge::PBRMaterial material;
material.SetAlbedoTexture(albedoTexture);
material.SetNormalTexture(normalTexture);
material.SetRoughnessTexture(roughnessTexture);
material.SetMetallicTexture(metallicTexture);

// Set material properties
material.SetAlbedoColor(vge::Vec3(1.0f, 0.5f, 0.2f));
material.SetRoughness(0.5f);
material.SetMetallic(0.0f);
material.SetAO(1.0f);

// Apply to mesh
mesh.SetMaterial(material);
```

## Lighting

```cpp
#include "rendering/lighting.h"

// Create light manager
vge::LightManager lights;

// Directional light (sun/moon)
vge::DirectionalLight sun;
sun.SetDirection(vge::Vec3(0.5f, -1.0f, 0.3f));
sun.SetColor(vge::Vec3(1.0f, 0.95f, 0.8f));
sun.SetIntensity(1.0f);
sun.SetCastShadows(true);
lights.AddDirectionalLight(sun);

// Point light (torch, lamp)
vge::PointLight torch;
torch.SetPosition(vge::Vec3(10, 5, 10));
torch.SetColor(vge::Vec3(1.0f, 0.6f, 0.2f));
torch.SetIntensity(5.0f);
torch.SetRange(20.0f);
lights.AddPointLight(torch);

// Spot light (flashlight)
vge::SpotLight flashlight;
flashlight.SetPosition(camera.GetPosition());
flashlight.SetDirection(camera.GetForward());
flashlight.SetColor(vge::Vec3(1, 1, 1));
flashlight.SetIntensity(10.0f);
flashlight.SetRange(30.0f);
flashlight.SetSpotAngle(30.0f);
lights.AddSpotLight(flashlight);

// Update in loop
void Update(float dt) {
    // Update flashlight to follow camera
    flashlight.SetPosition(camera.GetPosition());
    flashlight.SetDirection(camera.GetForward());
}
```

## Shadows

```cpp
#include "rendering/shadow_system.h"

// Setup shadow system
vge::ShadowSystem shadows;
shadows.Initialize(2048);  // Shadow map resolution
shadows.SetShadowDistance(100.0f);
shadows.SetShadowBias(0.005f);

// In render loop
shadows.BeginShadowPass(sun);
// Render all shadow-casting objects
world.RenderShadows(shadows);
shadows.EndShadowPass();

// Main pass with shadows
shader.Bind();
shadows.BindShadowMap(3);  // Bind to texture unit 3
shader.SetMat4("uLightSpaceMatrix", shadows.GetLightSpaceMatrix());
```

## Post-Processing

```cpp
#include "rendering/post_processing.h"

// Setup post-processing
vge::PostProcessing postProcess;
postProcess.Initialize(width, height);

// Add effects
postProcess.AddEffect(PostEffect::ToneMapping);
postProcess.AddEffect(PostEffect::FXAA);
postProcess.AddEffect(PostEffect::Bloom);

// In render loop
// 1. Render scene to framebuffer
postProcess.BeginScenePass();
renderer.RenderWorld(world, camera);
postProcess.EndScenePass();

// 2. Apply post-processing
postProcess.Process();

// 3. Render to screen
postProcess.RenderToScreen();
```

## Day/Night Cycle

```cpp
#include "rendering/sky/day_night_cycle.h"

// Create day/night cycle
vge::DayNightCycle cycle;
cycle.SetDayLength(600.0f);  // 10 minutes per day
cycle.SetTime(0.25f);        // Start at sunrise (0-1)

// Update in loop
void Update(float dt) {
    cycle.Update(dt);
    
    // Get sun properties
    vge::Vec3 sunDir = cycle.GetSunDirection();
    vge::Vec3 sunColor = cycle.GetSunColor();
    float sunIntensity = cycle.GetSunIntensity();
    
    // Apply to directional light
    sun.SetDirection(sunDir);
    sun.SetColor(sunColor);
    sun.SetIntensity(sunIntensity);
    
    // Get sky color
    vge::Vec3 skyColor = cycle.GetSkyColor();
    renderer.SetClearColor(skyColor);
}
```

## Particle System

```cpp
#include "rendering/particles/particle_system.h"

// Create particle system
vge::ParticleSystem particles;
particles.SetMaxParticles(1000);

// Define emitter
vge::ParticleEmitter emitter;
emitter.SetPosition(vge::Vec3(0, 10, 0));
emitter.SetEmissionRate(50.0f);  // particles per second
emitter.SetLifetime(2.0f, 3.0f);
emitter.SetStartSize(0.5f, 1.0f);
emitter.SetEndSize(0.0f);
emitter.SetStartColor(vge::Vec4(1, 0.5, 0, 1));
emitter.SetEndColor(vge::Vec4(1, 0, 0, 0));
emitter.SetVelocity(vge::Vec3(0, 2, 0), vge::Vec3(1, 0.5f, 1));

particles.AddEmitter(emitter);

// Update and render
particles.Update(dt);
particles.Render(camera);
```

## Culling and LOD

```cpp
#include "rendering/culling_system.h"

// Setup culling
vge::CullingSystem culling;
culling.SetCamera(camera.GetPosition(), camera.GetViewProj());
culling.SetViewDistance(500.0f);

// Setup LOD levels
culling.setupChunkLODs(32);  // 32-block chunk size

// Update chunk BVH
std::vector<ChunkBVH::BuildPrimitive> chunks;
// ... fill chunks from world
culling.updateChunkBVH(chunks);

// Cull chunks
CullingResult result = culling.cullChunks();

// Render only visible chunks
for (uint32_t chunkId : result.visibleChunks) {
    int lod = result.chunkLOD[chunkId];
    RenderChunk(chunkId, lod);
}
```
