#include "asset_importer.h"
#include "platform/platform_common.h"
#include "logger.h"
#include <fstream>
#include <sstream>
#include <algorithm>

namespace vge {

// TextureAtlas implementation
TextureAtlas::TextureAtlas(int width, int height)
    : m_width(width), m_height(height) {
    m_atlasData.resize(width * height * 4, 0);
}

TextureAtlas::~TextureAtlas() = default;

void TextureAtlas::SetSize(int width, int height) {
    m_width = width;
    m_height = height;
    m_atlasData.resize(width * height * 4, 0);
    m_regions.clear();
    m_currentX = 0;
    m_currentY = 0;
    m_rowHeight = 0;
}

bool TextureAtlas::AddTexture(const std::string& name, const Texture& texture) {
    if (texture.width > m_width || texture.height > m_height) {
        return false;
    }

    // Simple shelf packing
    if (m_currentX + texture.width > m_width) {
        m_currentX = 0;
        m_currentY += m_rowHeight;
        m_rowHeight = 0;
    }

    if (m_currentY + texture.height > m_height) {
        return false; // Atlas full
    }

    AtlasRegion region;
    region.x = m_currentX;
    region.y = m_currentY;
    region.w = texture.width;
    region.h = texture.height;
    region.u1 = (float)m_currentX / m_width;
    region.v1 = (float)m_currentY / m_height;
    region.u2 = (float)(m_currentX + texture.width) / m_width;
    region.v2 = (float)(m_currentY + texture.height) / m_height;

    // Copy texture data into atlas
    for (int y = 0; y < texture.height; ++y) {
        for (int x = 0; x < texture.width; ++x) {
            int srcIdx = (y * texture.width + x) * 4;
            int dstIdx = ((m_currentY + y) * m_width + (m_currentX + x)) * 4;
            if (srcIdx + 3 < (int)texture.data.size() && dstIdx + 3 < (int)m_atlasData.size()) {
                m_atlasData[dstIdx] = texture.data[srcIdx];
                m_atlasData[dstIdx + 1] = texture.data[srcIdx + 1];
                m_atlasData[dstIdx + 2] = texture.data[srcIdx + 2];
                m_atlasData[dstIdx + 3] = texture.data[srcIdx + 3];
            }
        }
    }

    m_regions[name] = region;
    m_currentX += texture.width;
    m_rowHeight = std::max(m_rowHeight, texture.height);

    return true;
}

bool TextureAtlas::Build() {
    LOG_INFO("Texture atlas built with %zu textures", m_regions.size());
    return true;
}

bool TextureAtlas::Save(const std::string& path) const {
    std::ofstream file(path, std::ios::binary);
    if (!file) return false;

    // Write header
    int version = 1;
    file.write(reinterpret_cast<const char*>(&version), sizeof(version));
    file.write(reinterpret_cast<const char*>(&m_width), sizeof(m_width));
    file.write(reinterpret_cast<const char*>(&m_height), sizeof(m_height));

    // Write region count
    int regionCount = (int)m_regions.size();
    file.write(reinterpret_cast<const char*>(&regionCount), sizeof(regionCount));

    // Write regions
    for (const auto& pair : m_regions) {
        int nameLen = (int)pair.first.length();
        file.write(reinterpret_cast<const char*>(&nameLen), sizeof(nameLen));
        file.write(pair.first.c_str(), nameLen);
        file.write(reinterpret_cast<const char*>(&pair.second), sizeof(AtlasRegion));
    }

    // Write atlas data
    int dataSize = (int)m_atlasData.size();
    file.write(reinterpret_cast<const char*>(&dataSize), sizeof(dataSize));
    file.write(reinterpret_cast<const char*>(m_atlasData.data()), dataSize);

    return true;
}

bool TextureAtlas::Load(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) return false;

    int version;
    file.read(reinterpret_cast<char*>(&version), sizeof(version));
    if (version != 1) return false;

    file.read(reinterpret_cast<char*>(&m_width), sizeof(m_width));
    file.read(reinterpret_cast<char*>(&m_height), sizeof(m_height));

    int regionCount;
    file.read(reinterpret_cast<char*>(&regionCount), sizeof(regionCount));

