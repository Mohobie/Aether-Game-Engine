#include "resource_pack.h"
#include "asset_manager.h"
#include "platform/platform_common.h"
#include "core/logger.h"
#include <fstream>
#include <algorithm>
#include <mutex>

namespace vge {

// ResourcePack implementation
ResourcePack::ResourcePack() = default;
ResourcePack::~ResourcePack() { Unload(); }

bool ResourcePack::LoadFromDirectory(const std::string& directory) {
    m_basePath = directory;
    if (!m_basePath.empty() && m_basePath.back() != '/') {
        m_basePath += '/';
    }

    // Load manifest
    std::string manifestPath = m_basePath + "manifest.json";
    if (!LoadManifest(manifestPath)) {
        Logger::Error("No manifest found for pack at %s, using defaults", directory.c_str());
        m_manifest.name = Path::GetFilename(directory);
        m_manifest.version = "1.0";
    }

    // Scan directory for files
    auto files = File::ListDirectoryRecursive(m_basePath);
    for (const auto& file : files) {
        std::string relativePath = file.substr(m_basePath.length());
        if (relativePath == "manifest.json") continue;

        // Read file into memory
        std::ifstream f(file, std::ios::binary);
        if (f) {
            std::vector<uint8_t> data((std::istreambuf_iterator<char>(f)),
                                     std::istreambuf_iterator<char>());
            m_files[relativePath] = std::move(data);
        }
    }

    m_loaded = true;
    Logger::Info("Loaded resource pack: %s (%zu files, priority %d)",
             m_manifest.name.c_str(), m_files.size(), m_manifest.priority);
    return true;
}

bool ResourcePack::LoadFromZip(const std::string& zipPath) {
    // Placeholder for ZIP loading
    // In a real implementation, use miniz or similar
    Logger::Info("ZIP loading not yet implemented: %s", zipPath.c_str());
    return false;
}

void ResourcePack::Unload() {
    m_files.clear();
    m_loaded = false;
}

bool ResourcePack::HasFile(const std::string& path) const {
    return m_files.find(path) != m_files.end();
}

std::vector<uint8_t> ResourcePack::ReadFile(const std::string& path) const {
    auto it = m_files.find(path);
    if (it != m_files.end()) {
        return it->second;
    }
    return {};
}

std::vector<std::string> ResourcePack::ListFiles(const std::string& pattern) const {
    std::vector<std::string> result;
    for (const auto& pair : m_files) {
        if (pattern == "*" || MatchesPattern(pair.first, pattern)) {
            result.push_back(pair.first);
        }
    }
    return result;
}

bool ResourcePack::ReplacesAsset(const std::string& assetPath) const {
    for (const auto& pattern : m_manifest.replaces) {
        if (MatchesPattern(assetPath, pattern)) {
            return true;
        }
    }
    return false;
}

size_t ResourcePack::GetFileCount() const {
    return m_files.size();
}

size_t ResourcePack::GetTotalSize() const {
    size_t total = 0;
    for (const auto& pair : m_files) {
        total += pair.second.size();
    }
    return total;
}

bool ResourcePack::LoadManifest(const std::string& manifestPath) {
    std::ifstream file(manifestPath);
    if (!file) return false;

    std::string line;
    while (std::getline(file, line)) {
        // Simple JSON parsing
        if (line.find("\"name\"") != std::string::npos) {
            size_t start = line.find('"', line.find(':'));
            size_t end = line.find('"', start + 1);
            if (start != std::string::npos && end != std::string::npos) {
                m_manifest.name = line.substr(start + 1, end - start - 1);
            }
        }
        else if (line.find("\"version\"") != std::string::npos) {
            size_t start = line.find('"', line.find(':'));
            size_t end = line.find('"', start + 1);
            if (start != std::string::npos && end != std::string::npos) {
                m_manifest.version = line.substr(start + 1, end - start - 1);
            }
        }
        else if (line.find("\"author\"") != std::string::npos) {
            size_t start = line.find('"', line.find(':'));
            size_t end = line.find('"', start + 1);
            if (start != std::string::npos && end != std::string::npos) {
                m_manifest.author = line.substr(start + 1, end - start - 1);
            }
        }
        else if (line.find("\"description\"") != std::string::npos) {
            size_t start = line.find('"', line.find(':'));
            size_t end = line.find('"', start + 1);
            if (start != std::string::npos && end != std::string::npos) {
                m_manifest.description = line.substr(start + 1, end - start - 1);
            }
        }
        else if (line.find("\"priority\"") != std::string::npos) {
            size_t start = line.find(':');
            if (start != std::string::npos) {
                m_manifest.priority = std::stoi(line.substr(start + 1));
            }
        }
        else if (line.find("\"replaces\"") != std::string::npos) {
            // Parse array
            size_t start = line.find('[');
            size_t end = line.find(']');
            if (start != std::string::npos && end != std::string::npos) {
                std::string arrayContent = line.substr(start + 1, end - start - 1);
                size_t pos = 0;
                while (pos < arrayContent.length()) {
                    size_t quoteStart = arrayContent.find('"', pos);
                    if (quoteStart == std::string::npos) break;
                    size_t quoteEnd = arrayContent.find('"', quoteStart + 1);
                    if (quoteEnd == std::string::npos) break;
                    m_manifest.replaces.push_back(arrayContent.substr(quoteStart + 1, quoteEnd - quoteStart - 1));
                    pos = quoteEnd + 1;
                }
            }
        }
    }

    return !m_manifest.name.empty();
}

bool ResourcePack::MatchesPattern(const std::string& path, const std::string& pattern) const {
    // Simple wildcard matching
    if (pattern == "*") return true;
    if (pattern.back() == '*') {
        std::string prefix = pattern.substr(0, pattern.length() - 1);
        return path.find(prefix) == 0;
    }
    return path == pattern;
}

// ResourcePackManager implementation
ResourcePackManager::ResourcePackManager() = default;
ResourcePackManager::~ResourcePackManager() { Shutdown(); }

bool ResourcePackManager::Initialize(AssetManager* assetManager) {
    m_assetManager = assetManager;
    Logger::Info("ResourcePackManager initialized");
    return true;
}

void ResourcePackManager::Shutdown() {
    UnloadAllPacks();
    m_assetManager = nullptr;
}

bool ResourcePackManager::LoadPackFromDirectory(const std::string& directory) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto pack = std::make_unique<ResourcePack>();
    if (!pack->LoadFromDirectory(directory)) {
        return false;
    }

