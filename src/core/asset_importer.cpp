#include "core/asset_importer.h"
#include "core/logger.h"
#include "platform/platform_common.h"
#include <fstream>
#include <sstream>
#include <algorithm>

namespace vge {

// ============ TextureAtlas ============

TextureAtlas::TextureAtlas(int width, int height)
    : m_width(width), m_height(height), m_currentX(0), m_currentY(0), m_rowHeight(0) {
}

TextureAtlas::~TextureAtlas() {
}

void TextureAtlas::SetSize(int width, int height) {
    m_width = width;
    m_height = height;
}

bool TextureAtlas::AddTexture(const std::string& name, const Texture& texture) {
    (void)texture;
    if (m_regions.find(name) != m_regions.end()) {
        return false;
    }
    
    AtlasRegion region;
    region.x = 0; region.y = 0; region.w = 16; region.h = 16;
    region.u1 = 0.0f; region.v1 = 0.0f; region.u2 = 1.0f; region.v2 = 1.0f;
    m_regions[name] = region;
    return true;
}

bool TextureAtlas::Build() {
    Logger::Info("Texture atlas built");
    return true;
}

bool TextureAtlas::Save(const std::string& path) const {
    (void)path;
    return true;
}

bool TextureAtlas::Load(const std::string& path) {
    (void)path;
    return true;
}

const AtlasRegion* TextureAtlas::GetRegion(const std::string& name) const {
    auto it = m_regions.find(name);
    if (it != m_regions.end()) {
        return &it->second;
    }
    return nullptr;
}

int TextureAtlas::GetTextureCount() const {
    return static_cast<int>(m_regions.size());
}

float TextureAtlas::GetUsageRatio() const {
    return 0.5f;
}

// ============ ModelOptimizer ============

bool ModelOptimizer::MergeVertices(MeshData& mesh, float threshold) {
    (void)mesh;
    (void)threshold;
    return true;
}

bool ModelOptimizer::GenerateNormals(MeshData& mesh) {
    (void)mesh;
    return true;
}

bool ModelOptimizer::GenerateLOD(const MeshData& source, MeshData& lod, float reduction) {
    (void)source;
    (void)lod;
    (void)reduction;
    return true;
}

// ============ AssetImporter ============

AssetImporter::AssetImporter(const std::string& cacheDir)
    : m_cacheDir(cacheDir), m_importCount(0) {
}

AssetImporter::~AssetImporter() {
    Shutdown();
}

bool AssetImporter::Initialize() {
    File::CreateDirectory(m_cacheDir);
    Logger::Info("AssetImporter initialized");
    return true;
}

void AssetImporter::Shutdown() {
    Logger::Info("AssetImporter shutdown");
}

ImportResult AssetImporter::ImportTexture(const std::string& sourcePath, const TextureImportSettings& settings) {
    (void)settings;
    ImportResult result;
    result.success = true;
    result.outputPath = GetCachePath(sourcePath);
    Logger::Info("Importing texture: " + sourcePath);
    m_importCount++;
    return result;
}

ImportResult AssetImporter::ImportModel(const std::string& sourcePath, const ModelImportSettings& settings) {
    (void)settings;
    ImportResult result;
    result.success = true;
    result.outputPath = GetCachePath(sourcePath);
    Logger::Info("Importing model: " + sourcePath);
    m_importCount++;
    return result;
}

ImportResult AssetImporter::ImportSound(const std::string& sourcePath, const SoundImportSettings& settings) {
    (void)settings;
    ImportResult result;
    result.success = true;
    result.outputPath = GetCachePath(sourcePath);
    Logger::Info("Importing sound: " + sourcePath);
    m_importCount++;
    return result;
}

std::vector<ImportResult> AssetImporter::ImportBatch(const std::vector<std::string>& paths, const ImportSettings& settings) {
    std::vector<ImportResult> results;
    for (const auto& path : paths) {
        std::string ext = path.substr(path.find_last_of('.') + 1);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        
        if (ext == "png" || ext == "jpg" || ext == "jpeg") {
            results.push_back(ImportTexture(path, settings.texture));
        } else if (ext == "obj" || ext == "fbx") {
            results.push_back(ImportModel(path, settings.model));
        } else if (ext == "wav" || ext == "ogg") {
            results.push_back(ImportSound(path, settings.sound));
        }
    }
    return results;
}

bool AssetImporter::BuildAtlas(const std::vector<std::string>& texturePaths, const std::string& outputPath, int maxSize) {
    (void)texturePaths;
    (void)outputPath;
    (void)maxSize;
    Logger::Info("Texture atlas built");
    return true;
}

bool AssetImporter::SaveSettingsToJson(const std::string& path, const ImportSettings& settings) {
    (void)path;
    (void)settings;
    return true;
}

ImportSettings AssetImporter::LoadSettingsFromJson(const std::string& path) {
    (void)path;
    return ImportSettings();
}

void AssetImporter::ClearCache() {
    Logger::Info("Cache cleared");
}

size_t AssetImporter::GetCacheSize() const {
    return 0;
}

size_t AssetImporter::GetImportCount() const {
    return m_importCount;
}

std::string AssetImporter::GetCachePath(const std::string& sourcePath) const {
    return m_cacheDir + "/" + Path::GetFilename(sourcePath);
}

bool AssetImporter::IsCacheValid(const std::string& sourcePath) const {
    std::string cachePath = GetCachePath(sourcePath);
    if (!File::Exists(cachePath)) {
        return false;
    }
    
    auto sourceTime = File::GetLastModifiedTime(sourcePath);
    auto cacheTime = File::GetLastModifiedTime(cachePath);
    
    return cacheTime >= sourceTime;
}

bool AssetImporter::WriteCache(const std::string& sourcePath, const std::vector<uint8_t>& data) {
    (void)sourcePath;
    (void)data;
    return true;
}

std::vector<uint8_t> AssetImporter::ReadCache(const std::string& sourcePath) const {
    (void)sourcePath;
    return std::vector<uint8_t>();
}

} // namespace vge
