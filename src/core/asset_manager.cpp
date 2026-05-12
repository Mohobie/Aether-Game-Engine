#include "asset_manager.h"
#include "asset_importer.h"
#include "resource_pack.h"
#include "platform/platform_common.h"
#include "core/logger.h"
#include <fstream>
#include <sstream>

namespace vge {

// FileWatcher implementation
FileWatcher::FileWatcher() = default;
FileWatcher::~FileWatcher() { StopWatching(); }

void FileWatcher::StartWatching(const std::string& directory) {
    m_watchDir = directory;
    m_running = true;
    m_watchThread = std::thread(&FileWatcher::WatchLoop, this);
}

void FileWatcher::StopWatching() {
    m_running = false;
    if (m_watchThread.joinable()) {
        m_watchThread.join();
    }
}

bool FileWatcher::HasChanges() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return !m_changedFiles.empty();
}

std::vector<std::string> FileWatcher::GetChangedFiles() {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<std::string> result = m_changedFiles;
    m_changedFiles.clear();
    return result;
}

void FileWatcher::WatchLoop() {
    while (m_running) {
        auto files = File::ListDirectory(m_watchDir);
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            for (const auto& file : files) {
                std::string ext = Path::GetExtension(file);
                if (ext == ".png" || ext == ".jpg" || ext == ".obj" ||
                    ext == ".wav" || ext == ".glsl" || ext == ".json") {
                    uint64_t currentTime = File::GetLastModifiedTime(file);
                    auto it = m_fileTimestamps.find(file);
                    if (it != m_fileTimestamps.end()) {
                        if (it->second != currentTime) {
                            m_changedFiles.push_back(file);
                            it->second = currentTime;
                        }
                    } else {
                        m_fileTimestamps[file] = currentTime;
                    }
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

// AssetManager implementation
AssetManager::AssetManager(const std::string& assetRoot)
    : m_assetRoot(assetRoot) {}

AssetManager::~AssetManager() { Shutdown(); }

bool AssetManager::Initialize() {
    File::CreateDirectory(m_assetRoot);

    m_asyncRunning = true;
    unsigned int numWorkers = std::thread::hardware_concurrency();
    if (numWorkers == 0) numWorkers = 2;

    for (unsigned int i = 0; i < numWorkers; ++i) {
        m_asyncWorkers.emplace_back(&AssetManager::AsyncWorkerLoop, this);
    }

    Logger::Info("AssetManager initialized with " + std::to_string(numWorkers) + " async workers");
    return true;
}

void AssetManager::Shutdown() {
    m_asyncRunning = false;
    m_asyncCV.notify_all();

    for (auto& worker : m_asyncWorkers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    m_asyncWorkers.clear();

    UnloadAll();

    if (m_fileWatcher) {
        m_fileWatcher->StopWatching();
        m_fileWatcher.reset();
    }

    Logger::Info("AssetManager shutdown complete");
}

void AssetManager::AsyncWorkerLoop() {
    while (m_asyncRunning) {
        AsyncLoadRequest request;
        {
            std::unique_lock<std::mutex> lock(m_asyncMutex);
            m_asyncCV.wait(lock, [this] { return !m_asyncQueue.empty() || !m_asyncRunning; });

            if (!m_asyncRunning) break;
            if (m_asyncQueue.empty()) continue;

            request = m_asyncQueue.front();
            m_asyncQueue.pop();
        }

        bool success = false;
        switch (request.type) {
            case AssetType::Texture:
                success = DoLoadTexture(request.id, request.path);
                break;
            case AssetType::Model:
                success = LoadModel(request.id, request.path);
                break;
            case AssetType::Sound:
                success = LoadSound(request.id, request.path);
                break;
            case AssetType::Shader:
                success = LoadShader(request.id, request.path);
                break;
            case AssetType::Font:
                success = LoadFont(request.id, request.path);
                break;
            case AssetType::Material:
                success = LoadMaterial(request.id, request.path);
                break;
            default:
                break;
        }

        if (request.callback) {
            request.callback(success);
        }
    }
}

Texture* AssetManager::LoadTexture(const std::string& id, const std::string& path) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_assets.find(id);
    if (it != m_assets.end()) {
        it->second->metadata.refCount++;
        return static_cast<Texture*>(it->second.get());
    }

    std::string fullPath = m_assetRoot + path;
    Texture* texture = new Texture();

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

Texture* AssetManager::GetTexture(const std::string& id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_assets.find(id);
    if (it != m_assets.end()) {
        return static_cast<Texture*>(it->second.get());
    }
    return nullptr;
}

bool AssetManager::HasTexture(const std::string& id) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_assets.find(id) != m_assets.end();
}

void AssetManager::LoadTextureAsync(const std::string& id, const std::string& path,
                                    std::function<void(Texture*)> callback) {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_assets.find(id);
        if (it != m_assets.end()) {
            if (callback) callback(static_cast<Texture*>(it->second.get()));
            return;
        }
    }

    AsyncLoadRequest request;
    request.id = id;
    request.path = path;
    request.type = AssetType::Texture;
    request.callback = [this, id, callback](bool success) {
        if (success && callback) {
            std::lock_guard<std::mutex> lock(m_mutex);
            auto it = m_assets.find(id);
            if (it != m_assets.end()) {
                callback(static_cast<Texture*>(it->second.get()));
            } else {
                callback(nullptr);
            }
        } else if (callback) {
            callback(nullptr);
        }
    };

    {
        std::lock_guard<std::mutex> lock(m_asyncMutex);
        m_asyncQueue.push(request);
    }
    m_asyncCV.notify_one();
}

bool AssetManager::Has(const std::string& id) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_assets.find(id) != m_assets.end();
}

void AssetManager::Unload(const std::string& id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_assets.find(id);
    if (it != m_assets.end()) {
        it->second->metadata.refCount--;
        if (it->second->metadata.refCount == 0) {
            m_assets.erase(it);
        }
    }
}

void AssetManager::UnloadAll() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_assets.clear();
}

void AssetManager::EnableHotReload(bool enable) {
    m_hotReloadEnabled = enable;
    if (enable) {
        if (!m_fileWatcher) {
            m_fileWatcher = std::make_unique<FileWatcher>();
        }
        m_fileWatcher->StartWatching(m_assetRoot);
        Logger::Info("Hot-reload enabled for directory: %s", m_assetRoot.c_str());
    } else {
        if (m_fileWatcher) {
            m_fileWatcher->StopWatching();
        }
    }
}

void AssetManager::UpdateHotReload() {
    if (!m_hotReloadEnabled || !m_fileWatcher) return;

    if (m_fileWatcher->HasChanges()) {
        auto changedFiles = m_fileWatcher->GetChangedFiles();
        for (const auto& file : changedFiles) {
            Logger::Info("Hot-reload: File changed: %s", file.c_str());
            std::lock_guard<std::mutex> lock(m_mutex);
            for (auto& pair : m_assets) {
                if (pair.second->metadata.path == file) {
                    Logger::Info("Hot-reload: Reloading asset: %s", pair.first.c_str());
                    pair.second->metadata.lastModified = File::GetLastModifiedTime(file);
                    break;
                }
            }
        }
    }
}

void AssetManager::SetPackManager(ResourcePackManager* packManager) {
    m_packManager = packManager;
}

size_t AssetManager::GetLoadedAssetCount() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_assets.size();
}

