#include "rendering/renderer.h"
#include "rendering/framebuffer_renderer.h"
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

Renderer::Renderer() : initialized(false), width(80), height(40), fb_renderer(nullptr) {}

Renderer::~Renderer() {
    if (initialized) Shutdown();
}

bool Renderer::Initialize() {
    std::cout << "[Renderer] Software renderer initialized (ASCII mode)\n";
    
    // Try to initialize framebuffer renderer
    fb_renderer = new FramebufferRenderer();
    if (fb_renderer->Initialize()) {
        std::cout << "[Renderer] Framebuffer mode available!\n";
    } else {
        std::cout << "[Renderer] Falling back to ASCII mode\n";
        delete fb_renderer;
        fb_renderer = nullptr;
    }
    
    initialized = true;
    return true;
}

void Renderer::Shutdown() {
    if (fb_renderer) {
        fb_renderer->Shutdown();
        delete fb_renderer;
        fb_renderer = nullptr;
    }
    initialized = false;
}

void Renderer::BeginFrame() {
    std::cout << "\033[2J\033[H";
    
    if (fb_renderer && fb_renderer->IsInitialized()) {
        fb_renderer->Clear(0x000000);
    }
}

void Renderer::EndFrame() {
    if (fb_renderer && fb_renderer->IsInitialized()) {
        fb_renderer->SwapBuffers();
    }
}

void Renderer::SetClearColor(float r, float g, float b, float a) {
    // No-op for ASCII renderer
}

void Renderer::SetViewport(int x, int y, int w, int h) {
    width = w;
    height = h;
}

Vec2 Project(const Vec3& worldPos, const Camera& camera, int screenW, int screenH) {
    Vec3 relative = worldPos - camera.GetPosition();
    
    float fov = 70.0f * 3.14159f / 180.0f;
    float tanHalfFov = std::tan(fov / 2.0f);
    float aspect = (float)screenW / (float)screenH;
    
    float yaw = camera.GetRotation().x * 3.14159f / 180.0f;
    float cosY = std::cos(yaw);
    float sinY = std::sin(yaw);
    
    float rx = relative.x * cosY - relative.z * sinY;
    float rz = relative.x * sinY + relative.z * cosY;
    float ry = relative.y;
    
    if (rz <= 0.1f) rz = 0.1f;
    
    float screenX = (rx / (rz * tanHalfFov * aspect)) * screenW / 2 + screenW / 2;
    float screenY = (ry / (rz * tanHalfFov)) * screenH / 2 + screenH / 2;
    
    return Vec2(screenX, screenY);
}

char GetBlockChar(BlockType type) {
    switch (type) {
        case BlockType::Grass: return '"';
        case BlockType::Dirt: return ':';
        case BlockType::Stone: return '#';
        case BlockType::Wood: return '+';
        case BlockType::Leaves: return '*';
        case BlockType::Sand: return '.';
        case BlockType::Water: return '~';
        case BlockType::Bedrock: return '@';
        default: return '?';
    }
}

void Renderer::RenderWorld(const World& world, const Camera& camera) {
    if (fb_renderer && fb_renderer->IsInitialized()) {
        RenderWorldFB(world, camera);
    } else {
        RenderWorldASCII(world, camera);
    }
}

void Renderer::RenderWorldASCII(const World& world, const Camera& camera) {
    Framebuffer fb;
    
    for (int cx = -2; cx <= 2; cx++) {
        for (int cy = -1; cy <= 1; cy++) {
            for (int cz = -2; cz <= 2; cz++) {
                Chunk* chunk = const_cast<World&>(world).GetChunk(cx, cy, cz);
                if (!chunk || !chunk->loaded) continue;
                
                for (int x = 0; x < CHUNK_SIZE; x += 2) {
                    for (int y = 0; y < CHUNK_SIZE; y += 2) {
                        for (int z = 0; z < CHUNK_SIZE; z += 2) {
                            BlockType block = chunk->GetBlock(x, y, z);
                            if (block == BlockType::Air) continue;
                            
                            bool visible = false;
                            if (x == 0 || chunk->GetBlock(x-1, y, z) == BlockType::Air) visible = true;
                            if (x == CHUNK_SIZE-1 || chunk->GetBlock(x+1, y, z) == BlockType::Air) visible = true;
                            if (y == 0 || chunk->GetBlock(x, y-1, z) == BlockType::Air) visible = true;
                            if (y == CHUNK_SIZE-1 || chunk->GetBlock(x, y+1, z) == BlockType::Air) visible = true;
                            if (z == 0 || chunk->GetBlock(x, y, z-1) == BlockType::Air) visible = true;
                            if (z == CHUNK_SIZE-1 || chunk->GetBlock(x, y, z+1) == BlockType::Air) visible = true;
                            
                            if (!visible) continue;
                            
                            Vec3 worldPos(
                                cx * CHUNK_SIZE + x,
                                cy * CHUNK_SIZE + y,
                                cz * CHUNK_SIZE + z
                            );
                            
                            Vec2 screenPos = Project(worldPos, camera, fb.WIDTH, fb.HEIGHT);
                            float depth = (worldPos - camera.GetPosition()).length();
                            fb.SetPixel((int)screenPos.x, (int)screenPos.y, depth, GetBlockChar(block));
                        }
                    }
                }
            }
        }
    }
    
    fb.Draw();
}

void Renderer::RenderWorldFB(const World& world, const Camera& camera) {
    int screenW = fb_renderer->GetWidth();
    int screenH = fb_renderer->GetHeight();
    
    for (int cx = -2; cx <= 2; cx++) {
        for (int cy = -1; cy <= 1; cy++) {
            for (int cz = -2; cz <= 2; cz++) {
                Chunk* chunk = const_cast<World&>(world).GetChunk(cx, cy, cz);
                if (!chunk || !chunk->loaded) continue;
                
                for (int x = 0; x < CHUNK_SIZE; x += 4) {
                    for (int y = 0; y < CHUNK_SIZE; y += 4) {
                        for (int z = 0; z < CHUNK_SIZE; z += 4) {
                            BlockType block = chunk->GetBlock(x, y, z);
                            if (block == BlockType::Air) continue;
                            
                            Vec3 worldPos(
                                cx * CHUNK_SIZE + x,
                                cy * CHUNK_SIZE + y,
                                cz * CHUNK_SIZE + z
                            );
                            
                            Vec2 screenPos = Project(worldPos, camera, screenW, screenH);
                            
                            if (screenPos.x >= 0 && screenPos.x < screenW && 
                                screenPos.y >= 0 && screenPos.y < screenH) {
                                uint32_t color = GetBlockColor(block);
                                fb_renderer->SetPixel((int)screenPos.x, (int)screenPos.y, color);
                            }
                        }
                    }
                }
            }
        }
    }
}

uint32_t Renderer::GetBlockColor(BlockType type) {
    switch (type) {
        case BlockType::Grass: return RGB(34, 139, 34);
        case BlockType::Dirt: return RGB(139, 69, 19);
        case BlockType::Stone: return RGB(128, 128, 128);
        case BlockType::Wood: return RGB(101, 67, 33);
        case BlockType::Leaves: return RGB(0, 100, 0);
        case BlockType::Sand: return RGB(194, 178, 128);
        case BlockType::Water: return RGB(30, 144, 255);
        case BlockType::Bedrock: return RGB(50, 50, 50);
        default: return RGB(255, 0, 255);
    }
}

void Renderer::RenderMesh(const Mesh& mesh, const Shader& shader, const Camera& camera) {
    // Not implemented for software renderer
}

} // namespace vge