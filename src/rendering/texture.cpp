#include "texture.h"
#include "platform/file_system.h"
#include <iostream>
#include <cmath>
#include <cstring>

namespace vge {

// ============================================
// Texture Implementation
// ============================================

bool Texture::LoadFromFile(const std::string& path) {
    // For now, just create a colored texture based on name
    // In a real implementation, this would load PNG/JPG using stb_image
    std::cout << "[Texture] Loading: " << path << std::endl;
    
    // Create a simple colored pattern based on hash of name
    size_t hash = std::hash<std::string>{}(path);
    Vec3 color(
        ((hash >> 16) & 0xFF) / 255.0f,
        ((hash >> 8) & 0xFF) / 255.0f,
        (hash & 0xFF) / 255.0f
    );
    
    *this = CreateSolidColor(color, 64, 64);
    name = path;
    return true;
}

bool Texture::LoadFromMemory(const uint8_t* src, int w, int h, int ch) {
    width = w;
    height = h;
    channels = ch;
    data.resize(w * h * ch);
    std::memcpy(data.data(), src, w * h * ch);
    return true;
}

Texture Texture::CreateSolidColor(const Vec3& color, int w, int h) {
    Texture tex;
    tex.width = w;
    tex.height = h;
    tex.channels = 4;
    tex.data.resize(w * h * 4);
    
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int idx = (y * w + x) * 4;
            tex.data[idx] = static_cast<uint8_t>(color.x * 255);
            tex.data[idx + 1] = static_cast<uint8_t>(color.y * 255);
            tex.data[idx + 2] = static_cast<uint8_t>(color.z * 255);
            tex.data[idx + 3] = 255; // Alpha
        }
    }
    
    return tex;
}

Texture Texture::CreateCheckerboard(int w, int h, int checkSize) {
    Texture tex;
    tex.width = w;
    tex.height = h;
    tex.channels = 4;
    tex.data.resize(w * h * 4);
    
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int idx = (y * w + x) * 4;
            bool check = ((x / checkSize) + (y / checkSize)) % 2 == 0;
            uint8_t val = check ? 255 : 64;
            tex.data[idx] = val;
            tex.data[idx + 1] = val;
            tex.data[idx + 2] = val;
            tex.data[idx + 3] = 255;
        }
    }
    
    return tex;
}

// ============================================
// Material Implementation
// ============================================

Material Material::LoadFromJson(const std::string& json) {
    Material mat;
    // Simple JSON parsing would go here
    // For now, return default material
    return mat;
}

} // namespace vge
