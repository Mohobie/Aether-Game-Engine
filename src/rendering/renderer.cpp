#include "rendering/renderer.h"
#include "rendering/shader.h"
#include "rendering/camera.h"
#include "rendering/mesh.h"
#include "voxel/world.h"
#include "voxel/chunk.h"
#include "voxel/block.h"
#include "voxel/block_registry.h"
#include "core/logger.h"
#include <iostream>
#include <cstring>
#include <cmath>
#include <algorithm>

// Software renderer - no OpenGL required!
// Renders voxels to ASCII/console output

namespace vge {

struct Framebuffer {
    static const int WIDTH = 80;
    static const int HEIGHT = 40;
    char pixels[HEIGHT][WIDTH];
    float depth[HEIGHT][WIDTH];
    
    Framebuffer() {
        Clear();
    }
    
    void Clear() {
        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {
                pixels[y][x] = ' ';
                depth[y][x] = 999999.0f;
            }
        }
    }
    
    void SetPixel(int x, int y, float z, char c) {
        if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) return;
        if (z < depth[y][x]) {
            depth[y][x] = z;
            pixels[y][x] = c;
        }
    }
    
    void Draw() const {
        std::cout << "+";
        for (int x = 0; x < WIDTH; x++) std::cout << "-";
        std::cout << "+\n";
        
        for (int y = 0; y < HEIGHT; y++) {
            std::cout << "|";
            for (int x = 0; x < WIDTH; x++) {
                std::cout << pixels[y][x];
            }
            std::cout << "|\n";
        }
        
        std::cout << "+";
        for (int x = 0; x < WIDTH; x++) std::cout << "-";
        std::cout << "+\n";
    }
};

Renderer::Renderer() : initialized(false), width(80), height(40) {}

Renderer::~Renderer() {
    if (initialized) Shutdown();
}

bool Renderer::Initialize() {
    std::cout << "[Renderer] Software renderer initialized (ASCII mode)\n";
    initialized = true;
    return true;
}

void Renderer::Shutdown() {
    initialized = false;
}

void Renderer::BeginFrame() {
    // Clear screen (ANSI escape)
    std::cout << "\033[2J\033[H";
}

void Renderer::EndFrame() {
    // Frame complete
}

void Renderer::SetClearColor(float r, float g, float b, float a) {
    // No-op for ASCII renderer
}

void Renderer::SetViewport(int x, int y, int w, int h) {
    width = w;
    height = h;
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

char GetBlockChar(BlockTypeID type) {
    if (type == BlockRegistry::GetInstance().GetBlockId("grass")) return '"';
    if (type == BlockRegistry::GetInstance().GetBlockId("dirt")) return ':';
    if (type == BlockRegistry::GetInstance().GetBlockId("stone")) return '#';
    if (type == BlockRegistry::GetInstance().GetBlockId("wood")) return '+';
    if (type == BlockRegistry::GetInstance().GetBlockId("leaves")) return '*';
    if (type == BlockRegistry::GetInstance().GetBlockId("sand")) return '.';
    if (type == BlockRegistry::GetInstance().GetBlockId("water")) return '~';
    if (type == BlockRegistry::GetInstance().GetBlockId("bedrock")) return '@';
    return '?';
}

void Renderer::RenderWorld(const World& world, const Camera& camera) {
    Framebuffer fb;
    
    // Render visible chunks
    for (int cx = -2; cx <= 2; cx++) {
        for (int cy = -1; cy <= 1; cy++) {
            for (int cz = -2; cz <= 2; cz++) {
                Chunk* chunk = const_cast<World&>(world).GetChunk(cx, cy, cz);
                if (!chunk || !chunk->loaded) continue;
                
                // Render each visible block in chunk
                for (int x = 0; x < CHUNK_SIZE; x += 2) {
                    for (int y = 0; y < CHUNK_SIZE; y += 2) {
                        for (int z = 0; z < CHUNK_SIZE; z += 2) {
                            BlockTypeID block = chunk->GetBlock(x, y, z);
                            if (block == BlockRegistry::GetInstance().GetBlockId("air")) continue;
                            
                            // Optimization: only draw if at least one face is exposed to air
                            bool visible = false;
                            if (x == 0 || chunk->GetBlock(x-1, y, z) == BlockRegistry::GetInstance().GetBlockId("air")) visible = true;
                            if (x == CHUNK_SIZE-1 || chunk->GetBlock(x+1, y, z) == BlockRegistry::GetInstance().GetBlockId("air")) visible = true;
                            if (y == 0 || chunk->GetBlock(x, y-1, z) == BlockRegistry::GetInstance().GetBlockId("air")) visible = true;
                            if (y == CHUNK_SIZE-1 || chunk->GetBlock(x, y+1, z) == BlockRegistry::GetInstance().GetBlockId("air")) visible = true;
                            if (z == 0 || chunk->GetBlock(x, y, z-1) == BlockRegistry::GetInstance().GetBlockId("air")) visible = true;
                            if (z == CHUNK_SIZE-1 || chunk->GetBlock(x, y, z+1) == BlockRegistry::GetInstance().GetBlockId("air")) visible = true;
                            
                            if (!visible) continue; // Skip hidden blocks
                            
                            // World position
                            Vec3 worldPos(
                                cx * CHUNK_SIZE + x,
                                cy * CHUNK_SIZE + y,
                                cz * CHUNK_SIZE + z
                            );
                            
                            // Project to screen
                            Vec2 screenPos = Project(worldPos, camera, fb.WIDTH, fb.HEIGHT);
                            
                            // Get depth (distance from camera)
                            float depth = (worldPos - camera.GetPosition()).length();
                            
                            // Draw
                            fb.SetPixel((int)screenPos.x, (int)screenPos.y, depth, GetBlockChar(block));
                        }
                    }
                }
            }
        }
    }
    
    fb.Draw();
}

void Renderer::RenderMesh(const Mesh& mesh, const Shader& shader, const Camera& camera) {
    // Not implemented for software renderer
}

} // namespace vge