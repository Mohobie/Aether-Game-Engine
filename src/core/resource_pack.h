#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <cstdint>

namespace vge {

// Pack manifest
struct PackManifest {
    std::string name;
    std::string version;
    std::string author;
    std::string description;
    int priority = 0;
    std::vector<std::string> replaces;
};

// Resource pack - loads assets from directory or archive
class ResourcePack {
public:
    ResourcePack();
    ~ResourcePack();

    // Load from directory
    bool LoadFromDirectory(const std::string& directory);

    // Load from ZIP file
    bool LoadFromZip(const std::string& zipPath);

    // Unload
    void Unload();

    // File access
    bool HasFile(const std::string& path) const;
    std::vector<uint8_t> ReadFile(const std::string& path) const;
    std::vector<std::string> ListFiles(const std::string& pattern = "*") const;

    // Queries
    bool ReplacesAsset(const std::string& assetPath) const;
    int GetPriority() const { return m_manifest.priority; }
    const PackManifest& GetManifest() const { return m_manifest; }
    std::string GetName() const { return m_manifest.name; }
    size_t GetFileCount() const;
    size_t GetTotalSize() const;

private:
    PackManifest m_manifest;
    std::string m_basePath;
    std::unordered_map<std::string, std::vector<uint8_t>> m_files;
    bool m_loaded = false;

    bool LoadManifest(const std::string& manifestPath);
    bool MatchesPattern(const std::string& path, const std::string& pattern) const;
};

// Resource pack manager - handles multiple packs with priority
class ResourcePackManager {
public:
    ResourcePackManager();
    ~ResourcePackManager();

    bool Initialize(class AssetManager* assetManager);
    void Shutdown();

    // Load packs
    bool LoadPackFromDirectory(const std::string& directory);
    bool LoadPackFromZip(const std::string& zipPath);

    // Unload packs
    void UnloadPack(const std::string& name);
    void UnloadAllPacks();

    // Asset resolution
    bool HasAsset(const std::string& path) const;
    std::vector<uint8_t> ReadAsset(const std::string& path) const;
    ResourcePack* FindAssetOwner(const std::string& path) const;

    // Queries
    size_t GetPackCount() const;
    size_t GetTotalAssetCount() const;
    void PrintStats() const;

private:
    AssetManager* m_assetManager = nullptr;
    std::vector<std::unique_ptr<ResourcePack>> m_packs;
    mutable std::mutex m_mutex;

    void SortPacksByPriority();
};

} // namespace vge
