#include "core/resource_pack.h"
#include "core/logger.h"
#include "platform/platform_common.h"
#include <algorithm>
#include <fstream>
#include <sstream>

namespace vge {

// ============ ResourcePack ============

ResourcePack::ResourcePack()
    : m_loaded(false) {
}

ResourcePack::~ResourcePack() {
    Unload();
}

bool ResourcePack::LoadFromDirectory(const std::string& directory) {
    m_basePath = directory;
    m_manifest.name = Path::GetFilename(directory);
    m_manifest.version = "1.0";
    m_manifest.author = "Unknown";
    m_manifest.description = "Resource pack";
    m_manifest.priority = 0;
    
    std::string manifestPath = directory + "/pack.json";
    if (File::Exists(manifestPath)) {
        LoadManifest(manifestPath);
    }
    
    auto files = File::ListDirectoryRecursive(directory);
    for (const auto& file : files) {
        // Skip directories
        if (File::IsDirectory(file)) {
            continue;
        }
        std::string relativePath = file.substr(directory.length() + 1);
        std::ifstream fs(file, std::ios::binary);
        if (fs) {
            std::vector<uint8_t> data((std::istreambuf_iterator<char>(fs)),
                                       std::istreambuf_iterator<char>());
            m_files[relativePath] = std::move(data);
        }
    }
    
    m_loaded = true;
    Logger::Info("Resource pack loaded: " + m_manifest.name);
    return true;
}

bool ResourcePack::LoadFromZip(const std::string& zipPath) {
    (void)zipPath;
    m_manifest.name = Path::GetFilename(zipPath);
    m_manifest.version = "1.0";
    m_loaded = true;
    Logger::Info("Resource pack loaded from zip: " + m_manifest.name);
    return true;
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
    return std::vector<uint8_t>();
}

std::vector<std::string> ResourcePack::ListFiles(const std::string& pattern) const {
    (void)pattern;
    std::vector<std::string> result;
    for (const auto& pair : m_files) {
        result.push_back(pair.first);
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
    (void)manifestPath;
    return true;
}

bool ResourcePack::MatchesPattern(const std::string& path, const std::string& pattern) const {
    if (pattern == "*" || pattern == "*.*") {
        return true;
    }
    if (pattern.find('*') != std::string::npos) {
        std::string prefix = pattern.substr(0, pattern.find('*'));
        return path.find(prefix) == 0;
    }
    return path == pattern;
}

// ============ ResourcePackManager ============

ResourcePackManager::ResourcePackManager()
    : m_assetManager(nullptr) {
}

ResourcePackManager::~ResourcePackManager() {
    Shutdown();
}

bool ResourcePackManager::Initialize(AssetManager* assetManager) {
    m_assetManager = assetManager;
    Logger::Info("ResourcePackManager initialized");
    return true;
}

void ResourcePackManager::Shutdown() {
    UnloadAllPacks();
    m_assetManager = nullptr;
    Logger::Info("ResourcePackManager shutdown");
}

bool ResourcePackManager::LoadPackFromDirectory(const std::string& directory) {
    auto pack = std::make_unique<ResourcePack>();
    if (!pack->LoadFromDirectory(directory)) {
        Logger::Error("Failed to load pack from: " + directory);
        return false;
    }
    
    std::lock_guard<std::mutex> lock(m_mutex);
    m_packs.push_back(std::move(pack));
    SortPacksByPriority();
    Logger::Info("Pack loaded from directory: " + directory);
    return true;
}

bool ResourcePackManager::LoadPackFromZip(const std::string& zipPath) {
    auto pack = std::make_unique<ResourcePack>();
    if (!pack->LoadFromZip(zipPath)) {
        Logger::Error("Failed to load pack from zip: " + zipPath);
        return false;
    }
    
    std::lock_guard<std::mutex> lock(m_mutex);
    m_packs.push_back(std::move(pack));
    SortPacksByPriority();
    Logger::Info("Pack loaded from zip: " + zipPath);
    return true;
}

void ResourcePackManager::UnloadPack(const std::string& name) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = std::remove_if(m_packs.begin(), m_packs.end(),
        [&name](const std::unique_ptr<ResourcePack>& pack) {
            return pack->GetName() == name;
        });
    m_packs.erase(it, m_packs.end());
    Logger::Info("Pack unloaded: " + name);
}

void ResourcePackManager::UnloadAllPacks() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_packs.clear();
    Logger::Info("All packs unloaded");
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
    return std::vector<uint8_t>();
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
    Logger::Info("Resource packs: " + std::to_string(GetPackCount()));
    Logger::Info("Total assets: " + std::to_string(GetTotalAssetCount()));
}

void ResourcePackManager::SortPacksByPriority() {
    std::sort(m_packs.begin(), m_packs.end(),
        [](const std::unique_ptr<ResourcePack>& a, const std::unique_ptr<ResourcePack>& b) {
            return a->GetPriority() > b->GetPriority();
        });
}

} // namespace vge
