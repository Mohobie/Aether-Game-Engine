#include "editor/asset_browser.h"
#include "core/logger.h"
#include <algorithm>
#include <filesystem>

namespace vge {

// ============================================
// Asset Filter Implementation
// ============================================

bool AssetFilter::Matches(const AssetInfo& asset) const {
    // Search query
    if (!searchQuery.empty()) {
        std::string lowerQuery = searchQuery;
        std::string lowerName = asset.name;
        std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);
        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
        
        if (lowerName.find(lowerQuery) == std::string::npos) {
            return false;
        }
    }
    
    // Type filter
    if (typeFilter != AssetType::Unknown && asset.type != typeFilter) {
        return false;
    }
    
    // Tags
    if (!tags.empty()) {
        bool hasTag = false;
        for (const auto& tag : tags) {
            if (std::find(asset.tags.begin(), asset.tags.end(), tag) != asset.tags.end()) {
                hasTag = true;
                break;
            }
        }
        if (!hasTag) return false;
    }
    
    // Favorites
    if (showFavoritesOnly && !asset.isFavorite) {
        return false;
    }
    
    // Directories
    if (!showDirectories && asset.isDirectory) {
        return false;
    }
    
    return true;
}

// ============================================
// Asset Preview Implementation
// ============================================

AssetPreview::AssetPreview() : textureID(0), width(0), height(0), loaded(false) {}

AssetPreview::~AssetPreview() {
    Unload();
}

bool AssetPreview::LoadFromFile(const std::string& path) {
    (void)path;
    // Would load texture from file using stb_image or similar
    // For now, just mark as not loaded
    loaded = false;
    return false;
}

bool AssetPreview::LoadFromAsset(const AssetInfo& asset) {
    if (!asset.thumbnailPath.empty()) {
        return LoadFromFile(asset.thumbnailPath);
    }
    return false;
}

void AssetPreview::Unload() {
    if (textureID != 0) {
        // glDeleteTextures(1, &textureID);
        textureID = 0;
    }
    loaded = false;
}

// ============================================
// Asset Browser Implementation
// ============================================

AssetBrowser::AssetBrowser()
    : currentPath(""), rootPath(""), selectedIndex(-1)
    , gridView(true), gridSize(64), showPreview(true), showDetails(false) {}

AssetBrowser::~AssetBrowser() {
    Shutdown();
}

void AssetBrowser::Initialize(const std::string& rootPath) {
    this->rootPath = rootPath;
    this->currentPath = rootPath;
    
    ScanDirectory(currentPath);
    
    Logger::Info("[AssetBrowser] Initialized with root: " + rootPath);
}

void AssetBrowser::Shutdown() {
    assets.clear();
    filteredAssets.clear();
    previews.clear();
}

// ============================================
// Navigation
// ============================================

void AssetBrowser::SetPath(const std::string& path) {
    currentPath = path;
    ScanDirectory(currentPath);
}

void AssetBrowser::GoUp() {
    if (CanGoUp()) {
        std::filesystem::path p(currentPath);
        SetPath(p.parent_path().string());
    }
}

void AssetBrowser::GoToRoot() {
    SetPath(rootPath);
}

bool AssetBrowser::CanGoUp() const {
    return currentPath != rootPath && currentPath.length() > rootPath.length();
}

// ============================================
// Refresh
// ============================================

void AssetBrowser::Refresh() {
    ScanDirectory(currentPath);
}

