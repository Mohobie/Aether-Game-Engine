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

Renderer::Renderer() : initialized(false), width(1280), height(720), 
                       fb_renderer(nullptr), sky_renderer(nullptr), 
                       weather_renderer(nullptr), day_night_cycle(nullptr), 
                       weather_system(nullptr), world_renderer(nullptr), 
                       world_shader(nullptr) {}

Renderer::~Renderer() {
    if (initialized) Shutdown();
}

bool Renderer::Initialize() {
    // Initialize OpenGL
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    
    // Initialize world renderer
    world_renderer = new WorldRenderer();
    world_shader = new Shader();
    world_shader->LoadFromSource(Shader::GetUnlitVertexShader(), 
                                  Shader::GetUnlitFragmentShader());
    world_renderer->Initialize(world_shader);
    
    std::cout << "[Renderer] OpenGL renderer initialized\n";
    initialized = true;
    return true;
}

void Renderer::Shutdown() {
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

// Simple projection: world -> screen
Vec2 Project(const Vec3& worldPos, const Camera& camera, int screenW, int screenH) {
    Vec3 relative = worldPos - camera.GetPosition();
    
    // Simple perspective projection
    float fov = 70.0f * 3.14159f / 180.0f;
    float tanHalfFov = std::tan(fov / 2.0f);
    
    float aspect = (float)screenW / (float)screenH;
    
    // Rotate by camera rotation (simplified - just yaw)
    float yaw = camera.GetRotation().x * 3.14159f / 180.0f;
    float cosY = std::cos(yaw);
    float sinY = std::sin(yaw);
    
    float rx = relative.x * cosY - relative.z * sinY;
    float rz = relative.x * sinY + relative.z * cosY;
    float ry = relative.y;
    
    // Project
    if (rz <= 0.1f) rz = 0.1f; // Prevent division by zero
    
    float screenX = (rx / (rz * tanHalfFov * aspect)) * screenW / 2 + screenW / 2;
    float screenY = (ry / (rz * tanHalfFov)) * screenH / 2 + screenH / 2;
    
    return Vec2(screenX, screenY);
}

uint32_t Renderer::GetBlockColor(BlockTypeID type) {
    // Return color for block type
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

void Renderer::RenderWorld(const World& world, const Camera& camera) {
    // Use OpenGL to render the world
    // For now, render as colored cubes
    
    // Set up matrices
    Mat4 view = camera.GetViewMatrix();
    Mat4 proj = camera.GetProjectionMatrix();
    
    // Simple colored cube rendering
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
                            
                            // Draw colored cube
                            uint32_t color = GetBlockColor(block);
                            float r = ((color >> 16) & 0xFF) / 255.0f;
                            float g = ((color >> 8) & 0xFF) / 255.0f;
                            float b = (color & 0xFF) / 255.0f;
                            
                            glPushMatrix();
                            glTranslatef(worldPos.x + 0.5f, worldPos.y + 0.5f, worldPos.z + 0.5f);
                            glScalef(0.5f, 0.5f, 0.5f);
                            
                            glColor3f(r, g, b);
                            
                            // Draw cube faces
                            glBegin(GL_QUADS);
                            
                            // Front face
                            glVertex3f(-1, -1, 1);
                            glVertex3f(1, -1, 1);
                            glVertex3f(1, 1, 1);
                            glVertex3f(-1, 1, 1);
                            
                            // Back face
                            glVertex3f(-1, -1, -1);
                            glVertex3f(-1, 1, -1);
                            glVertex3f(1, 1, -1);
                            glVertex3f(1, -1, -1);
                            
                            // Top face
                            glVertex3f(-1, 1, -1);
                            glVertex3f(-1, 1, 1);
                            glVertex3f(1, 1, 1);
                            glVertex3f(1, 1, -1);
                            
                            // Bottom face
                            glVertex3f(-1, -1, -1);
                            glVertex3f(1, -1, -1);
                            glVertex3f(1, -1, 1);
                            glVertex3f(-1, -1, 1);
                            
                            // Right face
                            glVertex3f(1, -1, -1);
                            glVertex3f(1, 1, -1);
                            glVertex3f(1, 1, 1);
                            glVertex3f(1, -1, 1);
                            
                            // Left face
                            glVertex3f(-1, -1, -1);
                            glVertex3f(-1, -1, 1);
                            glVertex3f(-1, 1, 1);
                            glVertex3f(-1, 1, -1);
                            
                            glEnd();
                            
                            glPopMatrix();
                        }
                    }
                }
            }
        }
    }
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
    // Not implemented for basic OpenGL renderer
}

void Renderer::RenderSky(const Camera& camera) {
    if (sky_renderer && day_night_cycle) {
        sky_renderer->RenderSkyASCII(*day_night_cycle, width, height);
    }
}

void Renderer::RenderWeatherEffects(const Camera& camera) {
    if (weather_renderer) {
        weather_renderer->RenderASCII(width, height);
    }
}

void Renderer::RenderCrosshair(int screenW, int screenH) {
    // Draw crosshair at screen center
    int cx = screenW / 2;
    int cy = screenH / 2;
    
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, screenW, screenH, 0, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glColor3f(1.0f, 1.0f, 1.0f);
    glLineWidth(2.0f);
    
    glBegin(GL_LINES);
    glVertex2f(cx - 10, cy);
    glVertex2f(cx + 10, cy);
    glVertex2f(cx, cy - 10);
    glVertex2f(cx, cy + 10);
    glEnd();
    
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void Renderer::RenderBlockHighlight(const Vec3& blockPos, const Camera& camera, int screenW, int screenH) {
    // Draw wireframe box around selected block
    glPushMatrix();
    glTranslatef(blockPos.x + 0.5f, blockPos.y + 0.5f, blockPos.z + 0.5f);
    glScalef(0.51f, 0.51f, 0.51f);
    
    glColor3f(1.0f, 1.0f, 0.0f);
    glLineWidth(2.0f);
    
    glBegin(GL_LINE_LOOP);
    glVertex3f(-1, -1, 1);
    glVertex3f(1, -1, 1);
    glVertex3f(1, 1, 1);
    glVertex3f(-1, 1, 1);
    glEnd();
    
    glBegin(GL_LINE_LOOP);
    glVertex3f(-1, -1, -1);
    glVertex3f(-1, 1, -1);
    glVertex3f(1, 1, -1);
    glVertex3f(1, -1, -1);
    glEnd();
    
    glBegin(GL_LINES);
    glVertex3f(-1, -1, 1);
    glVertex3f(-1, -1, -1);
    glVertex3f(1, -1, 1);
    glVertex3f(1, -1, -1);
    glVertex3f(1, 1, 1);
    glVertex3f(1, 1, -1);
    glVertex3f(-1, 1, 1);
    glVertex3f(-1, 1, -1);
    glEnd();
    
    glPopMatrix();
}

} // namespace vge
