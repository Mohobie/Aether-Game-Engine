#include "core/asset_manager.h"
#include "core/logger.h"
#include "rendering/texture.h"
#include <algorithm>
#include <thread>

namespace vge {

// FileWatcher implementation
FileWatcher::FileWatcher() = default;
FileWatcher::~FileWatcher() { StopWatching(); }

void FileWatcher::StartWatching(const std::string& directory) {
    (void)directory;
}

void FileWatcher::StopWatching() {
}

bool FileWatcher::HasChanges() {
    return false;
}

std::vector<std::string> FileWatcher::GetChangedFiles() {
    return std::vector<std::string>();
}

void FileWatcher::WatchLoop() {
}

AssetManager::AssetManager(const std::string& assetRoot)
    : m_assetRoot(assetRoot) {
}

AssetManager::~AssetManager() {
    Shutdown();
}

bool AssetManager::Initialize() {
    Logger::Info("AssetManager initialized");
    return true;
}

void AssetManager::Shutdown() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_assets.clear();
    Logger::Info("AssetManager shutdown");
}

Texture* AssetManager::LoadTexture(const std::string& id, const std::string& path) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_assets.find(id);
    if (it != m_assets.end()) {
        return static_cast<Texture*>(it->second.get());
    }
    
    std::string fullPath = m_assetRoot + path;
    Texture* texture = new Texture();
    
    if (!texture->LoadFromFile(fullPath)) {
        delete texture;
        Logger::Error("Failed to load texture: " + fullPath);
        return nullptr;
    }
    
    m_assets[id] = std::shared_ptr<Asset>(texture);
    Logger::Info("Loaded texture: " + id);
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

void AssetManager::LoadTextureAsync(const std::string& id, const std::string& path, std::function<void(Texture*)> callback) {
    std::thread([this, id, path, callback]() {
        Texture* texture = LoadTexture(id, path);
        if (callback) {
            callback(texture);
        }
    }).detach();
}

void AssetManager::Unload(const std::string& id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_assets.erase(id);
    Logger::Info("Unloaded asset: " + id);
}

void AssetManager::UnloadAll() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_assets.clear();
    Logger::Info("All assets unloaded");
}

bool AssetManager::Has(const std::string& id) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_assets.find(id) != m_assets.end();
}

void AssetManager::EnableHotReload(bool enable) {
    (void)enable;
}

void AssetManager::UpdateHotReload() {
}

void AssetManager::SetPackManager(ResourcePackManager* packManager) {
    m_packManager = packManager;
}

size_t AssetManager::GetLoadedAssetCount() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_assets.size();
}

void AssetManager::PrintStats() const {
    Logger::Info("Loaded assets: " + std::to_string(GetLoadedAssetCount()));
}

void AssetManager::GenerateDefaults() {
    Logger::Info("Generating default assets");
}

void AssetManager::AsyncWorkerLoop() {
}

bool AssetManager::DoLoadTexture(const std::string& id, const std::string& path) {
    return LoadTexture(id, path) != nullptr;
}

bool AssetManager::LoadModel(const std::string& id, const std::string& path) {
    (void)id;
    (void)path;
    return false;
}

bool AssetManager::LoadSound(const std::string& id, const std::string& path) {
    (void)id;
    (void)path;
    return false;
}

bool AssetManager::LoadShader(const std::string& id, const std::string& path) {
    (void)id;
    (void)path;
    return false;
}

bool AssetManager::LoadFont(const std::string& id, const std::string& path) {
    (void)id;
    (void)path;
    return false;
}

bool AssetManager::LoadMaterial(const std::string& id, const std::string& path) {
    (void)id;
    (void)path;
    return false;
}

AssetType AssetManager::GetAssetTypeFromExtension(const std::string& path) const {
    std::string ext = path.substr(path.find_last_of('.') + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    if (ext == "png" || ext == "jpg" || ext == "jpeg" || ext == "bmp" || ext == "tga") {
        return AssetType::Texture;
    } else if (ext == "obj" || ext == "fbx" || ext == "gltf" || ext == "glb") {
        return AssetType::Model;
    } else if (ext == "wav" || ext == "ogg" || ext == "mp3") {
        return AssetType::Sound;
    } else if (ext == "glsl" || ext == "vert" || ext == "frag") {
        return AssetType::Shader;
    } else if (ext == "ttf" || ext == "otf") {
        return AssetType::Font;
    } else if (ext == "json" || ext == "mat") {
        return AssetType::Material;
    }
    return AssetType::Unknown;
}

} // namespace vge