    m_regions.clear();
    for (int i = 0; i < regionCount; ++i) {
        int nameLen;
        file.read(reinterpret_cast<char*>(&nameLen), sizeof(nameLen));
        std::string name(nameLen, '\0');
        file.read(&name[0], nameLen);

        AtlasRegion region;
        file.read(reinterpret_cast<char*>(&region), sizeof(region));
        m_regions[name] = region;
    }

    int dataSize;
    file.read(reinterpret_cast<char*>(&dataSize), sizeof(dataSize));
    m_atlasData.resize(dataSize);
    file.read(reinterpret_cast<char*>(m_atlasData.data()), dataSize);

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
    return (int)m_regions.size();
}

float TextureAtlas::GetUsageRatio() const {
    int usedArea = 0;
    for (const auto& pair : m_regions) {
        usedArea += pair.second.w * pair.second.h;
    }
    return (float)usedArea / (m_width * m_height);
}

// ModelOptimizer implementation
bool ModelOptimizer::MergeVertices(MeshData& mesh, float threshold) {
    if (mesh.vertices.empty()) return false;

    std::vector<Vec3> newVertices;
    std::vector<Vec3> newNormals;
    std::vector<Vec2> newUVs;
    std::vector<uint32_t> newIndices;
    std::vector<int> vertexRemap(mesh.vertices.size(), -1);

    for (size_t i = 0; i < mesh.vertices.size(); ++i) {
        bool merged = false;
        for (size_t j = 0; j < newVertices.size(); ++j) {
            if ((mesh.vertices[i] - newVertices[j]).length() < threshold) {
                vertexRemap[i] = (int)j;
                merged = true;
                break;
            }
        }
        if (!merged) {
            vertexRemap[i] = (int)newVertices.size();
            newVertices.push_back(mesh.vertices[i]);
            if (!mesh.normals.empty()) newNormals.push_back(mesh.normals[i]);
            if (!mesh.uvs.empty()) newUVs.push_back(mesh.uvs[i]);
        }
    }

    for (size_t i = 0; i < mesh.indices.size(); ++i) {
        newIndices.push_back(vertexRemap[mesh.indices[i]]);
    }

    mesh.vertices = std::move(newVertices);
    mesh.normals = std::move(newNormals);
    mesh.uvs = std::move(newUVs);
    mesh.indices = std::move(newIndices);

    return true;
}

bool ModelOptimizer::GenerateNormals(MeshData& mesh) {
    if (mesh.vertices.empty() || mesh.indices.empty()) return false;

    mesh.normals.resize(mesh.vertices.size(), Vec3(0, 0, 0));

    for (size_t i = 0; i < mesh.indices.size(); i += 3) {
        if (i + 2 >= mesh.indices.size()) break;

        uint32_t i0 = mesh.indices[i];
        uint32_t i1 = mesh.indices[i + 1];
        uint32_t i2 = mesh.indices[i + 2];

        if (i0 >= mesh.vertices.size() || i1 >= mesh.vertices.size() || i2 >= mesh.vertices.size())
            continue;

        Vec3 v0 = mesh.vertices[i0];
        Vec3 v1 = mesh.vertices[i1];
        Vec3 v2 = mesh.vertices[i2];

        Vec3 normal = (v1 - v0).cross(v2 - v0);
        normal.normalize();

        mesh.normals[i0] = mesh.normals[i0] + normal;
        mesh.normals[i1] = mesh.normals[i1] + normal;
        mesh.normals[i2] = mesh.normals[i2] + normal;
    }

    for (auto& n : mesh.normals) {
        n.normalize();
    }

    return true;
}

bool ModelOptimizer::GenerateLOD(const MeshData& source, MeshData& lod, float reduction) {
    if (reduction <= 0.0f || reduction >= 1.0f) return false;

    // Simple every-Nth triangle reduction
    int step = (int)(1.0f / reduction);
    if (step < 1) step = 1;

    lod.vertices = source.vertices;
    lod.normals = source.normals;
    lod.uvs = source.uvs;
    lod.indices.clear();

    for (size_t i = 0; i < source.indices.size(); i += 3 * step) {
        if (i + 2 < source.indices.size()) {
            lod.indices.push_back(source.indices[i]);
            lod.indices.push_back(source.indices[i + 1]);
            lod.indices.push_back(source.indices[i + 2]);
        }
    }

    return true;
}