    m_packs.push_back(std::move(pack));
    SortPacksByPriority();
    return true;
}

bool ResourcePackManager::LoadPackFromZip(const std::string& zipPath) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto pack = std::make_unique<ResourcePack>();
    if (!pack->LoadFromZip(zipPath)) {
        return false;
    }

    m_packs.push_back(std::move(pack));
    SortPacksByPriority();
    return true;
}

void ResourcePackManager::UnloadPack(const std::string& name) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_packs.erase(
        std::remove_if(m_packs.begin(), m_packs.end(),
            [&name](const std::unique_ptr<ResourcePack>& pack) {
                return pack->GetName() == name;
            }),
        m_packs.end());
}

void ResourcePackManager::UnloadAllPacks() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_packs.clear();
}

bool ResourcePackManager::HasAsset(const std::string& path) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (const auto& pack : m_packs) {
        if (pack->HasFile(path)) {
            return true;
        }
    }
    return false;
}

std::vector<uint8_t> ResourcePackManager::ReadAsset(const std::string& path) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (const auto& pack : m_packs) {
        if (pack->HasFile(path)) {
            return pack->ReadFile(path);
        }
    }
    return {};
}

ResourcePack* ResourcePackManager::FindAssetOwner(const std::string& path) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (const auto& pack : m_packs) {
        if (pack->HasFile(path)) {
            return pack.get();
        }
    }
    return nullptr;
}

size_t ResourcePackManager::GetPackCount() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_packs.size();
}

size_t ResourcePackManager::GetTotalAssetCount() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    size_t total = 0;
    for (const auto& pack : m_packs) {
        total += pack->GetFileCount();
    }
    return total;
}

void ResourcePackManager::PrintStats() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    Logger::Info("=== Resource Pack Stats ===");
    Logger::Info("Total packs: %zu", m_packs.size());
    for (const auto& pack : m_packs) {
        Logger::Info("  %s (priority %d): %zu files, %zu bytes",
                 pack->GetName().c_str(), pack->GetPriority(),
                 pack->GetFileCount(), pack->GetTotalSize());
    }
}

void ResourcePackManager::SortPacksByPriority() {
    std::sort(m_packs.begin(), m_packs.end(),
        [](const std::unique_ptr<ResourcePack>& a, const std::unique_ptr<ResourcePack>& b) {
            return a->GetPriority() > b->GetPriority();
        });
}

} // namespace vge