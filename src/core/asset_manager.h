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
#include "asset_base.h"

// Forward declaration
namespace vge { struct Texture; }

namespace vge {

// Forward declarations
class AssetImporter;
class ResourcePackManager;

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

// Asset manager - centralized loading and caching
class AssetManager {
public:
    AssetManager(const std::string& assetRoot = "assets/");
    ~AssetManager();

    bool Initialize();
    void Shutdown();

    // Synchronous loading - non-template for Texture
    Texture* LoadTexture(const std::string& id, const std::string& path);

    // Get cached asset
    Texture* GetTexture(const std::string& id);

    // Check if asset exists
    bool HasTexture(const std::string& id) const;

    // Async loading
    void LoadTextureAsync(const std::string& id, const std::string& path,
                          std::function<void(Texture*)> callback);

    // Generic by string ID
    void Unload(const std::string& id);
    void UnloadAll();
    bool Has(const std::string& id) const;

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
    bool DoLoadTexture(const std::string& id, const std::string& path);
    bool LoadModel(const std::string& id, const std::string& path);
    bool LoadSound(const std::string& id, const std::string& path);
    bool LoadShader(const std::string& id, const std::string& path);
    bool LoadFont(const std::string& id, const std::string& path);
    bool LoadMaterial(const std::string& id, const std::string& path);

    AssetType GetAssetTypeFromExtension(const std::string& path) const;
};

} // namespace vge