// AssetImporter implementation
AssetImporter::AssetImporter(const std::string& cacheDir)
    : m_cacheDir(cacheDir) {}

AssetImporter::~AssetImporter() = default;

bool AssetImporter::Initialize() {
    File::CreateDirectory(m_cacheDir);
    LOG_INFO("AssetImporter initialized with cache: %s", m_cacheDir.c_str());
    return true;
}

void AssetImporter::Shutdown() {
    LOG_INFO("AssetImporter shutdown");
}

ImportResult AssetImporter::ImportTexture(const std::string& sourcePath, const TextureImportSettings& settings) {
    ImportResult result;
    result.originalSize = File::GetFileSize(sourcePath);

    // Check cache first
    if (IsCacheValid(sourcePath)) {
        result.success = true;
        result.outputPath = GetCachePath(sourcePath);
        result.importedSize = File::GetFileSize(result.outputPath);
        LOG_INFO("Texture import cache hit: %s", sourcePath.c_str());
        return result;
    }

    // Simulate import process
    LOG_INFO("Importing texture: %s (format: %s, mipmaps: %s)",
             sourcePath.c_str(), settings.format.c_str(),
             settings.generateMipmaps ? "yes" : "no");

    // Read source file
    std::ifstream file(sourcePath, std::ios::binary);
    if (!file) {
        result.errorMessage = "Failed to open source file";
        return result;
    }

    std::vector<uint8_t> data((std::istreambuf_iterator<char>(file)),
                               std::istreambuf_iterator<char>());

    // Write to cache
    std::string cachePath = GetCachePath(sourcePath);
    if (WriteCache(sourcePath, data)) {
        result.success = true;
        result.outputPath = cachePath;
        result.importedSize = data.size();
        m_importCount++;
    } else {
        result.errorMessage = "Failed to write cache";
    }

    return result;
}

ImportResult AssetImporter::ImportModel(const std::string& sourcePath, const ModelImportSettings& settings) {
    ImportResult result;
    result.originalSize = File::GetFileSize(sourcePath);

    LOG_INFO("Importing model: %s (scale: %.2f, normals: %s)",
             sourcePath.c_str(), settings.scale,
             settings.generateNormals ? "yes" : "no");

    // Placeholder for model import
    result.success = true;
    result.outputPath = GetCachePath(sourcePath);
    result.importedSize = result.originalSize;
    m_importCount++;

    return result;
}

ImportResult AssetImporter::ImportSound(const std::string& sourcePath, const SoundImportSettings& settings) {
    ImportResult result;
    result.originalSize = File::GetFileSize(sourcePath);

    LOG_INFO("Importing sound: %s (format: %s, quality: %d)",
             sourcePath.c_str(), settings.format.c_str(), settings.quality);

    // Placeholder for sound import
    result.success = true;
    result.outputPath = GetCachePath(sourcePath);
    result.importedSize = result.originalSize;
    m_importCount++;

    return result;
}

std::vector<ImportResult> AssetImporter::ImportBatch(const std::vector<std::string>& paths, const ImportSettings& settings) {
    std::vector<ImportResult> results;
    results.reserve(paths.size());

    for (const auto& path : paths) {
        std::string ext = Path::GetExtension(path);
        if (ext == ".png" || ext == ".jpg" || ext == ".tga" || ext == ".bmp") {
            results.push_back(ImportTexture(path, settings.texture));
        } else if (ext == ".obj" || ext == ".fbx" || ext == ".gltf") {
            results.push_back(ImportModel(path, settings.model));
        } else if (ext == ".wav" || ext == ".ogg") {
            results.push_back(ImportSound(path, settings.sound));
        } else {
            ImportResult result;
            result.errorMessage = "Unknown file type: " + ext;
            results.push_back(result);
        }
    }

    return results;
}

