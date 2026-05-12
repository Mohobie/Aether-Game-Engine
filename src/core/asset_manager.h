#pragma once

#include <string>
#include <unordered_map>
#include <functional>
#include <memory>
#include <vector>
#include <mutex>
#include <thread>
#include <atomic>
#include <queue>
#include <condition_variable>
#include "types.h"
#include "rendering/texture.h"

namespace vge {

// Forward declarations
class AssetImporter;
class ResourcePackManager;

// Base asset type enum
enum class AssetType {
    Unknown,
    Texture,
    Model,
    Sound,
    Shader,
    Font,
    Material,
    Count
};

// Asset metadata
struct AssetMetadata {
    std::string id;
    std::string path;
    AssetType type;
    size_t size = 0;
    uint64_t lastModified = 0;
    uint32_t refCount = 0;
    bool loaded = false;
    bool fromPack = false;
};

// Async load request
struct AsyncLoadRequest {
    std::string id;
    std::string path;
    AssetType type;
    std::function<void(bool)> callback;
};

// File watcher for hot-reload
class FileWatcher {
public:
    FileWatcher();
    ~FileWatcher();

    void StartWatching(const std::string& directory);
    void StopWatching();
    bool HasChanges();
    std::vector<std::string> GetChangedFiles();

private:
    void WatchLoop();

    std::string m_watchDir;
    std::atomic<bool> m_running{false};
    std::thread m_watchThread;
    std::mutex m_mutex;
    std::vector<std::string> m_changedFiles;
    std::unordered_map<std::string, uint64_t> m_fileTimestamps;
};

// Base asset class
class Asset {
public:
    virtual ~Asset() = default;
    AssetMetadata metadata;
};

// Asset manager - centralized loading and caching
class AssetManager {
public:
    AssetManager(const std::string& assetRoot = "assets/");
    ~AssetManager();

    bool Initialize();
    void Shutdown();

    // Synchronous loading
    template<typename T>
    T* Load(const std::string& id, const std::string& path);

    template<typename T>
    T* Get(const std::string& id);

    template<typename T>
    bool Has(const std::string& id) const;

    // Async loading
    template<typename T>
    void LoadAsync(const std::string& id, const std::string& path,
                   std::function<void(T*)> callback);

    // Unloading
    void Unload(const std::string& id);
    void UnloadAll();

    // Hot-reload
    void EnableHotReload(bool enable);
    void UpdateHotReload();

    // Resource packs
    void SetPackManager(ResourcePackManager* packManager);

    // Stats
    size_t GetLoadedAssetCount() const;
    void PrintStats() const;

    // Default assets
    void GenerateDefaults();

private:
    std::string m_assetRoot;
    std::unordered_map<std::string, std::shared_ptr<Asset>> m_assets;
    mutable std::mutex m_mutex;

    // Async loading
    std::queue<AsyncLoadRequest> m_asyncQueue;
    std::mutex m_asyncMutex;
    std::condition_variable m_asyncCV;
    std::vector<std::thread> m_asyncWorkers;
    std::atomic<bool> m_asyncRunning{false};

    // Hot-reload
    std::unique_ptr<FileWatcher> m_fileWatcher;
    bool m_hotReloadEnabled = false;

    // Resource packs
    ResourcePackManager* m_packManager = nullptr;

    void AsyncWorkerLoop();
    bool LoadTexture(const std::string& id, const std::string& path);
    bool LoadModel(const std::string& id, const std::string& path);
    bool LoadSound(const std::string& id, const std::string& path);
    bool LoadShader(const std::string& id, const std::string& path);
    bool LoadFont(const std::string& id, const std::string& path);
    bool LoadMaterial(const std::string& id, const std::string& path);

    AssetType GetAssetTypeFromExtension(const std::string& path) const;
};

// Template implementations

// Load asset by ID and path
inline Texture* AssetManager::Load<Texture>(const std::string& id, const std::string& path) {
    std::lock_guard<std::mutex> lock(m_mutex);

    // Check cache first
    auto it = m_assets.find(id);
    if (it != m_assets.end()) {
        it->second->metadata.refCount++;
        return static_cast<Texture*>(it->second.get());
    }

    // Load from file
    std::string fullPath = m_assetRoot + path;
    Texture* texture = new Texture();

    // Try to load - for now create a placeholder
    // In a real implementation, this would call stb_image or similar
    texture->width = 64;
    texture->height = 64;
    texture->channels = 4;
    texture->data.resize(64 * 64 * 4, 255);

    texture->metadata.id = id;
    texture->metadata.path = path;
    texture->metadata.type = AssetType::Texture;
    texture->metadata.loaded = true;
    texture->metadata.refCount = 1;

    m_assets[id] = std::shared_ptr<Asset>(texture);
    return texture;
}

// Get cached asset
inline Texture* AssetManager::Get<Texture>(const std::string& id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_assets.find(id);
    if (it != m_assets.end()) {
        return static_cast<Texture*>(it->second.get());
    }
    return nullptr;
}

// Check if asset exists
inline bool AssetManager::Has<Texture>(const std::string& id) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_assets.find(id) != m_assets.end();
}

} // namespace vge
