#pragma once
#include "math/vec3.h"
#include <string>
#include <vector>
#include <map>
#include <functional>

namespace vge {

// ============================================
// Asset Type
// ============================================
enum class AssetType {
    Unknown,
    Texture,
    Model,
    Material,
    Shader,
    Audio,
    Script,
    Prefab,
    Scene,
    BlockType,
    EntityArchetype,
    Font,
    Animation,
    ParticleSystem
};

// ============================================
// Asset Info
// ============================================
struct AssetInfo {
    std::string id;
    std::string name;
    std::string path;
    AssetType type;
    size_t fileSize;
    std::string lastModified;
    std::string thumbnailPath;
    bool isDirectory;
    bool isFavorite;
    std::vector<std::string> tags;
    
    AssetInfo() : type(AssetType::Unknown), fileSize(0), isDirectory(false), isFavorite(false) {}
};

// ============================================
// Asset Filter
// ============================================
struct AssetFilter {
    std::string searchQuery;
    AssetType typeFilter;
    std::vector<std::string> tags;
    bool showFavoritesOnly;
    bool showDirectories;
    
    AssetFilter() : typeFilter(AssetType::Unknown), showFavoritesOnly(false), showDirectories(true) {}
    
    bool Matches(const AssetInfo& asset) const;
};

// ============================================
// Asset Preview
// ============================================
class AssetPreview {
private:
    uint32_t textureID;
    int width;
    int height;
    bool loaded;
    
public:
    AssetPreview();
    ~AssetPreview();
    
    bool LoadFromFile(const std::string& path);
    bool LoadFromAsset(const AssetInfo& asset);
    void Unload();
    
    uint32_t GetTextureID() const { return textureID; }
    int GetWidth() const { return width; }
    int GetHeight() const { return height; }
    bool IsLoaded() const { return loaded; }
};

// ============================================
// Asset Browser
// ============================================
class AssetBrowser {
private:
    std::string currentPath;
    std::string rootPath;
    std::vector<AssetInfo> assets;
    std::vector<AssetInfo> filteredAssets;
    AssetFilter filter;
    
    // Selection
    int selectedIndex;
    std::vector<int> selectedIndices;
    
    // View settings
    bool gridView;
    int gridSize;
    bool showPreview;
    bool showDetails;
    
    // Preview
    std::map<std::string, AssetPreview> previews;
    
    // Callbacks
    std::function<void(const AssetInfo&)> onAssetSelected;
    std::function<void(const AssetInfo&)> onAssetDoubleClicked;
    std::function<void(const AssetInfo&)> onAssetRightClicked;
    std::function<void(const std::vector<AssetInfo>&)> onAssetsImported;
    
public:
    AssetBrowser();
    ~AssetBrowser();
    
    void Initialize(const std::string& rootPath);
    void Shutdown();
    
    // Navigation
    void SetPath(const std::string& path);
    void GoUp();
    void GoToRoot();
    bool CanGoUp() const;
    std::string GetCurrentPath() const { return currentPath; }
    
    // Refresh
    void Refresh();
    void ScanDirectory(const std::string& path);
    
    // Filtering
    void SetSearchQuery(const std::string& query);
    void SetTypeFilter(AssetType type);
    void SetShowFavoritesOnly(bool show);
    void ClearFilter();
    void ApplyFilter();
    
    // Selection
    void SelectAsset(int index);
    void SelectAsset(const std::string& id);
    void MultiSelect(int index, bool addToSelection);
    void ClearSelection();
    AssetInfo* GetSelectedAsset();
    std::vector<AssetInfo*> GetSelectedAssets();
    bool HasSelection() const { return selectedIndex >= 0; }
    
    // View settings
    void SetGridView(bool grid) { gridView = grid; }
    void SetGridSize(int size) { gridSize = size; }
    void SetShowPreview(bool show) { showPreview = show; }
    void SetShowDetails(bool show) { showDetails = show; }
    bool IsGridView() const { return gridView; }
    int GetGridSize() const { return gridSize; }
    bool IsShowingPreview() const { return showPreview; }
    bool IsShowingDetails() const { return showDetails; }
    
    // Asset operations
    bool ImportAsset(const std::string& sourcePath);
    bool DeleteAsset(const std::string& id);
    bool RenameAsset(const std::string& id, const std::string& newName);
    bool DuplicateAsset(const std::string& id);
    void ToggleFavorite(const std::string& id);
    
    // Get assets
    const std::vector<AssetInfo>& GetAssets() const { return filteredAssets; }
    int GetAssetCount() const { return filteredAssets.size(); }
    AssetInfo* GetAsset(int index);
    AssetInfo* GetAsset(const std::string& id);
    
    // Preview
    AssetPreview* GetPreview(const std::string& id);
    void LoadPreview(const std::string& id);
    void UnloadPreview(const std::string& id);
    
    // Callbacks
    void SetOnAssetSelected(std::function<void(const AssetInfo&)> callback);
    void SetOnAssetDoubleClicked(std::function<void(const AssetInfo&)> callback);
    void SetOnAssetRightClicked(std::function<void(const AssetInfo&)> callback);
    void SetOnAssetsImported(std::function<void(const std::vector<AssetInfo>&)> callback);
    
    // UI rendering (called by ImGui)
    void Render();
    void RenderToolbar();
    void RenderBreadcrumb();
    void RenderAssetGrid();
    void RenderAssetList();
    void RenderPreviewPanel();
    void RenderDetailsPanel();
    void RenderContextMenu();
    void RenderImportDialog();
    void RenderCreateMenu();
};

// ============================================
// Asset Type Utilities
// ============================================
std::string AssetTypeToString(AssetType type);
AssetType StringToAssetType(const std::string& str);
std::string GetAssetTypeIcon(AssetType type);

} // namespace vge