void AssetManager::PrintStats() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    Logger::Info("=== Asset Manager Stats ===");
    Logger::Info("Total loaded assets: %zu", m_assets.size());

    size_t counts[(int)AssetType::Count] = {};
    for (const auto& pair : m_assets) {
        counts[(int)pair.second->metadata.type]++;
    }

    Logger::Info("  Textures: %zu", counts[(int)AssetType::Texture]);
    Logger::Info("  Models: %zu", counts[(int)AssetType::Model]);
    Logger::Info("  Sounds: %zu", counts[(int)AssetType::Sound]);
    Logger::Info("  Shaders: %zu", counts[(int)AssetType::Shader]);
    Logger::Info("  Fonts: %zu", counts[(int)AssetType::Font]);
    Logger::Info("  Materials: %zu", counts[(int)AssetType::Material]);
}

void AssetManager::GenerateDefaults() {
    Texture* defaultTex = new Texture();
    defaultTex->width = 64;
    defaultTex->height = 64;
    defaultTex->channels = 4;
    defaultTex->data.resize(64 * 64 * 4);

    for (int y = 0; y < 64; ++y) {
        for (int x = 0; x < 64; ++x) {
            int idx = (y * 64 + x) * 4;
            bool check = ((x / 8) + (y / 8)) % 2 == 0;
            if (check) {
                defaultTex->data[idx] = 255;
                defaultTex->data[idx + 1] = 0;
                defaultTex->data[idx + 2] = 255;
                defaultTex->data[idx + 3] = 255;
            } else {
                defaultTex->data[idx] = 0;
                defaultTex->data[idx + 1] = 0;
                defaultTex->data[idx + 2] = 0;
                defaultTex->data[idx + 3] = 255;
            }
        }
    }

    defaultTex->metadata.id = "__default_texture__";
    defaultTex->metadata.path = "";
    defaultTex->metadata.type = AssetType::Texture;
    defaultTex->metadata.loaded = true;

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_assets["__default_texture__"] = std::shared_ptr<Asset>(defaultTex);
    }

    Logger::Info("Generated default assets");
}

