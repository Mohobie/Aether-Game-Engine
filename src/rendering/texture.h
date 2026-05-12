#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <unordered_map>
#include "math/vec3.h"

namespace vge {

// Texture data
struct Texture {
    std::string name;
    int width;
    int height;
    int channels;
    std::vector<uint8_t> data;
    
    Texture() : width(0), height(0), channels(4) {}
    
    bool LoadFromFile(const std::string& path);
    bool LoadFromMemory(const uint8_t* data, int w, int h, int ch);
    
    // Create a solid color texture
    static Texture CreateSolidColor(const Vec3& color, int w = 16, int h = 16);
    
    // Create a checkerboard pattern
    static Texture CreateCheckerboard(int w = 64, int h = 64, int checkSize = 8);
};

// Material with texture references
struct Material {
    std::string name;
    Vec3 diffuseColor;
    Vec3 specularColor;
    float shininess;
    float opacity;
    
    // Texture slots
    std::string diffuseTexture;
    std::string normalTexture;
    std::string specularTexture;
    
    Material() 
        : diffuseColor(1.0f, 1.0f, 1.0f)
        , specularColor(0.5f, 0.5f, 0.5f)
        , shininess(32.0f)
        , opacity(1.0f)
    {}
    
    // Create from JSON definition
    static Material LoadFromJson(const std::string& json);
};

} // namespace vge