void AssetBrowser::ScanDirectory(const std::string& path) {
    assets.clear();
    
    if (!std::filesystem::exists(path)) {
        Logger::Error("[AssetBrowser] Directory does not exist: " + path);
        return;
    }
    
    try {
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            AssetInfo info;
            info.path = entry.path().string();
            info.name = entry.path().filename().string();
            info.isDirectory = entry.is_directory();
            
            if (entry.is_regular_file()) {
                info.fileSize = entry.file_size();
                info.lastModified = "unknown"; // Would use file time
                
                // Determine type from extension
                std::string ext = entry.path().extension().string();
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                
                if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tga") {
                    info.type = AssetType::Texture;
                } else if (ext == ".obj" || ext == ".fbx" || ext == ".gltf" || ext == ".glb") {
                    info.type = AssetType::Model;
                } else if (ext == ".wav" || ext == ".mp3" || ext == ".ogg") {
                    info.type = AssetType::Audio;
                } else if (ext == ".lua" || ext == ".js" || ext == ".py") {
                    info.type = AssetType::Script;
                } else if (ext == ".json") {
                    info.type = AssetType::Prefab;
                } else if (ext == ".scene") {
                    info.type = AssetType::Scene;
                } else if (ext == ".vert" || ext == ".frag" || ext == ".glsl") {
                    info.type = AssetType::Shader;
                } else {
                    info.type = AssetType::Unknown;
                }
            } else if (entry.is_directory()) {
                info.type = AssetType::Unknown;
            }
            
            info.id = info.path;
            assets.push_back(info);
        }
        
        // Sort: directories first, then by name
        std::sort(assets.begin(), assets.end(), [](const AssetInfo& a, const AssetInfo& b) {
            if (a.isDirectory != b.isDirectory) {
                return a.isDirectory > b.isDirectory;
            }
            return a.name < b.name;
        });
        
    } catch (const std::exception& e) {
        Logger::Error("[AssetBrowser] Error scanning directory: " + std::string(e.what()));
    }
    
    ApplyFilter();
}

// ============================================
// Filtering
// ============================================

void AssetBrowser::SetSearchQuery(const std::string& query) {
    filter.searchQuery = query;
    ApplyFilter();
}

void AssetBrowser::SetTypeFilter(AssetType type) {
    filter.typeFilter = type;
    ApplyFilter();
}

void AssetBrowser::SetShowFavoritesOnly(bool show) {
    filter.showFavoritesOnly = show;
    ApplyFilter();
}

void AssetBrowser::ClearFilter() {
    filter = AssetFilter();
    ApplyFilter();
}

void AssetBrowser::ApplyFilter() {
    filteredAssets.clear();
    
    for (const auto& asset : assets) {
        if (filter.Matches(asset)) {
            filteredAssets.push_back(asset);
        }
    }
    
    // Clear selection if no longer valid
    if (selectedIndex >= (int)filteredAssets.size()) {
        selectedIndex = -1;
    }
}

// ============================================
// Selection
// ============================================

void AssetBrowser::SelectAsset(int index) {
    if (index >= 0 && index < (int)filteredAssets.size()) {
        selectedIndex = index;
        selectedIndices.clear();
        selectedIndices.push_back(index);
        
        if (onAssetSelected) {
            onAssetSelected(filteredAssets[index]);
        }
    }
}

void AssetBrowser::SelectAsset(const std::string& id) {
    for (size_t i = 0; i < filteredAssets.size(); ++i) {
        if (filteredAssets[i].id == id) {
            SelectAsset((int)i);
            return;
        }
    }
}

void AssetBrowser::MultiSelect(int index, bool addToSelection) {
    if (index < 0 || index >= (int)filteredAssets.size()) return;
    
    if (!addToSelection) {
        selectedIndices.clear();
    }
    
    auto it = std::find(selectedIndices.begin(), selectedIndices.end(), index);
    if (it != selectedIndices.end()) {
        selectedIndices.erase(it);
    } else {
        selectedIndices.push_back(index);
    }
    
    if (!selectedIndices.empty()) {
        selectedIndex = selectedIndices.back();
    } else {
        selectedIndex = -1;
    }
}

void AssetBrowser::ClearSelection() {
    selectedIndex = -1;
    selectedIndices.clear();
}

AssetInfo* AssetBrowser::GetSelectedAsset() {
    if (selectedIndex >= 0 && selectedIndex < (int)filteredAssets.size()) {
        return &filteredAssets[selectedIndex];
    }
    return nullptr;
}

std::vector<AssetInfo*> AssetBrowser::GetSelectedAssets() {
    std::vector<AssetInfo*> result;
    for (int idx : selectedIndices) {
        if (idx >= 0 && idx < (int)filteredAssets.size()) {
            result.push_back(&filteredAssets[idx]);
        }
    }
    return result;
}