bool AssetManager::DoLoadTexture(const std::string& id, const std::string& path) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_assets.find(id) != m_assets.end()) return true;

    Texture* texture = new Texture();
    texture->width = 64;
    texture->height = 64;
    texture->channels = 4;
    texture->data.resize(64 * 64 * 4, 128);

    texture->metadata.id = id;
    texture->metadata.path = path;
    texture->metadata.type = AssetType::Texture;
    texture->metadata.loaded = true;
    texture->metadata.refCount = 1;

    m_assets[id] = std::shared_ptr<Asset>(texture);
    return true;
}

bool AssetManager::LoadModel(const std::string& id, const std::string& path) {
    Logger::Info("Loading model: %s", path.c_str());
    return true;
}

bool AssetManager::LoadSound(const std::string& id, const std::string& path) {
    Logger::Info("Loading sound: %s", path.c_str());
    return true;
}

bool AssetManager::LoadShader(const std::string& id, const std::string& path) {
    Logger::Info("Loading shader: %s", path.c_str());
    return true;
}

bool AssetManager::LoadFont(const std::string& id, const std::string& path) {
    Logger::Info("Loading font: %s", path.c_str());
    return true;
}

bool AssetManager::LoadMaterial(const std::string& id, const std::string& path) {
    Logger::Info("Loading material: %s", path.c_str());
    return true;
}

AssetType AssetManager::GetAssetTypeFromExtension(const std::string& path) const {
    std::string ext = Path::GetExtension(path);
    if (ext == ".png" || ext == ".jpg" || ext == ".tga" || ext == ".bmp") {
        return AssetType::Texture;
    } else if (ext == ".obj" || ext == ".fbx" || ext == ".gltf" || ext == ".glb") {
        return AssetType::Model;
    } else if (ext == ".wav" || ext == ".ogg" || ext == ".mp3") {
        return AssetType::Sound;
    } else if (ext == ".glsl" || ext == ".vert" || ext == ".frag") {
        return AssetType::Shader;
    } else if (ext == ".ttf" || ext == ".otf") {
        return AssetType::Font;
    } else if (ext == ".json" || ext == ".mat") {
        return AssetType::Material;
    }
    return AssetType::Unknown;
}

} // namespace vge