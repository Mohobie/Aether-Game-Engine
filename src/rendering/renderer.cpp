#include "rendering/renderer.h"
#include "rendering/shader.h"
#include "rendering/camera.h"
#include "rendering/mesh.h"
#include "rendering/sky/sky_renderer.h"
#include "rendering/weather/weather_effects_renderer.h"
#include "rendering/sky/day_night_cycle.h"
#include "core/weather_system.h"
#include "voxel/world.h"
#include "voxel/chunk.h"
#include "voxel/block.h"
#include "voxel/block_registry.h"
#include "voxel/world_renderer.h"
#include "core/logger.h"
#include <GL/gl.h>
#include <iostream>
#include <cstring>
#include <cmath>
#include <algorithm>

namespace vge {

// Cube vertices (position + color)
// Each face: 4 vertices, 6 indices (2 triangles)
static const float cubeVertices[] = {
    // Front face (z+)
    -0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,
    
    // Back face (z-)
    -0.5f, -0.5f, -0.5f,  0.8f, 0.8f, 0.8f,
    -0.5f,  0.5f, -0.5f,  0.8f, 0.8f, 0.8f,
     0.5f,  0.5f, -0.5f,  0.8f, 0.8f, 0.8f,
     0.5f, -0.5f, -0.5f,  0.8f, 0.8f, 0.8f,
    
    // Top face (y+)
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,
    
    // Bottom face (y-)
    -0.5f, -0.5f, -0.5f,  0.5f, 0.5f, 0.5f,
     0.5f, -0.5f, -0.5f,  0.5f, 0.5f, 0.5f,
     0.5f, -0.5f,  0.5f,  0.5f, 0.5f, 0.5f,
    -0.5f, -0.5f,  0.5f,  0.5f, 0.5f, 0.5f,
    
    // Right face (x+)
     0.5f, -0.5f, -0.5f,  0.9f, 0.9f, 0.9f,
     0.5f,  0.5f, -0.5f,  0.9f, 0.9f, 0.9f,
     0.5f,  0.5f,  0.5f,  0.9f, 0.9f, 0.9f,
     0.5f, -0.5f,  0.5f,  0.9f, 0.9f, 0.9f,
    
    // Left face (x-)
    -0.5f, -0.5f, -0.5f,  0.9f, 0.9f, 0.9f,
    -0.5f, -0.5f,  0.5f,  0.9f, 0.9f, 0.9f,
    -0.5f,  0.5f,  0.5f,  0.9f, 0.9f, 0.9f,
    -0.5f,  0.5f, -0.5f,  0.9f, 0.9f, 0.9f,
};

static const uint32_t cubeIndices[] = {
    // Front
    0, 1, 2,  0, 2, 3,
    // Back
    4, 5, 6,  4, 6, 7,
    // Top
    8, 9, 10,  8, 10, 11,
    // Bottom
    12, 13, 14,  12, 14, 15,
    // Right
    16, 17, 18,  16, 18, 19,
    // Left
    20, 21, 22,  20, 22, 23
};

static const float crosshairVertices[] = {
    // Horizontal line
    -0.02f,  0.0f, 0.0f,  1.0f, 1.0f, 1.0f,
     0.02f,  0.0f, 0.0f,  1.0f, 1.0f, 1.0f,
    // Vertical line
     0.0f, -0.02f, 0.0f,  1.0f, 1.0f, 1.0f,
     0.0f,  0.02f, 0.0f,  1.0f, 1.0f, 1.0f,
};

static const float highlightVertices[] = {
    // Wireframe cube edges
    -0.51f, -0.51f, -0.51f,  1.0f, 1.0f, 0.0f,
     0.51f, -0.51f, -0.51f,  1.0f, 1.0f, 0.0f,
     0.51f,  0.51f, -0.51f,  1.0f, 1.0f, 0.0f,
    -0.51f,  0.51f, -0.51f,  1.0f, 1.0f, 0.0f,
    -0.51f, -0.51f,  0.51f,  1.0f, 1.0f, 0.0f,
     0.51f, -0.51f,  0.51f,  1.0f, 1.0f, 0.0f,
     0.51f,  0.51f,  0.51f,  1.0f, 1.0f, 0.0f,
    -0.51f,  0.51f,  0.51f,  1.0f, 1.0f, 0.0f,
};

static const uint32_t highlightIndices[] = {
    // Bottom face
    0, 1,  1, 2,  2, 3,  3, 0,
    // Top face
    4, 5,  5, 6,  6, 7,  7, 4,
    // Vertical edges
    0, 4,  1, 5,  2, 6,  3, 7
};

Renderer::Renderer() : initialized(false), width(1280), height(720), 
                       cubeVAO(0), cubeVBO(0), crosshairVAO(0), crosshairVBO(0),
                       highlightVAO(0), highlightVBO(0),
                       fb_renderer(nullptr), sky_renderer(nullptr), 
                       weather_renderer(nullptr), day_night_cycle(nullptr), 
                       weather_system(nullptr), world_renderer(nullptr), 
                       world_shader(nullptr) {}

Renderer::~Renderer() {
    if (initialized) Shutdown();
}

void Renderer::InitializeCubeBuffers() {
    // Create VAO, VBO, EBO for cube
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    
    uint32_t ebo;
    glGenBuffers(1, &ebo);
    
    glBindVertexArray(cubeVAO);
    
    // VBO
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
    
    // EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);
    
    // Position attribute (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Color attribute (location = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindVertexArray(0);
}

void Renderer::InitializeCrosshairBuffers() {
    glGenVertexArrays(1, &crosshairVAO);
    glGenBuffers(1, &crosshairVBO);
    
    glBindVertexArray(crosshairVAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, crosshairVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(crosshairVertices), crosshairVertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindVertexArray(0);
}

void Renderer::InitializeHighlightBuffers() {
    glGenVertexArrays(1, &highlightVAO);
    glGenBuffers(1, &highlightVBO);
    
    uint32_t ebo;
    glGenBuffers(1, &ebo);
    
    glBindVertexArray(highlightVAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, highlightVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(highlightVertices), highlightVertices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(highlightIndices), highlightIndices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindVertexArray(0);
}

void Renderer::CleanupBuffers() {
    if (cubeVAO) glDeleteVertexArrays(1, &cubeVAO);
    if (cubeVBO) glDeleteBuffers(1, &cubeVBO);
    if (crosshairVAO) glDeleteVertexArrays(1, &crosshairVAO);
    if (crosshairVBO) glDeleteBuffers(1, &crosshairVBO);
    if (highlightVAO) glDeleteVertexArrays(1, &highlightVAO);
    if (highlightVBO) glDeleteBuffers(1, &highlightVBO);
    
    cubeVAO = cubeVBO = crosshairVAO = crosshairVBO = highlightVAO = highlightVBO = 0;
}

bool Renderer::Initialize() {
    // Initialize OpenGL state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    
    // Initialize buffers
    InitializeCubeBuffers();
    InitializeCrosshairBuffers();
    InitializeHighlightBuffers();
    
    // Initialize world renderer
    world_renderer = new WorldRenderer();
    world_shader = new Shader();
    world_shader->LoadFromSource(Shader::GetUnlitVertexShader(), 
                                  Shader::GetUnlitFragmentShader());
    world_renderer->Initialize(world_shader);
    
    std::cout << "[Renderer] Modern OpenGL renderer initialized (VAOs/VBOs)\n";
    initialized = true;
    return true;
}

void Renderer::Shutdown() {
    CleanupBuffers();
    
    if (world_renderer) {
        delete world_renderer;
        world_renderer = nullptr;
    }
    if (world_shader) {
        delete world_shader;
        world_shader = nullptr;
    }
    initialized = false;
}

void Renderer::BeginFrame() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::EndFrame() {
    // Frame complete - swap buffers happens in window
}

void Renderer::SetClearColor(float r, float g, float b, float a) {
    glClearColor(r, g, b, a);
}

void Renderer::SetViewport(int x, int y, int w, int h) {
    width = w;
    height = h;
    glViewport(x, y, w, h);
}

uint32_t Renderer::GetBlockColor(BlockTypeID type) {
    if (type == BlockRegistry::GetInstance().GetBlockId("grass")) return 0xFF4CAF50;
    if (type == BlockRegistry::GetInstance().GetBlockId("dirt")) return 0xFF795548;
    if (type == BlockRegistry::GetInstance().GetBlockId("stone")) return 0xFF9E9E9E;
    if (type == BlockRegistry::GetInstance().GetBlockId("wood")) return 0xFF8D6E63;
    if (type == BlockRegistry::GetInstance().GetBlockId("leaves")) return 0xFF2E7D32;
    if (type == BlockRegistry::GetInstance().GetBlockId("sand")) return 0xFFFFF59D;
    if (type == BlockRegistry::GetInstance().GetBlockId("water")) return 0xFF2196F3;
    if (type == BlockRegistry::GetInstance().GetBlockId("bedrock")) return 0xFF424242;
    return 0xFFFFFFFF;
}

void Renderer::RenderWorldModern(const World& world, const Camera& camera) {
    // Use modern OpenGL with VAOs
    glBindVertexArray(cubeVAO);
    
    // Set up matrices
    Mat4 view = camera.GetViewMatrix();
    Mat4 proj = camera.GetProjectionMatrix();
    
    // Enable vertex attributes
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    
    // Render visible chunks
    for (int cx = -2; cx <= 2; cx++) {
        for (int cy = -1; cy <= 1; cy++) {
            for (int cz = -2; cz <= 2; cz++) {
                Chunk* chunk = const_cast<World&>(world).GetChunk(cx, cy, cz);
                if (!chunk || !chunk->loaded) continue;
                
                // Update mesh if needed
                if (chunk->IsDirty()) {
                    UpdateChunkMesh(chunk);
                    chunk->SetDirty(false);
                }
                
                // Render each visible block
                for (int x = 0; x < CHUNK_SIZE; x++) {
                    for (int y = 0; y < CHUNK_SIZE; y++) {
                        for (int z = 0; z < CHUNK_SIZE; z++) {
                            BlockTypeID block = chunk->GetBlock(x, y, z);
                            if (block == BlockRegistry::GetInstance().GetBlockId("air")) continue;
                            
                            // Check visibility
                            bool visible = false;
                            if (x == 0 || chunk->GetBlock(x-1, y, z) == BlockRegistry::GetInstance().GetBlockId("air")) visible = true;
                            if (x == CHUNK_SIZE-1 || chunk->GetBlock(x+1, y, z) == BlockRegistry::GetInstance().GetBlockId("air")) visible = true;
                            if (y == 0 || chunk->GetBlock(x, y-1, z) == BlockRegistry::GetInstance().GetBlockId("air")) visible = true;
                            if (y == CHUNK_SIZE-1 || chunk->GetBlock(x, y+1, z) == BlockRegistry::GetInstance().GetBlockId("air")) visible = true;
                            if (z == 0 || chunk->GetBlock(x, y, z-1) == BlockRegistry::GetInstance().GetBlockId("air")) visible = true;
                            if (z == CHUNK_SIZE-1 || chunk->GetBlock(x, y, z+1) == BlockRegistry::GetInstance().GetBlockId("air")) visible = true;
                            
                            if (!visible) continue;
                            
                            // World position
                            Vec3 worldPos(
                                cx * CHUNK_SIZE + x,
                                cy * CHUNK_SIZE + y,
                                cz * CHUNK_SIZE + z
                            );
                            
                            // Get color
                            uint32_t color = GetBlockColor(block);
                            float r = ((color >> 16) & 0xFF) / 255.0f;
                            float g = ((color >> 8) & 0xFF) / 255.0f;
                            float b = (color & 0xFF) / 255.0f;
                            
                            // Set color (we'll update the VBO color in a real implementation)
                            // For now, use immediate mode color
                            glColor3f(r, g, b);
                            
                            // Draw cube
                            glPushMatrix();
                            glTranslatef(worldPos.x + 0.5f, worldPos.y + 0.5f, worldPos.z + 0.5f);
                            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
                            glPopMatrix();
                        }
                    }
                }
            }
        }
    }
    
    glBindVertexArray(0);
}

void Renderer::RenderWorldASCII(const World& world, const Camera& camera) {
    // Fallback ASCII renderer for systems without OpenGL
    // (kept for compatibility)
    std::cout << "[Renderer] ASCII rendering not available in modern mode\n";
}

void Renderer::RenderWorldFB(const World& world, const Camera& camera) {
    // Framebuffer rendering (for post-processing)
    RenderWorldModern(world, camera);
}

void Renderer::RenderWorld(const World& world, const Camera& camera) {
    RenderWorldModern(world, camera);
}

void Renderer::RenderWorldMesh(const World& world, const Camera& camera) {
    if (world_renderer) {
        world_renderer->RenderWorld(world, camera);
    }
}

void Renderer::UpdateChunkMesh(const Chunk* chunk) {
    if (world_renderer && chunk) {
        world_renderer->UpdateChunkMesh(chunk);
    }
}

void Renderer::UpdateChunkMeshWithNeighbors(const Chunk* chunk,
                                             const Chunk* neighborXP, const Chunk* neighborXN,
                                             const Chunk* neighborYP, const Chunk* neighborYN,
                                             const Chunk* neighborZP, const Chunk* neighborZN) {
    if (world_renderer && chunk) {
        world_renderer->UpdateChunkMeshWithNeighbors(chunk,
                                                      neighborXP, neighborXN,
                                                      neighborYP, neighborYN,
                                                      neighborZP, neighborZN);
    }
}

void Renderer::RenderMesh(const Mesh& mesh, const Shader& shader, const Camera& camera) {
    // Modern mesh rendering
    if (!mesh.IsValid()) return;
    
    shader.Bind();
    
    // Set uniforms
    Mat4 view = camera.GetViewMatrix();
    Mat4 proj = camera.GetProjectionMatrix();
    shader.SetMat4("view", view.data);
    shader.SetMat4("projection", proj.data);
    
    // Draw
    mesh.Draw();
    
    shader.Unbind();
}

void Renderer::RenderSky(const Camera& camera) {
    if (sky_renderer && day_night_cycle) {
        // Modern sky rendering
        sky_renderer->RenderSky(*day_night_cycle, width, height);
    }
}

void Renderer::RenderWeatherEffects(const Camera& camera) {
    if (weather_renderer) {
        weather_renderer->Render(width, height);
    }
}

void Renderer::RenderCrosshair(int screenW, int screenH) {
    // Modern crosshair rendering with VAO
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, screenW, screenH, 0, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glBindVertexArray(crosshairVAO);
    glColor3f(1.0f, 1.0f, 1.0f);
    glLineWidth(2.0f);
    
    // Scale to screen
    int cx = screenW / 2;
    int cy = screenH / 2;
    glTranslatef(cx, cy, 0);
    glScalef(screenW, screenH, 1);
    
    glDrawArrays(GL_LINES, 0, 4);
    
    glBindVertexArray(0);
    
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void Renderer::RenderBlockHighlight(const Vec3& blockPos, const Camera& camera, int screenW, int screenH) {
    // Modern highlight rendering with VAO
    glBindVertexArray(highlightVAO);
    
    glPushMatrix();
    glTranslatef(blockPos.x + 0.5f, blockPos.y + 0.5f, blockPos.z + 0.5f);
    
    glColor3f(1.0f, 1.0f, 0.0f);
    glLineWidth(2.0f);
    
    glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
    
    glPopMatrix();
    
    glBindVertexArray(0);
}

} // namespace vge