// ============================================
// Asset Operations
// ============================================

bool AssetBrowser::ImportAsset(const std::string& sourcePath) {
    try {
        std::filesystem::path src(sourcePath);
        std::filesystem::path dst(currentPath + "/" + src.filename().string());
        
        std::filesystem::copy_file(src, dst, std::filesystem::copy_options::overwrite_existing);
        
        Refresh();
        
        Logger::Info("[AssetBrowser] Imported: " + src.filename().string());
        return true;
    } catch (const std::exception& e) {
        Logger::Error("[AssetBrowser] Import failed: " + std::string(e.what()));
        return false;
    }
}

bool AssetBrowser::DeleteAsset(const std::string& id) {
    try {
        if (std::filesystem::exists(id)) {
            if (std::filesystem::is_directory(id)) {
                std::filesystem::remove_all(id);
            } else {
                std::filesystem::remove(id);
            }
            
            Refresh();
            Logger::Info("[AssetBrowser] Deleted: " + id);
            return true;
        }
    } catch (const std::exception& e) {
        Logger::Error("[AssetBrowser] Delete failed: " + std::string(e.what()));
    }
    return false;
}

bool AssetBrowser::RenameAsset(const std::string& id, const std::string& newName) {
    try {
        std::filesystem::path oldPath(id);
        std::filesystem::path newPath = oldPath.parent_path() / newName;
        
        std::filesystem::rename(oldPath, newPath);
        
        Refresh();
        Logger::Info("[AssetBrowser] Renamed to: " + newName);
        return true;
    } catch (const std::exception& e) {
        Logger::Error("[AssetBrowser] Rename failed: " + std::string(e.what()));
        return false;
    }
}

bool AssetBrowser::DuplicateAsset(const std::string& id) {
    try {
        std::filesystem::path path(id);
        std::string stem = path.stem().string();
        std::string ext = path.extension().string();
        std::string newName = stem + "_copy" + ext;
        std::filesystem::path newPath = path.parent_path() / newName;
        
        if (std::filesystem::is_directory(path)) {
            std::filesystem::copy(path, newPath, std::filesystem::copy_options::recursive);
        } else {
            std::filesystem::copy_file(path, newPath);
        }
        
        Refresh();
        Logger::Info("[AssetBrowser] Duplicated: " + newName);
        return true;
    } catch (const std::exception& e) {
        Logger::Error("[AssetBrowser] Duplicate failed: " + std::string(e.what()));
        return false;
    }
}

void AssetBrowser::ToggleFavorite(const std::string& id) {
    for (auto& asset : assets) {
        if (asset.id == id) {
            asset.isFavorite = !asset.isFavorite;
            break;
        }
    }
    
    for (auto& asset : filteredAssets) {
        if (asset.id == id) {
            asset.isFavorite = !asset.isFavorite;
            break;
        }
    }
}

// ============================================
// Get Assets
// ============================================

AssetInfo* AssetBrowser::GetAsset(int index) {
    if (index >= 0 && index < (int)filteredAssets.size()) {
        return &filteredAssets[index];
    }
    return nullptr;
}

AssetInfo* AssetBrowser::GetAsset(const std::string& id) {
    for (auto& asset : filteredAssets) {
        if (asset.id == id) {
            return &asset;
        }
    }
    return nullptr;
}

// ============================================
// Preview
// ============================================

AssetPreview* AssetBrowser::GetPreview(const std::string& id) {
    auto it = previews.find(id);
    if (it != previews.end()) {
        return &it->second;
    }
    return nullptr;
}

void AssetBrowser::LoadPreview(const std::string& id) {
    AssetInfo* asset = GetAsset(id);
    if (!asset) return;
    
    auto it = previews.find(id);
    if (it == previews.end()) {
        previews[id] = AssetPreview();
        it = previews.find(id);
    }
    
    if (!it->second.IsLoaded()) {
        it->second.LoadFromAsset(*asset);
    }
}