bool AssetImporter::BuildAtlas(const std::vector<std::string>& texturePaths, const std::string& outputPath, int maxSize) {
    TextureAtlas atlas(maxSize, maxSize);

    for (const auto& path : texturePaths) {
        // Load texture and add to atlas
        Texture tex;
        tex.width = 64;
        tex.height = 64;
        tex.channels = 4;
        tex.data.resize(64 * 64 * 4, 128);

        std::string name = Path::GetFileName(path);
        atlas.AddTexture(name, tex);
    }

    if (!atlas.Build()) {
        return false;
    }

    return atlas.Save(outputPath);
}

bool AssetImporter::SaveSettingsToJson(const std::string& path, const ImportSettings& settings) {
    std::ofstream file(path);
    if (!file) return false;

    file << "{\n";
    file << "  \"texture\": {\n";
    file << "    \"format\": \"" << settings.texture.format << "\",\n";
    file << "    \"mipmaps\": " << (settings.texture.generateMipmaps ? "true" : "false") << ",\n";
    file << "    \"filter\": \"" << settings.texture.filter << "\"\n";
    file << "  },\n";
    file << "  \"model\": {\n";
    file << "    \"generate_normals\": " << (settings.model.generateNormals ? "true" : "false") << ",\n";
    file << "    \"scale\": " << settings.model.scale << "\n";
    file << "  },\n";
    file << "  \"sound\": {\n";
    file << "    \"format\": \"" << settings.sound.format << "\",\n";
    file << "    \"quality\": " << settings.sound.quality << "\n";
    file << "  }\n";
    file << "}\n";

    return true;
}

ImportSettings AssetImporter::LoadSettingsFromJson(const std::string& path) {
    ImportSettings settings;

    std::ifstream file(path);
    if (!file) return settings;

    std::string line;
    while (std::getline(file, line)) {
        // Very simple JSON parsing
        if (line.find("\"format\"") != std::string::npos) {
            size_t start = line.find('"', line.find(':'));
            size_t end = line.find('"', start + 1);
            if (start != std::string::npos && end != std::string::npos) {
                settings.texture.format = line.substr(start + 1, end - start - 1);
            }
        }
        if (line.find("\"mipmaps\"") != std::string::npos) {
            settings.texture.generateMipmaps = (line.find("true") != std::string::npos);
        }
        if (line.find("\"scale\"") != std::string::npos) {
            size_t start = line.find(':');
            if (start != std::string::npos) {
                settings.model.scale = std::stof(line.substr(start + 1));
            }
        }
    }

    return settings;
}

void AssetImporter::ClearCache() {
    auto files = File::ListDirectory(m_cacheDir);
    for (const auto& file : files) {
        File::DeleteFile(file);
    }
    m_importCount = 0;
    LOG_INFO("Cache cleared");
}

size_t AssetImporter::GetCacheSize() const {
    size_t totalSize = 0;
    auto files = File::ListDirectory(m_cacheDir);
    for (const auto& file : files) {
        totalSize += File::GetFileSize(file);
    }
    return totalSize;
}

size_t AssetImporter::GetImportCount() const {
    return m_importCount;
}

std::string AssetImporter::GetCachePath(const std::string& sourcePath) const {
    std::string filename = Path::GetFileName(sourcePath);
    return m_cacheDir + filename + ".cache";
}

bool AssetImporter::IsCacheValid(const std::string& sourcePath) const {
    std::string cachePath = GetCachePath(sourcePath);
    if (!File::Exists(cachePath)) return false;

    uint64_t sourceTime = File::GetLastModifiedTime(sourcePath);
    uint64_t cacheTime = File::GetLastModifiedTime(cachePath);
    return cacheTime >= sourceTime;
}

bool AssetImporter::WriteCache(const std::string& sourcePath, const std::vector<uint8_t>& data) {
    std::string cachePath = GetCachePath(sourcePath);
    std::ofstream file(cachePath, std::ios::binary);
    if (!file) return false;
    file.write(reinterpret_cast<const char*>(data.data()), data.size());
    return true;
}

std::vector<uint8_t> AssetImporter::ReadCache(const std::string& sourcePath) const {
    std::string cachePath = GetCachePath(sourcePath);
    std::ifstream file(cachePath, std::ios::binary);
    if (!file) return {};
    return std::vector<uint8_t>((std::istreambuf_iterator<char>(file)),
                                 std::istreambuf_iterator<char>());
}

} // namespace vge
