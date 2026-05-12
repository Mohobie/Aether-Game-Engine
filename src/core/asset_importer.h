#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "types.h"
#include "math/vec3.h"
#include "math/vec2.h"
#include "rendering/texture.h"

namespace vge {

// Import settings for different asset types
struct TextureImportSettings {
    std::string format = "RGBA8";
    bool generateMipmaps = true;
    std::string filter = "trilinear";
    bool compress = false;
    int maxSize = 2048;
};

struct ModelImportSettings {
    bool generateNormals = true;
    bool generateTangents = false;
    float scale = 1.0f;
    bool flipUVs = false;
    bool mergeVertices = false;
    int lodLevels = 3;
};

struct SoundImportSettings {
    std::string format = "ogg";
    int quality = 5; // 0-10, higher is better
    bool loop = false;
    float volume = 1.0f;
};

struct ImportSettings {
    TextureImportSettings texture;
    ModelImportSettings model;
    SoundImportSettings sound;
};

// Import result
struct ImportResult {
    bool success = false;
    std::string outputPath;
    std::string errorMessage;
    size_t originalSize = 0;
    size_t importedSize = 0;
    float importTime = 0.0f;
};

// Atlas region for texture atlases
struct AtlasRegion {
    int x, y, w, h;
    float u1, v1, u2, v2;
};

// Texture atlas builder
class TextureAtlas {
public:
    TextureAtlas(int width = 512, int height = 512);
    ~TextureAtlas();

    void SetSize(int width, int height);
    bool AddTexture(const std::string& name, const Texture& texture);
    bool Build();
    bool Save(const std::string& path) const;
    bool Load(const std::string& path);

    const AtlasRegion* GetRegion(const std::string& name) const;
    int GetTextureCount() const;
    float GetUsageRatio() const;

private:
    int m_width, m_height;
    std::unordered_map<std::string, AtlasRegion> m_regions;
    std::vector<uint8_t> m_atlasData;
    int m_currentX = 0, m_currentY = 0;
    int m_rowHeight = 0;
};

// Model optimizer for LOD generation
class ModelOptimizer {
public:
    struct MeshData {
        std::vector<Vec3> vertices;
        std::vector<Vec3> normals;
        std::vector<Vec2> uvs;
        std::vector<uint32_t> indices;
    };

    static bool MergeVertices(MeshData& mesh, float threshold = 0.001f);
    static bool GenerateNormals(MeshData& mesh);
    static bool GenerateLOD(const MeshData& source, MeshData& lod, float reduction);
};

// Main asset importer
class AssetImporter {
public:
    AssetImporter(const std::string& cacheDir = "assets/.cache/");
    ~AssetImporter();

    bool Initialize();
    void Shutdown();

    // Import individual assets
    ImportResult ImportTexture(const std::string& sourcePath, const TextureImportSettings& settings);
    ImportResult ImportModel(const std::string& sourcePath, const ModelImportSettings& settings);
    ImportResult ImportSound(const std::string& sourcePath, const SoundImportSettings& settings);

    // Batch import
    std::vector<ImportResult> ImportBatch(const std::vector<std::string>& paths, const ImportSettings& settings);

    // Texture atlas
    bool BuildAtlas(const std::vector<std::string>& texturePaths, const std::string& outputPath, int maxSize = 2048);

    // Settings
    bool SaveSettingsToJson(const std::string& path, const ImportSettings& settings);
    ImportSettings LoadSettingsFromJson(const std::string& path);

    // Cache management
    void ClearCache();
    size_t GetCacheSize() const;
    size_t GetImportCount() const;

private:
    std::string m_cacheDir;
    size_t m_importCount = 0;

    std::string GetCachePath(const std::string& sourcePath) const;
    bool IsCacheValid(const std::string& sourcePath) const;
    bool WriteCache(const std::string& sourcePath, const std::vector<uint8_t>& data);
    std::vector<uint8_t> ReadCache(const std::string& sourcePath) const;
};

} // namespace vge