void AssetBrowser::UnloadPreview(const std::string& id) {
    auto it = previews.find(id);
    if (it != previews.end()) {
        it->second.Unload();
        previews.erase(it);
    }
}

// ============================================
// Callbacks
// ============================================

void AssetBrowser::SetOnAssetSelected(std::function<void(const AssetInfo&)> callback) {
    onAssetSelected = callback;
}

void AssetBrowser::SetOnAssetDoubleClicked(std::function<void(const AssetInfo&)> callback) {
    onAssetDoubleClicked = callback;
}

void AssetBrowser::SetOnAssetRightClicked(std::function<void(const AssetInfo&)> callback) {
    onAssetRightClicked = callback;
}

void AssetBrowser::SetOnAssetsImported(std::function<void(const std::vector<AssetInfo>&)> callback) {
    onAssetsImported = callback;
}

// ============================================
// UI Rendering
// ============================================

void AssetBrowser::Render() {
    // Main asset browser window
    // This would be called from ImGui rendering loop
    // For now, just a stub that could be filled with ImGui calls
}

void AssetBrowser::RenderToolbar() {
    // Toolbar with navigation, view options, import button
}

void AssetBrowser::RenderBreadcrumb() {
    // Breadcrumb navigation showing current path
}

void AssetBrowser::RenderAssetGrid() {
    // Grid view of assets with thumbnails
}

void AssetBrowser::RenderAssetList() {
    // List view of assets with details
}

void AssetBrowser::RenderPreviewPanel() {
    // Preview panel showing selected asset
}

void AssetBrowser::RenderDetailsPanel() {
    // Details panel with asset properties
}

void AssetBrowser::RenderContextMenu() {
    // Right-click context menu
}

void AssetBrowser::RenderImportDialog() {
    // Import file dialog
}

void AssetBrowser::RenderCreateMenu() {
    // Create new asset menu
}

// ============================================
// Asset Type Utilities
// ============================================

std::string AssetTypeToString(AssetType type) {
    switch (type) {
        case AssetType::Texture: return "Texture";
        case AssetType::Model: return "Model";
        case AssetType::Material: return "Material";
        case AssetType::Shader: return "Shader";
        case AssetType::Audio: return "Audio";
        case AssetType::Script: return "Script";
        case AssetType::Prefab: return "Prefab";
        case AssetType::Scene: return "Scene";
        case AssetType::BlockType: return "Block Type";
        case AssetType::EntityArchetype: return "Entity Archetype";
        case AssetType::Font: return "Font";
        case AssetType::Animation: return "Animation";
        case AssetType::ParticleSystem: return "Particle System";
        default: return "Unknown";
    }
}

AssetType StringToAssetType(const std::string& str) {
    if (str == "Texture") return AssetType::Texture;
    if (str == "Model") return AssetType::Model;
    if (str == "Material") return AssetType::Material;
    if (str == "Shader") return AssetType::Shader;
    if (str == "Audio") return AssetType::Audio;
    if (str == "Script") return AssetType::Script;
    if (str == "Prefab") return AssetType::Prefab;
    if (str == "Scene") return AssetType::Scene;
    if (str == "Block Type") return AssetType::BlockType;
    if (str == "Entity Archetype") return AssetType::EntityArchetype;
    if (str == "Font") return AssetType::Font;
    if (str == "Animation") return AssetType::Animation;
    if (str == "Particle System") return AssetType::ParticleSystem;
    return AssetType::Unknown;
}

std::string GetAssetTypeIcon(AssetType type) {
    switch (type) {
        case AssetType::Texture: return "🖼️";
        case AssetType::Model: return "🧊";
        case AssetType::Material: return "🎨";
        case AssetType::Shader: return "📝";
        case AssetType::Audio: return "🔊";
        case AssetType::Script: return "📜";
        case AssetType::Prefab: return "📦";
        case AssetType::Scene: return "🎬";
        case AssetType::BlockType: return "🧱";
        case AssetType::EntityArchetype: return "👤";
        case AssetType::Font: return "🔤";
        case AssetType::Animation: return "🎭";
        case AssetType::ParticleSystem: return "✨";
        default: return "📄";
    }
}

} // namespace vge
