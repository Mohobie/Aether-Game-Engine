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

// Asset manager for loading and caching assets
class AssetManager {
private:
    std::unordered_map<std::string, Texture> textures;
    std::unordered_map<std::string, Material> materials;
    std::string basePath;
    
public:
    AssetManager(const std::string& base = "assets/");
    
    // Texture management
    Texture* LoadTexture(const std::string& name, const std::string& path);
    Texture* GetTexture(const std::string& name);
    void UnloadTexture(const std::string& name);
    
    // Material management
    Material* LoadMaterial(const std::string& name, const std::string& path);
    Material* GetMaterial(const std::string& name);
    void UnloadMaterial(const std::string& name);
    
    // Generate default textures
    void GenerateDefaultTextures();
    
    // Get stats
    size_t GetTextureCount() const { return textures.size(); }
    size_t GetMaterialCount() const { return materials.size(); }
    
    // Clear all
    void Clear();
};

} // namespace vge